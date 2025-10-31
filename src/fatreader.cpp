#include "fatreader.h"

#include <QMessageBox>
#include <QVector>

FatReader::FatReader(FatImage *fatImage, FatCache *fatCache)
{
	this->fatImage = fatImage;
	this->fatCache = fatCache;
}
QByteArray FatReader::readFile(const quint32 startCluster, const quint32 fileSize) const
{
	if (!fatImage->isClusterValid(startCluster) || fileSize == 0)
	{
		return {};
	}
	QByteArray data = readClusterChain(startCluster);
	if (data.size() > fileSize)
	{
		data.resize(static_cast< int >(fileSize));
	}
	return data;
}
QVector< FileInfo > FatReader::getDirectoryListing(const quint32 cluster) const
{
	const QByteArray data = readDirectoryData(cluster);
	QVector< FileInfo > files = parseDirectoryEntries(data, cluster);
	return files;
}
quint32 FatReader::getNextCluster(const quint32 cluster) const
{
	if (!fatImage->isClusterValid(cluster) || !fatImage->isReady())
	{
		return fatImage->getEofValue();
	}

	const BiosParameterBlock &bpb = fatImage->getBpb();
	const quint32 bytesPerEntry = fatImage->getBytesPerEntry();

	const quint32 fatOffset = cluster * bytesPerEntry;
	const quint32 fatSecNum = bpb.BPB_RsvdSecCnt + (fatOffset / bpb.BPB_BytsPerSec);
	const quint32 fatEntOffset = fatOffset % bpb.BPB_BytsPerSec;

	const QByteArray fatSector = fatCache->readSectorCached(fatSecNum);

	if (fatSector.size() < fatEntOffset + bytesPerEntry)
	{
		return fatImage->getEofValue();
	}

	quint32 nextCluster = 0;
	memcpy(&nextCluster, fatSector.constData() + fatEntOffset, bytesPerEntry);
	return nextCluster & fatImage->getClusterMask();
}
bool FatReader::isEndOfClusterChain(const quint32 cluster) const
{
	return cluster >= fatImage->getEofValue();
}
QByteArray FatReader::readClusterChain(const quint32 startCluster) const
{
	QByteArray data;
	quint32 currentCluster = startCluster;
	quint32 chainLength = 0;
	const quint32 countOfClusters = fatImage->getCountOfClusters();

	while (!isEndOfClusterChain(currentCluster))
	{
		QByteArray clusterData = fatCache->readClusterCached(currentCluster);
		if (clusterData.isEmpty())
		{
			break;
		}
		data.append(clusterData);
		currentCluster = getNextCluster(currentCluster);
		if (++chainLength > countOfClusters)
		{
			break;
		}
	}
	return data;
}
QByteArray FatReader::readDirectoryData(const quint32 cluster) const
{
	const BiosParameterBlock &bpb = fatImage->getBpb();

	if (cluster != 0)
	{
		return readClusterChain(cluster);
	}

	if (fatImage->getFatType() == FatType::FAT16)
	{
		const quint32 firstRootSector = bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * bpb.BPB_FATSz16);
		const quint32 rootSectors = (bpb.BPB_RootEntCnt * DIRECTORY_ENTRY_SIZE + bpb.BPB_BytsPerSec - 1) / bpb.BPB_BytsPerSec;

		QByteArray data = fatImage->readSectors(firstRootSector, rootSectors);

		return data;
	}

	// FAT32
	return readClusterChain(bpb.fat32.BPB_RootClus);
}
bool FatReader::isRoot(const quint32 cluster) const
{
	if (!fatImage || !fatImage->isReady())
	{
		return false;
	}
	if (const FatType fatType = fatImage->getFatType(); fatType == FatType::FAT16)
	{
		return cluster == 0;
	}
	else if (fatType == FatType::FAT32)
	{
		const BiosParameterBlock &bpb = fatImage->getBpb();
		return cluster == bpb.fat32.BPB_RootClus;
	}
	return false;
}
QVector< FileInfo > FatReader::parseDirectoryEntries(const QByteArray &data, const quint32 cluster)
{
	QVector< FileInfo > fileInfos;
	const int entryCount = data.size() / DIRECTORY_ENTRY_SIZE;
	const DirectoryEntry *entries = reinterpret_cast< const DirectoryEntry * >(data.constData());

	QString longNameBuffer;
	for (int i = 0; i < entryCount; ++i)
	{
		const DirectoryEntry &entry = entries[i];
		if (entry.DIR_Name[0] == DIR_ENTRY_END)
			break;
		if (entry.DIR_Name[0] == DIR_ENTRY_DELETED)
		{
			longNameBuffer.clear();
			continue;
		}
		if (entry.DIR_Attr & ATTR_LONG_NAME)
		{
			const auto *lfnEntry = reinterpret_cast< const LongDirectoryEntry * >(&entry);
			QString part = getPartFromLfnEntry(*lfnEntry);
			longNameBuffer.prepend(part);
			continue;
		}

		if (entry.DIR_Attr & ATTR_VOLUME_ID)
		{
			continue;
		}
		FileInfo fileInfo = processDirectoryEntry(entry, longNameBuffer);
		if (fileInfo.name == ".")
			continue;
		fileInfos.append(fileInfo);
	}
	return fileInfos;
}
FileInfo FatReader::processDirectoryEntry(const DirectoryEntry &dirEntry, QString &longNameBuffer)
{
	FileInfo fileInfo;
	if (!longNameBuffer.isEmpty())
	{
		fileInfo.name = longNameBuffer;
		longNameBuffer.clear();
	}
	else
	{
		fileInfo.name = parseShortName(dirEntry.DIR_Name);
	}
	fileInfo.size = dirEntry.DIR_FileSize;
	fileInfo.isDirectory = (dirEntry.DIR_Attr & ATTR_DIRECTORY);
	fileInfo.firstCluster = (static_cast< quint32 >(dirEntry.DIR_FstClusHI) << 16) | dirEntry.DIR_FstClusLO;
	fileInfo.lastModified = fatDateTimeToQDateTime(dirEntry.DIR_WrtDate, dirEntry.DIR_WrtTime);
	fileInfo.created = fatDateTimeToQDateTime(dirEntry.DIR_CrtDate, dirEntry.DIR_CrtTime);
	fileInfo.lastAccessed = fatDateTimeToQDateTime(dirEntry.DIR_LstAccDate, 0);
	fileInfo.attributes = dirEntry.DIR_Attr;
	return fileInfo;
}
QString FatReader::getPartFromLfnEntry(const LongDirectoryEntry &lfnEntry)
{
	QString part;
	auto appendArray = [&part](const auto &array)
	{
		for (const quint16 j : array)
		{
			part.append(QChar(j));
		}
	};
	appendArray(lfnEntry.LDIR_Name1);
	appendArray(lfnEntry.LDIR_Name2);
	appendArray(lfnEntry.LDIR_Name3);

	QString cleanPart = part;
	cleanPart.remove(QChar(LFN_FILL_CHAR));
	cleanPart.remove(QChar(LFN_NULL_CHAR));
	return cleanPart;
}
QString FatReader::parseShortName(const quint8 name[11])
{
	QByteArray shortName(reinterpret_cast< const char * >(name), 8);
	QByteArray extension(reinterpret_cast< const char * >(name + 8), 3);
	shortName = shortName.trimmed();
	extension = extension.trimmed();
	QString result = QString::fromLocal8Bit(shortName);

	if (!extension.isEmpty())
	{
		result += '.';
		result += QString::fromLocal8Bit(extension);
	}
	return result;
}
QDateTime FatReader::fatDateTimeToQDateTime(const quint16 date, const quint16 time)
{
	if (date == 0 && time == 0)
		return {};
	const int year = 1980 + (date >> 9);
	const int month = (date >> 5) & 0x0F;
	const int day = date & 0x1F;
	const int hour = time >> 11;
	const int minute = (time >> 5) & 0x3F;
	const int second = (time & 0x1F) * 2;
	return { QDate(year, month, day), QTime(hour, minute, second) };
}

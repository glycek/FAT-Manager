#include "fatimage.h"

#include <cstring>
using namespace FatConstants;

FatImage::FatImage(const QString &imagePath) : fatType(FatType::UNKNOWN)
{
	if (imagePath.isEmpty())
	{
		closeWithError("Empty image path provided");
		return;
	}
	imageFile.setFileName(imagePath);

	if (!imageFile.open(QIODevice::ReadOnly))
	{
		closeWithError(QString("Failed to open file: %1. Error: %2").arg(imagePath, imageFile.errorString()));
		return;
	}
	const QByteArray bpbByteArray = imageFile.read(BPB_SIZE);
	if (bpbByteArray.size() != BPB_SIZE)
	{
		closeWithError("Failed to read Boot Parameter Block");
		return;
	}

	memset(&bpb, 0, sizeof(bpb));
	memcpy(&bpb, bpbByteArray.constData(), BPB_SIZE);

	if (!isBpbValid())
	{
		closeWithError("Invalid Boot Parameter Block");
		return;
	}
	calculateFatParameters();
	if (fatType == FatType::FAT12)
	{
		closeWithError("FAT12 is not supported");
		return;
	}
}
FatImage::~FatImage()
{
	imageFile.close();
}
QByteArray FatImage::readSector(const quint32 sectorNum)
{
	if (!isReady())
	{
		return {};
	}

	imageFile.seek(sectorNum * bpb.BPB_BytsPerSec);
	QByteArray data = imageFile.read(bpb.BPB_BytsPerSec);

	if (data.size() != bpb.BPB_BytsPerSec)
	{
		return {};
	}

	return data;
}
QByteArray FatImage::readSectors(const quint32 startSector, const quint32 count)
{
	if (!isReady() || count < 1)
	{
		return {};
	}

	const qint64 offset = startSector * bpb.BPB_BytsPerSec;
	const qint64 totalBytes = count * bpb.BPB_BytsPerSec;

	if (!imageFile.seek(offset))
	{
		return {};
	}
	QByteArray data = imageFile.read(totalBytes);
	if (data.size() != totalBytes)
	{
		return {};
	}
	return data;
}
QByteArray FatImage::readCluster(const quint32 clusterNum)
{
	if (!isReady() || !isClusterValid(clusterNum))
	{
		return {};
	}
	const quint32 firstSector = ((clusterNum - 2) * bpb.BPB_SecPerClus) + firstDataSector;
	return readSectors(firstSector, bpb.BPB_SecPerClus);
}
bool FatImage::isClusterValid(const quint32 clusterNum) const
{
	return clusterNum >= 2 && clusterNum < countOfClusters + 2;
}
QString FatImage::getVolumeLabel() const
{
	switch (fatType)
	{
	case FatType::FAT16:
		return parseVolumeLabel(bpb.fat16.BS_VolLab);
	case FatType::FAT32:
		return parseVolumeLabel(bpb.fat32.BS_VolLab);
	default:
		return "Unknown";
	}
}
bool FatImage::isBpbValid() const
{
	const bool validJumpBoot =
		(bpb.BS_jmpBoot[0] == JUMP_SHORT && bpb.BS_jmpBoot[2] == JUMP_SHORT_NOP) || (bpb.BS_jmpBoot[0] == JUMP_NEAR);
	if (!validJumpBoot)
	{
		return false;
	}
	bool validSectorSize = false;
	for (const quint16 i : VALID_SECTOR_SIZES)
	{
		if (bpb.BPB_BytsPerSec == i)
		{
			validSectorSize = true;
			break;
		}
	}
	if (!validSectorSize)
	{
		return false;
	}
	if (bpb.BPB_SecPerClus == 0 || (bpb.BPB_SecPerClus & (bpb.BPB_SecPerClus - 1)) != 0)	// This value must be
																							// a power of 2 that
																							// is greater than 0
	{
		return false;
	}
	if (bpb.BPB_RsvdSecCnt == 0)
	{
		return false;
	}
	if (bpb.BPB_NumFATs == 0)
	{
		return false;
	}
	bool validMedia = false;
	for (const quint8 mediaType : MEDIA_TYPES)
	{
		if (bpb.BPB_Media == mediaType)
		{
			validMedia = true;
			break;
		}
	}
	if (!validMedia)
	{
		return false;
	}
	return true;
}
void FatImage::calculateFatParameters()
{
	rootDirSectors = ((bpb.BPB_RootEntCnt * DIRECTORY_ENTRY_SIZE) + (bpb.BPB_BytsPerSec - 1)) / bpb.BPB_BytsPerSec;
	quint32 totSec;

	if (bpb.BPB_FATSz16 != 0)
	{
		fatSz = bpb.BPB_FATSz16;
	}
	else
	{
		fatSz = bpb.fat32.BPB_FATSz32;
	}
	if (bpb.BPB_TotSec16 != 0)
	{
		totSec = bpb.BPB_TotSec16;
	}
	else
	{
		totSec = bpb.BPB_TotSec32;
	}
	const quint32 dataSec = totSec - (bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * fatSz) + rootDirSectors);
	countOfClusters = dataSec / bpb.BPB_SecPerClus;

	if (countOfClusters < FAT12_MAX_CLUSTERS)
	{
		fatType = FatType::FAT12;
	}
	else if (countOfClusters < FAT16_MAX_CLUSTERS)
	{
		fatType = FatType::FAT16;
		bytesPerEntry = 2;
		eofValue = FAT16_EOF_MIN;
		clusterMask = FAT16_MASK;
	}
	else
	{
		fatType = FatType::FAT32;
		bytesPerEntry = 4;
		eofValue = FAT32_EOF_MIN;
		clusterMask = FAT32_MASK;
	}
	firstDataSector = bpb.BPB_RsvdSecCnt + (bpb.BPB_NumFATs * fatSz) + rootDirSectors;
}
QString FatImage::parseVolumeLabel(const quint8 *labelData)
{
	const QByteArray label(reinterpret_cast< const char * >(labelData), VOLUME_LABEL_LENGTH);
	return QString::fromLocal8Bit(label.trimmed());
}
void FatImage::closeWithError(const QString &error)
{
	this->lastError = error;
	imageFile.close();
}

#pragma once
#include "fatcache.h"
#include "fatimage.h"

using namespace FatConstants;

struct FileInfo
{
	QString name;
	quint32 size;
	bool isDirectory;
	quint32 firstCluster;
	QDateTime lastModified;
	QDateTime created;
	QDateTime lastAccessed;
	quint8 attributes;

	bool isReadOnly() const { return attributes & ATTR_READ_ONLY; }
	bool isHidden() const { return attributes & ATTR_HIDDEN; }
	bool isSystem() const { return attributes & ATTR_SYSTEM; }
	bool isVolumeLabel() const { return attributes & ATTR_VOLUME_ID; }
	bool isArchive() const { return attributes & ATTR_ARCHIVE; }
};

class FatReader
{
  public:
	FatReader(FatImage* fatImage, FatCache* fatCache);
	QByteArray readFile(quint32 startCluster, quint32 fileSize) const;
	QVector< FileInfo > getDirectoryListing(quint32 cluster = 0) const;

  private:
	FatImage* fatImage;
	FatCache* fatCache;

	quint32 getNextCluster(quint32 cluster) const;
	bool isEndOfClusterChain(quint32 cluster) const;
	QByteArray readClusterChain(quint32 startCluster) const;

	QByteArray readDirectoryData(quint32 cluster) const;

	bool isRoot(quint32 cluster) const;

	static QVector< FileInfo > parseDirectoryEntries(const QByteArray& data, quint32 cluster);
	static FileInfo processDirectoryEntry(const DirectoryEntry& dirEntry, QString& longNameBuffer);
	static QString getPartFromLfnEntry(const LongDirectoryEntry& lfnEntry);
	static QString parseShortName(const quint8 name[SHORT_NAME_LENGTH]);
	static QDateTime fatDateTimeToQDateTime(quint16 date, quint16 time);
};

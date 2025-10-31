#pragma once

#include "fatreader.h"

class FatParser
{
  public:
	FatParser();
	~FatParser() = default;

	bool openImage(const QString &imagePath);
	void closeImage();
	QVector< FileInfo > getDirectoryListing(quint32 cluster = 0);
	QByteArray readFile(const FileInfo &info) const;
	QString getVolumeLabel() const;
	QString getLastErrorString() const;
	bool isReady() const;
	quint32 getRootCluster() const;

  private:
	QScopedPointer< FatImage > fatImage;
	QScopedPointer< FatCache > fatCache;
	QScopedPointer< FatReader > fatReader;

	QHash< quint32, QVector< FileInfo > > dirCache;
};

#pragma once
#include "fatstruct.h"

#include <QFile>
#include <QString>

class FatImage
{
  public:
	explicit FatImage(const QString& imagePath);
	~FatImage();

	FatImage(const FatImage&) = delete;
	FatImage& operator=(const FatImage&) = delete;

	QByteArray readSector(quint32 sectorNum);
	QByteArray readSectors(quint32 startSector, quint32 count);

	QByteArray readCluster(quint32 clusterNum);

	bool isClusterValid(quint32 clusterNum) const;
	bool isReady() const { return imageFile.isOpen(); }
	QString getLastError() const { return lastError; };

	const BiosParameterBlock& getBpb() const { return bpb; }
	FatType getFatType() const { return fatType; }

	quint32 getFirstDataSector() const { return firstDataSector; }
	quint32 getCountOfClusters() const { return countOfClusters; }
	quint32 getBytesPerEntry() const { return bytesPerEntry; }
	quint32 getEofValue() const { return eofValue; }
	quint32 getClusterMask() const { return clusterMask; }

	QString getVolumeLabel() const;

  private:
	QFile imageFile;
	BiosParameterBlock bpb;
	FatType fatType;

	quint32 fatSz;
	quint32 rootDirSectors;
	quint32 firstDataSector;
	quint32 countOfClusters;
	quint32 bytesPerEntry;
	quint32 eofValue;
	quint32 clusterMask;

	QString lastError;

	bool isBpbValid() const;
	void calculateFatParameters();
	static QString parseVolumeLabel(const quint8* labelData);

	void closeWithError(const QString& error);
};

#pragma once

#include "fatimage.h"

#include <QCache>

class FatCache
{
  public:
	explicit FatCache(FatImage* fatImage);

	QByteArray readSectorCached(quint32 sectorNum);
	QByteArray readClusterCached(quint32 clusterNum);
	void clearCache();

  private:
	FatImage* fatImage;
	QCache< quint32, QByteArray > sectorCache;
	QCache< quint32, QByteArray > clusterCache;

	static constexpr quint32 MAX_SECTOR_CACHE = 64;
	static constexpr quint32 MAX_CLUSTER_CACHE = 32;
};

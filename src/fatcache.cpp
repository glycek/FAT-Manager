#include "fatcache.h"
FatCache::FatCache(FatImage *fatImage)
{
	this->fatImage = fatImage;
	sectorCache.setMaxCost(MAX_SECTOR_CACHE);
	clusterCache.setMaxCost(MAX_CLUSTER_CACHE);
}
QByteArray FatCache::readSectorCached(const quint32 sectorNum)
{
	if (QByteArray *cached = sectorCache.object(sectorNum))
	{
		return *cached;
	}
	QByteArray data = fatImage->readSector(sectorNum);
	if (!data.isEmpty())
	{
		sectorCache.insert(sectorNum, new QByteArray(data));
	}
	return data;
}
QByteArray FatCache::readClusterCached(const quint32 clusterNum)
{
	if (QByteArray *cached = clusterCache.object(clusterNum))
	{
		return *cached;
	}
	QByteArray data = fatImage->readCluster(clusterNum);
	if (!data.isEmpty())
	{
		clusterCache.insert(clusterNum, new QByteArray(data));
	}
	return data;
}
void FatCache::clearCache()
{
	sectorCache.clear();
	clusterCache.clear();
}

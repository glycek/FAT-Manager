#include "fatparser.h"

#include <QDebug>
#include <QVector>

FatParser::FatParser() = default;
bool FatParser::openImage(const QString &imagePath)
{
	closeImage();
	fatImage.reset(new FatImage(imagePath));
	if (!fatImage->isReady())
	{
		return false;
	}
	fatCache.reset(new FatCache(fatImage.data()));
	fatReader.reset(new FatReader(fatImage.data(), fatCache.data()));
	return true;
}
void FatParser::closeImage()
{
	fatImage.reset();
	fatCache.reset();
	fatReader.reset();
	dirCache.clear();
}
QVector< FileInfo > FatParser::getDirectoryListing(const quint32 cluster)
{
	if (!fatReader)
	{
		return {};
	}
	if (!dirCache.contains(cluster))
	{
		dirCache[cluster] = fatReader->getDirectoryListing(cluster);
	}
	return dirCache[cluster];
}
QByteArray FatParser::readFile(const FileInfo &info) const
{
	if (!fatReader)
		return {};
	return fatReader->readFile(info.firstCluster, info.size);
}
QString FatParser::getVolumeLabel() const
{
	if (!isReady())
		return "No volume label available";

	return fatImage->getVolumeLabel();
}
QString FatParser::getLastErrorString() const
{
	return fatImage ? fatImage->getLastError() : "No image loaded";
}
bool FatParser::isReady() const
{
	return fatImage && fatImage->isReady();
}
quint32 FatParser::getRootCluster() const
{
	if (!fatImage || !fatImage->isReady())
	{
		return 0;
	}

	const BiosParameterBlock &bpb = fatImage->getBpb();

	if (fatImage->getFatType() == FatType::FAT32)
	{
		return bpb.fat32.BPB_RootClus;
	}
	return 0;	 // FAT16
}

#include "fatmodel.h"

#include <QDebug>

FatModel::FatModel(FatParser *parser, QObject *parent) : QAbstractTableModel(parent), parser(parser) {}
int FatModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	if (!isParserReady())
		return 0;

	return currentFiles.size();
}
int FatModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	if (!isParserReady())
		return 0;

	return COLUMN_COUNT;
}
QVariant FatModel::data(const QModelIndex &index, const int role) const
{
	if (!isParserReady() || !index.isValid())
		return {};
	if (index.row() >= rowCount() || index.column() >= columnCount())
		return {};

	if (role == Qt::DisplayRole)
	{
		const FileInfo fileInfo = getFileInfo(index.row());
		switch (static_cast< Columns >(index.column()))
		{
		case Columns::ColumnName:
			return fileInfo.name;
		case Columns::ColumnSize:
			return fileInfo.isDirectory ? QString("Dir") : QString::number(fileInfo.size);
		case Columns::ColumnTime:
			return fileInfo.created.toString("yyyy-MM-dd hh:mm");
		default:
			return {};
		}
	}
	return {};
}
QVariant FatModel::headerData(int section, const Qt::Orientation orientation, const int role) const
{
	if (role != Qt::DisplayRole)
		return {};

	if (orientation == Qt::Horizontal)
	{
		switch (static_cast< Columns >(section))
		{
		case Columns::ColumnName:
			return QString("Name");
		case Columns::ColumnSize:
			return QString("Size");
		case Columns::ColumnTime:
			return QString("Created");
		}
	}
	return {};
}
QByteArray FatModel::readFile(const FileInfo &info) const
{
	if (!isParserReady())
	{
		return {};
	}
	return parser->readFile(info);
}
FatModel::CopyData FatModel::prepareCopyDataRecursive(const FileInfo &entry) const
{
	CopyData data;
	data.info = entry;
	if (entry.isDirectory)
	{
		QVector< FileInfo > children = parser->getDirectoryListing(entry.firstCluster);
		for (const FileInfo &child : children)
		{
			if (child.name == "." || child.name == "..")
				continue;
			data.children.append(prepareCopyDataRecursive(child));
		}
	}
	else
	{
		data.content = readFile(entry);
	}
	return data;
}
QVector< FatModel::CopyData > FatModel::prepareCopyData(const QModelIndexList &indexes) const
{
	QVector< CopyData > result;
	for (const QModelIndex &index : indexes)
	{
		if (!index.isValid() || index.row() >= currentFiles.size())
			continue;

		FileInfo entry = currentFiles[index.row()];
		if (entry.name == "..")
			continue;
		result.append(prepareCopyDataRecursive(entry));
	}
	return result;
}
FileInfo FatModel::getFileInfo(const int row) const
{
	if (row < 0 || currentFiles.size() <= row)
		return {};
	return currentFiles.at(row);
}
void FatModel::setCurrentCluster(const quint32 cluster)
{
	if (!isParserReady() || currentCluster == cluster)
	{
		return;
	}
	beginResetModel();
	currentFiles = parser->getDirectoryListing(cluster);
	currentCluster = cluster;
	endResetModel();
	emit directoryChanged(cluster);
}
quint32 FatModel::getCurrentCluster() const
{
	return currentCluster;
}
QString FatModel::getVolumeLabel() const
{
	if (!isParserReady())
		return {};

	return parser->getVolumeLabel();
}
quint64 FatModel::calculateDirectorySize(const quint32 cluster)
{
	if (!isParserReady())
		return 0;

	quint64 totalSize = 0;
	QVector< FileInfo > entries = parser->getDirectoryListing(cluster);

	for (const FileInfo &entry : entries)
	{
		if (entry.name == "." || entry.name == "..")
			continue;
		if (entry.isDirectory)
			totalSize += calculateDirectorySize(entry.firstCluster);
		else
			totalSize += entry.size;
	}
	return totalSize;
}

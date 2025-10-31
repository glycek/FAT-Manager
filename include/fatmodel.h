#pragma once

#include "fatparser.h"

#include <QAbstractItemModel>

class FatModel final : public QAbstractTableModel
{
	Q_OBJECT

	static constexpr quint32 COLUMN_COUNT = 3;

  public:
	enum class Columns
	{
		ColumnName = 0,
		ColumnSize = 1,
		ColumnTime = 2
	};

	explicit FatModel(FatParser *parser, QObject *parent = nullptr);
	~FatModel() override = default;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	QByteArray readFile(const FileInfo &info) const;
	struct CopyData
	{
		FileInfo info;
		QByteArray content;
		QVector< CopyData > children;
	};
	QVector< CopyData > prepareCopyData(const QModelIndexList &indexes) const;

	FileInfo getFileInfo(int row) const;
	void setCurrentCluster(quint32 cluster);
	quint32 getCurrentCluster() const;
	QString getVolumeLabel() const;
	bool isParserReady() const { return parser && parser->isReady(); }
	quint64 calculateDirectorySize(quint32 cluster);
	QVector< FileInfo > getCurrentFiles() const { return currentFiles; }

  signals:
	void directoryChanged(quint32 cluster);

  private:
	CopyData prepareCopyDataRecursive(const FileInfo &entry) const;
	FatParser *parser;
	QVector< FileInfo > currentFiles;
	quint32 currentCluster = -1;
};

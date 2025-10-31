#pragma once

#include "fatmodel.h"

#include <QModelIndexList>
#include <QObject>

class FatModel;
class QFileSystemModel;
struct FileInfo;

class FileOperations final : public QObject
{
	Q_OBJECT

  public:
	explicit FileOperations(QObject* parent = nullptr);
	~FileOperations() override;

	void setFileSystemModel(QFileSystemModel* model);
	void setFatModel(FatModel* model);

	quint64 calculateDirectorySize(const QString& path, bool inFat);
	bool copyFatToSystem(const QModelIndexList& fatIndexes, const QString& systemPath);

  signals:
	void statusMessage(const QString& message);
	void errorOccurred(const QString& error);
	void progressUpdate(int current, int total);

  private:
	bool copyDataToSystem(const FatModel::CopyData& data, const QString& destPath);
	static QString generateUniqueFileName(const QString& basePath, const QString& fileName);
	qint64 getSystemDirectorySize(const QString& path);

	QFileSystemModel* fileSystemModel;
	FatModel* fatModel;
};

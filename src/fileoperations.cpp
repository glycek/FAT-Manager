#include "fileoperations.h"

#include "fatmodel.h"

#include <QDebug>
#include <QFile>
#include <QFileSystemModel>
#include <QIODevice>

FileOperations::FileOperations(QObject* parent) : QObject(parent), fileSystemModel(nullptr), fatModel(nullptr) {}
FileOperations::~FileOperations() = default;
void FileOperations::setFileSystemModel(QFileSystemModel* model)
{
	fileSystemModel = model;
}
void FileOperations::setFatModel(FatModel* model)
{
	fatModel = model;
}
qint64 FileOperations::getSystemDirectorySize(const QString& path)
{
	qint64 totalSize = 0;
	const QDir dir(path);
	if (!dir.exists())
		return 0;
	QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot, QDir::DirsFirst);
	for (const QFileInfo& entry : entries)
	{
		if (entry.isDir())
			totalSize += getSystemDirectorySize(entry.absoluteFilePath());
		else
			totalSize += entry.size();
	}
	return totalSize;
}
bool FileOperations::copyFatToSystem(const QModelIndexList& fatIndexes, const QString& systemPath)
{
	if (!fatModel || !fatModel->isParserReady())
	{
		emit errorOccurred(tr("No FAT image mounted"));
		return false;
	}
	const QDir destDir(systemPath);
	if (!destDir.exists())
	{
		emit errorOccurred(tr("Destination directory does not exist"));
		return false;
	}
	QVector< FatModel::CopyData > copyData = fatModel->prepareCopyData(fatIndexes);
	if (copyData.isEmpty())
	{
		emit errorOccurred(tr("No files selected"));
		return false;
	}
	int successCount = 0;
	const int totalCount = copyData.size();
	for (int i = 0; i < copyData.size(); ++i)
	{
		emit progressUpdate(i, totalCount);
		const FatModel::CopyData& data = copyData[i];
		if (QString destPath = destDir.absoluteFilePath(data.info.name); copyDataToSystem(data, destPath))
		{
			successCount++;
			emit statusMessage(tr("Copied: %1").arg(data.info.name));
		}
		else
		{
			emit errorOccurred(tr("Failed to copy: %1").arg(data.info.name));
		}
	}

	emit progressUpdate(totalCount, totalCount);
	emit statusMessage(tr("Copy completed: %1 of %2 files").arg(successCount).arg(totalCount));
	return successCount == totalCount;
}
bool FileOperations::copyDataToSystem(const FatModel::CopyData& data, const QString& destPath)
{
	if (data.info.isDirectory)
	{
		if (!QDir().mkpath(destPath))
			return false;

		bool allSuccess = true;
		for (const FatModel::CopyData& child : data.children)
		{
			if (QString childPath = QDir(destPath).absoluteFilePath(child.info.name); !copyDataToSystem(child, childPath))
				allSuccess = false;
		}
		return allSuccess;
	}
	const QString finalPath = generateUniqueFileName(QFileInfo(destPath).absolutePath(), QFileInfo(destPath).fileName());

	QFile destFile(finalPath);
	if (!destFile.open(QIODevice::WriteOnly))
		return false;

	const qint64 written = destFile.write(data.content);
	destFile.close();
	return written == data.content.size();
}
QString FileOperations::generateUniqueFileName(const QString& basePath, const QString& fileName)
{
	QString fullPath = QDir(basePath).absoluteFilePath(fileName);
	if (!QFile::exists(fullPath))
	{
		return fullPath;
	}
	const QFileInfo fileInfo(fileName);
	const QString baseName = fileInfo.completeBaseName();
	const QString extension = fileInfo.suffix();
	int counter = 1;
	do
	{
		QString newFileName = QString("%1 (%2)").arg(baseName).arg(counter);
		if (!extension.isEmpty())
		{
			newFileName += "." + extension;
		}
		fullPath = QDir(basePath).absoluteFilePath(newFileName);
		counter++;
	} while (QFile::exists(fullPath));

	return fullPath;
}
quint64 FileOperations::calculateDirectorySize(const QString& path, const bool inFat)
{
	quint64 size;
	if (inFat)
	{
		if (!fatModel || !fatModel->isParserReady())
		{
			emit errorOccurred(tr("No FAT image mounted"));
			return -1;
		}
		QVector< FileInfo > currentFiles = fatModel->getCurrentFiles();
		quint32 cluster = 0;
		for (auto& currentFile : currentFiles)
		{
			if (currentFile.name == path)
			{
				cluster = currentFile.firstCluster;
				break;
			}
		}
		if (cluster == 0)
		{
			emit errorOccurred(tr("Directory not found: %1").arg(path));
			return -1;
		}
		size = fatModel->calculateDirectorySize(cluster);
	}
	else
	{
		size = getSystemDirectorySize(path);
	}
	QString formattedSize;
	if (size < 1024)
		formattedSize = QString("%1 B").arg(size);
	else if (size < 1024 * 1024)
		formattedSize = QString("%1 KB").arg(size / 1024.0, 0, 'f', 2);
	else if (size < 1024 * 1024 * 1024)
		formattedSize = QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 2);
	else
		formattedSize = QString("%1 GB").arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
	emit statusMessage(tr("Directory size: %1").arg(formattedSize));
	return size;
}

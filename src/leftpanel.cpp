#include "leftpanel.h"

#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QString>

LeftPanel::LeftPanel(QWidget *parent) : BasePanel(parent), model(nullptr)
{
	setupBaseUI();
	setupModel();
	setupView();
	connectSignals();
	setRootPath(QDir::currentPath());
	updateHeader();
}
QString LeftPanel::currentPath() const
{
	if (!model || !currentRootIndex.isValid())
		return {};
	return model->filePath(currentRootIndex);
}
QStringList LeftPanel::selectedFiles() const
{
	QStringList result;
	if (!model || !panelView)
		return {};

	QModelIndexList selectedIndexList = panelView->selectionModel()->selectedRows();
	if (selectedIndexList.isEmpty())
	{
		if (const QModelIndex index = panelView->currentIndex(); index.isValid())
			selectedIndexList.append(index);
	}
	for (const QModelIndex &index : selectedIndexList)
	{
		if (QString path = model->filePath(index); !path.isEmpty())
		{
			result.append(path);
		}
	}
	return result;
}
bool LeftPanel::canNavigateUp() const
{
	if (!model || !currentRootIndex.isValid())
		return false;
	const QModelIndex parentIndex = model->parent(currentRootIndex);
	return parentIndex.isValid();
}
bool LeftPanel::navigateTo(const QString &path)
{
	if (!model)
		return false;
	const auto dir = QDir(path);
	if (!dir.exists())
	{
		emit navigationError(tr("Directory does not exist, path: %1").arg(path));
		return false;
	}
	if (!dir.isReadable())
	{
		emit navigationError(tr("Directory is not readable, path: %1").arg(path));
		return false;
	}
	const QModelIndex index = model->index(path);
	if (!index.isValid())
	{
		emit navigationError(tr("Cant open directory, path: %1").arg(path));
		return false;
	}
	currentRootIndex = index;
	panelView->setRootIndex(currentRootIndex);
	updateHeader();
	emit pathChanged(path);
	return true;
}
bool LeftPanel::navigateUp()
{
	if (!canNavigateUp())
		return false;
	currentRootIndex = model->parent(currentRootIndex);
	panelView->setRootIndex(currentRootIndex);
	updateHeader();
	emit pathChanged(currentPath());
	return true;
}
void LeftPanel::setRootPath(const QString &path)
{
	if (!model)
		return;
	model->setRootPath(path);
	navigateTo(path);
}
QFileInfo LeftPanel::getFileInfo(const QModelIndex &index) const
{
	if (!model || !index.isValid())
		return {};
	return model->fileInfo(index);
}
QString LeftPanel::getPanelPrefix() const
{
	return tr("System:");
}
void LeftPanel::onDoubleClicked(const QModelIndex &index)
{
	if (!model || !index.isValid())
		return;
	const QFileInfo info = model->fileInfo(index);
	if (info.isDir())
	{
		const QString dirPath = info.absoluteFilePath();
		navigateTo(dirPath);
		return;
	}
	const QString path = info.absoluteFilePath();
	emit fileActivated(path);
}
void LeftPanel::onSelectionChanged()
{
	if (!panelView)
	{
		return;
	}
	const QModelIndexList selectedIndexList = panelView->selectionModel()->selectedRows();
	emit selectionChanged(selectedIndexList);
}
void LeftPanel::onDirectoryLoaded(const QString &path) const
{
	Q_UNUSED(path)
	if (!model || !panelView)
		return;
	if (model->rowCount(currentRootIndex) > 0)
	{
		const QModelIndex firstIndex = model->index(0, 0, currentRootIndex);
		panelView->scrollTo(firstIndex, QAbstractItemView::PositionAtTop);
	}
	int fileCount = 0;
	int dirCount = 0;
	for (int i = 0; i < model->rowCount(currentRootIndex); ++i)
	{
		if (QModelIndex childIndex = model->index(i, 0, currentRootIndex); model->isDir(childIndex))
		{
			dirCount++;
		}
		else
		{
			fileCount++;
		}
	}
}
void LeftPanel::setupModel()
{
	model = new QFileSystemModel(this);
	model->setFilter(QDir::AllEntries | QDir::NoDot);
	model->sort(0, Qt::AscendingOrder);
	model->setReadOnly(false);
}
void LeftPanel::setupView() const
{
	if (!model || !panelView)
		return;
	panelView->setModel(model);
	panelView->verticalHeader()->setVisible(false);
	QHeaderView *horizontalHeader = panelView->horizontalHeader();
	horizontalHeader->setSectionResizeMode(0, QHeaderView::Interactive);
	horizontalHeader->setSectionResizeMode(1, QHeaderView::Interactive);
	horizontalHeader->setSectionResizeMode(2, QHeaderView::Interactive);
	horizontalHeader->setSectionResizeMode(3, QHeaderView::Stretch);
}
void LeftPanel::connectSignals()
{
	if (!model || !panelView)
		return;
	connect(panelView, &QTableView::doubleClicked, this, &LeftPanel::onDoubleClicked);
	connect(panelView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &LeftPanel::onSelectionChanged);
	connect(model, &QFileSystemModel::directoryLoaded, this, &LeftPanel::onDirectoryLoaded);
}
bool LeftPanel::isDir(const QModelIndex &index) const
{
	if (!model || !index.isValid())
		return false;
	return model->isDir(index);
}

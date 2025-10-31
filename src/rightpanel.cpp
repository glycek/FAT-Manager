#include "rightpanel.h"

#include "fatmodel.h"
#include "fatparser.h"

#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>

RightPanel::RightPanel(QWidget* parent) : BasePanel(parent), fatModel(nullptr), fatParser(nullptr), historyIndex(-1)
{
	setupBaseUI();
	setupModel();
	setupView();
	connectSignals();
	updateHeader();
}
bool RightPanel::mountImage(const QString& imagePath)
{
	if (!fatParser)
		return false;
	if (isImageMounted())
	{
		unmountImage();
	}
	if (!fatParser->openImage(imagePath))
	{
		QMessageBox::critical(this, tr("Error"), tr("Failed to mount FAT image: %1").arg(fatParser->getLastErrorString()));
		return false;
	}

	mountedImagePath = imagePath;
	navigationHistory.clear();
	navigationHistory.append(fatParser->getRootCluster());
	historyIndex = 0;
	fatModel->setCurrentCluster(fatParser->getRootCluster());

	if (panelView)
		panelView->reset();
	setupView();
	updateHeader();
	emit mountStatusChanged(true);
	return true;
}
void RightPanel::unmountImage()
{
	if (!isImageMounted())
		return;

	fatParser->closeImage();
	mountedImagePath.clear();
	navigationHistory.clear();
	historyIndex = -1;

	updateHeader();
	emit mountStatusChanged(false);
}
bool RightPanel::isImageMounted() const
{
	return fatParser && fatParser->isReady();
}
void RightPanel::navigateToRoot()
{
	if (!isImageMounted())
		return;

	navigationHistory.clear();
	navigationHistory.append(0);
	historyIndex = 0;
	updateHeader();
}
void RightPanel::navigateUp()
{
	if (!canNavigateUp())
		return;

	navigateBack();
}
void RightPanel::navigateBack()
{
	if (!canNavigateBack())
		return;

	historyIndex--;
	const quint32 previousCluster = navigationHistory[historyIndex];
	fatModel->setCurrentCluster(previousCluster);
	updateHeader();
}
bool RightPanel::canNavigateUp() const
{
	return isImageMounted() && historyIndex > 0;
}

bool RightPanel::canNavigateBack() const
{
	return isImageMounted() && historyIndex > 0;
}
QString RightPanel::currentPath() const
{
	if (!isImageMounted())
		return {};

	if (navigationHistory.isEmpty() || historyIndex < 0)
	{
		return "/";
	}

	if (const quint32 currentCluster = navigationHistory[historyIndex]; currentCluster == 0)
	{
		return "/";
	}
	QString path = "/";
	return path;
}
QStringList RightPanel::selectedFiles() const
{
	QStringList result;
	if (!fatModel || !panelView)
		return result;

	QModelIndexList selectedIndexes = panelView->selectionModel()->selectedRows();
	if (selectedIndexes.isEmpty())
	{
		if (panelView->currentIndex().isValid())
		{
			selectedIndexes.append(panelView->currentIndex());
		}
	}
	for (const QModelIndex& index : selectedIndexes)
	{
		FileInfo info = fatModel->getFileInfo(index.row());
		result.append(info.name);
	}
	return result;
}
QString RightPanel::volumeLabel() const
{
	return isImageMounted() ? fatParser->getVolumeLabel() : QString();
}
QModelIndexList RightPanel::selectedIndexes() const
{
	return panelView ? panelView->selectionModel()->selectedRows() : QModelIndexList();
}
void RightPanel::onDoubleClicked(const QModelIndex& index)
{
	if (!index.isValid() || !isImageMounted())
		return;

	const FileInfo fileInfo = fatModel->getFileInfo(index.row());

	if (fileInfo.name == "..")
	{
		navigateUp();
		return;
	}
	if (fileInfo.isDirectory)
		fatModel->setCurrentCluster(fileInfo.firstCluster);
	else
		emit fileActivated(fileInfo.name);
}
void RightPanel::onSelectionChanged()
{
	const QModelIndexList selected = selectedIndexes();
	emit selectionChanged(selected);
}
QString RightPanel::getPanelPrefix() const
{
	if (isImageMounted())
	{
		const QString fileName = QFileInfo(mountedImagePath).fileName();
		return tr("FAT: %1").arg(fileName);
	}
	return tr("FAT: [No Image]");
}
void RightPanel::setupModel()
{
	fatParser = new FatParser();
	fatModel = new FatModel(fatParser, this);
}
void RightPanel::setupView() const
{
	if (!panelView || !fatModel)
		return;
	panelView->setModel(fatModel);
	if (fatModel->columnCount() > 0)
	{
		QHeaderView* horizontalHeader = panelView->horizontalHeader();
		horizontalHeader->setSectionResizeMode(0, QHeaderView::Interactive);
		horizontalHeader->setSectionResizeMode(1, QHeaderView::Interactive);
		horizontalHeader->setSectionResizeMode(2, QHeaderView::Stretch);
	}
}
void RightPanel::connectSignals()
{
	if (!panelView)
		return;

	connect(panelView, &QTableView::doubleClicked, this, &RightPanel::onDoubleClicked);
	connect(panelView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RightPanel::onSelectionChanged);
	connect(fatModel, &FatModel::directoryChanged, this, &RightPanel::onDirectoryChanged);
}
void RightPanel::onDirectoryChanged(const quint32 cluster)
{
	updateNavigationHistory(cluster);
	updateHeader();
	emit pathChanged(currentPath());
}
void RightPanel::updateNavigationHistory(const quint32 cluster)
{
	if (historyIndex < navigationHistory.size() - 1)
	{
		navigationHistory.erase(navigationHistory.begin() + historyIndex + 1, navigationHistory.end());
	}
	navigationHistory.append(cluster);
	historyIndex = navigationHistory.size() - 1;
}

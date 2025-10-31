#include "mainwindow.h"

#include "fileoperations.h"
#include "keyboardhandler.h"
#include "leftpanel.h"
#include "rightpanel.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

struct ToolBarActionInfo
{
	KeyboardHandler::Action type;
	QString text;
};
MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), mainSplitter(nullptr), leftPanel(nullptr), rightPanel(nullptr), activePanel(nullptr),
	statusLabel(nullptr), leftInfoLabel(nullptr), rightInfoLabel(nullptr), keyboardHandler(nullptr), fileOperations(nullptr)
{
	setupUi();
	connectSignals();
	activePanel = leftPanel;
}
MainWindow::~MainWindow() = default;
void MainWindow::loadFatImage(const QString& imagePath) const
{
	rightPanel->mountImage(imagePath);
}
void MainWindow::onKeyboardAction(KeyboardHandler::Action action)
{
	switch (action)
	{
	case KeyboardHandler::About:
		onAbout();
		break;
	case KeyboardHandler::Mount:
		onMount();
		break;
	case KeyboardHandler::MountDialog:
		onMountDialog();
		break;
	case KeyboardHandler::Copy:
		onCopy();
		break;
	case KeyboardHandler::Exit:
		onExit();
		break;
	case KeyboardHandler::SwitchPanel:
		switchPanelFocus();
		break;
	case KeyboardHandler::NavigateUp:
		if (const BasePanel* panel = getActivePanel())
		{
			if (panel == leftPanel)
				leftPanel->navigateUp();
			else
				rightPanel->navigateUp();
		}
		break;
	case KeyboardHandler::NavigateBack:
		if (!rightPanel->canNavigateBack())
		{
			break;
		}
		if (getActivePanel() == rightPanel)
		{
			rightPanel->navigateBack();
		}
		break;
	case KeyboardHandler::CalculateSize:
		onCalculateSize();
		break;
	case KeyboardHandler::Move:
	case KeyboardHandler::Mkdir:
	case KeyboardHandler::Delete:
		showInfo(tr("This feature is not implemented yet."));
		break;
	default:
		break;
	}
}
void MainWindow::onPanelSelectionChanged() const
{
	const int leftCount = leftPanel->getView()->selectionModel()->selectedRows().size();
	const int rightCount = rightPanel->getView()->selectionModel()->selectedRows().size();
	leftInfoLabel->setText(tr("Left: %1 selected").arg(leftCount));
	rightInfoLabel->setText(tr("Right: %1 selected").arg(rightCount));
}
void MainWindow::onPanelError(const QString& error)
{
	showError(error);
}
void MainWindow::onOperationStatusMessage(const QString& message) const
{
	statusLabel->setText(message);
}
void MainWindow::onOperationProgress(const int current, const int total) const
{
	if (total > 0)
	{
		statusBar()->showMessage(tr("Processing: %1 of %2...").arg(current).arg(total), 2000);
	}
}
void MainWindow::setupUi()
{
	const auto centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);
	auto* mainLayout = new QVBoxLayout(centralWidget);
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	createPanels();
	mainLayout->addWidget(mainSplitter);
	createToolBar(mainLayout);
	createStatusBar();

	keyboardHandler = new KeyboardHandler(this);
	fileOperations = new FileOperations(this);
	resize(1200, 800);
	setMinimumSize(600, 400);
	setWindowTitle(tr("FAT Manager"));
}
void MainWindow::createPanels()
{
	mainSplitter = new QSplitter(Qt::Horizontal, this);
	leftPanel = new LeftPanel(mainSplitter);
	rightPanel = new RightPanel(mainSplitter);
	mainSplitter->addWidget(leftPanel);
	mainSplitter->addWidget(rightPanel);
	mainSplitter->setStretchFactor(0, 1);
	mainSplitter->setStretchFactor(1, 1);
	mainSplitter->setChildrenCollapsible(false);
}
void MainWindow::createToolBar(QVBoxLayout* mainLayout)
{
	const QVector< ToolBarActionInfo > actions = {
		{ KeyboardHandler::About, tr("About\nF1") },		{ KeyboardHandler::Mount, tr("Mount\nF2") },
		{ KeyboardHandler::CalculateSize, tr("Size\nF3") }, { KeyboardHandler::MountDialog, tr("Mount...\nF4") },
		{ KeyboardHandler::Copy, tr("Copy\nF5") },			{ KeyboardHandler::Move, tr("Move\nF6") },
		{ KeyboardHandler::Mkdir, tr("Mkdir\nF7") },		{ KeyboardHandler::Delete, tr("Delete\nF8") },
		{ KeyboardHandler::Exit, tr("Exit\nF10") }
	};
	auto* bottomPanel = new QWidget(this);
	bottomPanel->setFixedHeight(70);
	auto* layout = new QHBoxLayout(bottomPanel);
	layout->setSpacing(1);
	layout->setContentsMargins(0, 0, 0, 0);

	for (const auto& actionInfo : actions)
	{
		const auto* action = new QAction(this);
		connect(action, &QAction::triggered, this, [this, actionInfo] { onKeyboardAction(actionInfo.type); });

		auto* button = new QPushButton(this);
		button->setText(actionInfo.text);
		button->setFixedHeight(68);
		button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		connect(button, &QPushButton::clicked, action, &QAction::trigger);
		layout->addWidget(button);
	}
	mainLayout->addWidget(bottomPanel);
}
void MainWindow::createStatusBar()
{
	statusLabel = new QLabel(tr("Ready"));
	leftInfoLabel = new QLabel();
	rightInfoLabel = new QLabel();
	statusBar()->addWidget(statusLabel, 1);
	statusBar()->addPermanentWidget(leftInfoLabel);
	statusBar()->addPermanentWidget(rightInfoLabel);
}
void MainWindow::connectSignals()
{
	connect(keyboardHandler, &KeyboardHandler::actionTriggered, this, &MainWindow::onKeyboardAction);

	connect(fileOperations, &FileOperations::statusMessage, this, &MainWindow::onOperationStatusMessage);
	connect(fileOperations, &FileOperations::errorOccurred, this, &MainWindow::onPanelError);
	connect(fileOperations, &FileOperations::progressUpdate, this, &MainWindow::onOperationProgress);

	connect(leftPanel, &BasePanel::selectionChanged, this, &MainWindow::onPanelSelectionChanged);
	connect(rightPanel, &BasePanel::selectionChanged, this, &MainWindow::onPanelSelectionChanged);

	connect(leftPanel, &BasePanel::navigationError, this, &MainWindow::onPanelError);
	connect(rightPanel, &BasePanel::navigationError, this, &MainWindow::onPanelError);
}
void MainWindow::onAbout()
{
	QMessageBox::about(
		this,
		tr("About FAT Manager"),
		tr("<h2>FAT Manager</h2>"
		   "<p>A dual-panel file manager for FAT16/32</p>"
		   "<p>Arsen Shumilov M3134</p>"));
}
void MainWindow::onMount()
{
	if (getActivePanel() != leftPanel)
	{
		showInfo(tr("Select an image file in the left panel to mount."));
		return;
	}
	QStringList selected = leftPanel->selectedFiles();
	if (selected.isEmpty())
	{
		showInfo(tr("No file selected."));
		return;
	}
	const QFileInfo fileInfo(selected.first());
	if (fileInfo.isDir())
	{
		showInfo(tr("Cannot mount a directory. Please select a file."));
		return;
	}
	rightPanel->mountImage(fileInfo.absoluteFilePath());
}
void MainWindow::onMountDialog()
{
	const QString fileName =
		QFileDialog::getOpenFileName(this, tr("Open FAT Image"), QDir::homePath(), tr("FAT Images (*.img *.bin *.fat);;All Files (*)"));

	if (!fileName.isEmpty())
	{
		rightPanel->mountImage(fileName);
	}
}
void MainWindow::onCopy()
{
	if (!fileOperations)
		return;

	const BasePanel* source = getActivePanel();
	if (!source)
		return;

	if (source == rightPanel)
	{
		if (!rightPanel->isImageMounted())
		{
			showInfo(tr("No FAT image mounted"));
			return;
		}
		const QModelIndexList sourceIndexes = rightPanel->selectedIndexes();
		if (sourceIndexes.isEmpty())
		{
			showInfo(tr("No files selected"));
			return;
		}

		const QString destPath = leftPanel->currentPath();
		fileOperations->setFatModel(rightPanel->getModel());
		if (fileOperations->copyFatToSystem(sourceIndexes, destPath))
			statusLabel->setText(tr("Copy completed successfully"));
		else
			showError(tr("Some files were not copied"));
	}
	else
		showInfo(tr("Copy to FAT is not supported. FAT image is read-only."));
}
void MainWindow::onCalculateSize()
{
	if (!fileOperations)
		return;
	const BasePanel* active = getActivePanel();
	if (!active)
		return;

	QStringList selected = active->selectedFiles();
	if (selected.isEmpty())
	{
		showInfo(tr("No directories selected"));
		return;
	}
	const bool isFat = (active == rightPanel);
	quint64 totalSize = 0;
	for (const QString& path : selected)
	{
		if (QFileInfo info(path); isFat || info.isDir())
		{
			totalSize += fileOperations->calculateDirectorySize(path, isFat);
		}
	}
	if (selected.size() > 1)
	{
		statusLabel->setText(tr("Total size calculated"));
	}
}
void MainWindow::onExit()
{
	QApplication::quit();
}
void MainWindow::switchPanelFocus()
{
	if (activePanel == leftPanel)
		activePanel = rightPanel;
	else
		activePanel = leftPanel;
	leftPanel->setActive(activePanel == leftPanel);
	rightPanel->setActive(activePanel == rightPanel);
}
BasePanel* MainWindow::getActivePanel() const
{
	return activePanel;
}
void MainWindow::showError(const QString& message)
{
	QMessageBox::critical(this, tr("Error"), message);
}
void MainWindow::showInfo(const QString& message)
{
	QMessageBox::information(this, tr("Information"), message);
}

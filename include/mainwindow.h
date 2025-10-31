#pragma once

#include "keyboardhandler.h"

#include <QMainWindow>

class QSplitter;
class QLabel;
class QAction;
class QHBoxLayout;
class QVBoxLayout;

class LeftPanel;
class RightPanel;
class BasePanel;
class FileOperations;

class MainWindow final : public QMainWindow
{
	Q_OBJECT

  public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow() override;

	void loadFatImage(const QString& imagePath) const;
	void onKeyboardAction(KeyboardHandler::Action action);

  private slots:
	void onPanelSelectionChanged() const;
	void onPanelError(const QString& error);
	void onOperationStatusMessage(const QString& message) const;
	void onOperationProgress(int current, int total) const;

  private:
	void setupUi();
	void createPanels();
	void createToolBar(QVBoxLayout* mainLayout);
	void createStatusBar();
	void connectSignals();

	void onAbout();
	void onMount();
	void onMountDialog();
	void onCopy();
	void onCalculateSize();
	static void onExit();

	void switchPanelFocus();
	BasePanel* getActivePanel() const;

	void showError(const QString& message);
	void showInfo(const QString& message);

	QSplitter* mainSplitter;
	LeftPanel* leftPanel;
	RightPanel* rightPanel;
	BasePanel* activePanel;

	QLabel* statusLabel;
	QLabel* leftInfoLabel;
	QLabel* rightInfoLabel;

	KeyboardHandler* keyboardHandler;
	FileOperations* fileOperations;
};

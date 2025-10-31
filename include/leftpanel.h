#pragma once

#include "basepanel.h"

#include <QFileSystemModel>

class LeftPanel final : public BasePanel
{
	Q_OBJECT

  public:
	explicit LeftPanel(QWidget* parent = nullptr);
	~LeftPanel() override = default;
	QString currentPath() const override;
	QStringList selectedFiles() const override;
	bool canNavigateUp() const override;

	bool navigateTo(const QString& path);
	bool navigateUp();
	void setRootPath(const QString& path);
	QFileInfo getFileInfo(const QModelIndex& index) const;
	QString getPanelPrefix() const override;
	QFileSystemModel* getModel() const { return model; }

  private slots:
	void onDoubleClicked(const QModelIndex& index);
	void onSelectionChanged();
	void onDirectoryLoaded(const QString& path) const;

  private:
	QFileSystemModel* model;
	QModelIndex currentRootIndex;

	void setupModel();
	void setupView() const;
	void connectSignals();
	bool isDir(const QModelIndex& index) const;
};

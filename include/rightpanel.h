#pragma once

#include "basepanel.h"

class FatModel;
class FatParser;

class RightPanel final : public BasePanel
{
	Q_OBJECT

  public:
	explicit RightPanel(QWidget* parent = nullptr);
	~RightPanel() override = default;

	bool mountImage(const QString& imagePath);
	void unmountImage();
	bool isImageMounted() const;

	void navigateToRoot();
	void navigateUp();
	void navigateBack();
	bool canNavigateUp() const override;
	bool canNavigateBack() const;

	QString currentPath() const override;
	QStringList selectedFiles() const override;
	QString volumeLabel() const;
	QModelIndexList selectedIndexes() const;
	FatModel* getModel() const { return fatModel; }

  signals:
	void mountStatusChanged(bool mounted);

  private slots:
	void onDoubleClicked(const QModelIndex& index);
	void onSelectionChanged();

  protected:
	QString getPanelPrefix() const override;

  private:
	void setupModel();
	void setupView() const;
	void connectSignals();
	void onDirectoryChanged(quint32 cluster);
	void updateNavigationHistory(quint32 cluster);

	FatModel* fatModel;
	FatParser* fatParser;
	QString mountedImagePath;

	QVector< quint32 > navigationHistory;
	int historyIndex;
};

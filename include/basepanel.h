#pragma once
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

class BasePanel : public QWidget
{
	Q_OBJECT

  public:
	explicit BasePanel(QWidget* parent = nullptr);
	~BasePanel() override = default;

	virtual QString currentPath() const = 0;
	virtual QStringList selectedFiles() const = 0;
	virtual bool canNavigateUp() const = 0;
	virtual QString getPanelPrefix() const = 0;

	bool isActive() const;
	void setActive(bool active);
	QTableView* getView() const;

  signals:
	void pathChanged(const QString& path);
	void selectionChanged(const QModelIndexList& indexes);
	void fileActivated(const QString& path);
	void navigateUpRequest(const QString& path);
	void copyRequest(const QString& path);
	void navigationError(const QString& message);

  protected:
	QTableView* panelView;
	QLabel* header;
	bool active;

	void setupBaseUI();
	void updateHeader() const;

  private:
	QVBoxLayout* layout;
};

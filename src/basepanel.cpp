#include "basepanel.h"

#include <QHeaderView>

BasePanel::BasePanel(QWidget *parent) :
	QWidget(parent), panelView(nullptr), header(nullptr), active(false), layout(nullptr)
{
}

bool BasePanel::isActive() const
{
	return active;
}
void BasePanel::setActive(const bool active)
{
	this->active = active;
	updateHeader();
	if (active && panelView)
	{
		panelView->setFocus();
	}
}
QTableView *BasePanel::getView() const
{
	return panelView;
}
/*
enum GlobalColor {
	color0,
	color1,
	black,
	white,
	darkGray,
	gray,
	lightGray,
	red,
	green,
	blue,
	cyan,
	magenta,
	yellow,
	darkRed,
	darkGreen,
	darkBlue,
	darkCyan,
	darkMagenta,
	darkYellow,
	transparent
};*/
void BasePanel::setupBaseUI()
{
	layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	header = new QLabel(this);
	header->setAlignment(Qt::AlignCenter);
	header->setAutoFillBackground(true);

	panelView = new QTableView(this);
	panelView->setSelectionBehavior(QAbstractItemView::SelectRows);
	panelView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	panelView->setAlternatingRowColors(true);
	panelView->setSortingEnabled(true);

	QHeaderView *horizontalHeader = panelView->horizontalHeader();
	horizontalHeader->setSectionsClickable(true);
	horizontalHeader->setSectionsMovable(false);
	horizontalHeader->setHighlightSections(true);
	horizontalHeader->setSectionResizeMode(QHeaderView::Stretch);
	horizontalHeader->setMinimumSize(QSize(60, 40));

	layout->addWidget(header);
	layout->addWidget(panelView);

	setMinimumSize(QSize(300, 200));
}
void BasePanel::updateHeader() const
{
	if (!header)
		return;
	QString path = currentPath();
	if (path.isEmpty())
	{
		path = "/";
	}
	const QString displayText = getPanelPrefix();
	header->setText(displayText);
	header->setToolTip(path);
}

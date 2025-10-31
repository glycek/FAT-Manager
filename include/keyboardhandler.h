#pragma once

#include <QHash>
#include <QKeySequence>
#include <QObject>

class QShortcut;
class QWidget;

class KeyboardHandler final : public QObject
{
	Q_OBJECT

  public:
	enum Action
	{
		About,			  // F1
		Mount,			  // F2
		CalculateSize,	  // F3
		MountDialog,	  // F4
		Copy,			  // F5
		Move,			  // F6
		Mkdir,			  // F7
		Delete,			  // F8
		Exit,			  // F10
		SwitchPanel,	  // Tab
		NavigateUp,		  // Up
		NavigateBack,	  // Down
		ActionCount
	};

	explicit KeyboardHandler(QWidget* parent = nullptr);
	~KeyboardHandler() override;

	void setupShortcuts();
	QString shortcutText(Action action) const;
	void setEnabled(bool enabled);

  signals:
	void actionTriggered(Action action);

  private:
	void createShortcut(Action action, const QKeySequence& sequence);

	QWidget* parentWidget;
	QHash< Action, QShortcut* > shortcuts;
	bool enabled;
};

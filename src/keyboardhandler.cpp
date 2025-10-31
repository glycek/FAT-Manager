#include "keyboardhandler.h"

#include <QKeySequence>
#include <QShortcut>
#include <QWidget>

KeyboardHandler::KeyboardHandler(QWidget* parent) : QObject(parent), parentWidget(parent), enabled(true)
{
	setupShortcuts();
}
KeyboardHandler::~KeyboardHandler() = default;
void KeyboardHandler::setupShortcuts()
{
	if (!parentWidget)
		return;
	createShortcut(About, QKeySequence(Qt::Key_F1));
	createShortcut(Mount, QKeySequence(Qt::Key_F2));
	createShortcut(CalculateSize, QKeySequence(Qt::Key_F3));
	createShortcut(MountDialog, QKeySequence(Qt::Key_F4));
	createShortcut(Copy, QKeySequence(Qt::Key_F5));
	createShortcut(Move, QKeySequence(Qt::Key_F6));
	createShortcut(Mkdir, QKeySequence(Qt::Key_F7));
	createShortcut(Delete, QKeySequence(Qt::Key_F8));
	createShortcut(Exit, QKeySequence(Qt::Key_F10));

	createShortcut(SwitchPanel, QKeySequence(Qt::Key_Tab));
	createShortcut(NavigateUp, QKeySequence(Qt::Key_Up));
	createShortcut(NavigateBack, QKeySequence(Qt::Key_Down));
}
QString KeyboardHandler::shortcutText(const Action action) const
{
	if (const auto it = shortcuts.find(action); it != shortcuts.end())
	{
		return it.value()->key().toString();
	}
	return {};
}
void KeyboardHandler::setEnabled(const bool enabled)
{
	this->enabled = enabled;
	for (auto it = shortcuts.begin(); it != shortcuts.end(); ++it)
	{
		it.value()->setEnabled(enabled);
	}
}
void KeyboardHandler::createShortcut(Action action, const QKeySequence& sequence)
{
	if (!parentWidget)
		return;
	const auto shortcut = new QShortcut(sequence, parentWidget);
	shortcut->setEnabled(enabled);
	connect(shortcut,
			&QShortcut::activated,
			[this, action]()
			{
				if (enabled)
				{
					emit actionTriggered(action);
				}
			});
	shortcuts[action] = shortcut;
}

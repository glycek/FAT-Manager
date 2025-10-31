#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window;
	if (argc == 2)
	{
		const QString imagePath = argv[1];
		window.loadFatImage(imagePath);
	}
	window.show();
	return app.exec();
}

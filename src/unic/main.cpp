#include "unic.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	unic w;
	w.show();
	return a.exec();
}

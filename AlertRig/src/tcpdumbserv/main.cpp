#include "tcpdumbserv.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	tcpdumbserv w;
	w.show();
	return a.exec();
}

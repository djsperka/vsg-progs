#include "dummytcpserver.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	dummytcpserver w;
	w.show();
	return a.exec();
}

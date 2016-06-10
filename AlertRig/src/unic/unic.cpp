#include "unic.h"

unic::unic(QTcpSocket& socket, QWidget *parent)
: QMainWindow(parent)
, m_socket(socket)
{
	ui.setupUi(this);
}

unic::~unic()
{

}

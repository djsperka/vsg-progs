#ifndef UNIC_H
#define UNIC_H

#include <QtWidgets/QMainWindow>
#include "ui_unic.h"

class unic : public QMainWindow
{
	Q_OBJECT

public:
	unic(QWidget *parent = 0);
	~unic();

private:
	Ui::unicClass ui;
};

#endif // UNIC_H

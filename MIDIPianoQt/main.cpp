#include "MIDIPianoQt.h"
#include <iostream>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MIDIPianoQt w;
	w.show();
	return a.exec();
}

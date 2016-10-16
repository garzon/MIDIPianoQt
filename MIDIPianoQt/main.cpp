#include "MIDIPianoQt.h"
#include <iostream>
#include <fstream>
#include <QtWidgets/QApplication>

using namespace std;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MIDIPianoQt w;
	w.show();
	return a.exec();
}

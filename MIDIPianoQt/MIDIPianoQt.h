#ifndef MIDIPIANOQT_H
#define MIDIPIANOQT_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qpushbutton.h>
#include <qmessagebox.h>
#include <QFileDialog>
#include "SettingsDialog.h"
#include "ui_MIDIPianoQt.h"
#include "MidiIOManager.h"
#include <set>

#include "visualizationdialog.h"


#ifndef REP
#define REP(i,n) for(i=0;i!=(n);++i)
#endif

#ifndef FOR
#define FOR(i,a,b) for(i=(a);i!=(b);++i)
#endif

#ifndef FORE
#define FORE(i,a,b) for(i=(a);i<=(b);++i)
#endif

class myButton:public QPushButton{
	Q_OBJECT
public:
	int index;
	myButton(QWidget *parent,int _index):index(_index),QPushButton(parent){}
};

class MIDIPianoQt : public QMainWindow{
	Q_OBJECT
public:
	MIDIPianoQt(QWidget *parent = 0);
	~MIDIPianoQt();
	void setupButtons(int minIndex,int maxIndex);
    void playNote(int note, int volume=100, int channel=0);
	void clearNote(int note);
    void stopNote(int note, int channel = 0);
	void stopAll();
	bool isSubstained;

public slots:
    // doX()s
    void doPressed(int index, int vol = -1, int channel = 0);
    void doReleased(int index, int channel = 0);


private slots:
    // in buttonX()s: get index and forward to doX()
	void buttonPressed();
	void buttonReleased();

    // menu related
	void actionProcessor(QAction *);
private:
	Ui::MIDIPianoQtClass ui;
	SettingsDialog *configDlg;
	myButton *buttons[130];
	int _minIndex,_maxIndex;
    MidiIOManager *midiPointer;
	void showConfig();
	void keyPressEvent(QKeyEvent *);
	std::set<int> playedNotes;
};

class MIDICallback:public QObject{
	Q_OBJECT
public:
	std::function<void(long,long,long,long)> midiInCallback;
signals:
	void pressed(int note,int vol);
	void released(int note);
};

#endif // MIDIPIANOQT_H

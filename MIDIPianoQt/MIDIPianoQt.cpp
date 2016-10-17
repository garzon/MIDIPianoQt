#include "MIDIPianoQt.h"

#define getIndex int index=qobject_cast<myButton*>(sender())->index;

static const QString defaultStyleString[] = {
	"background-color: black;",
	"background-color: white;"
};

static const QString pressedStyleString = "background-color: #00FF00;";

bool isWhite(int note){
	while (note>71) note -= 12;
	while (note<60) note += 12;
	static const int whiteKey[] = { 60, 62, 64, 65, 67, 69, 71 };
	for (int white : whiteKey){
		if (note == white) return true;
	}
	return false;
}

void MIDIPianoQt::actionProcessor(QAction *a){
    if(a == ui.actionConfig){
		showConfig();
    }
    if(a == ui.actionOpenFile) {
        //QString fileName = QFileDialog::getOpenFileName(this, tr("Open Midi File"), QString(""), tr("Midi Files (*.mid;*.midi)"));
        //if(fileName.size()) {
            VisualizationDialog *dialog = (new VisualizationDialog(this));//->loadMidiFile(fileName);
            if(dialog) dialog->show();
        //}
    }
}

void MIDIPianoQt::setupButtons(int minIndex,int maxIndex){
	long i;
	FORE(i,_minIndex,_maxIndex){
		delete buttons[i];
	}
	_minIndex=minIndex;
	_maxIndex=maxIndex;
	static const int	widthWhite=16,
						widthBlack=11,
						heightWhite=70,
						heightBlack=50,
						x0=20,
						y0=30,
						padding=1,
						windowPadding=20;
	long x; // y=y0
	buttons[_minIndex]=new myButton(this,_minIndex);
	QRect tmp;
	i=_minIndex;
	if(isWhite(i)){
		tmp.setRect(x0,y0,widthWhite,heightWhite);
	}else{
		tmp.setRect(x0,y0,widthBlack,heightBlack);
	}
	buttons[i]->setStyleSheet(defaultStyleString[isWhite(i)]);
	buttons[i]->setGeometry(tmp);
	connect(buttons[i],SIGNAL(pressed()),this,SLOT(buttonPressed()));
	connect(buttons[i],SIGNAL(released()),this,SLOT(buttonReleased()));
	FORE(i,_minIndex+1,_maxIndex){
		buttons[i]=new myButton(this,i);
		if(isWhite(i)){
			if(isWhite(i-1)) 
				x=tmp.x()+tmp.width()+padding;
			else
				x=tmp.x()+tmp.width()/2+padding;
			tmp.setRect(x,y0,widthWhite,heightWhite);
		}else{
			x=tmp.x()+tmp.width()/2+padding;
			tmp.setRect(x,y0,widthBlack,heightBlack);
		}
		buttons[i]->setGeometry(tmp);
		buttons[i]->setStyleSheet(defaultStyleString[isWhite(i)]);
		connect(buttons[i],SIGNAL(pressed()),this,SLOT(buttonPressed()));
		connect(buttons[i],SIGNAL(released()),this,SLOT(buttonReleased()));
	}
	FORE(i,_minIndex,_maxIndex){
		if(!isWhite(i)){
			buttons[i]->raise();
		}
		buttons[i]->setFocusPolicy(Qt::NoFocus);
	}
	this->resize(tmp.x()+tmp.width()+windowPadding,tmp.y()+heightWhite+windowPadding);
}

void MIDIPianoQt::playNote(int note,int volume){
	if(note>=_minIndex)
		if(note<=_maxIndex)
			buttons[note]->setStyleSheet(pressedStyleString);
	playedNotes.insert(note);
	midiPointer->sendMsg(volume,note,9);
}

void MIDIPianoQt::clearNote(int note){
	if(note>=_minIndex)
		if(note<=_maxIndex)
			buttons[note]->setStyleSheet(defaultStyleString[isWhite(note)]);
}
void MIDIPianoQt::stopNote(int note){
	clearNote(note);
	playedNotes.erase(note);
	midiPointer->sendMsg(0,note,8);
}

void MIDIPianoQt::stopAll(){
	std::set<int>::iterator p;
	for (p = playedNotes.begin(); p != playedNotes.end();p++){
		clearNote(*p);
		midiPointer->sendMsg(0, *p, 8);
	}
	playedNotes.clear();
}

void MIDIPianoQt::doPressed(int index,int vol){
	if(vol==-1)
		playNote(index);
	else playNote(index,vol);
}

void MIDIPianoQt::doReleased(int index){
	if(isSubstained){
		clearNote(index);
		return;
	}
	stopNote(index);
}

void MIDIPianoQt::buttonPressed(){
	getIndex;
	doPressed(index);
}

void MIDIPianoQt::buttonReleased(){
	getIndex;
	doReleased(index);
}

void MIDIPianoQt::keyPressEvent(QKeyEvent *e){
	if (e->key() == Qt::Key_Space){
		stopAll();
	}
}

namespace _MIDIPianoQt{
	MIDICallback caller;
};

MIDIPianoQt::MIDIPianoQt(QWidget *parent)
	: QMainWindow(parent){

	ui.setupUi(this);
	_minIndex=0;
	_maxIndex=-1;
	isSubstained=true;

	_MIDIPianoQt::caller.midiInCallback=[](long vol,long note,long evt,long channel){
		if(evt==9){
			emit _MIDIPianoQt::caller.pressed(note,vol);
		}
		if(evt==8){
			emit _MIDIPianoQt::caller.released(note);
		}
	};

	connect(&_MIDIPianoQt::caller,SIGNAL(pressed(int,int)),this,SLOT(doPressed(int,int)));
	connect(&_MIDIPianoQt::caller,SIGNAL(released(int)),this,SLOT(doReleased(int)));

	try{
		midiPointer=new myMIDI(&_MIDIPianoQt::caller.midiInCallback);
	}catch(myMIDI::midiOutError){
		QMessageBox::warning(0,"Error","Cannot open midi output device!",QMessageBox::StandardButton::Close);
		midiPointer=NULL;
		// this->~MIDIPianoQt();
		qApp->exit(1);
		return;
	}
	if(!midiPointer->inputValid()){
        // QMessageBox::warning(0,"Warning","Cannot open midi input device!",QMessageBox::StandardButton::Ok);
	}
	setupButtons(36,96);
}

void MIDIPianoQt::showConfig(){
	configDlg=new SettingsDialog(midiPointer);
	configDlg->show();
}

MIDIPianoQt::~MIDIPianoQt(){
	if(midiPointer!=NULL) delete midiPointer;
}

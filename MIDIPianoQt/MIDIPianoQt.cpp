#include "MIDIPianoQt.h"

#define getIndex int index=qobject_cast<myButton*>(sender())->index;

static const QString defaultStyleString[] = {
	"background-color: black;",
	"background-color: white;"
};

static const QString pressedStyleString = "background-color: #00FF00;";

static bool isWhite(int note){
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
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Midi File"), QString(""), tr("Midi Files (*.mid;*.midi)"));
        if(fileName.size()) {
            VisualizationDialog *dialog = (new VisualizationDialog(this))->loadMidiFile(fileName);
            if(dialog) dialog->show();
        }
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

#define ENCODE_NOTE_CHANNEL(note, channel) ((note << 8) | (channel))
#define DECODE_NOTE_CHANNEL(v, note, channel) note = (((unsigned int)(v) & 0xFF00) >> 8); channel = (v & 0xFF)

void MIDIPianoQt::playNote(int note, int volume, int channel){
	if(note>=_minIndex)
		if(note<=_maxIndex)
			buttons[note]->setStyleSheet(pressedStyleString);

    noteSetMutex.lock();
    playedNotes.emplace(ENCODE_NOTE_CHANNEL(note, channel));
    noteSetMutex.unlock();

    midiPointer->sendMsg(volume, note, 9, channel);
}

void MIDIPianoQt::clearNote(int note) {
	if(note>=_minIndex)
		if(note<=_maxIndex)
			buttons[note]->setStyleSheet(defaultStyleString[isWhite(note)]);
}

void MIDIPianoQt::stopNote(int note, int channel){
    clearNote(note);
    noteSetMutex.lock();
    playedNotes.erase(ENCODE_NOTE_CHANNEL(note, channel));
    noteSetMutex.unlock();
    midiPointer->sendMsg(0, note, 0x8, channel);
}

void MIDIPianoQt::stopAll(){
    noteSetMutex.lock();
    for(int p: playedNotes){
        DECODE_NOTE_CHANNEL(p, unsigned int note, unsigned int channel);
        clearNote(note);
        midiPointer->sendMsg(0, note, 8, channel);
	}
	playedNotes.clear();
    noteSetMutex.unlock();
}

void MIDIPianoQt::doPressed(int index, int vol, int channel){
	if(vol==-1)
		playNote(index);
    else playNote(index, vol, channel);
}

void MIDIPianoQt::doReleased(int index, int channel, bool _isSubstained){
    if(_isSubstained && isSubstained){
		clearNote(index);
		return;
	}
    stopNote(index, channel);
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

static MIDICallback caller;

MIDIPianoQt::MIDIPianoQt(QWidget *parent)
	: QMainWindow(parent){

	ui.setupUi(this);
	_minIndex=0;
	_maxIndex=-1;
    isSubstained = false;

    caller.midiInCallback = [](long vol,long note,long evt,long channel){
		if(evt==9){
            emit caller.pressed(note,vol);
		}
		if(evt==8){
            emit caller.released(note);
		}
	};

    connect(&caller, SIGNAL(pressed(int,int)),this,SLOT(doPressed(int,int)));
    connect(&caller, SIGNAL(released(int)),this,SLOT(doReleased(int)));

    try {
        midiPointer = new MidiIOManager(&(caller.midiInCallback));
    } catch(MidiIOManager::midiOutError) {
		QMessageBox::warning(0,"Error","Cannot open midi output device!",QMessageBox::StandardButton::Close);
		midiPointer=NULL;
		// this->~MIDIPianoQt();
		qApp->exit(1);
		return;
	}
	if(!midiPointer->inputValid()){
        // QMessageBox::warning(0,"Warning","Cannot open midi input device!",QMessageBox::StandardButton::Ok);
    }
    setupButtons(21,21+87);
}

void MIDIPianoQt::showConfig(){
    configDlg = new SettingsDialog(midiPointer);
	configDlg->show();
}

MIDIPianoQt::~MIDIPianoQt(){
    if(midiPointer != NULL) delete midiPointer;
}

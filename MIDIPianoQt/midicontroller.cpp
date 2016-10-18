#include "midicontroller.h"

MidiController::MidiController(MidiData &model, MidiOpenGLWidget *view, QMainWindow *mainUI, QObject *parent) :
    data(model), v(view), now(0), p(model.begin()), isPause(true), ui(mainUI), QObject(parent)
{
    connect(this, SIGNAL(pressed(int,int,int)), ui, SLOT(doPressed(int,int,int)));
    connect(this, SIGNAL(released(int,int,bool)), ui, SLOT(doReleased(int,int,bool)));
    connect(this, SIGNAL(mute()), ui, SLOT(stopAll()));
}

inline void MidiController::updateView() {
    v->setTime(now);
}

#define setNow(newNow) now = (newNow); updateView()

void MidiController::playLoop() {
    if(isPause) return;

    playLoopMutex.lock();

    auto end = data.end();
    while(p != end) {
        setNow(now + timeSincePlay.elapsed());
        timeSincePlay.restart();
        if(p->absoluteTime > now) break;
        while(p != end) {
            if(p->absoluteTime > now) break;
            playEvent(*p);
            ++p;
        }
    }

    if(p != end) {
        unsigned long leftTime = p->absoluteTime - now;
        QTimer::singleShot(leftTime, this, SLOT(playLoop()));
    } else {
        isPause = true;
        v->isPaused = true;
        setNow(data.totalTime);
    }

    playLoopMutex.unlock();
}

static const int keyStart = 21, keyEnd = keyStart + 87;

void MidiController::playEvent(MidiEvent &event) {
    MidiIOManager *io = MidiIOManager::getInstance();
    if(io == NULL) throw "MidiIOManager::getInstance() return NULL; Cannot open io dev.";

    switch(event.subtype) {
    case MidiEvent::NOTE_ON:
        if(event.lastTime == -1) return;
        if(event.noteNumber >= keyStart && event.noteNumber <= keyEnd) {
            emit pressed(event.noteNumber, event.velocity, event.channel);
        }
        // else io->sendMsg(event.velocity, event.noteNumber, 0x9, event.channel);
        break;
    case MidiEvent::NOTE_OFF:
        if(event.noteNumber >= keyStart && event.noteNumber <= keyEnd) {
            emit released(event.noteNumber, event.channel, false);
        }
        // else io->sendMsg(0, event.noteNumber, 0x8, event.channel);
        break;
    case MidiEvent::PROG_CHANGE:
        io->sendMsg(0, event.programNumber, 0xC, event.channel);
        break;
    default:
        break;
    }
}

void MidiController::pause() {
    if(isPause) return;
    isPause = true;
    v->isPaused = true;
    emit mute();
    playLoopMutex.lock();
    setNow(now + timeSincePlay.elapsed());
    playLoopMutex.unlock();
}

void MidiController::play() {
    if(!isPause) return;
    timeSincePlay.restart();
    isPause = false;
    v->isPaused = false;
    playLoop();
}

void MidiController::jumpTo(unsigned long time) {
    bool prevPaused = isPause;
    pause();
    timeSincePlay.restart();
    setNow(time);
    p = data.find(time);
    if(!prevPaused) play();
}

#include "midicontroller.h"

MidiController::MidiController(MidiData &model, MidiOpenGLWidget *view, QMainWindow *mainUI, QObject *parent) :
    data(model), v(view), now(0), p(model.begin()), isPause(true), ui(mainUI), QObject(parent)
{
    connect(this, SIGNAL(pressed(int,int,int)), ui, SLOT(doPressed(int,int,int)));
    connect(this, SIGNAL(released(int,int)), ui, SLOT(doReleased(int,int)));
}

void MidiController::playLoop() {
    if(isPause) return;

    playLoopMutex.lock();

    auto end = data.end();
    while(p != end) {
        now += timeSincePlay.elapsed();
        v->setTime(now);
        timeSincePlay.restart();
        if(p->absoluteTime > now) break;
        while(p != end) {
            if(p->absoluteTime > now) break;
            playEvent(*p);
            ++p;
        }
    }

    if(p != data.end()) {
        unsigned long leftTime = p->absoluteTime - now;
        QTimer::singleShot(leftTime, this, SLOT(playLoop()));
    } else {
        now = data.totalTime;
        v->setTime(now);
        pause();
    }

    playLoopMutex.unlock();
}

static const int keyStart = 21, keyEnd = keyStart + 87;

void MidiController::playEvent(MidiEvent &event) {
    MidiIOManager *io = MidiIOManager::getInstance();
    if(io == NULL) throw "MidiIOManager::getInstance() return NULL; Cannot open io dev.";

    switch(event.subtype) {
    case MidiEvent::NOTE_ON:
        if(event.noteNumber >= keyStart && event.noteNumber <= keyEnd) {
            emit pressed(event.noteNumber, event.velocity, event.channel);
        } else io->sendMsg(event.velocity, event.noteNumber, 0x9, event.channel);
        break;
    case MidiEvent::NOTE_OFF:
        if(event.noteNumber >= keyStart && event.noteNumber <= keyEnd) {
            emit released(event.noteNumber, event.channel);
        } else io->sendMsg(0, event.noteNumber, 0x8, event.channel);
        break;
    case MidiEvent::PROG_CHANGE:
        break;
    default:
        break;
    }
}

void MidiController::pause() {
    if(isPause) return;
    isPause = true;
    v->isPaused = true;
    playLoopMutex.lock();
    now += timeSincePlay.elapsed();
    v->setTime(now);
    playLoopMutex.unlock();
}

void MidiController::play() {
    if(!isPause) return;
    timeSincePlay.restart();
    isPause = false;
    v->isPaused = false;
    playLoop();
}

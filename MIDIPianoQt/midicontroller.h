#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include <mutex>

#include <QMainWindow>
#include <QTime>
#include <QTimer>

#include "midifilereader.h"
#include "MidiIOManager.h"
#include "midiopenglwidget.h"

class MidiController: public QObject {
    Q_OBJECT

    MidiData &data;
    MidiOpenGLWidget *v;
    MidiData::iterator p;

    QMainWindow *ui;

    bool isPause;
    QTime timeSincePlay;
    unsigned long now;

    std::mutex playLoopMutex;

    void playEvent(MidiEvent &event);
    void updateView();

private slots:
    void playLoop();

public:
    MidiController(MidiData &model, MidiOpenGLWidget *view, QMainWindow *mainUI, QObject *parent = 0);

    void play();
    void pause();
    void jumpTo(unsigned long time = 0);

    bool isPaused() const { return isPause; }
    unsigned long playTime() const { return now; }

signals:
    void mute();
    void pressed(int index, int vol = -1, int channel = 0);
    void released(int index, int channel = 0, bool isSubstained = true);
};

#endif // MIDICONTROLLER_H

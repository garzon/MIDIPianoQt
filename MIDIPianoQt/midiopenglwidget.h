#ifndef MIDIOPENGLWIDGET_H
#define MIDIOPENGLWIDGET_H

#include <QTimer>
#include <QTime>

#include "openglwidget.h"
#include "midifilereader.h"

class MidiOpenGLWidget : public OpenGLWidget {
    Q_OBJECT
public:
    MidiOpenGLWidget(QWidget *parent = 0):
        OpenGLWidget(parent)
    {}

    void switchView();
    void loadMidiData(MidiData &midiData);

    void setTime(unsigned long actualTime) {
        now = actualTime;
        lastEventTimer.restart();
    }

    void pause(unsigned long actualTime) {
        now = actualTime;
        isPaused = true;
    }

    bool isPaused;
protected:
    unsigned long totalTime;
    double now;

    void addMidiNoteBar(unsigned long absTime, unsigned long lastToTime, int note, int channel);

    void resizeGL(int w, int h);
    void paintGL();

    QTime lastEventTimer;
    bool is2dView;
};

#endif // MIDIOPENGLWIDGET_H

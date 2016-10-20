#ifndef MIDIOPENGLWIDGET_H
#define MIDIOPENGLWIDGET_H

#include <functional>

#include <QTimer>
#include <QTime>

#include "openglwidget.h"
#include "midifilereader.h"

class MidiOpenGLWidget : public OpenGLWidget {
    Q_OBJECT
public:
    MidiOpenGLWidget(QWidget *parent = 0):
        OpenGLWidget(parent), callback([](){})
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

    void setUpdateFrameCallback(std::function<void()> _callback) {
        callback = _callback;
    }

    static GLfloat calcNoteXCor(int noteNumber);

    bool isPaused;
protected:
    unsigned long totalTime;
    double now;

    void addMidiNoteBar(unsigned long absTime, unsigned long lastToTime, int note, int channel);


    void drawArrivalPlane();
    void drawVerticalKeyboard();
    void drawHorizontalKeyboard();

    void resizeGL(int w, int h);
    void paintGL();

    std::function<void()> callback;

    QTime lastEventTimer;
    bool is2dView;
};

#endif // MIDIOPENGLWIDGET_H

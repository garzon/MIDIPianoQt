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

    void addMidiNoteBar(unsigned long absTime, unsigned long lastToTime, int note, int channel);

    void switchView();
    void loadMidiData(MidiData &midiData);

    double now;
protected:
    unsigned long totalTime;

    void resizeGL(int w, int h);
    void paintGL();

    QTime fromStart;
    bool is2dView;
};

#endif // MIDIOPENGLWIDGET_H

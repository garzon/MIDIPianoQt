#include "midiopenglwidget.h"

#include <time.h>

using namespace std;

// 1px/3000ms
#define convertMsToPx(ms) (ms*1.0/3000.0)
#define nowY ((GLfloat)(convertMsToPx(now)))

static const QVector4D DUMMY_COLOR(0,0,0,1);

static inline bool isWhite(int note){
    note %= 12;
    if(note == 0) return true;
    if(note == 2) return true;
    if(note == 4) return true;
    if(note == 5) return true;
    if(note == 7) return true;
    if(note == 9) return true;
    if(note == 11) return true;
    return false;
}

static GLfloat linearMap(GLfloat v, GLfloat oMin, GLfloat oMax, GLfloat nMin, GLfloat nMax) {
    return (v - oMin) / (oMax - oMin) * (nMax - nMin) + nMin;
}

static const GLfloat whiteW = 16.0, blackW = 11.0;
static const int keyStart = 21, keyEnd = keyStart + 87;
static const GLfloat whiteNum = 45.0, blackNum = 33.0;
static const GLfloat maxZ = 2;

void MidiOpenGLWidget::addMidiNoteBar(unsigned long absTime, unsigned long lastToTime, int note, int channel) {
    if(note < keyStart || note > keyEnd) return;
    GLfloat w = isWhite(note) ? whiteW : blackW;
    GLfloat sx = linearMap(note, keyStart, keyEnd, -1.0, 1.0);
    GLfloat nw = w/(whiteW*whiteNum)*2.0;
    GLfloat nChan = ((GLfloat)((channel & 1) ? (-((channel+1) >> 1)) : (channel >> 1))) / 8.0 * maxZ;
    GLfloat nh = maxZ / 8.0 / 2.0;
    addQuad(
        QVector3D(sx, convertMsToPx(absTime), nChan),
        QVector3D(sx+nw, convertMsToPx(absTime), nChan),
        QVector3D(sx, convertMsToPx(lastToTime), nChan),
        QVector3D(sx, convertMsToPx(absTime), nChan+nh),
        rgba(128,128,128)
    );
}

void MidiOpenGLWidget::loadMidiData(MidiData &midiData) {
    totalTime = midiData.totalTime;

    addStaticVertexBegin();

    auto end = midiData.end();
    int counter = 0;
    for(auto p=midiData.begin(); p!=end; ++p) {
        MidiEvent &event = *p;
        if(event.subtype == MidiEvent::NOTE_ON && event.lastEventIdx != -1) {
            addMidiNoteBar(event.absoluteTime, midiData.tracks[event.trackId][event.lastEventIdx].absoluteTime, event.noteNumber, event.channel);
        }
        counter ++;
    }

    addStaticVertexEnd();

    // ---------------------------------------------- auto refresher
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);
    fromStart.start();
}

void MidiOpenGLWidget::paintGL() {
    now = (fromStart.elapsed() % totalTime);

    mProj.setToIdentity();
    mWorld.setToIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    if(!is2dView) {
        mProj.perspective(18.0f, GLfloat(width()) / height(), 0.01f, 100.0f);
        mWorld.lookAt(
            QVector3D(4,nowY+5,maxZ+2),
            QVector3D(0,nowY+1,0),
            QVector3D(0,0,1)
        );
        mWorld.scale(-1,1,1);
    } else {
        mWorld.translate(0, -nowY-1, 0);
    }

    OpenGLWidget::paintGL();

    vector<GLfloat> planeVertexs {
        -1, nowY, -maxZ,
        -1, nowY, maxZ,
        1, nowY, maxZ,

        1, nowY, maxZ,
        1, nowY, -maxZ,
        -1, nowY, -maxZ,
    };
    drawDynamics(planeVertexs, QVector4D(1,1,1,0.3f));

    // -------------------  draw keyboard
    const GLfloat zDown = -0.25;
    const GLfloat zUp = 0.25;
    const GLfloat alpha = 0.5;
    const GLfloat eps = 0.001f;

    // draw white keys
    drawDynamicsBegin(planeVertexs);
    addRect(
        QVector3D(1.0, nowY+eps, zUp),
        QVector3D(1.0, nowY+eps, zDown),
        QVector3D(-1.0, nowY+eps, zUp),
        DUMMY_COLOR
    );
    drawDynamics(planeVertexs, QVector4D(1,1,1,1));

    // draw black keys
    drawDynamicsBegin(planeVertexs);
    for(int i = keyStart; i <= keyEnd; i++){
        if(isWhite(i)) continue;
        GLfloat sx = linearMap(i, keyStart, keyEnd, -1.0, 1.0);
        GLfloat nw = blackW/(whiteW*whiteNum)*2.0;
        addRect(
            QVector3D(sx, nowY+eps*2, zUp),
            QVector3D(sx, nowY+eps*2, zDown+0.125),
            QVector3D(sx+nw, nowY+eps*2, zUp),
            DUMMY_COLOR
        );
    }
    drawDynamics(planeVertexs, QVector4D(0,0,0,alpha));
}

void MidiOpenGLWidget::switchView() {
    is2dView = !is2dView;
    resizeGL(width(), height());
}

void MidiOpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

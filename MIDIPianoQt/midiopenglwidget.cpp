#include "midiopenglwidget.h"

#include <time.h>

using namespace std;

// 1px/3000ms
#define convertMsToPx(ms) (ms*1.0/3000.0)

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

static const QVector4D beautifulColors[16]{
    OpenGLWidget::rgba(68,206,246, 1),
    OpenGLWidget::rgba(37,248,203, 1),
    OpenGLWidget::rgba(255,137,54, 1),
    OpenGLWidget::rgba(250,255,114, 1),
    OpenGLWidget::rgba(195,39,43, 1),
    OpenGLWidget::rgba(255,179,167, 1),
    OpenGLWidget::rgba(255,51,0, 1),
    OpenGLWidget::rgba(64,222,90, 1),
    OpenGLWidget::rgba(75,92,196,1),
    OpenGLWidget::rgba(48,223,243,1),
    OpenGLWidget::rgba(141,75,187, 1),
    OpenGLWidget::rgba(163,217,0, 1),
    OpenGLWidget::rgba(137,108,57, 1),
    OpenGLWidget::rgba(255,0,151, 1),
    OpenGLWidget::rgba(242,12,0, 1),
    OpenGLWidget::rgba(167,142,68, 1),
};

static const GLfloat whiteW = 16.0, blackW = 11.0, whiteH = 70.0, blackH = 50.0, padding = 1;
static const GLfloat whiteNum = 45.0, blackNum = 33.0;
static const int keyStart = 21, keyEnd = keyStart + 87;
static const GLfloat nWhiteW = whiteW/(whiteW*whiteNum + padding * (keyEnd-keyStart))*2.0;
static const GLfloat nBlackW = blackW/(whiteW*whiteNum + padding * (keyEnd-keyStart))*2.0;
static const GLfloat nPadding = padding/(whiteW*whiteNum + padding * (keyEnd-keyStart))*2.0;
static const GLfloat maxZ = 0.8;
static const GLfloat maxOriginalX = padding * (keyEnd - keyStart) + whiteNum * whiteW;

static const int whiteNumLeftInOctave[] = {
    3, 4, 4, 5, 5, 7, 8, 8, 9, 0, 1, 1,
};
static const int blackNumLeftInOctave[] = {
    1, 1, 2, 2, 3, 3, 3, 4, 4, 0, 0, 1,
};


GLfloat MidiOpenGLWidget::calcNoteXCor(int noteNumber) {
    GLfloat x = 0;
    x += padding * (noteNumber - keyStart);
    int octaves = (noteNumber - keyStart) / 12;
    int mods = noteNumber % 12;
    x += (octaves * (9 * whiteW + 5 * blackW) +
          whiteNumLeftInOctave[mods] * whiteW +
          blackNumLeftInOctave[mods] * blackW) * 0.5;
    return linearMap(x, 0, 810, -1, 1);
}


void MidiOpenGLWidget::addMidiNoteBar(unsigned long absTime, unsigned long lastToTime, int note, int channel) {
    if(note < keyStart || note > keyEnd) return;
    GLfloat nw = isWhite(note) ? nWhiteW : nBlackW;
    GLfloat sx = calcNoteXCor(note);
    GLfloat nChan = ((GLfloat)((channel & 1) ? (-((channel+1) >> 1)) : (channel >> 1))) / 8.0 * maxZ;
    GLfloat nh = maxZ / 8.0 / 2.0;
    addQuad(
        QVector3D(sx, convertMsToPx(absTime), nChan),
        QVector3D(sx+nw, convertMsToPx(absTime), nChan),
        QVector3D(sx, convertMsToPx(lastToTime), nChan),
        QVector3D(sx, convertMsToPx(absTime), nChan+nh),
        beautifulColors[channel]
    );
}

void MidiOpenGLWidget::loadMidiData(MidiData &midiData) {
    totalTime = midiData.totalTime;

    // add the static vertex(midi notes)
    addStaticVertexBegin();
    for(auto &event: midiData) {
        if(event.subtype == MidiEvent::NOTE_ON && event.lastEventIdx != -1) {
            addMidiNoteBar(event.absoluteTime, midiData.tracks[event.trackId][event.lastEventIdx].absoluteTime, event.noteNumber, event.channel);
        }
    }
    addStaticVertexEnd();

    // auto refresh each frame(100fps)
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);
    lastEventTimer.start();
}

static const GLfloat zDown = -0.2;
static const GLfloat zUp = 0.2;
static const GLfloat alpha = 1;
static const GLfloat eps = 0.001f;
static const GLfloat keyboardH = 0.3;

GLfloat nowY;

void MidiOpenGLWidget::paintGL() {
    unsigned long currFrameTime = now + lastEventTimer.elapsed();
    if(currFrameTime > totalTime) currFrameTime = totalTime;
    if(isPaused) currFrameTime = now;
    nowY = ((GLfloat)(convertMsToPx(currFrameTime)));

    // set projection matrix and world matrix (move the camera)
    mProj.setToIdentity();
    mWorld.setToIdentity();
    if(!is2dView) {
        mProj.perspective(14.5f, GLfloat(width()) / height(), 0.01f, 100.0f);
        mWorld.lookAt(
            QVector3D(4,nowY+5,maxZ+1.5),
            QVector3D(0.3,nowY+1.4,0+0.4),
            QVector3D(0,0,1)
        );
        mWorld.scale(-1,1,1);
    } else {
        mWorld.translate(0, -nowY-1+keyboardH, 0.1);
    }

    // openGL settings
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // call parent's function to draw statics
    OpenGLWidget::paintGL();

    // ------------------ draw dynamics --------------------

    drawArrivalPlane();
    drawVerticalKeyboard();
    drawHorizontalKeyboard();

    callback();
}

void MidiOpenGLWidget::drawArrivalPlane() {
    vector<GLfloat> planeVertexs {
        -1, nowY, -maxZ,
        -1, nowY, maxZ,
        1, nowY, maxZ,

        1, nowY, maxZ,
        1, nowY, -maxZ,
        -1, nowY, -maxZ,
    };
    drawDynamicsBegin(planeVertexs);
    drawDynamicsEnd(QVector4D(1,1,1,0.3f));
}

void MidiOpenGLWidget::drawVerticalKeyboard() {
    vector<GLfloat> planeVertexs;

    // draw white keys
    drawDynamicsBegin(planeVertexs);
    for(int i = keyStart; i <= keyEnd; i++){
        if(!isWhite(i)) continue;
        GLfloat sx = calcNoteXCor(i);
        addRect(
            QVector3D(sx, nowY+eps, zUp),
            QVector3D(sx, nowY+eps, zDown),
            QVector3D(sx+nWhiteW, nowY+eps, zUp)
        );
    }
    drawDynamicsEnd(QVector4D(1,1,1,alpha));


    // draw black keys & border lines of white keys
    drawDynamicsBegin(planeVertexs);
    for(int i = keyStart; i <= keyEnd; i++){
        GLfloat sx = calcNoteXCor(i);
        if(isWhite(i)) {
            addRect(
                QVector3D(sx+nWhiteW, nowY+eps*3, zUp),
                QVector3D(sx+nWhiteW, nowY+eps*3, zDown),
                QVector3D(sx+nWhiteW+nPadding, nowY+eps*3, zUp)
            );
        } else {
            addRect(
                QVector3D(sx, nowY+eps*2, zUp),
                QVector3D(sx, nowY+eps*2, zDown+0.125),
                QVector3D(sx+nBlackW, nowY+eps*2, zUp)
            );
        }
    }
    drawDynamicsEnd(QVector4D(0,0,0,alpha));
}

void MidiOpenGLWidget::drawHorizontalKeyboard() {
    vector<GLfloat> planeVertexs;

    // draw white keys
    drawDynamicsBegin(planeVertexs);
    for(int i = keyStart; i <= keyEnd; i++){
        if(!isWhite(i)) continue;
        GLfloat sx = calcNoteXCor(i);
        addRect(
            QVector3D(sx, nowY, -maxZ),
            QVector3D(sx, nowY-keyboardH, -maxZ),
            QVector3D(sx+nWhiteW, nowY, -maxZ)
        );
    }
    drawDynamicsEnd(QVector4D(1,1,1,alpha));


    // draw black keys & border lines of white keys
    drawDynamicsBegin(planeVertexs);
    for(int i = keyStart; i <= keyEnd; i++){
        GLfloat sx = calcNoteXCor(i);
        if(isWhite(i)) {
            addRect(
                QVector3D(sx+nWhiteW, nowY, -maxZ-eps*3),
                QVector3D(sx+nWhiteW, nowY-keyboardH, -maxZ-eps*3),
                QVector3D(sx+nPadding+nWhiteW, nowY, -maxZ-eps*3)
            );
        } else {
            addRect(
                QVector3D(sx, nowY, -maxZ-eps*2),
                QVector3D(sx, nowY-keyboardH*blackH/whiteH, -maxZ-eps*2),
                QVector3D(sx+nBlackW, nowY, -maxZ-eps*2)
            );
        }
    }
    drawDynamicsEnd(QVector4D(0,0,0,alpha));
}

void MidiOpenGLWidget::switchView() {
    is2dView = !is2dView;
}

void MidiOpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

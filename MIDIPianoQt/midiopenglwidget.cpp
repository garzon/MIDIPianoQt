#include "midiopenglwidget.h"

#include <time.h>

using namespace std;

#define EMPLACE_BACK_3(v, p) v.emplace_back(p.x()); v.emplace_back(p.y()); v.emplace_back(p.z())
#define EMPLACE_BACK_4(v, p) EMPLACE_BACK_3(v, p); v.emplace_back(p.w())
#define EMPLACE_COLOR(v, c) EMPLACE_BACK_4(v, c);EMPLACE_BACK_4(v, c);EMPLACE_BACK_4(v, c)

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


MidiOpenGLWidget::MidiOpenGLWidget(QWidget *parent):
    QOpenGLWidget(parent), outputPosVec(&vertexPositions), outputColorVec(&vertexColors)
{}

static GLfloat linearMap(GLfloat v, GLfloat oMin, GLfloat oMax, GLfloat nMin, GLfloat nMax) {
    return (v - oMin) / (oMax - oMin) * (nMax - nMin) + nMin;
}

void MidiOpenGLWidget::addRect(const QVector3D &p, const QVector3D &pBottom, const QVector3D &pRight, const QVector4D &c) {
    QVector3D p_ = pBottom + pRight - p;
    addTriangle(p, pBottom, p_, c);
    addTriangle(p, p_, pRight, c);
}

void MidiOpenGLWidget::addTriangle(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3, const QVector4D &c) {
    if(outputPosVec) {
        EMPLACE_BACK_3((*outputPosVec), p1);
        EMPLACE_BACK_3((*outputPosVec), p2);
        EMPLACE_BACK_3((*outputPosVec), p3);
    }
    if(outputColorVec) {
        EMPLACE_COLOR((*outputColorVec), c);
    }
}

void MidiOpenGLWidget::addQuad(const QVector3D& p, const QVector3D& px, const QVector3D& py, const QVector3D& pz, const QVector4D &c) {
    QVector3D pxpy = px + py - p;
    QVector3D pypz = py + pz - p;
    QVector3D pxpz = px + pz - p;
    QVector3D p_ = pxpz + pypz - pz;

    // bottom
    // p -> px -> pxpy
    // p -> pxpy -> py
    addTriangle(p, px, pxpy, c);
    addTriangle(p, pxpy, py, c);

    // p -> pxpz -> px
    // p -> pz -> pxpz
    addTriangle(p, pxpz, px, c);
    addTriangle(p, pz, pxpz, c);

    // p -> py -> pypz
    // p -> pypz -> pz
    addTriangle(p, py, pypz, c);
    addTriangle(p, pypz, pz, c);

    // top
    // p' -> pz -> pxpz
    // p' -> pypz -> pz
    addTriangle(p_, pz, pxpz, c);
    addTriangle(p_, pypz, pz, c);

    // px -> p' -> pxpz
    // px -> pxpy -> p'
    addTriangle(px, p_, pxpz, c);
    addTriangle(px, pxpy, p_, c);

    // pxpy -> py -> p'
    // py -> pypz -> p'
    addTriangle(pxpy, py, p_, c);
    addTriangle(py, pypz, p_, c);
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

    auto end = midiData.end();
    int counter = 0;
    for(auto p=midiData.begin(); p!=end; ++p) {
        MidiEvent &event = *p;
        if(event.subtype == MidiEvent::NOTE_ON && event.lastEventIdx != -1) {
            addMidiNoteBar(event.absoluteTime, midiData.tracks[event.trackId][event.lastEventIdx].absoluteTime, event.noteNumber, event.channel);
        }
        counter ++;
    }

    // ---------------------------------------------- auto refresher
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);
    fromStart.start();
}

void MidiOpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();

    if(!program.addShaderFromSourceCode(QOpenGLShader::Vertex,
          "attribute vec4 position; attribute vec4 color; varying lowp vec4 vColor; uniform mat4 projMatrix; uniform mat4 mvMatrix; void main() { gl_Position = projMatrix * mvMatrix * position; vColor = color; }")) {
        QMessageBox::warning(this, "QOpenGLShader::Vertex", "QOpenGLShader::Vertex" + program.log());
        close();
    }
    if(!program.addShaderFromSourceCode(QOpenGLShader::Fragment,
          "varying lowp vec4 vColor; void main() { gl_FragColor = vColor; }")) {
        QMessageBox::warning(this, "QOpenGLShader::Fragment", "QOpenGLShader::Fragment" + program.log());
        close();
    }
    program.bindAttributeLocation("position", 0);
    program.bindAttributeLocation("color", 1);
    if (!program.link()) close();
    if (!program.bind()) close();

    m_projMatrixLoc = program.uniformLocation("projMatrix");
    m_mvMatrixLoc = program.uniformLocation("mvMatrix");

    m_vao.create();

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    posVBO.create();
    posVBO.bind();
    posVBO.allocate(vertexPositions.data(), sizeof(GLfloat)*vertexPositions.size());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    colorVBO.create();
    colorVBO.bind();
    colorVBO.allocate(vertexColors.data(), sizeof(GLfloat)*vertexColors.size());
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

    program.release();

    if(!planeProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
          "attribute vec4 position; uniform mat4 projMatrix; uniform mat4 mvMatrix; void main() { gl_Position = projMatrix * mvMatrix * position; }")) {
        QMessageBox::warning(this, "QOpenGLShader::Vertex", "QOpenGLShader::Vertex" + program.log());
        close();
    }
    if(!planeProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
          "uniform highp vec4 color; void main() { gl_FragColor = color; }")) {
        QMessageBox::warning(this, "QOpenGLShader::Fragment", "QOpenGLShader::Fragment" + program.log());
        close();
    }
    planeProgram.bindAttributeLocation("position", 2);
    if (!planeProgram.link()) close();
    if (!planeProgram.bind()) close();

    planeProjMatrixLoc = planeProgram.uniformLocation("projMatrix");
    planeMvMatrixLoc = planeProgram.uniformLocation("mvMatrix");
    planeVBO.create();
    planeVBO.bind();
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    /*planeColorVBO.create();
    planeColorVBO.bind();
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);*/

    planeProgram.release();
}

void MidiOpenGLWidget::paintGL() {
    now = (fromStart.elapsed() % totalTime);

    m_proj.setToIdentity();
    m_world.setToIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(!is2dView) {
        m_proj.perspective(18.0f, GLfloat(width()) / height(), 0.01f, 100.0f);
        m_world.lookAt(
            QVector3D(4,nowY+5,maxZ+2),
            QVector3D(0,nowY+1,0),
            QVector3D(0,0,1)
        );
        m_world.scale(-1,1,1);
    } else {
        m_world.translate(0, -nowY-1, 0);
    }

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    program.bind();
    program.setUniformValue(m_projMatrixLoc, m_proj);
    program.setUniformValue(m_mvMatrixLoc, m_world);
    glDrawArrays(GL_TRIANGLES, 0, vertexPositions.size()/3);
    program.release();

    planeProgram.bind();
    planeProgram.setUniformValue(m_projMatrixLoc, m_proj);
    planeProgram.setUniformValue(m_mvMatrixLoc, m_world);

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

    // for debugging
    drawDynamicsBegin(planeVertexs);
    addRect(
        QVector3D(-1, 0, -1),
        QVector3D(1, 0, -1),
        QVector3D(-1, 5, -1),
        DUMMY_COLOR
    );
    drawDynamics(planeVertexs, QVector4D(0,1,0,alpha));

    planeProgram.release();
}

void MidiOpenGLWidget::drawDynamicsBegin(vector<GLfloat> &buffer) {
    buffer.clear();
    outputPosVec = &buffer;
    outputColorVec = NULL;
}

void MidiOpenGLWidget::drawDynamics(vector<GLfloat> &buffer, const QVector4D &color, GLenum mode) {
    outputPosVec = &vertexPositions;
    outputColorVec = &vertexColors;
    size_t bufferSize = buffer.size();
    planeVBO.allocate(buffer.data(), bufferSize*sizeof(GLfloat));
    planeVBO.bind();
    planeProgram.setUniformValue("color", color);
    glDrawArrays(mode, 0, bufferSize/3);
}

void MidiOpenGLWidget::switchView() {
    is2dView = !is2dView;
    resizeGL(width(), height());
}

void MidiOpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

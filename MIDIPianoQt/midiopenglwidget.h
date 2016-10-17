#ifndef MIDIOPENGLWIDGET_H
#define MIDIOPENGLWIDGET_H

#include <tuple>

#include <QWidget>
#include <qopengl.h>
#include <qmath.h>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <QMatrix>
#include <QTimer>
#include <QTime>
#include <QOpenGLShaderProgram>

#include <QMessageBox>

#include "midifilereader.h"


class MidiOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    MidiOpenGLWidget(QWidget *parent = 0);

    void addTriangle(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3, const QVector4D& c);
    void addRect(const QVector3D &p, const QVector3D &pBottom, const QVector3D &pRight, const QVector4D &c);
    void addQuad(const QVector3D& p, const QVector3D& px, const QVector3D& py, const QVector3D& pz, const QVector4D& c);
    void addMidiNoteBar(unsigned long absTime, unsigned long lastToTime, int note, int channel);

    static QVector4D rgba(int r, int g, int b, GLfloat a=1.0) {
        r = rand()%256;
        g = rand()%256;
        b = rand()%256;
        return QVector4D(r/255.0, g/255.0, b/255.0, a);
    }

    void switchView();
    void loadMidiData(MidiData &midiData);

    double now;
protected:
    unsigned long totalTime;

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void drawDynamicsBegin(std::vector<GLfloat> &buffer);
    void drawDynamics(std::vector<GLfloat> &buffer, const QVector4D &color, GLenum mode = GL_TRIANGLES);

    QOpenGLShaderProgram program, planeProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer posVBO, colorVBO, planeVBO;
    int m_projMatrixLoc;
    int m_mvMatrixLoc;
    int planeProjMatrixLoc, planeMvMatrixLoc;

    QMatrix4x4 m_proj;
    QMatrix4x4 m_world;

    QTime fromStart;
    bool is2dView;
    std::vector<GLfloat> vertexPositions, vertexColors, *outputPosVec, *outputColorVec;
};

#endif // MIDIOPENGLWIDGET_H

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
#include <QVector3D>
#include <QVector2D>
#include <QMatrix>
#include <QTimer>
#include <QTime>
#include <QOpenGLShaderProgram>
#include <QPushButton>

#include <QMessageBox>

#include "midifilereader.h"

typedef std::tuple<GLfloat, GLfloat, GLfloat> MyVertex;
typedef std::tuple<GLfloat, GLfloat, GLfloat, GLfloat> MyColor;
MyVertex operator + (const MyVertex &a, const MyVertex &b);
MyVertex operator - (const MyVertex &a, const MyVertex &b);

class MidiOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    MidiOpenGLWidget(QWidget *parent = 0);

    void addTriangle(const MyVertex& p1, const MyVertex& p2, const MyVertex& p3, const MyColor& c);
    void addRect(const MyVertex &p, const MyVertex &pBottom, const MyVertex &pRight, const MyColor &c);
    void addQuad(const MyVertex& p, const MyVertex& px, const MyVertex& py, const MyVertex& pz, const MyColor& c);
    void addMidiNoteBar(unsigned long absTime, unsigned long lastToTime, int note, int channel);

    static MyColor rgba(int r, int g, int b, GLfloat a=1.0) {
        r = rand()%256;
        g = rand()%256;
        b = rand()%256;
        return MyColor(GLfloat(r/255.0), GLfloat(g/255.0), GLfloat(b/255.0), a);
    }

    void switchView();
    void loadMidiData(MidiData &midiData);


protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void calcPos();

    QOpenGLShaderProgram program, planeProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer posVBO, colorVBO, planeVBO;
    int m_projMatrixLoc;
    int m_mvMatrixLoc;
    int planeProjMatrixLoc, planeMvMatrixLoc;

    QMatrix4x4 m_proj;
    QMatrix4x4 m_world;

    double now;

    QTime fromStart;

    bool is2dView;
    std::vector<GLfloat> vertexPositions, vertexColors, *outputPosVec, *outputColorVec;
};

#endif // MIDIOPENGLWIDGET_H

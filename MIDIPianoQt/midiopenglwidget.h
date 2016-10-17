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

#include <QMessageBox>

class Logo
{
public:
    Logo();
    const GLfloat *constData() const { return m_data.constData(); }
    int count() const { return m_count; }
    int vertexCount() const { return m_count / 6; }

private:
    void quad(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3, GLfloat x4, GLfloat y4);
    void extrude(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    void add(const QVector3D &v, const QVector3D &n);

    QVector<GLfloat> m_data;
    int m_count;
};

typedef std::tuple<GLfloat, GLfloat, GLfloat> MyVertex;
typedef std::tuple<GLfloat, GLfloat, GLfloat, GLfloat> MyColor;
MyVertex operator + (const MyVertex &a, const MyVertex &b);
MyVertex operator - (const MyVertex &a, const MyVertex &b);

class MidiOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    MidiOpenGLWidget(QWidget *parent = 0);

    void addTriangle(const MyVertex& p1, const MyVertex& p2, const MyVertex& p3, const MyColor& c);
    void addQuad(const MyVertex& p, const MyVertex& px, const MyVertex& py, const MyVertex& pz, const MyColor& c);

    static MyColor rgba(int r, int g, int b, GLfloat a=1.0) {
        r = rand()%256;
        g = rand()%256;
        b = rand()%256;
        return MyColor(GLfloat(r/255.0), GLfloat(g/255.0), GLfloat(b/255.0), a);
    }

    void switchView();
    void drawPlane();

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
    std::vector<GLfloat> vertexPositions, vertexColors;
};

#endif // MIDIOPENGLWIDGET_H

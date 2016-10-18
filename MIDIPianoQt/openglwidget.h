#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QVector>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMessageBox>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    OpenGLWidget(QWidget *parent = 0):
        QOpenGLWidget(parent), outputPosVec(NULL), outputColorVec(NULL)
    {}

    static QVector4D rgba(int r, int g, int b, GLfloat a=1.0) {
        r = rand()%256;
        g = rand()%256;
        b = rand()%256;
        return QVector4D(r/255.0, g/255.0, b/255.0, a);
    }

protected:
    virtual void initializeGL();
    virtual void paintGL();

    void addTriangle(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3, QVector4D color = QVector4D());
    void addRect(const QVector3D &p, const QVector3D &pBottom, const QVector3D &pRight, QVector4D color = QVector4D());
    void addQuad(const QVector3D& p, const QVector3D& px, const QVector3D& py, const QVector3D& pz, QVector4D color = QVector4D());

    void addStaticVertexBegin() {
        outputPosVec = &staticVertexPositions;
        outputColorVec = &staticVertexColors;
    }

    void addStaticVertexEnd(GLenum mode = GL_TRIANGLES) {
        outputPosVec = NULL;
        outputColorVec = NULL;
        staticDrawMode = mode;
    }

    void drawDynamicsBegin(std::vector<GLfloat> &buffer);
    void drawDynamicsEnd(const QVector4D &color, GLenum mode = GL_TRIANGLES);

    QMatrix4x4 mProj;
    QMatrix4x4 mWorld;

private:
    QOpenGLShaderProgram program, planeProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer posVBO, colorVBO, planeVBO;

    GLenum staticDrawMode;

    int projMatrixLoc;
    int mvMatrixLoc;
    int planeProjMatrixLoc, planeMvMatrixLoc;

    std::vector<GLfloat> *outputPosVec, *outputColorVec;
    std::vector<GLfloat> staticVertexPositions, staticVertexColors;
};

#endif // OPENGLWIDGET_H

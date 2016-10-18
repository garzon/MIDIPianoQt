#include "openglwidget.h"

using namespace std;

#define EMPLACE_BACK_3(v, p) v.emplace_back(p.x()); v.emplace_back(p.y()); v.emplace_back(p.z())
#define EMPLACE_BACK_4(v, p) EMPLACE_BACK_3(v, p); v.emplace_back(p.w())
#define EMPLACE_COLOR(v, c) EMPLACE_BACK_4(v, c);EMPLACE_BACK_4(v, c);EMPLACE_BACK_4(v, c)

void OpenGLWidget::addRect(const QVector3D &p, const QVector3D &pBottom, const QVector3D &pRight, QVector4D color) {
    QVector3D p_ = pBottom + pRight - p;
    addTriangle(p, pBottom, p_, color);
    addTriangle(p, p_, pRight, color);
}

void OpenGLWidget::addTriangle(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3, QVector4D color) {
    if(outputPosVec) {
        EMPLACE_BACK_3((*outputPosVec), p1);
        EMPLACE_BACK_3((*outputPosVec), p2);
        EMPLACE_BACK_3((*outputPosVec), p3);
    } else {
        throw "OpenGLWidget - You should call Xbegin() functions first.";
    }
    if(outputColorVec) {
        EMPLACE_COLOR((*outputColorVec), color);
    }
}

void OpenGLWidget::addQuad(const QVector3D& p, const QVector3D& px, const QVector3D& py, const QVector3D& pz, QVector4D color) {
    QVector3D pxpy = px + py - p;
    QVector3D pypz = py + pz - p;
    QVector3D pxpz = px + pz - p;
    QVector3D p_ = pxpz + pypz - pz;

    // bottom
    // p -> px -> pxpy
    // p -> pxpy -> py
    addTriangle(p, px, pxpy, color);
    addTriangle(p, pxpy, py, color);

    // p -> pxpz -> px
    // p -> pz -> pxpz
    addTriangle(p, pxpz, px, color);
    addTriangle(p, pz, pxpz, color);

    // p -> py -> pypz
    // p -> pypz -> pz
    addTriangle(p, py, pypz, color);
    addTriangle(p, pypz, pz, color);

    // top
    // p' -> pz -> pxpz
    // p' -> pypz -> pz
    addTriangle(p_, pz, pxpz, color);
    addTriangle(p_, pypz, pz, color);

    // px -> p' -> pxpz
    // px -> pxpy -> p'
    addTriangle(px, p_, pxpz, color);
    addTriangle(px, pxpy, p_, color);

    // pxpy -> py -> p'
    // py -> pypz -> p'
    addTriangle(pxpy, py, p_, color);
    addTriangle(py, pypz, p_, color);
}

void OpenGLWidget::initializeGL() {
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

    projMatrixLoc = program.uniformLocation("projMatrix");
    mvMatrixLoc = program.uniformLocation("mvMatrix");

    m_vao.create();

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    posVBO.create();
    posVBO.bind();
    posVBO.allocate(staticVertexPositions.data(), sizeof(GLfloat)*staticVertexPositions.size());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    colorVBO.create();
    colorVBO.bind();
    colorVBO.allocate(staticVertexColors.data(), sizeof(GLfloat)*staticVertexColors.size());
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

    planeProgram.release();
}

void OpenGLWidget::paintGL() {
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    program.bind();
    program.setUniformValue(projMatrixLoc, mProj);
    program.setUniformValue(mvMatrixLoc, mWorld);
    glDrawArrays(staticDrawMode, 0, staticVertexPositions.size()/3);
    program.release();

    planeProgram.bind();
    planeProgram.setUniformValue(projMatrixLoc, mProj);
    planeProgram.setUniformValue(mvMatrixLoc, mWorld);
}

void OpenGLWidget::drawDynamicsBegin(vector<GLfloat> &buffer) {
    outputPosVec = &buffer;
    outputColorVec = NULL;
}

void OpenGLWidget::drawDynamicsEnd(const QVector4D &color, GLenum mode) {
    if(!outputPosVec) throw "OpenGLWidget::drawDynamicsEnd() - should call drawDynamicsBegin first.";
    vector<GLfloat> &buffer(*outputPosVec);
    size_t bufferSize = buffer.size();
    planeVBO.allocate(buffer.data(), bufferSize*sizeof(GLfloat));
    planeVBO.bind();
    planeProgram.setUniformValue("color", color);
    glDrawArrays(mode, 0, bufferSize/3);
    buffer.clear();
    outputPosVec = NULL;
    outputColorVec = NULL;
}

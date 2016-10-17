#include "midiopenglwidget.h"

#include <time.h>

using namespace std;

MidiOpenGLWidget::MidiOpenGLWidget(QWidget *parent):
    QOpenGLWidget(parent)
{
    vertexPositions = {
        /*-1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f,  -1.0f, 0.0f,*/
    };

    vertexColors = {
       /* 0.0f, 0.0f, (GLfloat)((time(0)%25)/25.0), 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f,
        0.0f,  1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f,
        0.0f,  1.0f, 0.0f, 0.0f,*/
    };

    MyVertex p(0, 1.5, 0);
    MyVertex px(0.5, 1, 0);
    MyVertex py(0.5, 2, 0);
    MyVertex pz(0, 1.5, 1);
    addQuad(p, px, py, pz, rgba(128, 255, 255));

    MyVertex p2(0, 0, 0);
    MyVertex px2(1, 0, 0);
    MyVertex py2(0, 1, 0);
    MyVertex pz2(0, 0, 1);
    addQuad(p2, px2, py2, pz2, rgba(128, 255, 255, 0.3));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);

    fromStart.start();
}

#define EMPLACE_BACK_3(v, p) v.emplace_back(get<0>(p)); v.emplace_back(get<1>(p)); v.emplace_back(get<2>(p))
#define EMPLACE_BACK_4(v, p) EMPLACE_BACK_3(v, p); v.emplace_back(get<3>(p))
#define EMPLACE_COLOR(v, c) EMPLACE_BACK_4(v, c);EMPLACE_BACK_4(v, c);EMPLACE_BACK_4(v, c)

// 1px/3s
#define nowY (now * 1.0/3.0)

MyVertex operator + (const MyVertex &a, const MyVertex &b) {
    return MyVertex(std::get<0>(a)+std::get<0>(b), std::get<1>(a)+std::get<1>(b), std::get<2>(a)+std::get<2>(b));
}
MyVertex operator - (const MyVertex &a, const MyVertex &b) {
    return MyVertex(std::get<0>(a)-std::get<0>(b), std::get<1>(a)-std::get<1>(b), std::get<2>(a)-std::get<2>(b));
}

void MidiOpenGLWidget::addTriangle(const MyVertex& p1, const MyVertex& p2, const MyVertex& p3, const MyColor& c) {
    EMPLACE_BACK_3(vertexPositions, p1);
    EMPLACE_BACK_3(vertexPositions, p2);
    EMPLACE_BACK_3(vertexPositions, p3);
    EMPLACE_COLOR(vertexColors, c);
}

void MidiOpenGLWidget::addQuad(const MyVertex& p, const MyVertex& px, const MyVertex& py, const MyVertex& pz, const MyColor& c) {
    MyVertex pxpy = px + py - p;
    MyVertex pypz = py + pz - p;
    MyVertex pxpz = px + pz - p;
    MyVertex p_ = pxpz + pypz - pz;

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

void MidiOpenGLWidget::calcPos() {
    now = (fromStart.elapsed() % 10000) *1.0 / 1000;
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

// - /////////////////////////////////////////

    if(!planeProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
          "attribute vec4 position; uniform mat4 projMatrix; uniform mat4 mvMatrix; void main() { gl_Position = projMatrix * mvMatrix * position; }")) {
        QMessageBox::warning(this, "QOpenGLShader::Vertex", "QOpenGLShader::Vertex" + program.log());
        close();
    }
    if(!planeProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
          "void main() { gl_FragColor = vec4(1.0,1.0,1.0,0.3); }")) {
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

void MidiOpenGLWidget::paintGL() {
    calcPos();

    m_proj.setToIdentity();
    m_world.setToIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(!is2dView) {
        m_proj.perspective(45.0f, GLfloat(width()) / height(), 0.01f, 100.0f);
        m_world.lookAt(
            QVector3D(4,nowY+3,3),
            QVector3D(0,nowY,0),
            QVector3D(0,0,1)
        );
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
    const GLfloat maxZ = 2;
    const GLfloat planeVertexs[] = {
        -1, nowY, -maxZ,
        -1, nowY, maxZ,
        1, nowY, maxZ,

        1, nowY, maxZ,
        1, nowY, -maxZ,
        -1, nowY, -maxZ,
    };
    planeVBO.allocate(planeVertexs, sizeof(planeVertexs));
    planeVBO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    planeProgram.release();
}

void MidiOpenGLWidget::switchView() {
    is2dView = !is2dView;
    resizeGL(width(), height());
}

void MidiOpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

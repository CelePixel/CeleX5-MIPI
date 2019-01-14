#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QDebug>
#include <QColorDialog>
#include "glut/glut.h"
#include <qevent.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <./include/celex4/celex4.h>
using namespace std;

//Material
struct MaterialStruct {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat shininess;
};
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    ~GLWidget();
    void setCeleX4(CeleX4* pCelexSensor);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent  *event);
public:

private:
    int eyeXleft, eyeXright, eyeYup, eyeYdown, eyeZin, eyeZout;
    GLdouble centerX, centerY, centerZ;
    int centerXleft, centerXright, centerYup, centerYdown;
    GLdouble onCenterX = 0, onCenterY = 0, onCenterZ = 0;
    GLdouble centerXdif, centerYdif, centerZdif;

    GLdouble boundingBoxThick = 0.01;

    GLdouble eyeR, eyeTheta, eyePhi, onEyeTheta, onEyePhi, theta, phi;

    GLdouble upX, upY,upZ, fovy, cameraDistance;
    GLdouble onFovy = 30.0, fovydif;

    bool phiState = true;

    float axisSize; //坐标轴的长度

    const double MY_MINDISTANCE;
    const double MY_MAXDISTANCE;
    const double CAMERA_DISTANCE; //相机和坐标系的距离

    GLdouble eyeX, eyeY, eyeZ;
    float cubeSize,cubeSize_;
    float axisThick,axisThick_;
    bool M_left, M_right, M_middle;
    bool M_wheel_up;
    bool M_wheel_down;

    int onMouseX, onMouseY;
    int MouseX, MouseY;
    float velocity;
    QPoint lastPos;
    GLfloat moveX,moveY;
    uint32_t eventTimeSlice;
    CeleX4*  m_pCelexSensor;


public:

private:
    void initFlag(void);
    void initParam(void);
    void initParamMove(void);
    void initialize();

    void drawText(int offset);
    void drawPointsAsCube();
    void drawBoundingBox();
    void drawString(string str, int w, int h, int x0, int y0);
    void drawSphere(double r, double x, double y, double z, MaterialStruct color);
    void drawCube(double a, double b, double c,
                  double x, double y, double z,
                  MaterialStruct color);
    void drawCube(double a, double b, double c,
                  double x, double y, double z,
                  MaterialStruct color, double theta,
                  double nx, double ny, double nz);
    void glPerspective( GLdouble fov, GLdouble aspectRatio, GLdouble zNear, GLdouble zFar);
signals:

public slots:
};

#endif // GLWIDGET_H

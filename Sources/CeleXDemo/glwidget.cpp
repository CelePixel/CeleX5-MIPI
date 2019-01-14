#include "glwidget.h"

MaterialStruct ms_jade = {
    {0.135,     0.2225,   0.1575,   1.0},
    {0.54,      0.89,     0.63,     1.0},
    {0.316228,  0.316228, 0.316228, 1.0},
    12.8 };

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    ,MY_MINDISTANCE(0.1)
    ,MY_MAXDISTANCE(100.0)
    ,CAMERA_DISTANCE(30.0)
    ,M_left(false)
    ,M_right(false)
    ,M_middle(false)
    ,M_wheel_up(false)
    ,M_wheel_down(false)
    ,phiState(true)
    ,cameraDistance(3.0)
    ,fovy(30.0)
    ,onFovy(30.0)
    ,axisSize(0.25)
    ,cubeSize(0.005)
    ,axisThick(0.01)
    ,upX(0.0)
    ,upY(-1.0)
    ,upZ(0.0)
    ,velocity(0.002)
    ,eventTimeSlice(60)
{
    axisThick_=axisThick;
    cubeSize_=cubeSize;
}

GLWidget::~GLWidget()
{

}

void GLWidget::setCeleX4(CeleX4 *pCelexSensor)
{
    m_pCelexSensor = pCelexSensor;
}

void GLWidget::initializeGL()
{
    //    qDebug()<<"init";
    initializeOpenGLFunctions();
    initFlag();
    initParam();
    initialize();
}

void GLWidget::paintGL()
{
    if (M_left && M_right) {
    }
    else if (M_left) {
        eyeTheta = (GLdouble)(moveX*velocity*2.0);
        eyePhi = (GLdouble)(moveY*velocity*2.0);
    }
    else if (M_middle) {
        centerXdif = (GLdouble)(moveX*velocity);
        centerYdif = (GLdouble)(moveY*velocity);
    }
    else if (M_right) {
        fovydif = (GLdouble)(moveY*velocity*20.0);
        //        centerXdif = (GLdouble)(moveX*velocity);
        //        centerYdif = (GLdouble)(moveY*velocity);
    }
    else {
        onCenterX = centerX;
        onCenterY = centerY;
        onCenterZ = centerZ;
        onEyeTheta = theta;
        onEyePhi = phi;
        onFovy = fovy;
        initParamMove();
        if (M_wheel_up == true) {
            centerZdif = (GLdouble)velocity * 50;
            M_wheel_up = false;
        }
        else if (M_wheel_down == true) {
            centerZdif = (-1.0) * (GLdouble)velocity * 50;
            M_wheel_down = false;
        }
    }

    fovy = onFovy + fovydif;

    theta = onEyeTheta + eyeTheta;
    phi = onEyePhi + eyePhi;

    if (phi >= M_PI*1.5) {
        if (phiState == false) {
            upY = -upY;
            phiState = true;
            onEyePhi = onEyePhi - M_PI*2.0;
            phi = onEyePhi + eyePhi;
        }
    }
    else if (phi >= M_PI*0.5) {
        if (phiState == true) {
            upY = -upY;
            phiState = false;
        }
    }
    else if (phi <= -M_PI*1.5) {
        if (phiState == false) {
            upY = -upY;
            phiState = true;
            onEyePhi = onEyePhi + M_PI*2.0;
            phi = onEyePhi + eyePhi;
        }
    }
    else if (phi <= -M_PI*0.5) {
        if (phiState == true) {
            upY = -upY;
            phiState = false;
        }
    }
    else if (phi > -M_PI*0.5 && phi < M_PI*0.5) {
        if (phiState == false) {
            upY = -upY;
            phiState = true;
        }
    }

    centerX = onCenterX - cos(theta)*centerXdif + sin(phi)*sin(theta)*centerYdif + cos(phi)*sin(theta)*centerZdif;
    centerY = onCenterY - cos(phi)*centerYdif + sin(phi)*centerZdif;
    centerZ = onCenterZ + sin(theta)*centerXdif + sin(phi)*cos(theta)*centerYdif + cos(phi)*cos(theta)*centerZdif;

    eyeX = centerX - eyeR * sin(theta) * cos(phi);
    eyeY = centerY - eyeR * sin(phi);
    eyeZ = centerZ - eyeR * cos(theta) * cos(phi);

    //视点
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, 1.0, MY_MINDISTANCE, MY_MAXDISTANCE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //照相机的坐标，原点，坐标系的方向
    gluLookAt(  eyeX, eyeY, eyeZ,
                centerX, centerY, centerZ,
                upX, upY, upZ);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // 坐标轴显示
    glColor3d(1.0, 0.0, 0.0);
    drawCube(axisSize, axisThick, axisThick, axisSize / 2., 0, 0, ms_jade, 45.0, 1.0, 0.0, 0.0);
    glColor3d(0.0, 1.0, 0.0);
    drawCube(axisThick, axisSize, axisThick, 0, axisSize / 2., 0, ms_jade, 45.0, 0.0, 1.0, 0.0);
    glColor3d(0.0, 0.0, 1.0);
    drawCube(axisThick, axisThick, axisSize, 0, 0, axisSize / 2., ms_jade, 45.0, 0.0, 0.0, 1.0);
    drawBoundingBox();
    drawPointsAsCube();
    update();
}

void GLWidget::resizeGL(int w, int h)
{
    if (h == 0)
    {
        h = 1;
    }
    //重置当前的视口（Viewport）
    glViewport(0, 0, (GLint)w, (GLint)h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //选择投影矩阵。
    glMatrixMode(GL_PROJECTION);

    //重置投影矩阵
    glLoadIdentity();

    //建立透视投影矩阵
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
    //gluPerspective(45.0,(GLfloat)width() / (GLfloat)height(),0.1,100.0);
    // 选择模型观察矩阵
    glMatrixMode(GL_MODELVIEW);

    // 重置模型观察矩阵。
    glLoadIdentity();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)
    {
        M_left = true;
        M_right = false;
        M_middle = false;
        M_wheel_up = false;
        M_wheel_down = false;
    }
    else if(event->buttons() == Qt::RightButton)
    {
        M_right = true;
        M_left = false;
        M_middle = false;
        M_wheel_up = false;
        M_wheel_down = false;
    }
    else if(event->buttons() == Qt::MidButton)
    {
        M_middle = true;
        M_left = false;
        M_right = false;
        M_wheel_up = false;
        M_wheel_down = false;
    }
    lastPos = event->globalPos();
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    //    qDebug()<<"move";
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //    qDebug()<<"release";
    moveX = GLfloat(event->globalX() - lastPos.x());
    moveY = GLfloat(event->globalY() - lastPos.y());
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0)
    {
        //        qDebug()<<"wheel";
        //        M_wheel_up = true;
        //        M_left = false;
        //        M_right = false;
        //        M_middle = false;
//        cubeSize += cubeSize_;
    }
    else
    {
        //        M_wheel_down = true;
        //        M_left = false;
        //        M_right = false;
        //        M_middle = false;
//        cubeSize -= cubeSize_;
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    if(event->key()== Qt::Key_R)
    {
        qDebug()<<"reset";
        M_left = false;
        M_right = false;
        M_middle = false;
        M_wheel_up = false;
        M_wheel_down = false;
        cubeSize=cubeSize_;
        initFlag();
        initParam();
        initParamMove();
    }
    else if(event->key()== Qt::Key_O)
    {
        if (axisThick == 0.0) {
            axisThick = axisThick_;
        }
        else {
            axisThick = 0.0;
        }
    }
    else if(event->key()== Qt::Key_Plus)
    {
        cubeSize += cubeSize_;
    }
    else if(event->key()== Qt::Key_Minus)
    {
        cubeSize -= cubeSize_;
    }
}

void GLWidget::initFlag(void)
{
    eyeXleft = 0;
    eyeXright = 0;
    eyeYup = 0;
    eyeYdown = 0;
    centerXleft = 0;
    centerXright = 0;
    centerYup = 0;
    centerYdown = 0;
    eyeZin = 0;
    eyeZout = 0;
}
void GLWidget::initParam(void)
{
    centerX = 0;
    centerY = 0;
    centerZ = 0.5;
    onCenterX = 0;
    onCenterY = 0;
    onCenterZ = 0;
    centerXdif = 0;
    centerYdif = 0;
    centerZdif = 0;
    onEyeTheta = 0;
    eyeTheta = 0;
    onEyePhi = 0;
    eyePhi = 0;
    theta = 45;	//
    phi = 0;
    eyeR = cameraDistance;
    fovy = 50.0;	//
    onFovy = 30.0;
    fovydif = 0;
    upX = 0.0;
    upY = -1.0;
    upZ = 0.0;
    phiState = true;
}

void GLWidget::initialize() {
    //设置光源--------------------------------------
    ///GLfloat lightPosition[4] = { 10.0, -10.0, -20.0, 0.0 }; //光源位置
    ///GLfloat lightDirection[3] = { 0.0, 0.0, 0.0}; //光源方向
    ///GLfloat lightDiffuse[3]  = { 1.2,  1.2, 1.2  }; //漫反射
    ///GLfloat lightAmbient[3]  = { 0.15, 0.15, 0.15 }; //环境光
    ///GLfloat lightSpecular[3] = { 1.0,   1.0, 1.0  }; //镜面
    GLfloat lightPosition[4] = { 0.0, -10.0, -5.0, 0.0 }; //光源位置
    GLfloat lightDirection[3] = { 0.0, 0.0, 0.0 }; //光源方向
    GLfloat lightDiffuse[3] = { 0.7,  0.7, 0.7 }; //漫反射
    GLfloat lightAmbient[3] = { 0.25, 0.25, 0.25 }; //环境光
    //GLfloat lightSpecular[3] = { 1.0,   1.0, 1.0  }; //镜面

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

void GLWidget::initParamMove(void) {
    centerXdif = 0;
    centerYdif = 0;
    centerZdif = 0;
    eyeTheta = 0;
    eyePhi = 0;
    fovydif = 0;
}

//draw points
void GLWidget::drawPointsAsCube()
{
    std::vector<EventData> v;	//vector to store the event data
    if (m_pCelexSensor->getEventDataVector(v))	//get the event data and show corrospodding buffer
    {
        for (int i = 0; i < v.size() - 1; ++i)
        {
            if(v[i].t<=eventTimeSlice&&v[i].t>eventTimeSlice-20)
            {
                ///位置
                GLdouble x = (768 - (GLdouble)v[i].col - 383) / 768;
                GLdouble y = (640 - (GLdouble)v[i].row - 321) / 640;
                GLdouble z = ((GLdouble)v[i].t-40) / (eventTimeSlice/6);

                ///颜色
                if (z == 0)
                {
                    glColor3d(1.0, 1.0, 1.0);
                }
                else if (z < 0.2)	//blue
                {
                    glColor3d(0.0, 0.0, 1.0);
                }
                else if (z < 0.5)
                {
                    glColor3d(0.0, 1.0, 1.0);
                }
                else if (z < 0.9)	//green
                {
                    glColor3d(0.0, 1.0, 0.0);
                }
                else if (z < 1.4)
                {
                    glColor3d(1.0, 1.0, 0.0);
                }
                else	//red
                {
                    glColor3d(1.0, 0.0, 0.0);
                }

                glPushName(i);
                drawCube(cubeSize, cubeSize, cubeSize, x, y, z, ms_jade, 0.0, 1.0, 1.0, 0.0);
                glPopName();
            }
        }
    }
    //    imshow("cvdemo", m_pCelexSensor->getEventPicMat(EventBinaryPic));
}

//draw bounding box
void GLWidget::drawBoundingBox()
{
    glColor3d(0.5, 0.5, 0.5);
    drawCube(1, boundingBoxThick, boundingBoxThick, 0, 1 / 2., 0, ms_jade, 45.0, 1.0, 0.0, 0.0);
    //glColor3d(0.0, 1.0, 0.0);
    drawCube(boundingBoxThick, 1, boundingBoxThick, -1 / 2., 0, 0, ms_jade, 45.0, 0.0, 1.0, 0.0);
    //glColor3d(0.0, 0.0, 1.0);
    drawCube(boundingBoxThick, boundingBoxThick, 2, -1 / 2., 1 / 2., 1, ms_jade, 45.0, 0.0, 0.0, 1.0);

    //glColor3d(1.0, 0.0, 0.0);
    drawCube(1, boundingBoxThick, boundingBoxThick, 0, -1 / 2., 0, ms_jade, 45.0, 1.0, 0.0, 0.0);
    //glColor3d(0.0, 1.0, 0.0);
    drawCube(boundingBoxThick, 1, boundingBoxThick, 1 / 2., 0, 0, ms_jade, 45.0, 0.0, 1.0, 0.0);
    //glColor3d(0.0, 0.0, 1.0);
    drawCube(boundingBoxThick, boundingBoxThick, 2, 1 / 2., -1 / 2., 1, ms_jade, 45.0, 0.0, 0.0, 1.0);

    //glColor3d(1.0, 0.0, 0.0);
    drawCube(1, boundingBoxThick, boundingBoxThick, 0, -1 / 2., 2, ms_jade, 45.0, 1.0, 0.0, 0.0);
    //glColor3d(0.0, 1.0, 0.0);
    drawCube(boundingBoxThick, 1, boundingBoxThick, 1 / 2., 0, 2, ms_jade, 45.0, 0.0, 1.0, 0.0);
    //glColor3d(0.0, 0.0, 1.0);
    drawCube(boundingBoxThick, boundingBoxThick, 2, -1 / 2., -1 / 2., 1, ms_jade, 45.0, 0.0, 0.0, 1.0);

    //glColor3d(1.0, 0.0, 0.0);
    drawCube(1, boundingBoxThick, boundingBoxThick, 0, 1 / 2., 2, ms_jade, 45.0, 1.0, 0.0, 0.0);
    //glColor3d(0.0, 1.0, 0.0);
    drawCube(boundingBoxThick, 1, boundingBoxThick, -1 / 2., 0, 2, ms_jade, 45.0, 0.0, 1.0, 0.0);
    //glColor3d(0.0, 0.0, 1.0);
    drawCube(boundingBoxThick, boundingBoxThick, 2, 1 / 2., 1 / 2., 1, ms_jade, 45.0, 0.0, 0.0, 1.0);
}

//立方体绘制
void GLWidget::drawCube(double a, double b, double c, double x, double y, double z, MaterialStruct color)
{
    GLdouble vertex[][3] = {
        { -a / 2.0, -b / 2.0, -c / 2.0 },
        { a / 2.0, -b / 2.0, -c / 2.0 },
        { a / 2.0,  b / 2.0, -c / 2.0 },
        { -a / 2.0,  b / 2.0, -c / 2.0 },
        { -a / 2.0, -b / 2.0,  c / 2.0 },
        { a / 2.0, -b / 2.0,  c / 2.0 },
        { a / 2.0,  b / 2.0,  c / 2.0 },
        { -a / 2.0,  b / 2.0,  c / 2.0 }
    };
    int face[][4] = {//面
                     { 3, 2, 1, 0 },
                     { 1, 2, 6, 5 },
                     { 4, 5, 6, 7 },
                     { 0, 4, 7, 3 },
                     { 0, 1, 5, 4 },
                     { 2, 3, 7, 6 }
                    };
    GLdouble normal[][3] = {//法向
                            { 0.0, 0.0, -1.0 },
                            { 1.0, 0.0, 0.0 },
                            { 0.0, 0.0, 1.0 },
                            { -1.0, 0.0, 0.0 },
                            { 0.0,-1.0, 0.0 },
                            { 0.0, 1.0, 0.0 }
                           };
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT, color.ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, color.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, color.specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, &color.shininess);
    glTranslated(x, y, z);//平移
    glBegin(GL_QUADS);
    for (int j = 0; j < 6; ++j) {
        glNormal3dv(normal[j]); //设置法向
        for (int i = 0; i < 4; ++i) {
            glVertex3dv(vertex[face[j][i]]);
        }
    }
    glEnd();
    glPopMatrix();
}


// 旋转的立方体
void GLWidget::drawCube(double a, double b, double c,
                        double x, double y, double z,
                        MaterialStruct color,
                        double theta,
                        double nx, double ny, double nz)
{
    double nn = sqrt(pow(nx, 2) + pow(ny, 2) + pow(nz, 2));
    if (nn > 0.0) {
        nx = nx / nn;
        ny = ny / nn;
        nz = nz / nn;
    }
    glPushMatrix();
    glTranslated(x, y, z);//平移
    glPushMatrix();
    if (theta != 0 && nn > 0.0)
        glRotated(theta, nx, ny, nz);
    drawCube(a, b, c, 0, 0, 0, color);
    glPopMatrix();
    glPopMatrix();
}

//文字绘制
void GLWidget::drawString(string str, int w, int h, int x0, int y0)
{
    glDisable(GL_LIGHTING);
    // 平行投影
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 绘制
    glRasterPos2f(x0, y0);
    int size = (int)str.size();
    for (int i = 0; i < size; ++i) {
        char ic = str[i];
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ic);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

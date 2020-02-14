#include "celex5widget.h"

#include <QTimer>
#include <QFileDialog>
#include <QCoreApplication>
#include <QPainter>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QDebug>
#include <QTextCodec>
#include <QThread>
#include <QScrollArea>
#include <QDesktopWidget>
#include "qcustomplot.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define READ_BIN_TIME            1
#define UPDATE_PLAY_INFO_TIME    30

bool               g_bRecordingData = false;
bool               g_bShowImageEndabled = true;
VideoStream*       m_pVideoStream = new VideoStream;
std::ofstream      g_ofWriteMat;
QString            g_qsPicFormat = "JPG";
QString            g_qsBinFileName;
QString            g_qsCurrentRecordPath;
int                g_iEventCountThreshold = 10;
int                g_iEventCountMean = 0;
bool               g_bStartGenerateOFFPN = false;
double             g_dFPNProgessValue = 0;
//
QLabel*            m_pArrowLeft;
QLabel*            m_pArrowRight;
QLabel*            m_pArrowUp;
QLabel*            m_pArrowDown;
int                m_iLastDirection = -1; //1:left, 2:right, 3:up, 4:down
int                m_iDirectionCount = 0;

CeleX5* m_pCeleX5 = new CeleX5;

QVector<double> g_vecEventCol(1280), g_vecEventCountPerCol(1280);
QVector<double> g_vecEventRow(800), g_vecEventCountPerRow(800);

SensorDataObserver::SensorDataObserver(CX5SensorDataServer *sensorData, QWidget *parent)
    : QWidget(parent)
    , m_imageMode1(CELEX5_COL, CELEX5_ROW, QImage::Format_RGB888)
    , m_imageMode2(CELEX5_COL, CELEX5_ROW, QImage::Format_RGB888)
    , m_imageMode3(CELEX5_COL, CELEX5_ROW, QImage::Format_RGB888)
    , m_imageMode4(CELEX5_COL, CELEX5_ROW, QImage::Format_RGB888)
    , m_imageEvent1(CELEX5_COL, CELEX5_ROW, QImage::Format_RGB888)
    , m_imageEvent2(CELEX5_COL, CELEX5_ROW, QImage::Format_RGB888)
    , m_imageForSavePic(CELEX5_COL, CELEX5_ROW, QImage::Format_RGB888)
    , m_bLoopModeEnabled(false)
    , m_iPicMode(0)
    , m_iLoopPicMode(0)
    , m_bShowMultipleWindows(false)
    , m_uiTemperature(0)
    , m_iFPS(30)
    , m_iPlaybackFPS(1000)
    , m_uiFullFrameFPS(100)
    , m_uiRealFullFrameFPS(0)
    , m_bSavePics(false)
    , m_bRecordingImages(false)
    , m_lFullFrameCount(0)
    , m_lEventFrameCount(0)
    , m_lOpticalFrameCount(0)
    , m_emDisplayType(Realtime)
    , m_iIntervalCounter(0)
    , m_iPicIntervalCount(0)
{
    m_pSensorData = sensorData;
    m_pSensorData->registerData(this, CeleX5DataManager::CeleX_Frame_Data);

    m_imageMode1.fill(Qt::black);
    m_imageMode2.fill(Qt::black);
    m_imageMode3.fill(Qt::black);
    m_imageMode4.fill(Qt::black);

    m_pEventBuffer = new uint8_t[CELEX5_PIXELS_NUMBER];
    memset(m_pEventBuffer, 0, CELEX5_PIXELS_NUMBER);

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateImage()));
    m_pUpdateTimer->start(33);

    for (int i = 0; i < 4; i++)
        m_pBuffer[i] = new uchar[CELEX5_PIXELS_NUMBER];
}

SensorDataObserver::~SensorDataObserver()
{
    m_pSensorData->unregisterData(this, CeleX5DataManager::CeleX_Frame_Data);
    for (int i = 0; i < 4; i++)
    {
        delete[] m_pBuffer[i];
        m_pBuffer[i] = NULL;
    }
}

//This function is only for playback
bool bAdjust = true;
int frameNum = 0;
int totalEventCount = 0;
#define FRAME_NUM    15
void SensorDataObserver::onFrameDataUpdated(CeleX5ProcessedData* pSensorData)
{
    if (frameNum < FRAME_NUM)
    {
        cout << m_pCeleX5->getEventRatePerFrame() << endl;
        if (frameNum >= 5)
            totalEventCount += m_pCeleX5->getEventRatePerFrame();
        if (frameNum == FRAME_NUM-1)
        {
            int averNum = totalEventCount / (FRAME_NUM-5);
            cout << "average event count = " << averNum << endl;
            if (averNum > 100000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(250);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
            else if (averNum > 80000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(240);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
            else if (averNum > 70000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(230);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
            else if (averNum > 60000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(220);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
            else if (averNum > 50000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(210);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
            else if (averNum > 40000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(200);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
            else if (averNum > 30000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(191);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
            else if (averNum > 20000)
            {
                if (bAdjust)
                {
                    m_pCeleX5->setThreshold(181);
                    cout << "Threshold = " << m_pCeleX5->getThreshold() << endl;
                    bAdjust = false;
                }
            }
        }
        frameNum++;
    }

    if (g_bStartGenerateOFFPN)
    {
        g_dFPNProgessValue = double(pSensorData->getFPNProgress())*100/CELEX5_PIXELS_NUMBER;
        //cout << g_dFPNProgessValue << endl;
        if (pSensorData->getFPNProgress() >= CELEX5_PIXELS_NUMBER-10)
        {
            m_pCeleX5->stopGenerateFPN();
            g_bStartGenerateOFFPN = false;
            pSensorData->updateFPNProgress(0);
        }
    }
    //cout << __FUNCTION__ << endl;
    if (m_emDisplayType == Realtime)
    {
        if (!m_pCeleX5->isLoopModeEnabled())
            return;
    }
    //cout << (int)pSensorData->getSensorMode() << ", loop = " << pSensorData->getLoopNum() << endl;
    m_uiRealFullFrameFPS = pSensorData->getFullFrameFPS();
    m_uiTemperature = pSensorData->getTemperature();

    processSensorBuffer(pSensorData->getSensorMode(), pSensorData->getLoopNum());

    if (m_emDisplayType == ConvertBin2Video)
    {
        char jpegfile[32]="temp.jpg";
        m_imageMode1.save("temp.jpg", "JPG");
        m_pVideoStream->avWtiter(jpegfile);
    }
    else if (m_emDisplayType == ConvertBin2CSV)
    {
        writeCSVData(pSensorData->getSensorMode());
    }

    if (m_bSavePics)
    {
        savePics(pSensorData);
    }

    if (m_emDisplayType != Realtime)
    {
        if (1000 / m_iPlaybackFPS > 1)
        {
#ifdef _WIN32
            Sleep(1000 / m_iPlaybackFPS);
#else
            usleep(1000000 / m_iPlaybackFPS);
#endif
        }
    }
}

void SensorDataObserver::setLoopModeEnabled(bool enable)
{
    m_bLoopModeEnabled = enable;
}

void SensorDataObserver::setPictureMode(int picMode)
{
    if(m_pCeleX5->isLoopModeEnabled())
        m_iLoopPicMode = picMode;
    else
        m_iPicMode = picMode;
}

void SensorDataObserver::setDisplayFPS(int count)
{
    if (m_emDisplayType == Realtime)
    {
        m_iFPS = count;
        m_pUpdateTimer->start(1000/m_iFPS);
    }
    else
    {
        m_iPlaybackFPS = count;
    }
}

void SensorDataObserver::setFullFrameFPS(uint16_t value)
{
    m_uiFullFrameFPS = value;
}

void SensorDataObserver::setMultipleShowEnabled(bool enable)
{
    m_bShowMultipleWindows = enable;
}

bool SensorDataObserver::getMultipleShowEnable()
{
    return m_bShowMultipleWindows;
}

void SensorDataObserver::setDisplayType(DisplayType type)
{
    m_lFullFrameCount = 0;
    m_lEventFrameCount = 0;
    m_lOpticalFrameCount = 0;
    m_emDisplayType = type;
    if (Realtime == type)
    {
        m_pUpdateTimer->start(1000/m_iFPS);
    }
    else
    {
        m_pUpdateTimer->stop();
    }
}

void SensorDataObserver::setSaveBmp(bool save)
{
    m_bSavePics = save;
}

bool SensorDataObserver::isSavingBmp()
{
    return m_bSavePics;
}

void SensorDataObserver::setBinFileName(QString filePath)
{
    QStringList nameList = filePath.split("/");

    QString fileName = nameList[nameList.size()-1];

    if (fileName.size() > 15)
        m_qsBinFileName = fileName.mid(9, 15);
    else
        m_qsBinFileName = fileName.left(filePath.size() -4);
}

void SensorDataObserver::setPicIntervalCount(int count)
{
    m_iPicIntervalCount = count;
}

void SensorDataObserver::setRecordingImgState(bool state)
{
    m_bRecordingImages = state;
    m_lFullFrameCount = 0;
    m_lEventFrameCount = 0;
}

void SensorDataObserver::setRecordingTime(QString time)
{
    m_qsRecordingTime = time;
}

void SensorDataObserver::updateQImageBuffer(unsigned char *pBuffer1, int loopNum, int colorMode)
{
#ifdef _LOG_TIME_CONSUMING_
    struct timeval tv_begin, tv_end;
    gettimeofday(&tv_begin, NULL);
#endif    
    if (NULL == pBuffer1)
    {
        return;
    }
    if (-1 == loopNum)
        loopNum = 1;
    //cout << "loopNum = " << loopNum << endl;
    uchar* pp1 = NULL;
    if (loopNum == 1)
        pp1 = m_imageMode1.bits();
    else if (loopNum == 2)
        pp1 = m_imageMode2.bits();
    else if (loopNum == 3)
        pp1 = m_imageMode3.bits();
    else if (loopNum == 4)
        pp1 = m_imageMode4.bits();
    else
        pp1 = m_imageMode1.bits();

    int threshold = 2;
    if (5 == colorMode) //event count density
    {
        if (-g_iEventCountThreshold > 0)
            threshold = (g_iEventCountThreshold+100)*g_iEventCountMean/100;
        //cout << "threshold = " << threshold << endl;
    }

    //cout << "type = " << (int)m_pCeleX5->getFixedSenserMode() << endl;
    int value = 0;
    for (int i = 0; i < CELEX5_ROW; ++i)
    {
        for (int j = 0; j < CELEX5_COL; ++j)
        {
            value = pBuffer1[i*CELEX5_COL+j];
            if (1 == colorMode) //optical-flow frame
            {
                if (0 == value)
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
                else if (value < 50) //blue
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 255;
                }
                else if (value < 100)
                {
                    *pp1 = 0;
                    *(pp1+1) = 255;
                    *(pp1+2) = 255;
                }
                else if (value < 150) //green
                {
                    *pp1 = 0;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else if (value < 200) //yellow
                {
                    *pp1 = 255;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else //red
                {
                    *pp1 = 255;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
            }
            else if (2 == colorMode) //optical-flow speed
            {
                if (0 == value)
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
                else if (value < 20) //red
                {
                    *pp1 = 255;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
                else if (value < 40) //yellow
                {
                    *pp1 = 255;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else if (value < 60) //green
                {
                    *pp1 = 0;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else if (value < 80) //green blue
                {
                    *pp1 = 0;
                    *(pp1+1) = 255;
                    *(pp1+2) = 255;
                }
                else //blue
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 255;
                }
            }
            else if (3 == colorMode) //optical-flow direction
            {
                if (0 == value)
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
                //else if (value < 21 || value > 210) //30 300 red
                else if (value <= 32 || value > 223) //30 300 red
                {
                    *pp1 = 255;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
                else if (value > 32 && value <= 96) //45 135 blue
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 255;
                }
                else if (value > 96 && value <= 159) //135 225 green
                {
                    *pp1 = 0;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else if (value > 159 && value <= 223) //225 315 yellow
                {
                    *pp1 = 255;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
            }
            else if (4 == colorMode) //event superimposed pic
            {
                if (255 == value)
                {
                    *pp1 = 0;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else
                {
                    *pp1 = value;
                    *(pp1+1) = value;
                    *(pp1+2) = value;
                }
            }
            else if (5 == colorMode) //event count density
            {
                if (value < threshold)
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
                else if (value < 32) //blue
                {
                    *pp1 = 0;
                    *(pp1+1) = 0;
                    *(pp1+2) = 255;
                }
                else if (value < 64) //green
                {
                    *pp1 = 0;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else if (value < 96) //yellow
                {
                    *pp1 = 255;
                    *(pp1+1) = 255;
                    *(pp1+2) = 0;
                }
                else //red
                {
                    *pp1 = 255;
                    *(pp1+1) = 0;
                    *(pp1+2) = 0;
                }
            }
            else
            {
                *pp1 = value;
                *(pp1+1) = value;
                *(pp1+2) = value;
            }
            pp1+= 3;
        }
    }

#ifdef _LOG_TIME_CONSUMING_
    gettimeofday(&tv_end, NULL);
    //    cout << "updateImage time = " << tv_end.tv_usec - tv_begin.tv_usec << endl;
#endif
    update();
}

void SensorDataObserver::updateOPDirection(unsigned char *pBuffer1)
{
    if (nullptr == pBuffer1)
    {
        return;
    }
    int value = 0;
    uint32_t dir_count1 = 0;
    uint32_t dir_count2 = 0;
    uint32_t dir_count3 = 0;
    uint32_t dir_count4 = 0;
    int direction = 0;
    for (int i = 0; i < CELEX5_ROW; ++i)
    {
        for (int j = 0; j < CELEX5_COL; ++j)
        {
            value = pBuffer1[i*CELEX5_COL+j];
            if (0 == value)
            {
                ;
            }
            //else if (value < 21 || value > 210) //30 300 red
            else if (value <= 32 || value > 223) //30 300 red
            {
                dir_count1++;
            }
            else if (value > 32 && value <= 96) //45 135 blue
            {
                dir_count2++;
            }
            else if (value > 96 && value <= 159) //135 225 green
            {
                dir_count3++;
            }
            else if (value > 159 && value <= 223) //225 315 yellow
            {
                dir_count4++;
            }
            else
            {
                ;
            }
        }
    }
    //cout << dir_count1 << ", " << dir_count2 << ", " << dir_count3 << ", " << dir_count4 << endl;
    //cout << dir_value_total / (dir_count1+dir_count2+dir_count3+dir_count4+1) << endl;
    int dir1 = 0;
    int dir2 = 0;
    int max1 = 0;
    int max2 = 0;
    if (dir_count1 > dir_count2) //right
    {
        dir1 = 2;
        max1 = dir_count1;
    }
    else //down
    {
        dir1 = 4;
        max1 = dir_count2;
    }
    //
    if (dir_count3 > dir_count4) //left
    {
        dir2 = 1;
        max2 = dir_count3;
    }
    else //up
    {
        dir2 = 3;
        max2 = dir_count4;
    }
    //
    if (max1 > max2)
    {
        if (max1 > 5000)
        {
            direction = dir1;
            //cout << max1 << endl;
        }
        else
        {
            m_iLastDirection = 0;
            direction = 0;
            m_iDirectionCount = 0;
        }
    }
    else
    {
        if (max2 > 5000)
        {
            direction = dir2;
            //cout << max2 << endl;
        }
        else
        {
            m_iLastDirection = 0;
            direction = 0;
            m_iDirectionCount = 0;
        }
    }

    if (direction > 0)
    {
        if (direction != m_iLastDirection)
        {
            //cout << "direction = " << direction << ", m_iLastDirection = " << m_iLastDirection << endl;
            if (m_iLastDirection == 0)
            {
                if (direction == 1) //left
                {
                    m_pArrowLeft->show();
                    m_pArrowRight->hide();
                    m_pArrowUp->hide();
                    m_pArrowDown->hide();
                }
                else if (direction == 2) //right
                {
                    m_pArrowLeft->hide();
                    m_pArrowRight->show();
                    m_pArrowUp->hide();
                    m_pArrowDown->hide();
                }
                else if (direction == 3) //up
                {
                    m_pArrowLeft->hide();
                    m_pArrowRight->hide();
                    m_pArrowUp->show();
                    m_pArrowDown->hide();
                }
                else if (direction == 4) //down
                {
                    m_pArrowLeft->hide();
                    m_pArrowRight->hide();
                    m_pArrowUp->hide();
                    m_pArrowDown->show();
                }
                //cout << "show direction = " << direction << endl;
            }
            //if (m_iDirectionCount <= 1)
            {
                m_iLastDirection = direction;
                m_iDirectionCount = 1;
            }
        }
        else
        {
            m_iDirectionCount++;
            //cout << "------ direction = " << direction << endl;
            //cout << "------ m_iDirectionCount = " << m_iDirectionCount << endl;
        }
        if (m_iDirectionCount > 1)
        {
            if (direction == 1) //left
            {
                m_pArrowLeft->show();
                m_pArrowRight->hide();
                m_pArrowUp->hide();
                m_pArrowDown->hide();
            }
            else if (direction == 2) //right
            {
                m_pArrowLeft->hide();
                m_pArrowRight->show();
                m_pArrowUp->hide();
                m_pArrowDown->hide();
            }
            else if (direction == 3) //up
            {
                m_pArrowLeft->hide();
                m_pArrowRight->hide();
                m_pArrowUp->show();
                m_pArrowDown->hide();
            }
            else if (direction == 4) //down
            {
                m_pArrowLeft->hide();
                m_pArrowRight->hide();
                m_pArrowUp->hide();
                m_pArrowDown->show();
            }
            //cout << "------ show direction = " << direction << endl;
        }
    }
    update();
}

void SensorDataObserver::updateEventImage(CeleX5::EventPicType type)
{
    m_pCeleX5->getEventPicBuffer(m_pEventBuffer, type);
    if (NULL == m_pEventBuffer)
    {
        return;
    }
    uchar* pImage = pImage = m_imageForSavePic.bits();

    if (type == CeleX5::EventSuperimposedPic)
    {
        int value = 0;
        for (int i = 0; i < CELEX5_ROW; ++i)
        {
            for (int j = 0; j < CELEX5_COL; ++j)
            {
                value = m_pEventBuffer[i*CELEX5_COL+j];
                if (255 == value)
                {
                    *pImage = 0;
                    *(pImage+1) = 255;
                    *(pImage+2) = 0;
                }
                else
                {
                    *pImage = value;
                    *(pImage+1) = value;
                    *(pImage+2) = value;
                }
                pImage += 3;
            }
        }
    }
    else
    {
        int value = 0;
        for (int i = 0; i < CELEX5_ROW; ++i)
        {
            for (int j = 0; j < CELEX5_COL; ++j)
            {
                value = m_pEventBuffer[i*CELEX5_COL+j];
                *pImage = value;
                *(pImage+1) = value;
                *(pImage+2) = value;
                pImage += 3;
            }
        }
    }
}

void SensorDataObserver::processSensorBuffer(CeleX5::CeleX5Mode mode, int loopNum)
{
    int colorMode = 0;
    if (mode == CeleX5::Event_Off_Pixel_Timestamp_Mode)
    {
        if (m_bShowMultipleWindows)
        {
            m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventDenoisedBinaryPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[2], CeleX5::EventCountPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[3], CeleX5::EventDenoisedCountPic);
            updateQImageBuffer(m_pBuffer[0], 1, 0);
            updateQImageBuffer(m_pBuffer[1], 2, 0);
            updateQImageBuffer(m_pBuffer[2], 3, 0);
            updateQImageBuffer(m_pBuffer[3], 4, 0);
        }
        else
        {
            if(loopNum > 0)
            {
                if (0 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
                else if (1 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventDenoisedBinaryPic);
                else if (2 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountPic);
                else if (3 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventDenoisedCountPic);
            }
            else
            {
                if (0 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
                else if (1 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventDenoisedBinaryPic);
                else if (2 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountPic);
                else if (3 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventDenoisedCountPic);
                else if (4 == m_iPicMode)
                {
                    colorMode = 5;
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountDensityPic);
                }
            }
            updateQImageBuffer(m_pBuffer[0], loopNum, colorMode);
        }
        if (m_bRecordingImages)
        {
            saveRecordingImage(m_pBuffer[0], 2);
        }
    }
    else if (mode == CeleX5::Event_In_Pixel_Timestamp_Mode)
    {
        if (m_bShowMultipleWindows)
        {
            m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventBinaryPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[2], CeleX5::EventCountPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[3], CeleX5::EventDenoisedBinaryPic);
            updateQImageBuffer(m_pBuffer[0], 1, 1);
            updateQImageBuffer(m_pBuffer[1], 2, 0);
            updateQImageBuffer(m_pBuffer[2], 3, 0);
            updateQImageBuffer(m_pBuffer[3], 4, 0);
        }
        else
        {
            if(loopNum > 0)
            {
                if (0 == m_iLoopPicMode)
                {
                    colorMode = 1;
                    m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowPic);
                }
                else if (1 == m_iLoopPicMode)
                {
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
                }
            }
            else
            {
                if (0 == m_iPicMode)
                {
                    colorMode = 1;
                    m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowPic);

                    m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[1], CeleX5::OpticalFlowDirectionPic);
                    updateOPDirection(m_pBuffer[1]);
                }
                else if (1 == m_iPicMode)
                {
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
                }
                else if (2 == m_iPicMode)
                {
                    colorMode = 5;
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountDensityPic);
                }
                else if (3 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventDenoisedBinaryPic);
                else if (4 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountPic);
                else if (5 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventDenoisedCountPic);
            }
            updateQImageBuffer(m_pBuffer[0], loopNum, colorMode);
        }
    }
    else if (mode == CeleX5::Event_Intensity_Mode)
    {
        if (m_bShowMultipleWindows)
        {
            m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventGrayPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[2], CeleX5::EventAccumulatedPic);
            m_pCeleX5->getEventPicBuffer(m_pBuffer[3], CeleX5::EventSuperimposedPic);
            updateQImageBuffer(m_pBuffer[0], 1, 0);
            updateQImageBuffer(m_pBuffer[1], 2, 0);
            updateQImageBuffer(m_pBuffer[2], 3, 0);
            updateQImageBuffer(m_pBuffer[3], 4, 4);
        }
        else
        {
            if(loopNum > 0)
            {
                if (0 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
                else if (1 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventGrayPic);
                else if (2 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventAccumulatedPic);
                else if (3 == m_iLoopPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountPic);
            }
            else
            {
                if (0 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventBinaryPic);
                else if (1 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventGrayPic);
                else if (2 == m_iPicMode)
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventAccumulatedPic);
                else if (3 == m_iPicMode)
                {
                    colorMode = 4;
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventSuperimposedPic);
                }
                else if (4 == m_iPicMode)
                {
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountPic);
                }
                else if (5 == m_iPicMode)
                {
                    colorMode = 5;
                    m_pCeleX5->getEventPicBuffer(m_pBuffer[0], CeleX5::EventCountDensityPic);
                }
            }
            updateQImageBuffer(m_pBuffer[0], loopNum, colorMode);
        }
    }
    else if (mode == CeleX5::Full_Picture_Mode)
    {
        m_pCeleX5->getFullPicBuffer(m_pBuffer[0]);
        updateQImageBuffer(m_pBuffer[0], loopNum, 0);
        if (m_bRecordingImages)
        {
            saveRecordingImage(m_pBuffer[0], 1);
        }
    }
    else
    {
        if (m_bShowMultipleWindows)
        {
            m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowPic);
            m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[1], CeleX5::OpticalFlowSpeedPic);
            m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[2], CeleX5::OpticalFlowDirectionPic);
            updateQImageBuffer(m_pBuffer[0], 1, 1);
            updateQImageBuffer(m_pBuffer[1], 2, 2);
            updateQImageBuffer(m_pBuffer[2], 3, 3);
        }
        else
        {
            if (loopNum > 0)
            {
                m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowPic);
                int colorMode = 1;
                updateQImageBuffer(m_pBuffer[0], loopNum, colorMode);
            }
            else
            {
                if (0 == m_iPicMode)
                    m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowPic);
                else if (1 == m_iPicMode)
                    m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowSpeedPic);
                else if (2 == m_iPicMode)
                    m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[0], CeleX5::OpticalFlowDirectionPic);
                int colorMode = m_iPicMode+1;
                updateQImageBuffer(m_pBuffer[0], loopNum, colorMode);
            }
        }
    }
}

void SensorDataObserver::saveRecordingImage(unsigned char *pBuffer, int index)
{
    if (NULL == pBuffer)
    {
        return;
    }
    uchar* pp = NULL;
    if (1 == index)
        pp = m_imageEvent1.bits();
    else if (2 == index)
        pp = m_imageEvent2.bits();

    int value = 0;
    for (int i = 0; i < CELEX5_ROW; ++i)
    {
        for (int j = 0; j < CELEX5_COL; ++j)
        {
            value = pBuffer[i*CELEX5_COL+j];
            *pp = value;
            pp+= 1;
        }
    }
    if (m_lFullFrameCount > 999999 || m_lEventFrameCount > 999999)
    {
        m_lFullFrameCount = 0;
        m_lEventFrameCount = 0;

        const QDateTime now = QDateTime::currentDateTime();
        m_qsRecordingTime = now.toString(QLatin1String("yyyyMMdd_hhmmss"));
    }
    if (index == 1)
    {
        //-------- Full_Picture_Mode --------
        QString qsNum1 = QString("%1").arg(m_lFullFrameCount, 6, 10, QChar('0'));
        QString name1 = QCoreApplication::applicationDirPath() + "/image_fullpic/" + m_qsRecordingTime + "_" + qsNum1 + ".jpg";
        char nn1[256] = {0};
        memcpy(nn1, name1.toStdString().c_str(), name1.size());
        m_imageEvent1.save(nn1, "JPG");
        m_lFullFrameCount++;
    }
    else
    {
        //-------- Event_Off_Pixel_Timestamp_Mode --------
        QString qsNum2 = QString("%1").arg(m_lEventFrameCount, 6, 10, QChar('0'));
        QString name2 = QCoreApplication::applicationDirPath() + "/image_event_count/" + m_qsRecordingTime + "_" + qsNum2 + ".jpg";
        char file_path2[256] = {0};
        memcpy(file_path2, name2.toStdString().c_str(), name2.size());
        m_imageEvent2.save(file_path2, "JPG");
        m_lEventFrameCount++;
    }
}

void SensorDataObserver::savePics(CeleX5ProcessedData *pSensorData)
{
    QString qsFormat;
    if (g_qsPicFormat == "JPG")
        qsFormat = ".jpg";
    else if (g_qsPicFormat == "BMP")
        qsFormat = ".bmp";

    QDir dir;
    dir.cd(QCoreApplication::applicationDirPath());
    g_qsCurrentRecordPath = dir.path();
    //qDebug() << "Path = " << g_qsCurrentRecordPath;
    if (!dir.exists("data"))
    {
        dir.mkdir("data");
    }
    dir.cd("data");
    g_qsCurrentRecordPath += "/data/";
    QString folderName = g_qsBinFileName.left(g_qsBinFileName.size() - 4);
    QStringList qsList = folderName.split("/");
    folderName = qsList[qsList.size()-1];
    if (!dir.exists(folderName))
    {
        dir.mkdir(folderName);
    }
    dir.cd(folderName);
    g_qsCurrentRecordPath += folderName;
    //qDebug() << g_qsCurrentRecordPath << endl;

    if (pSensorData->getSensorMode() == CeleX5::Full_Picture_Mode)
    {
        m_iIntervalCounter++;
        if ((m_iIntervalCounter-1) % (m_iPicIntervalCount+1) != 0)
            return;

        if (!dir.exists("image_fullpic"))
        {
            dir.mkdir("image_fullpic");
        }
        QString qsNum = QString("%1").arg(m_lFullFrameCount, 6, 10, QChar('0'));
        QString picName = g_qsCurrentRecordPath + "/image_fullpic/" + m_qsBinFileName + "_" + qsNum + qsFormat;
        char file_path[256] = {0};
        memcpy(file_path, picName.toStdString().c_str(), picName.size());

        m_imageMode1.save(file_path, g_qsPicFormat.toStdString().data());
        m_lFullFrameCount++;
    }
    else if (pSensorData->getSensorMode() == CeleX5::Event_Off_Pixel_Timestamp_Mode ||
             pSensorData->getSensorMode() == CeleX5::Event_Intensity_Mode ||
             pSensorData->getSensorMode() == CeleX5::Event_In_Pixel_Timestamp_Mode)
    {
        m_iIntervalCounter++;
        if ((m_iIntervalCounter-1) % (m_iPicIntervalCount+1) != 0)
            return;

        QString qsNum = QString("%1").arg(m_lEventFrameCount, 6, 10, QChar('0'));
        QStringList folderNameList;
        int picTypeList[5] = {0};
        if (pSensorData->getSensorMode() == CeleX5::Event_Off_Pixel_Timestamp_Mode)
        {
            folderNameList << "image_event_binary" << "image_event_count" << "image_denoised_event_binary" << "image_denoised_event_count";

            picTypeList[0] = 0; //CeleX5::EventBinaryPic
            picTypeList[1] = 3; //CeleX5::EventCountPic
            picTypeList[2] = 4; //CeleX5::EventDenoisedBinaryPic
            picTypeList[3] = 6; //CeleX5::EventDenoisedCountPic
        }
        else if (pSensorData->getSensorMode() == CeleX5::Event_Intensity_Mode)
        {
            folderNameList << "image_event_binary" << "image_event_count" << "image_event_gray"
                           << "image_event_accumulated" << "image_event_superimposed";

            picTypeList[0] = 0; //CeleX5::EventBinaryPic
            picTypeList[1] = 3; //CeleX5::EventCountPic
            picTypeList[2] = 2; //CeleX5::EventGrayPic
            picTypeList[3] = 1; //CeleX5::EventAccumulatedPic
            picTypeList[4] = 5; //CeleX5::EventSuperimposedPic
        }
        else if (pSensorData->getSensorMode() == CeleX5::Event_In_Pixel_Timestamp_Mode)
        {
            folderNameList << "EventBinaryPic" << "EventInPixelT";

            picTypeList[0] = 0; //CeleX5::EventBinaryPic
            picTypeList[1] = 8; //CeleX5::EventCountDensityPic
        }
        for (int i = 0; i < folderNameList.size(); i++)
        {
            if (!dir.exists(folderNameList[i]))
            {
                dir.mkdir(folderNameList[i]);
            }
            QString picName = g_qsCurrentRecordPath + "/" + folderNameList[i] + "/" + m_qsBinFileName + "_" + qsNum + qsFormat;
            char file_path[256] = {0};
            memcpy(file_path, picName.toStdString().c_str(), picName.size());

            updateEventImage((CeleX5::EventPicType)picTypeList[i]);
            m_imageForSavePic.save(file_path, g_qsPicFormat.toStdString().data());
        }
        m_lEventFrameCount++;
    }
    else if (pSensorData->getSensorMode() == CeleX5::Optical_Flow_Mode ||
             pSensorData->getSensorMode() == CeleX5::Multi_Read_Optical_Flow_Mode)
    {
        m_iIntervalCounter++;
        if ((m_iIntervalCounter-1) % (m_iPicIntervalCount+1) != 0)
            return;

        if (!dir.exists("image_optical"))
        {
            dir.mkdir("image_optical");
        }

        QString qsNum = QString("%1").arg(m_lOpticalFrameCount, 6, 10, QChar('0'));

        QString picName = g_qsCurrentRecordPath + "/image_optical/" + m_qsBinFileName + "_" + qsNum + qsFormat;
        char file_path[256] = {0};
        memcpy(file_path, picName.toStdString().c_str(), picName.size());

        m_imageMode1.save(file_path, g_qsPicFormat.toStdString().data());
        m_lOpticalFrameCount++;
    }
}

void SensorDataObserver::writeCSVData(CeleX5::CeleX5Mode sensorMode)
{    
    std::vector<EventData> vecEvent;
    uint32_t frameNo = 0;
    m_pCeleX5->getEventDataVector(vecEvent, frameNo);
    size_t dataSize = vecEvent.size();
    if (sensorMode == CeleX5::Event_Off_Pixel_Timestamp_Mode)
    {
        for (int i = 0; i < dataSize; i++)
        {
            g_ofWriteMat << vecEvent[i].row << ',' << vecEvent[i].col << ',' << vecEvent[i].tOffPixelIncreasing << endl;
        }
    }
    else if (sensorMode == CeleX5::Event_Intensity_Mode)
    {
        for (int i = 0; i < dataSize; i++)
        {
            g_ofWriteMat << vecEvent[i].row << ',' << vecEvent[i].col << ',' << vecEvent[i].adc
                         << ',' << vecEvent[i].polarity << ',' <<  vecEvent[i].tOffPixelIncreasing << endl;
        }
    }
    else if (sensorMode == CeleX5::Event_In_Pixel_Timestamp_Mode)
    {
        for (int i = 0; i < dataSize; i++)
        {
            g_ofWriteMat << vecEvent[i].row << ',' << vecEvent[i].col << ','
                         << vecEvent[i].tInPixelIncreasing << ',' << vecEvent[i].tOffPixelIncreasing << endl;
        }
    }
}

void SensorDataObserver::paintEvent(QPaintEvent *event)
{
#ifdef _LOG_TIME_CONSUMING_
    struct timeval tv_begin, tv_end;
    gettimeofday(&tv_begin, NULL);
#endif

    Q_UNUSED(event);
    QFont font;
    font.setPixelSize(20);
    QPainter painter(this);
    painter.setPen(QColor(255, 255, 255));
    painter.setFont(font);

    if (m_bLoopModeEnabled)
    {
        painter.drawPixmap(0, 0, QPixmap::fromImage(m_imageMode1).scaled(640, 400));
        painter.drawPixmap(660, 0, QPixmap::fromImage(m_imageMode2).scaled(640, 400));
        painter.drawPixmap(0, 440, QPixmap::fromImage(m_imageMode3).scaled(640, 400));
    }
    else
    {
        if (m_bShowMultipleWindows)
        {
            if (m_pCeleX5->getSensorFixedMode() == CeleX5::Event_Off_Pixel_Timestamp_Mode)
            {
                painter.drawPixmap(0, 0, QPixmap::fromImage(m_imageMode1).scaled(640, 400));
                painter.fillRect(QRect(0, 0, 165, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 0, 165, 22), "Event Binary Pic");

                painter.drawPixmap(660, 0, QPixmap::fromImage(m_imageMode2).scaled(640, 400));
                painter.fillRect(QRect(660, 0, 255, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(660, 0, 255, 22), "Event Denoised Binary Pic");

                painter.drawPixmap(0, 440, QPixmap::fromImage(m_imageMode3).scaled(640, 400));
                painter.fillRect(QRect(0, 440, 160, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 440, 160, 22), "Event Count Pic");

                painter.drawPixmap(660, 440, QPixmap::fromImage(m_imageMode4).scaled(640, 400));
                painter.fillRect(QRect(660, 440, 230, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(660, 440, 230, 22), "Event Denoised Count Pic");
            }
            else if (m_pCeleX5->getSensorFixedMode() == CeleX5::Event_In_Pixel_Timestamp_Mode)
            {
                painter.drawPixmap(0, 0, QPixmap::fromImage(m_imageMode1).scaled(640, 400));
                painter.fillRect(QRect(0, 0, 230, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 0, 230, 22), "Event Optical-flow Pic");

                painter.drawPixmap(660, 0, QPixmap::fromImage(m_imageMode2).scaled(640, 400));
                painter.fillRect(QRect(660, 0, 165, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(660, 0, 165, 22), "Event Binary Pic");

                painter.drawPixmap(0, 440, QPixmap::fromImage(m_imageMode3).scaled(640, 400));
                painter.fillRect(QRect(0, 440, 160, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 440, 160, 22), "Event Count Pic");

                painter.drawPixmap(660, 440, QPixmap::fromImage(m_imageMode4).scaled(640, 400));
                painter.fillRect(QRect(660, 440, 230, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(660, 440, 230, 22), "Event Denoised Binary Pic");
            }
            else if (m_pCeleX5->getSensorFixedMode() == CeleX5::Event_Intensity_Mode)
            {
                painter.drawPixmap(0, 0, QPixmap::fromImage(m_imageMode1).scaled(640, 400));
                painter.fillRect(QRect(0, 0, 170, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 0, 170, 22), "Event Binary Pic");

                painter.drawPixmap(660, 0, QPixmap::fromImage(m_imageMode2).scaled(640, 400));
                painter.fillRect(QRect(660, 0, 200, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(660, 0, 200, 22), "Event Gray Pic");

                painter.drawPixmap(0, 440, QPixmap::fromImage(m_imageMode3).scaled(640, 400));
                painter.fillRect(QRect(0, 440, 230, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 440, 230, 22), "Event Accumulated Pic");

                painter.drawPixmap(660, 440, QPixmap::fromImage(m_imageMode4).scaled(640, 400));
                painter.fillRect(QRect(660, 440, 230, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(660, 440, 230, 22), "Event Superimposed Pic");
            }
            else if (m_pCeleX5->getSensorFixedMode() == CeleX5::Optical_Flow_Mode)
            {
                painter.drawPixmap(0, 0, QPixmap::fromImage(m_imageMode1).scaled(640, 400));
                painter.fillRect(QRect(0, 0, 170, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 0, 170, 22), "Optical-flow Pic");

                painter.drawPixmap(660, 0, QPixmap::fromImage(m_imageMode2).scaled(640, 400));
                painter.fillRect(QRect(660, 0, 230, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(660, 0, 230, 22), "Optical-flow Speed Pic");

                painter.drawPixmap(0, 440, QPixmap::fromImage(m_imageMode3).scaled(640, 400));
                painter.fillRect(QRect(0, 440, 270, 22), QBrush(Qt::darkBlue));
                painter.drawText(QRect(0, 440, 270, 22), "Optical-flow Direction Pic");
            }
            else
            {
                painter.drawPixmap(0, 0, QPixmap::fromImage(m_imageMode1).scaled(640, 400));
            }
        }
        else
        {
            painter.drawPixmap(0, 0, QPixmap::fromImage(m_imageMode1));
            if (m_pCeleX5->getSensorFixedMode() == CeleX5::Event_Off_Pixel_Timestamp_Mode ||
                m_pCeleX5->getSensorFixedMode() == CeleX5::Event_In_Pixel_Timestamp_Mode ||
                m_pCeleX5->getSensorFixedMode() == CeleX5::Event_Intensity_Mode)
            {
                painter.fillRect(QRect(10, 10, 280, 30), QBrush(Qt::blue));
                painter.drawText(QRect(14, 14, 280, 30), "Event Rate: " + QString::number(m_pCeleX5->getEventRate()) + " eps");
                if (g_bStartGenerateOFFPN)
                {
                    painter.drawText(QRect(550, 400, 280, 30), "Fpn Progress: " + QString::number(g_dFPNProgessValue) + "%");
                }
            }
            else
            {
                painter.fillRect(QRect(10, 10, 120, 30), QBrush(Qt::blue));
                painter.drawText(QRect(14, 14, 120, 30), "FPS: " + QString::number(m_uiRealFullFrameFPS) + "/" + QString::number(m_uiFullFrameFPS));
            }
        }
    }

#ifdef _LOG_TIME_CONSUMING_
    gettimeofday(&tv_end, NULL);
    //cout << "paintEvent time = " << tv_end.tv_usec - tv_begin.tv_usec << endl;
#endif
}

void SensorDataObserver::onUpdateImage()
{
    if (m_pCeleX5->isLoopModeEnabled())
    {
        return;
        m_pCeleX5->getFullPicBuffer(m_pBuffer[0]);
        updateQImageBuffer(m_pBuffer[0], 1, 0);

        CeleX5::CeleX5Mode mode = m_pCeleX5->getSensorLoopMode(2);
        if (mode == CeleX5::Event_Off_Pixel_Timestamp_Mode)
        {
            if (0 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventBinaryPic);
            else if (1 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventDenoisedBinaryPic);
            else if (2 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventCountPic);
            else if (3 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventDenoisedCountPic);
        }
        else if (mode == CeleX5::Event_In_Pixel_Timestamp_Mode)
        {
            if (0 == m_iLoopPicMode)
                m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[1], CeleX5::OpticalFlowPic);
            else if (1 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventBinaryPic);
        }
        else if (mode == CeleX5::Event_Intensity_Mode)
        {
            if (0 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventBinaryPic);
            else if (1 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventGrayPic);
            else if (2 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventAccumulatedPic);
            else if (3 == m_iLoopPicMode)
                m_pCeleX5->getEventPicBuffer(m_pBuffer[1], CeleX5::EventCountPic);
        }
        updateQImageBuffer(m_pBuffer[1], 2, 0);

        m_pCeleX5->getOpticalFlowPicBuffer(m_pBuffer[2], CeleX5::OpticalFlowPic);
        updateQImageBuffer(m_pBuffer[2], 3, 1);

        if (m_bRecordingImages)
        {
            saveRecordingImage(m_pBuffer[0], 1);

            m_pCeleX5->getEventPicBuffer(m_pBuffer[3], CeleX5::EventCountPic);
            saveRecordingImage(m_pBuffer[3], 2);
        }
    }
    else
    {
        CeleX5::CeleX5Mode mode = m_pCeleX5->getSensorFixedMode();

        processSensorBuffer(mode, -1);

        m_uiRealFullFrameFPS = m_pCeleX5->getFullFrameFPS();
    }
}

CeleX5Widget::CeleX5Widget(QWidget *parent)
    : QWidget(parent)
    , m_pAdSettingWidget(NULL)
    , m_pCeleX5Cfg(NULL)
    , m_pSettingsWidget(NULL)
    , m_pPlaybackBg(NULL)
    , m_pVersionWidget(NULL)
    , m_pBtnPlayPause(NULL)
    , m_emDeviceType(CeleX5::CeleX5_MIPI)
    , m_emSensorFixedMode(CeleX5::Event_Off_Pixel_Timestamp_Mode)
    , m_uiSaveBinDuration(300)
    , m_bPlaybackPaused(false)
    , m_iCurrCbBoxImageType(0)
    , m_iCurrCbBoxLoopImageType(0)
    , m_iPackageCountBegin(0)
    , m_iPackageCountEnd(0)
{
    m_pCeleX5->openSensor(m_emDeviceType);
    m_pCeleX5->enableFrameDenoising();
    m_pCeleX5->enableEventOpticalFlow();
    m_pCeleX5->disableEventStreamModule();
    //m_pCeleX5->enableEventDenoising();

    m_pCeleX5->getCeleX5Cfg();
    m_mapCfgDefault = m_pCeleX5->getCeleX5Cfg();

    this->setGeometry(10, 50, 1850, 1000);

    m_pReadBinTimer = new QTimer(this);
    connect(m_pReadBinTimer, SIGNAL(timeout()), this, SLOT(onReadBinTimer()));

    m_pUpdatePlayInfoTimer = new QTimer(this);
    connect(m_pUpdatePlayInfoTimer, SIGNAL(timeout()), this, SLOT(onUpdatePlayInfo()));

    m_pRecordDataTimer = new QTimer(this);
    connect(m_pRecordDataTimer, SIGNAL(timeout()), this, SLOT(onRecordDataTimer()));

    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();
    //scroll area
    QScrollArea* pScrollArea = new QScrollArea(this);
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pScrollArea->setGeometry(0, 0, screenRect.width(), screenRect.height()-70);
    //set m_pScrollWidger to m_pScrollArea
    m_pScrollWidget = new QWidget(pScrollArea);
    m_pScrollWidget->setGeometry(0, 0, 1900, 1000);
    pScrollArea->setWidget(m_pScrollWidget);

    QGridLayout* layoutBtn = new QGridLayout;
    //layoutBtn->setContentsMargins(0, 0, 0, 880);
    createButtons(layoutBtn);

    m_pSensorDataObserver = new SensorDataObserver(m_pCeleX5->getSensorDataServer(), m_pScrollWidget);
    m_pSensorDataObserver->show();
    //m_pSensorDataObserver->setGeometry(40, 130, 1280, 1000);

    //--- create comboBox to select image type
    int comboTop = 110;
    m_pCbBoxImageType = new QComboBox(m_pScrollWidget);
    QString style1 = "QComboBox {font: 18px Calibri; color: #FFFF00; border: 2px solid darkgrey; "
                     "border-radius: 5px; background: green;}";
    QString style2 = "QComboBox:editable {background: green;}";

    m_pCbBoxImageType->setGeometry(1070, comboTop, 250, 30);
    m_pCbBoxImageType->show();
    m_pCbBoxImageType->setStyleSheet(style1 + style2);
    m_pCbBoxImageType->insertItem(0, "Event Binary Pic");
    m_pCbBoxImageType->insertItem(1, "Event Denoised Binary Pic");
    m_pCbBoxImageType->insertItem(2, "Event Count Pic");
    m_pCbBoxImageType->insertItem(3, "Event Denoised Count Pic");
    m_pCbBoxImageType->insertItem(4, "Event CountDensity Pic");
    m_pCbBoxImageType->setCurrentIndex(0);
    connect(m_pCbBoxImageType, SIGNAL(currentIndexChanged(int)), this, SLOT(onImageTypeChanged(int)));

    m_pCbBoxLoopEventType = new QComboBox(m_pScrollWidget);
    m_pCbBoxLoopEventType->setGeometry(1085, comboTop, 250, 30);
    m_pCbBoxLoopEventType->show();
    m_pCbBoxLoopEventType->setStyleSheet(style1 + style2);
    m_pCbBoxLoopEventType->insertItem(0, "Event Binary Pic");
    m_pCbBoxLoopEventType->insertItem(1, "Event Denoised Binary Pic");
    m_pCbBoxLoopEventType->insertItem(2, "Event Count Pic");
    m_pCbBoxLoopEventType->insertItem(3, "Event Denoised Count Pic");
    m_pCbBoxLoopEventType->setCurrentIndex(0);
    connect(m_pCbBoxLoopEventType, SIGNAL(currentIndexChanged(int)), this, SLOT(onLoopEventTypeChanged(int)));
    m_pCbBoxLoopEventType->hide();

    //--- create comboBox to select sensor mode
    //Fixed Mode
    m_pCbBoxFixedMode = createModeComboBox("Fixed", QRect(40, comboTop, 300, 30), m_pScrollWidget, false, 0);
    m_pCbBoxFixedMode->setCurrentText("Fixed - Event_Off_Pixel_Timestamp_Mode ");
    m_pCbBoxFixedMode->show();
    //Loop A
    m_pCbBoxLoopAMode = createModeComboBox("LoopA", QRect(40, comboTop, 300, 30), m_pScrollWidget, true, 1);
    //Loop B
    m_pCbBoxLoopBMode = createModeComboBox("LoopB", QRect(40+660, comboTop, 300, 30), m_pScrollWidget, true, 2);
    //Loop C
    m_pCbBoxLoopCMode = createModeComboBox("LoopC", QRect(40, comboTop+440, 300, 30), m_pScrollWidget, true, 3);

    m_pBtnShowStyle = createButton("Show Multiple Windows", QRect(350, comboTop, 220, 30), m_pScrollWidget);
    connect(m_pBtnShowStyle, SIGNAL(released()), this, SLOT(onShowMultipleWindows()));

    QHBoxLayout* layoutSensorImage = new QHBoxLayout;
    layoutSensorImage->setContentsMargins(30, 0, 0, 0);
    layoutSensorImage->addWidget(m_pSensorDataObserver);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(layoutBtn);
    pMainLayout->addSpacing(65);
    //pMainLayout->addLayout(layoutComboBox);
    pMainLayout->addLayout(layoutSensorImage);
    m_pScrollWidget->setLayout(pMainLayout);

    //--- create comboBox to select show type ---
    QComboBox* pEventShowType = new QComboBox(m_pScrollWidget);
    pEventShowType->setGeometry(1350, comboTop, 250, 30);
    pEventShowType->show();
    pEventShowType->setStyleSheet(style1 + style2);
    pEventShowType->insertItem(0, "Event Show By Time");
    pEventShowType->insertItem(1, "Event Show By Count");
    pEventShowType->insertItem(2, "Event Show By Row Cycle");
    pEventShowType->setCurrentIndex(0);
    connect(pEventShowType, SIGNAL(currentIndexChanged(int)), this, SLOT(onEventShowTypeChanged(int)));
    //--- show by time: frame time
    m_pFrameSlider = new CfgSlider(m_pScrollWidget, 100, 1000000, 1, 30000, this);
    m_pFrameSlider->setGeometry(1350, 150, 500, 70);
    m_pFrameSlider->setBiasType("Frame Time");
    m_pFrameSlider->setDisplayName("   Frame Time (μs)");
    m_pFrameSlider->setObjectName("Frame Time");
    //--- show by row cycle: row cycle count
    m_pRowCycleSlider = new CfgSlider(m_pScrollWidget, 1, 100, 1, 6, this);
    m_pRowCycleSlider->setGeometry(1350, 150, 500, 70);
    m_pRowCycleSlider->setBiasType("Row Cycle Count");
    m_pRowCycleSlider->setDisplayName("Row Cycle Count");
    m_pRowCycleSlider->setObjectName("Row Cycle Count");
    m_pRowCycleSlider->hide();
    //--- show by count: event count
    m_pEventCountSlider = new CfgSlider(m_pScrollWidget, 10000, 1000000, 1, 100000, this);
    m_pEventCountSlider->setGeometry(1350, 150, 500, 70);
    m_pEventCountSlider->setBiasType("Event Count");
    m_pEventCountSlider->setDisplayName("Event Count");
    m_pEventCountSlider->setObjectName("Event Count");
    m_pEventCountSlider->hide();

    //--- Display FPS ---
    m_pFPSSlider = new CfgSlider(m_pScrollWidget, 1, 1000, 1, 30, this);
    m_pFPSSlider->setGeometry(1350, 250, 500, 70);
    m_pFPSSlider->setBiasType("Display FPS");
    m_pFPSSlider->setDisplayName("   Display FPS (f/s)");
    m_pFPSSlider->setObjectName("Display FPS");

    //--- Event Count Step ---
    CfgSlider* pSlider3 = new CfgSlider(m_pScrollWidget, 1, 255, 1, 30, this);
    pSlider3->setGeometry(1350, 350, 500, 70);
    pSlider3->setBiasType("Event Count Step");
    pSlider3->setDisplayName("Event Count Amplify Factor");
    pSlider3->setObjectName("Event Count Step");

    //--- Event Start Pos ---
    m_pEventStartPosSlider = new CfgSlider(m_pScrollWidget, 0, 100, 1, 0, this);
    m_pEventStartPosSlider->setGeometry(1350, 500, 500, 70);
    m_pEventStartPosSlider->setBiasType("Event Start Pos");
    m_pEventStartPosSlider->setDisplayName("Event Start Pos (ms)");
    m_pEventStartPosSlider->setObjectName("Event Start Pos");
    m_pEventStartPosSlider->hide();

    //--- Event Count Density ---
    m_pEventCountDensitySlider = new CfgSlider(m_pScrollWidget, 5, 8, 1, 5, this);
    m_pEventCountDensitySlider->setGeometry(1350, 450, 500, 70);
    m_pEventCountDensitySlider->setBiasType("Event Count Density");
    m_pEventCountDensitySlider->setDisplayName("Event Count Density");
    m_pEventCountDensitySlider->setObjectName("Event Count Density");

    //--- Event Count  ---
    m_pEventCountThresholdSlider = new CfgSlider(m_pScrollWidget, 0, 100, 1, 10, this);
    m_pEventCountThresholdSlider->setGeometry(1350, 550, 500, 70);
    m_pEventCountThresholdSlider->setBiasType("Event Count Threshold");
    m_pEventCountThresholdSlider->setDisplayName("Event Count Threshold");
    m_pEventCountThresholdSlider->setObjectName("Event Count Threshold");
    //
    m_pRowPlotWidget = new QCustomPlot;
    m_pRowPlotWidget->resize(800, 200);
    m_pRowPlotWidget->show();

    for (int i = 0; i < 800; ++i)
    {
        g_vecEventRow[i] = i; // x goes from -1 to 1
    }
    m_pRowPlotWidget->addGraph();
    m_pRowPlotWidget->xAxis->setRange(0, 800);
    m_pRowPlotWidget->yAxis->setRange(0, 60000);
    m_pRowPlotWidget->xAxis->setTicks(false);
    m_pRowPlotWidget->yAxis->setTicks(false);

    QGraphicsScene* pScene = new QGraphicsScene(this);
    pScene->addWidget(m_pRowPlotWidget);
    m_pPlotGraphicsView = new QGraphicsView(pScene, this);
    m_pPlotGraphicsView->setStyleSheet("QGraphicsView {background: transparent;}");
    m_pPlotGraphicsView->setGeometry(1132, 143, 202, 802);
    m_pPlotGraphicsView->rotate(270);
    m_pPlotGraphicsView->hide();
    //
    m_pColPlotWidget = new QCustomPlot(this);
    m_pColPlotWidget->setGeometry(25, 800, 1310, 200);
    m_pColPlotWidget->hide();
    m_pColPlotWidget->setBackgroundScaled(false);

    for (int i = 0; i < 1280; ++i)
    {
        g_vecEventCol[i] = i; // x goes from -1 to 1
        //g_vecEventCountPerCol[i] = 1000;  // let's plot a quadratic function
    }
    m_pColPlotWidget->addGraph();
    //m_pColPlotWidget->xAxis->setLabel("column");
    //m_pColPlotWidget->yAxis->setLabel("event count");
    m_pColPlotWidget->xAxis->setRange(0, 1280);
    m_pColPlotWidget->yAxis->setRange(0, 60000);
    m_pColPlotWidget->xAxis->setTicks(false);
    m_pColPlotWidget->yAxis->setTicks(false);

    m_pUpdateSlotTimer = new QTimer(this);
    connect(m_pUpdateSlotTimer, SIGNAL(timeout()), this, SLOT(onUpdateEventCountPlot()));
    //
    m_pArrowLeft = new QLabel(this);
    m_pArrowLeft->setGeometry(1100, 170, 190, 56);
    m_pArrowLeft->setPixmap(QPixmap(":/images/arrow_left.png"));
    m_pArrowLeft->hide();

    m_pArrowRight = new QLabel(this);
    m_pArrowRight->setGeometry(1100, 170, 190, 56);
    m_pArrowRight->setPixmap(QPixmap(":/images/arrow_right.png"));
    m_pArrowRight->hide();

    m_pArrowUp = new QLabel(this);
    m_pArrowUp->setGeometry(1200, 170, 58, 160);
    m_pArrowUp->setPixmap(QPixmap(":/images/arrow_up.png"));
    m_pArrowUp->hide();

    m_pArrowDown = new QLabel(this);
    m_pArrowDown->setGeometry(1200, 170, 64, 156);
    m_pArrowDown->setPixmap(QPixmap(":/images/arrow_down.png"));
    m_pArrowDown->hide();
}

CeleX5Widget::~CeleX5Widget()
{
    if (m_pCeleX5)
    {
        delete m_pCeleX5;
        m_pCeleX5 = NULL;
    }
    if (m_pSensorDataObserver)
    {
        delete m_pSensorDataObserver;
        m_pSensorDataObserver = NULL;
    }
}

void CeleX5Widget::resizeEvent(QResizeEvent *)
{
    //cout << "CeleX5Widget::resizeEvent" << endl;
}

void CeleX5Widget::closeEvent(QCloseEvent *)
{
    //cout << "CeleX5Widget::closeEvent" << endl;
    //QWidget::closeEvent(event);
    if (m_pSettingsWidget)
    {
        m_pSettingsWidget->close();
    }
    if (m_pAdSettingWidget)
    {
        m_pAdSettingWidget->close();
    }
    if (m_pCeleX5Cfg)
    {
        m_pCeleX5Cfg->close();
    }
}

void CeleX5Widget::playback(QPushButton *pButton)
{
    if ("Playback" == pButton->text())
    {
        m_pBtnShowStyle->hide();

        QString filePath = QFileDialog::getOpenFileName(this, "Open a bin file", QCoreApplication::applicationDirPath(), "Bin Files (*.bin)");

        if (filePath.isEmpty())
            return;

        g_qsBinFileName = filePath;

        if (m_pCeleX5->openBinFile(filePath.toLocal8Bit().data()))
        {
            pButton->setText("Exit Playback");
            setButtonEnable(pButton);
            //
            m_pSensorDataObserver->setBinFileName(filePath);
            m_pSensorDataObserver->setDisplayType(Playback);
            m_pSensorDataObserver->setMultipleShowEnabled(false);
            //
            showPlaybackBoard(true);
            //
            m_pBtnShowStyle->setText("Show Multiple Windows");

            QList<QPushButton*> button = this->findChildren<QPushButton *>("Enter Loop Mode");
            if (m_pCeleX5->isLoopModeEnabled())
            {
                switchMode(button[0], true, true);
            }
            else
            {
                m_pCeleX5->setLoopModeEnabled(false);
                switchMode(button[0], false, true);
                m_pCbBoxFixedMode->setCurrentIndex((int)m_pCeleX5->getSensorFixedMode());
            }
            m_pReadBinTimer->start(READ_BIN_TIME);
            m_pUpdatePlayInfoTimer->start(UPDATE_PLAY_INFO_TIME);

            m_pCeleX5->pauseSensor(); //puase sensor: the sensor will not output data when playback
        }
    }
    else
    {
        pButton->setText("Playback");
        setButtonNormal(pButton);
        //
        m_pSensorDataObserver->setDisplayType(Realtime);
        m_pBtnShowStyle->show();
        showPlaybackBoard(false);
        //
        m_pCeleX5->restartSensor(); //restart sensor: the sensor will output data agatin
        m_pCeleX5->setIsPlayBack(false);
        m_pCeleX5->play();
        if (!m_pCeleX5->isLoopModeEnabled())
        {
            m_pCbBoxFixedMode->setCurrentIndex(m_emSensorFixedMode);
            m_pCeleX5->setSensorFixedMode(m_emSensorFixedMode);
        }
        else
        {
            m_pCeleX5->setLoopModeEnabled(true);
            m_pCeleX5->setSensorLoopMode(CeleX5::Full_Picture_Mode, 1);
            m_pCeleX5->setSensorLoopMode(CeleX5::Event_Off_Pixel_Timestamp_Mode, 2);
            m_pCeleX5->setSensorLoopMode(CeleX5::Optical_Flow_Mode, 3);
        }
        m_bPlaybackPaused = false;
        m_pBtnPlayPause->setStyleSheet("QPushButton {background-color: #002F6F; background-image: url(:/images/player_pause.png); "
                                       "border-style: outset; border-radius: 10px; border-color: #002F6F;} "
                                       "QPushButton:pressed {background: #992F6F; background-image: url(:/images/player_play.png); }");
    }
}

QComboBox *CeleX5Widget::createModeComboBox(QString text, QRect rect, QWidget *parent, bool bLoop, int loopNum)
{
    QString style1 = "QComboBox {font: 18px Calibri; color: white; border: 2px solid darkgrey; "
                     "border-radius: 5px; background: green;}";
    QString style2 = "QComboBox:editable {background: green;}";

    QComboBox* pComboBoxMode = new QComboBox(parent);
    pComboBoxMode->setGeometry(rect);
    pComboBoxMode->show();
    pComboBoxMode->setStyleSheet(style1 + style2);
    QStringList modeList;
    if (bLoop)
    {
        if (loopNum == 1)
            modeList << "Event_Off_Pixel_Timestamp Mode" << "Event_In_Pixel_Timestamp Mode" << "Event_Intensity Mode"
                     << "Full_Picture Mode";
        else if (loopNum == 2)
            modeList << "Event_Off_Pixel_Timestamp Mode" << "Event_In_Pixel_Timestamp Mode" << "Event_Intensity Mode"
                     << "Full_Picture Mode";
        else if (loopNum == 3)
            modeList << "Event_Off_Pixel_Timestamp Mode" << "Event_In_Pixel_Timestamp Mode" << "Event_Intensity Mode"
                     << "Full_Picture Mode" << "Optical_Flow Mode";
    }
    else
    {
        modeList << "Event_Off_Pixel_Timestamp Mode" << "Event_In_Pixel_Timestamp Mode" << "Event_Intensity Mode"
                 << "Full_Picture Mode" << "Optical_Flow Mode"/* << "Optical_Flow_FPN Mode"*/;
    }

    for (int i = 0; i < modeList.size(); i++)
    {
        pComboBoxMode->insertItem(i, text+" - "+modeList.at(i));
    }
    pComboBoxMode->hide();
    connect(pComboBoxMode, SIGNAL(currentIndexChanged(QString)), this, SLOT(onSensorModeChanged(QString)));

    return pComboBoxMode;
}

void CeleX5Widget::createButtons(QGridLayout* layout)
{
    QStringList btnNameList;
    btnNameList.push_back("RESET");
    btnNameList.push_back("Generate FPN");
    btnNameList.push_back("Change FPN");
    btnNameList.push_back("Start Recording Bin");
    //btnNameList.push_back("Start Recording Images");
    btnNameList.push_back("Playback");
    btnNameList.push_back("Enter Loop Mode");
    btnNameList.push_back("Configurations");
    btnNameList.push_back("Enable Auto ISP");
    //btnNameList.push_back("More Parameters ...");
    //btnNameList.push_back("Test: Save Pic");
    btnNameList.push_back("Rotate_LR");
    btnNameList.push_back("Rotate_UD");
    btnNameList.push_back("ConvertBin2Video");
    btnNameList.push_back("ConvertBin2CSV");
    btnNameList.push_back("Version");
    //btnNameList.push_back("Save Parameters");
    //btnNameList.push_back("Enable Anti-Flashlight");
    btnNameList.push_back("Advanced Setting");

    m_pButtonGroup = new QButtonGroup;
    int index = 0;
    for (int i = 0; i < btnNameList.count(); ++i)
    {
        QPushButton* pButton = createButton(btnNameList.at(i), QRect(20, 20, 100, 36), this);
        pButton->setObjectName(btnNameList.at(i));
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F;}");
        m_pButtonGroup->addButton(pButton, i);
        if ("Change FPN" == btnNameList.at(i) ||
            "Rotate_UD" == btnNameList.at(i) ||
            "ConvertBin2CSV" == btnNameList.at(i) ||
            "Start Recording Images" == btnNameList.at(i))
        {
            layout->addWidget(pButton, 1, index - 1);
        }
        else
        {
            layout->addWidget(pButton, 0, index);
            index++;
        }
    }
    connect(m_pButtonGroup, SIGNAL(buttonReleased(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));
}

void CeleX5Widget::changeFPN()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open a FPN file", QCoreApplication::applicationDirPath(), "FPN Files (*.txt)");

    if (filePath.isEmpty())
        return;
    m_pCeleX5->setFpnFile(filePath.toLocal8Bit().data());
}

void CeleX5Widget::record(QPushButton* pButton)
{
    if ("Start Recording Bin" == pButton->text())
    {
        g_bRecordingData = true;
        pButton->setText("Stop Recording Bin");
        setButtonEnable(pButton);
        //
        const QDateTime now = QDateTime::currentDateTime();
        const QString timestamp = now.toString(QLatin1String("yyyyMMdd_hhmmsszzz"));

        QString qstrBinName;
        if (CeleX5::CeleX5_OpalKelly != m_emDeviceType)
        {
            qstrBinName = QCoreApplication::applicationDirPath() + "/MipiData_" + timestamp;
        }
        else
        {
            qstrBinName = QCoreApplication::applicationDirPath() + "/ParaData_" + timestamp;
        }
        QStringList modeList;
        modeList << "_E_" << "_EO_" << "_EI_" << "_F_" << "_FO1_" << "_FO2_" << "_FO3_" << "_FO4_";
        if (m_pCeleX5->isLoopModeEnabled())
            qstrBinName += "_Loop_";
        else
            qstrBinName += modeList.at(int(m_pCeleX5->getSensorFixedMode()));

        qstrBinName += QString::number(m_pCeleX5->getClockRate());
        qstrBinName += "M.bin"; //MHz
        //std::string filePath = qstrBinName.toStdString();

        m_pCeleX5->startRecording(qstrBinName.toLocal8Bit().data());

        m_pRecordDataTimer->start(m_uiSaveBinDuration*1000);
    }
    else
    {
        g_bRecordingData = false;
        pButton->setText("Start Recording Bin");
        setButtonNormal(pButton);
        m_pCeleX5->stopRecording();
        m_pRecordDataTimer->stop();
    }
}

void CeleX5Widget::recordImages(QPushButton *pButton)
{
    if ("Start Recording Images" == pButton->text())
    {
        pButton->setText("Stop Recording Images");
        setButtonEnable(pButton);
        m_pSensorDataObserver->setRecordingImgState(true);

        const QDateTime now = QDateTime::currentDateTime();
        const QString timestamp = now.toString(QLatin1String("yyyyMMdd_hhmmss"));

        m_pSensorDataObserver->setRecordingTime(timestamp);

        QDir dir;
        dir.cd(QCoreApplication::applicationDirPath());
        if (!dir.exists("image_fullpic"))
        {
            dir.mkdir("image_fullpic");
        }
        if (!dir.exists("image_event_count"))
        {
            dir.mkdir("image_event_count");
        }
    }
    else
    {
        pButton->setText("Start Recording Images");
        setButtonNormal(pButton);
        m_pSensorDataObserver->setRecordingImgState(false);
    }
}

void CeleX5Widget::recordVideo(QPushButton* pButton)
{
    if ("Start Recording Video" == pButton->text())
    {
        pButton->setText("Stop Recording Video");
        setButtonEnable(pButton);

        const QDateTime now = QDateTime::currentDateTime();
        const QString timestamp = now.toString(QLatin1String("yyyyMMdd_hhmmsszzz"));

        QString qstrVideoName;
        if (CeleX5::CeleX5_OpalKelly != m_emDeviceType)
        {
            qstrVideoName = QCoreApplication::applicationDirPath() + "/MipiData_" + timestamp;
        }
        else
        {
            qstrVideoName = QCoreApplication::applicationDirPath() + "/ParaData_" + timestamp;
        }
        QStringList modeList;
        modeList << "_E_" << "_EO_" << "_EI_" << "_F_" << "_FO1_" << "_FO2_" << "_FO3_" << "_FO4_";
        if (m_pCeleX5->isLoopModeEnabled())
            qstrVideoName += "_Loop_";
        else
            qstrVideoName += modeList.at(int(m_pCeleX5->getSensorFixedMode()));

        qstrVideoName += QString::number(m_pCeleX5->getClockRate());
        qstrVideoName += "M.avi"; //MHz
    }
    else
    {
        pButton->setText("Start Recording Video");
        setButtonNormal(pButton);
    }
}

void CeleX5Widget::switchMode(QPushButton* pButton, bool isLoopMode, bool bPlayback)
{
    if (isLoopMode)
    {
        m_pSensorDataObserver->setMultipleShowEnabled(false);
        m_pSensorDataObserver->setLoopModeEnabled(true);
        if (!bPlayback)
        {
            m_pCeleX5->setSensorLoopMode(CeleX5::Full_Picture_Mode, 1);
            m_pCeleX5->setSensorLoopMode(CeleX5::Event_Off_Pixel_Timestamp_Mode, 2);
            m_pCeleX5->setSensorLoopMode(CeleX5::Optical_Flow_Mode, 3);
            m_pCeleX5->setLoopModeEnabled(true);
        }
        m_pCbBoxLoopAMode->show();
        m_pCbBoxLoopBMode->show();
        m_pCbBoxLoopCMode->show();
        //
        m_pCbBoxLoopEventType->show();
        m_pCbBoxImageType->hide();
        m_pCbBoxFixedMode->hide();
        m_pBtnShowStyle->hide();
        m_pEventStartPosSlider->show();
        m_pEventCountDensitySlider->hide();
        m_pEventCountThresholdSlider->hide();
        m_pColPlotWidget->hide();
        m_pPlotGraphicsView->hide();
        pButton->setText("Enter Fixed Mode");
        //
        //Loop A
        m_pCbBoxLoopAMode->setCurrentText("LoopA - Full_Picture Mode");
        //Loop B
        m_pCbBoxLoopBMode->setCurrentText("LoopB - Event_Off_Pixel_Timestamp Mode");
        //Loop C
        m_pCbBoxLoopCMode->setCurrentText("LoopC - Optical_Flow Mode");

        cout << "m_iCurrCbBoxLoopImageType" << m_iCurrCbBoxLoopImageType << endl;
        m_pCbBoxLoopEventType->setCurrentIndex(m_iCurrCbBoxLoopImageType);
    }
    else
    {
        m_pSensorDataObserver->setLoopModeEnabled(false);
        if (!bPlayback)
        {
            m_pCeleX5->setLoopModeEnabled(false);
        }
        m_pCbBoxLoopAMode->hide();
        m_pCbBoxLoopBMode->hide();
        m_pCbBoxLoopCMode->hide();
        //
        m_pCbBoxLoopEventType->hide();
        m_pCbBoxImageType->show();
        m_pCbBoxFixedMode->show();
        m_pEventStartPosSlider->hide();
        m_pEventCountDensitySlider->show();
        m_pEventCountThresholdSlider->show();
        if (m_pCbBoxImageType->currentText() == "Event CountDensity Pic")
        {
            m_pColPlotWidget->show();
            m_pPlotGraphicsView->show();
        }
        if (m_pSensorDataObserver->getDisplayType() == Realtime)
        {
            m_pBtnShowStyle->show();
            m_pCeleX5->setSensorFixedMode(m_emSensorFixedMode);
        }
        pButton->setText("Enter Loop Mode");
        m_pCbBoxImageType->setCurrentIndex(m_iCurrCbBoxImageType);
    }
    m_pSensorDataObserver->update();
}

void CeleX5Widget::showCFG()
{
    if (!m_pSettingsWidget)
    {
        m_pSettingsWidget = new SettingsWidget(m_pCeleX5, this);
    }
    m_pSettingsWidget->raise();
    m_pSettingsWidget->updateCfgs();
    m_pSettingsWidget->show();
    if (m_pSettingsWidget->isMinimized())
        m_pSettingsWidget->showNormal();
}

void CeleX5Widget::showAdvancedSetting()
{
    if (!m_pAdSettingWidget)
    {
        m_pAdSettingWidget = new QWidget;
        m_pAdSettingWidget->setWindowTitle("Advanced Settings");
        m_pAdSettingWidget->setGeometry(200, 50, 1100, 780);

        QDesktopWidget* desktopWidget = QApplication::desktop();
        QRect screenRect = desktopWidget->screenGeometry();
        int scrollWidth = screenRect.width() - 100;
        int scrollHeight = screenRect.height() - 100;
        if (scrollWidth > m_pAdSettingWidget->width())
            scrollWidth = m_pAdSettingWidget->width();
        if (scrollHeight > m_pAdSettingWidget->height())
            scrollHeight = m_pAdSettingWidget->height();

        //scroll area
        QScrollArea* pScrollArea = new QScrollArea(m_pAdSettingWidget);
        pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pScrollArea->setGeometry(0, 0, scrollWidth, scrollHeight);

        //set m_pScrollWidger to m_pScrollArea
        QWidget* pScrollWidget = new QWidget(pScrollArea);
        pScrollWidget->setGeometry(0, 0, m_pAdSettingWidget->width(), m_pAdSettingWidget->height());
        pScrollArea->setWidget(pScrollWidget);

        QString style1 = "QGroupBox {"
                         "border: 2px solid #990000;"
                         "font: 20px Calibri; color: #990000;"
                         "border-radius: 9px;"
                         "margin-top: 0.5em;"
                         "background: rgba(50,0,0,10);"
                         "}";
        QString style2 = "QGroupBox::title {"
                         "subcontrol-origin: margin;"
                         "left: 10px;"
                         "padding: 0 3px 0 3px;"
                         "}";

        QGroupBox* recordGroup = new QGroupBox("Data Record && Playback Parameters: ", pScrollWidget);
        recordGroup->setGeometry(50, 20, 700, 440);
        recordGroup->setStyleSheet(style1 + style2);
        //
        QLabel* pLabel = new QLabel(tr("Whether to display the images while recording"), pScrollWidget);
        pLabel->setGeometry(100, 50, 600, 50);
        pLabel->setStyleSheet("QLabel {background: transparent; font: 20px Calibri; }");

        QRadioButton *pRadioBtn = new QRadioButton(tr(" open"), pScrollWidget);
        pRadioBtn->setGeometry(120, 90, 600, 50);
        pRadioBtn->setStyleSheet("QRadioButton {background: transparent; color: #990000; font: 20px Calibri; }");
        pRadioBtn->setChecked(1);
        pRadioBtn->show();
        connect(pRadioBtn, SIGNAL(toggled(bool)), this, SLOT(onShowImagesSwitch(bool)));
        pRadioBtn->setObjectName("Display Switch");
        pRadioBtn->setAutoExclusive(false);
        //
        QStringList cfgList1;
        cfgList1 << "BinFile Time Duration (s)" << "   SavePic Count Interval";
        QStringList cfgObj1;
        cfgObj1 << "Time Duration" << "Count Interval";
        int min1[4] = {1, 0};
        int max1[4] = {3600, 10};
        int value1[4] = {300, 0};
        for (int i = 0; i < cfgList1.size(); i++)
        {
            CfgSlider* pSlider = new CfgSlider(pScrollWidget, min1[i], max1[i], 1, value1[i], this);
            pSlider->setGeometry(90, 180+i*90, 600, 70);
            pSlider->setBiasType(QString(cfgObj1.at(i)).toStdString());
            pSlider->setDisplayName(cfgList1.at(i));
            pSlider->setObjectName(cfgObj1.at(i));
        }

        QLabel* pLabelPicFormat = new QLabel(tr("The picture format for saving: "), pScrollWidget);
        pLabelPicFormat->setGeometry(100, 360, 600, 50);
        pLabelPicFormat->setStyleSheet("QLabel {background: transparent; font: 20px Calibri; }");

        QWidget* pWidget = new QWidget(pScrollWidget);
        pWidget->setGeometry(100, 400, 600, 50);

        QRadioButton *pRadioBtnJPG = new QRadioButton(tr(" JPG"), pWidget);
        pRadioBtnJPG->setGeometry(20, 0, 600, 50);
        pRadioBtnJPG->setStyleSheet("QRadioButton {background: transparent; color: #990000; font: 20px Calibri; }");
        pRadioBtnJPG->setChecked(1);
        pRadioBtnJPG->show();
        connect(pRadioBtnJPG, SIGNAL(toggled(bool)), this, SLOT(onJPGFormatClicked(bool)));
        pRadioBtnJPG->setObjectName("JPG");
        //pRadioBtnJPG->setAutoExclusive(false);

        QRadioButton *pRadioBtnBMP = new QRadioButton(tr(" BMP"), pWidget);
        pRadioBtnBMP->setGeometry(220, 0, 600, 50);
        pRadioBtnBMP->setStyleSheet("QRadioButton {background: transparent; color: gray; font: 20px Calibri; }");
        pRadioBtnBMP->setChecked(0);
        pRadioBtnBMP->show();
        connect(pRadioBtnBMP, SIGNAL(toggled(bool)), this, SLOT(onBMPFormatClicked(bool)));
        pRadioBtnBMP->setObjectName("BMP");
        //pRadioBtnBMP->setAutoExclusive(false);

        QGroupBox* otherGroup = new QGroupBox("Other Parameters: ", pScrollWidget);
        otherGroup->setGeometry(50, 500, 700, 200);
        otherGroup->setStyleSheet(style1 + style2);
        //
        QStringList cfgList2;
        cfgList2 << "Resolution Parameter";
        QStringList cfgObj2;
        cfgObj2 << "Resolution";
        int min2[1] = {0};
        int max2[1] = {255};
        int value2[1] = {0};
        for (int i = 0; i < cfgList2.size(); i++)
        {
            CfgSlider* pSlider = new CfgSlider(pScrollWidget, min2[i], max2[i], 1, value2[i], this);
            pSlider->setGeometry(90, 550+i*100, 600, 70);
            pSlider->setBiasType(QString(cfgObj2.at(i)).toStdString());
            pSlider->setDisplayName(cfgList2.at(i));
            pSlider->setObjectName(cfgObj2.at(i));
        }
        m_pAdSettingWidget->setFocus();
    }
    m_pAdSettingWidget->raise();
    m_pAdSettingWidget->show();
    if (m_pAdSettingWidget->isMinimized())
        m_pAdSettingWidget->showNormal();

    QPushButton* pButton = createButton("More Parameters ...", QRect(20, 20, 100, 36), m_pAdSettingWidget);
    pButton->setObjectName("More Parameters ...");
    pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                           "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                           "font: 20px Calibri; }"
                           "QPushButton:pressed {background: #992F6F;}");
    pButton->setGeometry(850, 30, 200, 30);
    pButton->show();
    connect(pButton, SIGNAL(released()), this, SLOT(onShowMoreParameters()));
}

void CeleX5Widget::setSliderMaxValue(QWidget *parent, QString objName, int value)
{
    for (int i = 0; i < parent->children().size(); ++i)
    {
        CfgSlider* pWidget = (CfgSlider*)parent->children().at(i);
        if (pWidget->objectName() == objName)
        {
            pWidget->setMaximum(value);
            return;
        }
    }
}

int CeleX5Widget::getSliderMax(QWidget *parent, QString objName)
{
    for (int i = 0; i < parent->children().size(); ++i)
    {
        CfgSlider* pWidget = (CfgSlider*)parent->children().at(i);
        if (pWidget->objectName() == objName)
        {
            return pWidget->maximum();
        }
    }
    return 0;
}

void CeleX5Widget::showPlaybackBoard(bool show)
{
    if (!m_pPlaybackBg)
    {
        m_pPlaybackBg = new QWidget(m_pScrollWidget);
        m_pPlaybackBg->setStyleSheet("background-color: lightgray; ");
        m_pPlaybackBg->setGeometry(1350, 500, 550, 300);

        CfgSlider* pSlider2 = new CfgSlider(m_pPlaybackBg, 0, 100000, 1, 0, this);
        pSlider2->setGeometry(0, 30, 500, 70);
        pSlider2->setBiasType("Package Count");
        pSlider2->setDisplayName("   Package Count");
        pSlider2->setObjectName("Package Count");

        int left = 140;
        int top = 130;
        int spacing = 20;
        QPushButton* m_pBtnReplay = new QPushButton("Replay", m_pPlaybackBg);
        m_pBtnReplay->setGeometry(left, top, 80, 30);
        m_pBtnReplay->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                    "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                    "font: 20px Calibri; }"
                                    "QPushButton:pressed {background: #992F6F;}");
        connect(m_pBtnReplay, SIGNAL(released()), this, SLOT(onBtnReplayRelease()));

        left += m_pBtnReplay->width()+spacing;
        m_pBtnPlayPause = new QPushButton(m_pPlaybackBg);
        m_pBtnPlayPause->setGeometry(left, top, 80, 30);
        m_pBtnPlayPause->setStyleSheet("QPushButton {background-color: #002F6F; background-image: url(:/images/player_pause.png); "
                                       "border-style: outset; border-radius: 10px; border-color: #002F6F;} "
                                       "QPushButton:pressed {background: #992F6F; background-image: url(:/images/player_play.png); }");
        connect(m_pBtnPlayPause, SIGNAL(released()), this, SLOT(onBtnPlayPauseReleased()));

        left += m_pBtnPlayPause->width()+spacing;
        m_pBtnSavePic = new QPushButton("Start Saving Pic", m_pPlaybackBg);
        m_pBtnSavePic->setGeometry(left, top, 150, 30);
        m_pBtnSavePic->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                     "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                     "font: 20px Calibri; }"
                                     "QPushButton:pressed {background: #992F6F;}");
        connect(m_pBtnSavePic, SIGNAL(released()), this, SLOT(onBtnSavePicReleased()));

        m_pBtnSavePicEx = new QPushButton("Start Saving Pic (Replay)", m_pPlaybackBg);
        m_pBtnSavePicEx->setGeometry(140, 200, 220, 30);
        m_pBtnSavePicEx->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                       "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                       "font: 20px Calibri; }"
                                       "QPushButton:pressed {background: #992F6F;}");
        connect(m_pBtnSavePicEx, SIGNAL(released()), this, SLOT(onBtnSavePicExReleased()));

        QPushButton* pBtnSaveBin = new QPushButton("Save Bin As...", m_pPlaybackBg);
        pBtnSaveBin->setGeometry(340, 250, 150, 30);
        pBtnSaveBin->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                       "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                       "font: 20px Calibri; }"
                                       "QPushButton:pressed {background: #992F6F;}");
        connect(pBtnSaveBin, SIGNAL(released()), this, SLOT(onBtnSaveBinReleased()));

        QLineEdit* pLineEditBegin = new QLineEdit(m_pPlaybackBg);
        connect(pLineEditBegin, SIGNAL(editingFinished()), this, SLOT(onBinBeginFinished()));
        pLineEditBegin->setObjectName("PackageCountBegin");
        pLineEditBegin->setGeometry(50, 250, 100, 30);
        pLineEditBegin->setAlignment(Qt::AlignCenter);
        pLineEditBegin->setStyleSheet("QLineEdit {background: rgb(255, 255, 222); color: rgb(255, 0, 0); font: 18px Calibri}");
        pLineEditBegin->setText(QString::number(0));

        QLineEdit* pLineEditEnd = new QLineEdit(m_pPlaybackBg);
        connect(pLineEditEnd, SIGNAL(editingFinished()), this, SLOT(onBinEndFinished()));
        pLineEditEnd->setObjectName("PackageCountEnd");
        pLineEditEnd->setGeometry(200, 250, 100, 30);
        pLineEditEnd->setAlignment(Qt::AlignCenter);
        pLineEditEnd->setStyleSheet("QLineEdit {background: rgb(255, 255, 222); color: rgb(255, 0, 0); font: 18px Calibri}");
        pLineEditEnd->setText(QString::number(0));
    }
    if (show)
        m_pPlaybackBg->show();
    else
        m_pPlaybackBg->hide();
}

void CeleX5Widget::convertBin2Video(QPushButton* pButton)
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open a bin file", QCoreApplication::applicationDirPath(), "Bin Files (*.bin)");

    if (filePath.isEmpty())
        return;

    showPlaybackBoard(true);
    m_pSensorDataObserver->setDisplayType(ConvertBin2Video);
    pButton->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                           "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                           "font: 20px Calibri; }"
                           "QPushButton:pressed {background: #992F6F; }");
    std::string path = filePath.left(filePath.size() - 4).toStdString();
#ifdef _WIN32
    path += ".mkv";
#else
    path += ".mp4";
#endif
    if (m_pCeleX5->openBinFile(filePath.toLocal8Bit().data()))
    {
        QList<QPushButton*> button=this->findChildren<QPushButton *>("Enter Loop Mode");

        if(m_pCeleX5->isLoopModeEnabled())
        {
            switchMode(button[0], true, true);
        }
        else
        {
            switchMode(button[0], false, true);
            m_pCbBoxFixedMode->setCurrentIndex((int)m_pCeleX5->getSensorFixedMode());
        }
        m_pReadBinTimer->start(READ_BIN_TIME);
        m_pUpdatePlayInfoTimer->start(UPDATE_PLAY_INFO_TIME);
        m_pVideoStream->avWriterInit(path.c_str());
        m_pCeleX5->pauseSensor(); //puase sensor: the sensor will not output data when playback
    }
}

void CeleX5Widget::convertBin2CSV(QPushButton *pButton)
{
    m_pCeleX5->enableEventStreamModule();

    m_qstBinfilePathList.clear();
    m_qstBinfilePathList = QFileDialog::getOpenFileNames(this, "Open a bin file", QCoreApplication::applicationDirPath(), "Bin Files (*.bin)");
    //qDebug() << __FUNCTION__ << m_qstBinfilePathList << endl;
    if (m_qstBinfilePathList.size() <= 0)
        return;
    QString filePath = m_qstBinfilePathList[0];
    if (filePath.isEmpty())
        return;
    //qDebug() << __FUNCTION__ << m_qstBinfilePathList << endl;

    g_qsBinFileName = filePath;

    showPlaybackBoard(true);
    m_pSensorDataObserver->setDisplayType(ConvertBin2CSV);
    pButton->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                           "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                           "font: 20px Calibri; }"
                           "QPushButton:pressed {background: #992F6F; }");
    QString path = filePath.left(filePath.size() - 4);
    path += ".csv";
    if (m_pCeleX5->openBinFile(filePath.toLocal8Bit().data()))
    {
        //qDebug() << __FUNCTION__ << filePath << endl;
        m_qstBinfilePathList.removeAt(0);

        QList<QPushButton*> button = this->findChildren<QPushButton *>("Enter Loop Mode");

        if (m_pCeleX5->isLoopModeEnabled())
        {
            switchMode(button[0], true, true);
        }
        else
        {
            switchMode(button[0], false, true);
            m_pCbBoxFixedMode->setCurrentIndex((int)m_pCeleX5->getSensorFixedMode());
        }
        m_pReadBinTimer->start(READ_BIN_TIME);
        m_pUpdatePlayInfoTimer->start(UPDATE_PLAY_INFO_TIME);
        //
        g_ofWriteMat.open(path.toLocal8Bit().data());
        //
        m_pCeleX5->pauseSensor(); //puase sensor: the sensor will not output data when playback
    }
}

void CeleX5Widget::setCurrentPackageNo(int value)
{
    for (int i = 0; i < m_pPlaybackBg->children().size(); ++i)
    {
        CfgSlider* pWidget = (CfgSlider*)m_pPlaybackBg->children().at(i);
        if (pWidget->objectName() == "Package Count")
        {
            pWidget->updateValue(value);
            break;
        }
    }
}

void CeleX5Widget::onBtnRotateLRReleased(QPushButton* pButton)
{
    if (m_pCeleX5->getRotateType() >= 2)
    {
        m_pCeleX5->setRotateType(-2);
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F;}");
    }
    else
    {
        m_pCeleX5->setRotateType(2);
        pButton->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
    }
}

void CeleX5Widget::onBtnRotateUDReleased(QPushButton* pButton)
{
    if (m_pCeleX5->getRotateType()%2 == 1)
    {
        m_pCeleX5->setRotateType(-1);
        pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F;}");
    }
    else
    {
        m_pCeleX5->setRotateType(1);
        pButton->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                               "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                               "font: 20px Calibri; }"
                               "QPushButton:pressed {background: #992F6F; }");
    }
}

void CeleX5Widget::onButtonClicked(QAbstractButton *button)
{
    //cout << "MainWindow::onButtonClicked: " << button->objectName().toStdString() << endl;
    if ("RESET" == button->objectName())
    {
        m_pCeleX5->reset();
    }
    else if ("Generate FPN" == button->objectName())
    {
        m_pCeleX5->generateFPN("");
        if (m_emSensorFixedMode == CeleX5::Event_In_Pixel_Timestamp_Mode)
            g_bStartGenerateOFFPN = true;
    }
    else if ("Change FPN" == button->objectName())
    {
        changeFPN();
    }
    else if ("Start Recording Bin" == button->objectName())
    {
        record((QPushButton*)button);
    }
    else if ("Start Recording Images" == button->objectName())
    {
        recordImages((QPushButton*)button);
    }
    else if("Start Recording Video" == button->objectName())
    {
        recordVideo((QPushButton*)button);
    }
    else if ("Playback" == button->objectName())
    {
        playback((QPushButton*)button);
    }
    else if ("Enter Loop Mode" == button->objectName())
    {
        switchMode((QPushButton*)button, !m_pCeleX5->isLoopModeEnabled(), false);
    }
    else if ("Configurations" == button->objectName())
    {
        showCFG();
    }
    else if ("Enable Auto ISP" == button->objectName())
    {
        if (CeleX5::Full_Picture_Mode == m_pCeleX5->getSensorFixedMode() ||
                m_pCeleX5->isLoopModeEnabled())
        {
            if ("Enable Auto ISP" == button->text())
            {
                button->setText("Disable Auto ISP");
                m_pCeleX5->setAutoISPEnabled(true);
                setButtonEnable((QPushButton*)button);
            }
            else
            {
                button->setText("Enable Auto ISP");
                m_pCeleX5->setAutoISPEnabled(false);
                setButtonNormal((QPushButton*)button);
            }
        }
    }
    else if ("More Parameters ..." == button->objectName())
    {
        showMoreParameters(5);
    }
    else if ("Test: Save Pic" == button->objectName())
    {
        m_pCeleX5->saveFullPicRawData();
    }
    else if ("Rotate_LR" == button->objectName())
    {
        onBtnRotateLRReleased((QPushButton*)button);
    }
    else if ("Rotate_UD" == button->objectName())
    {
        onBtnRotateUDReleased((QPushButton*)button);
    }
    else if ("ConvertBin2Video" == button->objectName())
    {
        convertBin2Video((QPushButton*)button);
    }
    else if ("ConvertBin2CSV" == button->objectName())
    {
        convertBin2CSV((QPushButton*)button);
    }
    else if ("Enable Anti-Flashlight" == button->objectName())
    {
        m_pCeleX5->setAntiFlashlightEnabled(true);
    }
    else if ("Version" == button->objectName())
    {
        //m_pCeleDriver->writeSerialNumber("CX5-MP-0001-HXQ");
        //Sleep(1000);
        //std::string serial_number = m_pCeleX5->getSerialNumber();
        if (NULL == m_pVersionWidget)
        {
            m_pVersionWidget = new QWidget;
            m_pVersionWidget->setGeometry(600, 100, 600, 300);

            QLabel* pLabelSNum = new QLabel(m_pVersionWidget);
            pLabelSNum->setGeometry(10, 10, 500, 30);
            pLabelSNum->setText("Device Serial Number:   " + QString::fromStdString(m_pCeleX5->getSerialNumber()));
            pLabelSNum->setStyleSheet("QLabel { color: black; font: 20px Calibri; }");

            QLabel* pLabelFVer = new QLabel(m_pVersionWidget);
            //pLabelFVer->setGeometry(43, 10+40, 400, 30);
            pLabelFVer->setGeometry(10, 10+40, 500, 30);
            pLabelFVer->setText("Firmware Version:   " + QString::fromStdString(m_pCeleX5->getFirmwareVersion()));
            pLabelFVer->setStyleSheet("QLabel { color: black; font: 20px Calibri; }");

            QLabel* pLabelFDate = new QLabel(m_pVersionWidget);
            //pLabelFDate->setGeometry(66, 10+40*2, 400, 30);
            pLabelFDate->setGeometry(10, 10+40*2, 500, 30);
            pLabelFDate->setText("Firmware Date:   " + QString::fromStdString(m_pCeleX5->getFirmwareDate()));
            pLabelFDate->setStyleSheet("QLabel { color: black; font: 20px Calibri; }");

            QLabel* pLabelAPIVer = new QLabel(m_pVersionWidget);
            //pLabelAPIVer->setGeometry(94, 10+40*3, 400, 30);
            pLabelAPIVer->setGeometry(10, 10+40*3, 500, 30);
            pLabelAPIVer->setText("API Version:   2.0");
            pLabelAPIVer->setStyleSheet("QLabel { color: black; font: 20px Calibri; }");
        }
        m_pVersionWidget->show();
    }
    else if ("Advanced Setting" == button->objectName())
    {
        showAdvancedSetting();
    }
}

void CeleX5Widget::onValueChanged(uint32_t value, CfgSlider *slider)
{
    cout << "CeleX5Widget::onValueChanged: " << slider->getBiasType() << ", " << value << endl;
    if ("Clock" == slider->getBiasType())
    {
        m_pCeleX5->setClockRate(value);
        m_pSensorDataObserver->setFullFrameFPS(value);
    }
    else if ("Brightness" == slider->getBiasType())
    {
        m_pCeleX5->setBrightness(value);
    }
    else if ("Threshold" == slider->getBiasType())
    {
        m_pCeleX5->setThreshold(value);
    }
    else if ("ISO" == slider->getBiasType())
    {
        m_pCeleX5->setISOLevel(value); //the api will change fpn file according to the ISO level

        /*QString filePath = QCoreApplication::applicationDirPath() + "/FPN_" + QString::number(value) + ".txt";
        if (filePath.isEmpty())
            return;
        m_pCeleX5->setFpnFile(filePath.toStdString());*/
    }
    else if ("Event Duration" == slider->getBiasType())
    {
        m_pCeleX5->setEventDuration(value);
    }
    else if ("FullPic Num" == slider->getBiasType())
    {
        m_pCeleX5->setPictureNumber(value, CeleX5::Full_Picture_Mode);
    }
    else if ("S FO Pic Num" == slider->getBiasType())
    {
        m_pCeleX5->setPictureNumber(value, CeleX5::Optical_Flow_Mode);
    }
    else if ("M FO Pic Num" == slider->getBiasType())
    {
        m_pCeleX5->setPictureNumber(value, CeleX5::Multi_Read_Optical_Flow_Mode);
    }
    else if ("Display FPS" == slider->getBiasType())
    {
        m_pSensorDataObserver->setDisplayFPS(value);
    }
    else if ("Package Count" == slider->getBiasType())
    {
        m_pCeleX5->setCurrentPackageNo(value);
        m_pReadBinTimer->start(READ_BIN_TIME);
        m_pUpdatePlayInfoTimer->start(UPDATE_PLAY_INFO_TIME);
    }
    else if ("Event Count Step" == slider->getBiasType())
    {
        m_pCeleX5->setEventCountStepSize(value);
    }
    else if ("Brightness 1" == slider->getBiasType())
    {
        m_pCeleX5->setISPBrightness(value, 1);
    }
    else if ("Brightness 2" == slider->getBiasType())
    {
        m_pCeleX5->setISPBrightness(value, 2);
    }
    else if ("Brightness 3" == slider->getBiasType())
    {
        m_pCeleX5->setISPBrightness(value, 3);
    }
    else if ("Brightness 4" == slider->getBiasType())
    {
        m_pCeleX5->setISPBrightness(value, 4);
    }
    else if ("BRT Threshold 1" == slider->getBiasType())
    {
        m_pCeleX5->setISPThreshold(value, 1);
    }
    else if ("BRT Threshold 2" == slider->getBiasType())
    {
        m_pCeleX5->setISPThreshold(value, 2);
    }
    else if ("BRT Threshold 3" == slider->getBiasType())
    {
        m_pCeleX5->setISPThreshold(value, 3);
    }
    else if ("Frame Time" == slider->getBiasType())
    {
        m_pCeleX5->setEventFrameTime(value);
    }
    else if ("Row Cycle Count" == slider->getBiasType())
    {
        m_pCeleX5->setEventShowMethod(EventShowByRowCycle, value);
    }
    else if ("Event Count" == slider->getBiasType())
    {
        m_pCeleX5->setEventShowMethod(EventShowByCount, value);
    }
    else if ("Time Duration" == slider->getBiasType())
    {
        m_uiSaveBinDuration = value;
    }
    else if ("Count Interval" == slider->getBiasType())
    {
        m_pSensorDataObserver->setPicIntervalCount(value);
    }
    else if ("Resolution" == slider->getBiasType())
    {
        m_pCeleX5->setRowDisabled(value);
    }
    else if ("Event Start Pos" == slider->getBiasType())
    {
        m_pCeleX5->setEventFrameStartPos(value);
    }
    else if ("Event Count Density" == slider->getBiasType())
    {
        m_pCeleX5->setEventCountSliceNum(value);
    }
    else if ("Event Count Threshold" == slider->getBiasType())
    {
        g_iEventCountThreshold = value;
    }
}

void CeleX5Widget::onReadBinTimer()
{
    //cout << __FUNCTION__ << endl;
    if (m_emDeviceType == CeleX5::CeleX5_MIPI)
    {
        if (m_pCeleX5->readBinFileData())
        {
            //m_pReadBinTimer->start(READ_BIN_TIME);
            m_pReadBinTimer->stop();
        }
    }
}

void CeleX5Widget::onRecordDataTimer()
{
    if (!m_pRecordDataTimer->isActive())
        return;

    m_pCeleX5->stopRecording();
    //
    const QDateTime now = QDateTime::currentDateTime();
    const QString timestamp = now.toString(QLatin1String("yyyyMMdd_hhmmsszzz"));
    QString qstrBinName;
    if (CeleX5::CeleX5_OpalKelly != m_emDeviceType)
    {
        qstrBinName = QCoreApplication::applicationDirPath() + "/MipiData_" + timestamp;
    }
    else
    {
        qstrBinName = QCoreApplication::applicationDirPath() + "/ParaData_" + timestamp;
    }
    QStringList modeList;
    modeList << "_E_" << "_EO_" << "_EI_" << "_F_" << "_FO1_" << "_FO2_" << "_FO3_" << "_FO4_";
    if (m_pCeleX5->isLoopModeEnabled())
        qstrBinName += "_Loop_";
    else
        qstrBinName += modeList.at(int(m_pCeleX5->getSensorFixedMode()));

    qstrBinName += QString::number(m_pCeleX5->getClockRate());
    qstrBinName += "M.bin"; //MHz
    std::string filePath = qstrBinName.toStdString();
    m_pCeleX5->startRecording(filePath);
}

void CeleX5Widget::onUpdatePlayInfo()
{
    bool bAllFinished = true;
    if (m_pCeleX5->getPlaybackState() == PlayFinished)
    {
        cout << "------------- Playback Finished! -------------" << endl;
        if (m_pSensorDataObserver->getDisplayType() == ConvertBin2Video)
        {
            m_pVideoStream->avWriterRelease();
            QList<QPushButton*> button=this->findChildren<QPushButton *>("ConvertBin2Video");
            button[0]->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                     "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                     "font: 20px Calibri; }"
                                     "QPushButton:pressed {background: #992F6F;}");
            QMessageBox::information(NULL, "convertBin2Video", "Convert Bin to Video completely!", QMessageBox::Yes, QMessageBox::Yes);
        }
        else if (m_pSensorDataObserver->getDisplayType() == ConvertBin2CSV)
        {
            g_ofWriteMat.close();
            if (m_qstBinfilePathList.size() <= 0)
            {
                QList<QPushButton*> button = this->findChildren<QPushButton *>("ConvertBin2CSV");
                button[0]->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                         "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                         "font: 20px Calibri; }"
                                         "QPushButton:pressed {background: #992F6F;}");
                QMessageBox::information(NULL, "convertBin2CSV", "Convert Bin to CSV completely!", QMessageBox::Yes, QMessageBox::Yes);

                m_pCeleX5->disableEventStreamModule();
            }
            else
            {
                bAllFinished = false;
                QString filePath = m_qstBinfilePathList[0];
                if (filePath.isEmpty())
                    return;
                if (m_pCeleX5->openBinFile(filePath.toLocal8Bit().data()))
                {
                    qDebug() << __FUNCTION__ << filePath << endl;
                    m_qstBinfilePathList.removeAt(0);

                    m_pReadBinTimer->start(READ_BIN_TIME);
                    m_pUpdatePlayInfoTimer->start(UPDATE_PLAY_INFO_TIME);
                    //
                    QString path = filePath.left(filePath.size() - 4);
                    path += ".csv";
                    g_ofWriteMat.open(path.toLocal8Bit().data());
                }
            }
        }
        if (bAllFinished)
        {
            if(m_pSensorDataObserver->getDisplayType() == ConvertBin2Video ||
               m_pSensorDataObserver->getDisplayType() == ConvertBin2CSV)
            {
                m_pSensorDataObserver->setDisplayType(Realtime);
                showPlaybackBoard(false);
                m_pCeleX5->restartSensor(); //restart sensor: the sensor will output data agatin
                m_pCeleX5->setIsPlayBack(false);
                m_pCeleX5->play();
                if (!m_pCeleX5->isLoopModeEnabled())
                {
                    m_pCbBoxFixedMode->setCurrentIndex(m_emSensorFixedMode);
                    m_pCeleX5->setSensorFixedMode(m_emSensorFixedMode);
                }
                else
                {
                    m_pCeleX5->setLoopModeEnabled(true);
                    m_pCeleX5->setSensorLoopMode(CeleX5::Full_Picture_Mode, 1);
                    m_pCeleX5->setSensorLoopMode(CeleX5::Event_Off_Pixel_Timestamp_Mode, 2);
                    m_pCeleX5->setSensorLoopMode(CeleX5::Optical_Flow_Mode, 3);
                }
            }

            m_pSensorDataObserver->setSaveBmp(false);
            m_pBtnSavePic->setText("Start Saving Pic");
            m_pBtnSavePic->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                         "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                         "font: 20px Calibri; }"
                                         "QPushButton:pressed {background: #992F6F;}");
            m_pBtnSavePicEx->setText("Start Saving Pic (Replay)");
            m_pBtnSavePicEx->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                           "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                           "font: 20px Calibri; }"
                                           "QPushButton:pressed {background: #992F6F;}");
            m_pUpdatePlayInfoTimer->stop();
        }
    }
    //cout << "--------- CurrentPackageNo = " << m_pCeleX5->getCurrentPackageNo() << endl;
    int value;
    if (m_pCeleX5->getTotalPackageCount() > 0xFFFFFF)
        value = 0xFFFFFF;
    else
        value = m_pCeleX5->getTotalPackageCount();
    setSliderMaxValue(m_pPlaybackBg, "Package Count", value);
    setCurrentPackageNo(m_pCeleX5->getCurrentPackageNo());
}

void CeleX5Widget::onBtnPlayPauseReleased()
{
    if (m_bPlaybackPaused)
    {
        m_bPlaybackPaused = false;
        m_pBtnPlayPause->setStyleSheet("QPushButton {background-color: #002F6F; background-image: url(:/images/player_pause.png); "
                                       "border-style: outset; border-radius: 10px; border-color: #002F6F;} "
                                       "QPushButton:pressed {background: #992F6F; background-image: url(:/images/player_play.png); }");
        m_pCeleX5->play();
    }
    else
    {
        m_bPlaybackPaused = true;
        m_pBtnPlayPause->setStyleSheet("QPushButton {background-color: #992F6F; background-image: url(:/images/player_play.png); "
                                       "border-style: outset;  border-radius: 10px; border-color: #992F6F;} "
                                       "QPushButton:pressed {background: #992F6F; background-image: url(:/images/player_play.png); }");
        m_pCeleX5->pause();
    }
}

void CeleX5Widget::onBtnReplayRelease()
{
    m_pReadBinTimer->stop();
    setCurrentPackageNo(0);
    m_pCeleX5->replay();
    m_pReadBinTimer->start(READ_BIN_TIME);
    m_pUpdatePlayInfoTimer->start(UPDATE_PLAY_INFO_TIME);
}

void CeleX5Widget::onBtnSavePicReleased()
{
    if (m_pSensorDataObserver->isSavingBmp())
    {
        m_pSensorDataObserver->setSaveBmp(false);
        m_pBtnSavePic->setText("Start Saving Pic");
        m_pBtnSavePic->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                     "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                     "font: 20px Calibri; }"
                                     "QPushButton:pressed {background: #992F6F;}");
    }
    else
    {
        m_pSensorDataObserver->setSaveBmp(true);
        m_pBtnSavePic->setText("Stop Saving Pic");
        m_pBtnSavePic->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                                     "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                     "font: 20px Calibri; }"
                                     "QPushButton:pressed {background: #992F6F; }");
    }
}

void CeleX5Widget::onBtnSavePicExReleased()
{
    if (m_pSensorDataObserver->isSavingBmp())
    {
        m_pSensorDataObserver->setSaveBmp(false);
        m_pBtnSavePicEx->setText("Start Saving Pic (Replay)");
        m_pBtnSavePicEx->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                                       "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                       "font: 20px Calibri; }"
                                       "QPushButton:pressed {background: #992F6F;}");
    }
    else
    {
        onBtnReplayRelease();
        m_pSensorDataObserver->setSaveBmp(true);
        m_pBtnSavePicEx->setText("Stop Saving Pic (Replay)");
        m_pBtnSavePicEx->setStyleSheet("QPushButton {background: #992F6F; color: yellow; "
                                       "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                                       "font: 20px Calibri; }"
                                       "QPushButton:pressed {background: #992F6F; }");
    }
}

unsigned char * event_buffer = new unsigned char[1024000];
void CeleX5Widget::onUpdateEventCountPlot()
{
    m_pCeleX5->getEventPicBuffer(event_buffer, CeleX5::EventCountDensityPic);

    uint64_t total = 0;
    uint32_t count = 0;
    for (int i = 0; i < 1024000; i++)
    {
        if (event_buffer[i] > 0)
        {
            total += (int)event_buffer[i];
            count++;
        }
    }
    if (count == 0)
        return;
    g_iEventCountMean = total / count;
    //cout << "g_iEventCountMean = " << g_iEventCountMean << endl;

    for (int i = 0; i < 1280; i++)
    {
        g_vecEventCountPerCol[i] = 0;
        if (i < 800)
            g_vecEventCountPerRow[i] = 0;
    }

    int j = 0;
    int k = 0;
    for (int i = 0; i < 1024000; i++)
    {
        if (event_buffer[i] > 0)
        {
            j = i % 1280;
            k = 799 - i / 1280;
            g_vecEventCountPerCol[j] += event_buffer[i];
            g_vecEventCountPerRow[k] += event_buffer[i];
        }
    }

    // create graph and assign data to it:
    if(!m_pSensorDataObserver->getMultipleShowEnable())
    {
        m_pColPlotWidget->graph(0)->setData(g_vecEventCol, g_vecEventCountPerCol);
        m_pColPlotWidget->replot();

        m_pRowPlotWidget->graph(0)->setData(g_vecEventRow, g_vecEventCountPerRow);
        m_pRowPlotWidget->replot();
    }
}

void CeleX5Widget::onBtnSaveBinReleased()
{
    m_pCeleX5->saveBinFile(g_qsBinFileName.toStdString(), m_iPackageCountBegin, m_iPackageCountEnd);
}

void CeleX5Widget::onBinBeginFinished()
{
    QList<QLineEdit*> slider = m_pPlaybackBg->findChildren<QLineEdit *>("PackageCountBegin");
    if (slider.size() > 0)
    {
        if (slider[0]->hasFocus())
        {
            this->setFocus();
        }
        else
        {
            m_iPackageCountBegin = slider[0]->text().toInt();
            cout << "m_iPackageCountBegin = " << m_iPackageCountBegin << endl;
        }
    }
}

void CeleX5Widget::onBinEndFinished()
{
    QList<QLineEdit*> slider = m_pPlaybackBg->findChildren<QLineEdit *>("PackageCountEnd");
    if (slider.size() > 0)
    {
        if (slider[0]->hasFocus())
        {
            this->setFocus();
        }
        else
        {
            m_iPackageCountEnd = slider[0]->text().toInt();
            cout << "m_iPackageCountEnd = " << m_iPackageCountEnd << endl;
        }
    }
}

void CeleX5Widget::onSensorModeChanged(QString text)
{
    cout << text.toStdString() << endl;
    if (m_pCeleX5->isLoopModeEnabled())
    {
        int loopNum = 0;
        if (text.contains("LoopA"))
            loopNum = 1;
        else if (text.contains("LoopB"))
            loopNum = 2;
        else if (text.contains("LoopC"))
            loopNum = 3;

        QString mode = text.mid(8);
        //cout << loopNum << ", " << mode.toStdString() << endl;
        if (mode == "Event_Off_Pixel_Timestamp Mode")
        {
            m_pCeleX5->setSensorLoopMode(CeleX5::Event_Off_Pixel_Timestamp_Mode, loopNum);
            if(loopNum == 2)
            {
                m_pCbBoxLoopEventType->clear();
                m_pCbBoxLoopEventType->insertItem(0, "Event Binary Pic");
                m_pCbBoxLoopEventType->insertItem(1, "Event Denoised Binary Pic");
                m_pCbBoxLoopEventType->insertItem(2, "Event Count Pic");
                m_pCbBoxLoopEventType->insertItem(3, "Event Denoised Count Pic");
                m_pCbBoxLoopEventType->setCurrentIndex(0);
            }
        }
        else if (mode == "Event_In_Pixel_Timestamp Mode")
        {
            m_pCeleX5->setSensorLoopMode(CeleX5::Event_In_Pixel_Timestamp_Mode, loopNum);
            if(loopNum == 2)
            {
                m_pCbBoxLoopEventType->clear();
                m_pCbBoxLoopEventType->insertItem(0, "Event OpticalFlow Pic");
                m_pCbBoxLoopEventType->insertItem(1, "Event Binary Pic");
                m_pCbBoxLoopEventType->setCurrentIndex(0);
            }
        }
        else if (mode == "Event_Intensity Mode")
        {
            m_pCeleX5->setSensorLoopMode(CeleX5::Event_Intensity_Mode, loopNum);
            if(loopNum == 2)
            {
                m_pCbBoxLoopEventType->clear();
                m_pCbBoxLoopEventType->insertItem(0, "Event Binary Pic");
                m_pCbBoxLoopEventType->insertItem(1, "Event Gray Pic");
                m_pCbBoxLoopEventType->insertItem(2, "Event Accumulated Pic");
                m_pCbBoxLoopEventType->insertItem(3, "Event Count Pic");
                m_pCbBoxLoopEventType->setCurrentIndex(0);
            }
        }
        else if (mode == "Full_Picture Mode")
            m_pCeleX5->setSensorLoopMode(CeleX5::Full_Picture_Mode, loopNum);
        else if (mode == "Optical_Flow Mode")
            m_pCeleX5->setSensorLoopMode(CeleX5::Optical_Flow_Mode, loopNum);
        else if (mode == "Multi_Read_Optical_Flow Mode")
            m_pCeleX5->setSensorLoopMode(CeleX5::Multi_Read_Optical_Flow_Mode, loopNum);
    }
    else
    {
        QString mode = text.mid(8);
        //cout << mode.toStdString() << endl;
        if (mode == "Event_Off_Pixel_Timestamp Mode")
        {
            m_pCeleX5->setSensorFixedMode(CeleX5::Event_Off_Pixel_Timestamp_Mode);
            m_pCbBoxImageType->clear();
            m_pCbBoxImageType->insertItem(0, "Event Binary Pic");
            m_pCbBoxImageType->insertItem(1, "Event Denoised Binary Pic");
            m_pCbBoxImageType->insertItem(2, "Event Count Pic");
            m_pCbBoxImageType->insertItem(3, "Event Denoised Count Pic");
            m_pCbBoxImageType->insertItem(4, "Event CountDensity Pic");
            m_pCbBoxImageType->setCurrentIndex(0);
            m_pSensorDataObserver->setMultipleShowEnabled(false);
            m_pBtnShowStyle->setText("Show Multiple Windows");
        }
        else if (mode == "Event_In_Pixel_Timestamp Mode")
        {
            m_pCeleX5->setSensorFixedMode(CeleX5::Event_In_Pixel_Timestamp_Mode);
            m_pCbBoxImageType->clear();
            m_pCbBoxImageType->insertItem(0, "Event OpticalFlow Pic");
            m_pCbBoxImageType->insertItem(1, "Event Binary Pic");
            m_pCbBoxImageType->insertItem(2, "Event CountDensity Pic");
            m_pCbBoxImageType->insertItem(3, "Event Denoised Binary Pic");
            m_pCbBoxImageType->insertItem(4, "Event Count Pic");
            m_pCbBoxImageType->insertItem(5, "Event Denoised Count Pic");
        }
        else if (mode == "Event_Intensity Mode")
        {
            m_pCeleX5->setSensorFixedMode(CeleX5::Event_Intensity_Mode);
            m_pCbBoxImageType->clear();
            m_pCbBoxImageType->insertItem(0, "Event Binary Pic");
            m_pCbBoxImageType->insertItem(1, "Event Gray Pic");
            m_pCbBoxImageType->insertItem(2, "Event Accumulated Pic");
            m_pCbBoxImageType->insertItem(3, "Event Superimposed Pic");
            m_pCbBoxImageType->insertItem(4, "Event Count Pic");
            m_pCbBoxImageType->insertItem(5, "Event CountDensity Pic");
            m_pCbBoxImageType->setCurrentIndex(1);
        }
        else if (mode == "Full_Picture Mode")
        {
            m_pCeleX5->setSensorFixedMode(CeleX5::Full_Picture_Mode);
            m_pCbBoxImageType->clear();
            m_pCbBoxImageType->insertItem(0, "Full Pic");

            m_pSensorDataObserver->setMultipleShowEnabled(false);
            m_pBtnShowStyle->setText("Show Multiple Windows");
        }
        else if (mode == "Optical_Flow Mode")
        {
            m_pCeleX5->setSensorFixedMode(CeleX5::Optical_Flow_Mode);
            m_pCbBoxImageType->clear();
            m_pCbBoxImageType->insertItem(0, "Full OpticalFlow Pic");
            m_pCbBoxImageType->insertItem(1, "Full OpticalFlow Speed Pic");
            m_pCbBoxImageType->insertItem(2, "Full OpticalFlow Direction Pic");
            m_pCbBoxImageType->setCurrentIndex(0);

            m_pSensorDataObserver->setMultipleShowEnabled(true);
            m_pBtnShowStyle->setText("Show Single Windows");
        }
        else if (mode == "Multi_Read_Optical_Flow Mode")
        {
            m_pCeleX5->setSensorFixedMode(CeleX5::Multi_Read_Optical_Flow_Mode);
            m_pCbBoxImageType->clear();
            m_pCbBoxImageType->insertItem(0, "Full OpticalFlow Pic");
            m_pCbBoxImageType->insertItem(1, "Full OpticalFlow Speed Pic");
            m_pCbBoxImageType->insertItem(2, "Full OpticalFlow Direction Pic");
            m_pCbBoxImageType->setCurrentIndex(0);
        }
        else if (mode == "Optical_Flow_FPN Mode")
        {
            m_pCeleX5->setSensorFixedMode(CeleX5::Optical_Flow_FPN_Mode);
            m_pCbBoxImageType->clear();
            m_pCbBoxImageType->insertItem(0, "Full OpticalFlow Pic");
            m_pCbBoxImageType->insertItem(1, "Full OpticalFlow Speed Pic");
            m_pCbBoxImageType->insertItem(2, "Full OpticalFlow Direction Pic");
            m_pCbBoxImageType->setCurrentIndex(0);
        }
        if (m_pSensorDataObserver->getDisplayType() == Realtime)
        {
            m_emSensorFixedMode = m_pCeleX5->getSensorFixedMode();
        }
    }
    m_pArrowLeft->hide();
    m_pArrowRight->hide();
    m_pArrowUp->hide();
    m_pArrowDown->hide();
}

void CeleX5Widget::onImageTypeChanged(int index)
{
    cout << "CeleX5Widget::onImageTypeChanged: " << index << endl;
    m_iCurrCbBoxImageType = index;
    m_pSensorDataObserver->setPictureMode(index);
    if (m_pCbBoxImageType->currentText() == "Event CountDensity Pic")
    {
        if(!m_pSensorDataObserver->getMultipleShowEnable())
        {
            m_pColPlotWidget->show();
            m_pPlotGraphicsView->show();
            m_pUpdateSlotTimer->start(50);
        }
        else
        {
            m_pColPlotWidget->hide();
            m_pPlotGraphicsView->hide();
        }
    }
    else
    {
        m_pUpdatePlayInfoTimer->stop();
        m_pColPlotWidget->hide();
        m_pPlotGraphicsView->hide();
    }
}

void CeleX5Widget::onLoopEventTypeChanged(int index)
{
    cout << "CeleX5Widget::onLoopEventTypeChanged: " << index << endl;
    m_iCurrCbBoxLoopImageType = index;
    m_pSensorDataObserver->setPictureMode(index);
}

void CeleX5Widget::onShowMultipleWindows()
{
    if ("Show Multiple Windows" == m_pBtnShowStyle->text())
    {
        m_pSensorDataObserver->setMultipleShowEnabled(true);
        m_pBtnShowStyle->setText("Show Single Windows");
        m_pColPlotWidget->hide();
        m_pPlotGraphicsView->hide();
    }
    else
    {
        m_pSensorDataObserver->setMultipleShowEnabled(false);
        m_pBtnShowStyle->setText("Show Multiple Windows");
        if (m_pCbBoxImageType->currentText() == "Event CountDensity Pic")
        {
            m_pColPlotWidget->show();
            m_pPlotGraphicsView->show();
            m_pUpdateSlotTimer->start(50);
        }
    }
}

void CeleX5Widget::onEventShowTypeChanged(int index)
{
    cout << __FUNCTION__ << ": " << index << endl;
    if (0 == index) //show by time
    {
        m_pFrameSlider->show();
        m_pEventCountSlider->hide();
        m_pRowCycleSlider->hide();
        m_pCeleX5->setEventShowMethod(EventShowByTime, m_pFrameSlider->getValue());
    }
    else if (1 == index) //show by event count
    {
        m_pFrameSlider->hide();
        m_pEventCountSlider->show();
        m_pRowCycleSlider->hide();
        m_pCeleX5->setEventShowMethod(EventShowByCount, m_pEventCountSlider->getValue());
    }
    else if (2 == index)
    {
        m_pFrameSlider->hide();
        m_pEventCountSlider->hide();
        m_pRowCycleSlider->show();
        m_pCeleX5->setEventShowMethod(EventShowByRowCycle, m_pRowCycleSlider->getValue());
    }
}

void CeleX5Widget::onShowImagesSwitch(bool state)
{
    cout << "CeleX5Widget::onShowImagesSwitch: state = " << state << endl;
    g_bShowImageEndabled = state;
    m_pCeleX5->setShowImagesEnabled(state);
    QList<QRadioButton*> radio1 = m_pAdSettingWidget->findChildren<QRadioButton *>("Display Switch");
    if (state)
    {
        if (radio1.size() > 0)
        {
            radio1[0]->setText(" open");
            radio1[0]->setStyleSheet("QRadioButton {background: transparent; color: #990000; font: 20px Calibri; }");
        }
    }
    else
    {
        if (radio1.size() > 0)
        {
            radio1[0]->setText(" close");
            radio1[0]->setStyleSheet("QRadioButton {background: transparent; color: gray; font: 20px Calibri; }");
        }
    }
}

void CeleX5Widget::onJPGFormatClicked(bool state)
{
    g_qsPicFormat = "JPG";
    QList<QRadioButton*> radioJPG = m_pAdSettingWidget->findChildren<QRadioButton *>("JPG");
    if (radioJPG.size() > 0)
    {
        radioJPG[0]->setStyleSheet("QRadioButton {background: transparent; color: #990000; font: 20px Calibri; }");
    }

    QList<QRadioButton*> radioBMP = m_pAdSettingWidget->findChildren<QRadioButton *>("BMP");
    if (radioBMP.size() > 0)
    {
        radioBMP[0]->setChecked(false);
        radioBMP[0]->setStyleSheet("QRadioButton {background: transparent; color: gray; font: 20px Calibri; }");
    }
}

void CeleX5Widget::onBMPFormatClicked(bool state)
{
    g_qsPicFormat = "BMP";
    QList<QRadioButton*> radioBMP = m_pAdSettingWidget->findChildren<QRadioButton *>("BMP");
    if (radioBMP.size() > 0)
    {
        radioBMP[0]->setStyleSheet("QRadioButton {background: transparent; color: #990000; font: 20px Calibri; }");
    }

    QList<QRadioButton*> radioJPG = m_pAdSettingWidget->findChildren<QRadioButton *>("JPG");
    if (radioJPG.size() > 0)
    {
        radioJPG[0]->setChecked(false);
        radioJPG[0]->setStyleSheet("QRadioButton {background: transparent; color: gray; font: 20px Calibri; }");
    }
}

void CeleX5Widget::onShowMoreParameters()
{
    showMoreParameters(5);
    m_pAdSettingWidget->hide();
}

QPushButton *CeleX5Widget::createButton(QString text, QRect rect, QWidget *parent)
{
    QPushButton* pButton = new QPushButton(text, parent);
    pButton->setGeometry(rect);

    pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                           "border-style: outset; border-width: 2px; border-radius: 10px; "
                           "font: 20px Calibri; }"
                           "QPushButton:pressed {background: #992F6F; }"
                           "QPushButton:disabled {background: #777777; color: lightgray;}");
    return pButton;
}

SliderWidget *CeleX5Widget::createSlider(CeleX5::CfgInfo cfgInfo, int value, QRect rect, QWidget *parent, QWidget *widgetSlot)
{
    SliderWidget* pSlider = new SliderWidget(parent, cfgInfo.min, cfgInfo.max, cfgInfo.step, value, widgetSlot, false);
    pSlider->setGeometry(rect);
    pSlider->setBiasType(cfgInfo.name);
    pSlider->setDisplayName(QString::fromStdString(cfgInfo.name));
    pSlider->setBiasAddr(cfgInfo.highAddr, cfgInfo.middleAddr, cfgInfo.lowAddr);
    pSlider->setObjectName(QString::fromStdString(cfgInfo.name));
    pSlider->setDisabled(true);
    if (value < 0)
        pSlider->setLineEditText("--");
    return pSlider;
}

void CeleX5Widget::setButtonEnable(QPushButton *pButton)
{
    pButton->setStyleSheet("QPushButton {background: #008800; color: white; "
                           "border-style: outset; border-width: 2px; border-radius: 10px; "
                           "font: 20px Calibri; }"
                           "QPushButton:pressed {background: #992F6F; }"
                           "QPushButton:disabled {background: #777777; color: lightgray;}");
}

void CeleX5Widget::setButtonNormal(QPushButton *pButton)
{
    pButton->setStyleSheet("QPushButton {background: #002F6F; color: white; "
                           "border-style: outset; border-width: 2px; border-radius: 10px; border-color: #002F6F; "
                           "font: 20px Calibri; }"
                           "QPushButton:pressed {background: #992F6F;}");
}

void CeleX5Widget::showMoreParameters(int index)
{
    if (!m_pCeleX5Cfg)
    {
        m_pCeleX5Cfg = new CeleX5Cfg(m_pCeleX5);
        //m_pCeleX5Cfg->setTestWidget(this);
    }
    m_pCeleX5Cfg->setCurrentIndex(index);
    m_pCeleX5Cfg->raise();
    m_pCeleX5Cfg->show();
}

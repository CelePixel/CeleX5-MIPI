#ifndef CELEX5WIDGET_H
#define CELEX5WIDGET_H

#include "sliderwidget.h"
#include "cfgslider.h"
#include "celex5cfg.h"
#include "doubleslider.h"
#include "./include/celex5/celex5.h"
#include "./include/celex5/celex5datamanager.h"
#include "videostream.h"
#include <QTime>
#include <QMessageBox>
#include <QRadioButton>

#ifdef _WIN32
#include <windows.h>
#else
#include<unistd.h>
#endif

using namespace std;

#pragma execution_character_set("utf-8")

enum DisplayType {
    Realtime = 0,
    Playback = 1,
    ConvertBin2Video = 2,
    ConvertBin2CSV = 3
};

class QLabel;
class MainWindow;
class QComboBox;
class SensorDataObserver : public QWidget, public CeleX5DataManager
{
    Q_OBJECT
public:
    SensorDataObserver(CX5SensorDataServer* sensorData, QWidget *parent);
    ~SensorDataObserver();
    virtual void onFrameDataUpdated(CeleX5ProcessedData* pSensorData); //overrides Observer operation
    void setCeleX5(CeleX5* pCeleX5);
    void setLoopModeEnabled(bool enable);
    void setPictureMode(int picMode);
    void setDisplayFPS(int count);
    void setFullFrameFPS(uint16_t value);
    void setMultipleShowEnabled(bool enable);
    void setDisplayType(DisplayType type);
    DisplayType getDisplayType(){ return m_emDisplayType; }
    void setSaveBmp(bool save);
    bool isSavingBmp();

private:
    void updateImage(unsigned char* pBuffer1, CeleX5::CeleX5Mode mode, int loopNum, int colorMode);
    void savePics();

protected:
    void paintEvent(QPaintEvent *event);

protected slots:
    void onUpdateImage();

private:
    QImage         m_imageMode1;
    QImage         m_imageMode2;
    QImage         m_imageMode3;
    QImage         m_imageMode4;

    CX5SensorDataServer*    m_pSensorData;
    CeleX5*                 m_pCeleX5;
    bool                    m_bLoopModeEnabled;
    int                     m_iPicMode;
    bool                    m_bShowMultipleWindows; //for fixed mode
    uint16_t                m_uiTemperature;
    uint16_t                m_uiFullFrameFPS;
    uint16_t                m_uiRealFullFrameFPS;
    int                     m_iFPS;
    int                     m_iPlaybackFPS;
    uchar*                  m_pBuffer[4];
    QTimer*                 m_pUpdateTimer;
    DisplayType             m_emDisplayType;
    bool                    m_bSavePics;
    long                    m_lFrameCount;
};

class QHBoxLayout;
class QAbstractButton;
class QPushButton;
class QButtonGroup;
class CeleX5Widget : public QWidget
{
    Q_OBJECT
public:
    explicit CeleX5Widget(QWidget *parent = 0);
    ~CeleX5Widget();
    void closeEvent(QCloseEvent *event);

private:
    void playback(QPushButton* pButton);
    QComboBox *createModeComboBox(QString text, QRect rect, QWidget* parent, bool bLoop, int loopNum);
    void createButtons(QHBoxLayout* layout);
    void changeFPN();
    void record(QPushButton* pButton);
    void recordVideo(QPushButton* pButton);
    void switchMode(QPushButton* pButton,bool isLoopMode);
    void showCFG();
    void setSliderMaxValue(QWidget* parent, QString objName, int value);
    int  getSliderMax(QWidget* parent, QString objName);
    void showPlaybackBoard(bool show);

    QPushButton* createButton(QString text, QRect rect, QWidget *parent);
    SliderWidget* createSlider(CeleX5::CfgInfo cfgInfo, int value, QRect rect, QWidget *parent, QWidget *widgetSlot);
    void setButtonEnable(QPushButton* pButton);
    void setButtonNormal(QPushButton* pButton);
    void showMoreParameters(int index);
    void onBtnRotateLRReleased(QPushButton* pButton);
    void onBtnRotateUDReleased(QPushButton* pButton);
    void convertBin2Video(QPushButton* pButton);
    void convertBin2CSV(QPushButton* pButton);

signals:

protected slots:
    void onButtonClicked(QAbstractButton* button);
    void onRadioButtonClicked();
    //
    void onValueChanged(uint32_t value, CfgSlider* slider);
    //
    void onPipeoutDataTimer();
    void onReadBinTimer();
    void onUpdatePlayInfo();
    void onBtnSavePicReleased();
    //
    void onSensorModeChanged(QString text);
    void onImageTypeChanged(int index);
    void onShowMultipleWindows();
    void onEventShowTypeChanged(int index);

private:
    CeleX5*             m_pCeleX5;
    QWidget*            m_pCFGWidget;
    QButtonGroup*       m_pButtonGroup;
    //
    QComboBox*          m_pCbBoxFixedMode;
    QComboBox*          m_pCbBoxLoopAMode;
    QComboBox*          m_pCbBoxLoopBMode;
    QComboBox*          m_pCbBoxLoopCMode;
    //
    QComboBox*          m_pCbBoxImageType;
    QPushButton*        m_pBtnShowStyle;
    //
    QWidget*            m_pPlaybackBg;
    DoubleSlider*       m_pSliderPlayer;
    CfgSlider*          m_pFPSSlider;
    CfgSlider*          m_pFrameSlider;
    CfgSlider*          m_pEventCountSlider;
    CfgSlider*          m_pRowCycleSlider;
    QPushButton*        m_pBtnSavePic;
    QButtonGroup*       m_pBtnGroup;
    CeleX5Cfg*          m_pCeleX5Cfg;

    QLabel*             m_pLabelCurrentTime;
    QLabel*             m_pLabelEndTime;
    QTime               m_timeCurrent;
    //
    QTimer*             m_pPipeOutDataTimer;
    QTimer*             m_pReadBinTimer;
    QTimer*             m_pUpdateTimer;

    SensorDataObserver* m_pSensorDataObserver;
    map<string, vector<CeleX5::CfgInfo>> m_mapCfgDefault;

    CeleX5::DeviceType  m_emDeviceType;
    CeleX5::CeleX5Mode  m_emSensorFixedMode;
    CeleX5::CeleX5Mode  m_emSensorLoopMode;
};

#endif // CELEX5WIDGET_H

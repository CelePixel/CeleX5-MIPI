#include "configwidget.h"
#include "sliderwidget.h"
#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

ConfigWidget::ConfigWidget(CelexSensorDLL* pCelexSensor, QWidget *parent)
    : QWidget(parent)
    , m_pCelexSensor(pCelexSensor)
{
    this->setWindowTitle("Sensor Config");
    this->move(600, 70);

    //---------------------- Full-Picture Mode ----------------------
    QGroupBox* configGroup = new QGroupBox("Full-Picture Mode", this);
    configGroup->setGeometry(10, 10, 1200, 120);
    configGroup->setStyleSheet("font: 20px Calibri; color: #000099;");

    SliderWidget* pSlider1 = new SliderWidget(this, 0, 1023, 1, 263, this, false);
    pSlider1->setGeometry(20, 30, 580, 50);
    pSlider1->setBiasType("BrightnessF");
    pSlider1->setDisplayName("Brightness: ");

    SliderWidget* pSlider2 = new SliderWidget(this, 10, 900, 1, 263, this, false);
    pSlider2->setGeometry(20, 80, 580, 50);
    pSlider2->setBiasType("ContrastF");
    pSlider2->setDisplayName("Contrast:   ");

    SliderWidget* pSlider3 = new SliderWidget(this, 1, 10, 1, 1, this, false);
    pSlider3->setGeometry(620, 50, 580, 50);
    pSlider3->setBiasType("FullPic FrameCount");
    pSlider3->setDisplayName("Frame-Count: ");
    pSlider3->setObjectName("FullPic FrameCount");

    //---------------------- Event Mode ----------------------
    QGroupBox* configGroup1 = new QGroupBox("Event Mode", this);
    configGroup1->setGeometry(10, 150, 1200, 170);
    configGroup1->setStyleSheet("font: 20px Calibri; color: #000099;");

    SliderWidget* pSlider11 = new SliderWidget(this, 0, 1023, 1, 263, this, false);
    pSlider11->setGeometry(20, 30+140, 580, 50);
    pSlider11->setBiasType("BrightnessE");
    pSlider11->setDisplayName("Brightness: ");

    SliderWidget* pSlider12 = new SliderWidget(this, 10, 900, 1, 342, this, false);
    pSlider12->setGeometry(20, 80+140, 580, 50);
    pSlider12->setBiasType("ContrastE");
    pSlider12->setDisplayName("Contrast:   ");

    SliderWidget* pSlider13 = new SliderWidget(this, 25, 200, 1, 40, this, false);
    pSlider13->setGeometry(20, 130+140, 580, 50);
    pSlider13->setBiasType("ThresholdE");
    pSlider13->setDisplayName("Threshold: ");

    SliderWidget* pSlider14 = new SliderWidget(this, 20, 1000, 1, 40, this, false);
    pSlider14->setGeometry(620, 220, 580, 50);
    pSlider14->setBiasType("Event FrameTime");
    pSlider14->setDisplayName("Frame-Time: ");
    pSlider14->setObjectName("Event FrameTime");

    //---------------------- Optical-Flow Mode ----------------------
    QGroupBox* configGroup2 = new QGroupBox("Optical-Flow Mode", this);
    configGroup2->setGeometry(10, 330, 1200, 200);
    configGroup2->setStyleSheet("font: 20px Calibri; color: #000099;");

    SliderWidget* pSlider21 = new SliderWidget(this, 0, 1023, 1, 263, this, false);
    pSlider21->setGeometry(20, 30+320, 580, 50);
    pSlider21->setBiasType("BrightnessO");
    pSlider21->setDisplayName("Brightness: ");

    SliderWidget* pSlider22 = new SliderWidget(this, 10, 900, 1, 342, this, false);
    pSlider22->setGeometry(20, 80+320, 580, 50);
    pSlider22->setBiasType("ContrastO");
    pSlider22->setDisplayName("Contrast:   ");

    SliderWidget* pSlider23 = new SliderWidget(this, 25, 200, 1, 80, this, false);
    pSlider23->setGeometry(20, 130+320, 580, 50);
    pSlider23->setBiasType("ThresholdO");
    pSlider23->setDisplayName("Threshold: ");

    SliderWidget* pSlider24 = new SliderWidget(this, 20, 1000, 1, 100, this, false);
    pSlider24->setGeometry(620, 400, 580, 50);
    pSlider24->setBiasType("Optical FrameTime");
    pSlider24->setDisplayName("Optical-Time: ");
    pSlider24->setObjectName("Optical FrameTime");

    //------------ for test ------------
    createResetSlider();
}

void ConfigWidget::createResetSlider()
{
    //---------------------- Reset-Time ----------------------
    QGroupBox* configGroup = new QGroupBox("Reset Time", this);
    configGroup->setGeometry(10, 570, 600, 180);
    configGroup->setStyleSheet("font: 20px Calibri; color: #000099;");

    SliderWidget* pSlider1 = new SliderWidget(this, 0, 20000, 1, 1024, this, false);
    pSlider1->setGeometry(20, 600, 580, 50);
    pSlider1->setBiasType("ResetTimeF");
    pSlider1->setDisplayName("Full-Picture: ");

    SliderWidget* pSlider2 = new SliderWidget(this, 10, 20000, 1, 2, this, false);
    pSlider2->setGeometry(20, 650, 580, 50);
    pSlider2->setBiasType("ResetTimeE");
    pSlider2->setDisplayName("Event: ");

    SliderWidget* pSlider3 = new SliderWidget(this, 10, 20000, 1, 16384, this, false);
    pSlider3->setGeometry(20, 700, 580, 50);
    pSlider3->setBiasType("ResetTimeO");
    pSlider3->setDisplayName("Optical-Flow: ");
}

void ConfigWidget::onValueChanged(uint32_t value, SliderWidget *slider)
{
    qDebug() << QString::fromStdString(slider->getBiasType()) << value;
    /*if ("BrightnessF" == slider->getBiasType())
    {
        m_pCelexSensor->setBrightness(value, FullPictureMode);
    }
    else if ("ContrastF" == slider->getBiasType())
    {
        m_pCelexSensor->setContrast(value, FullPictureMode);;
    }
    else if ("BrightnessE" == slider->getBiasType())
    {
        m_pCelexSensor->setBrightness(value, EventMode);
    }
    else if ("ContrastE" == slider->getBiasType())
    {
        m_pCelexSensor->setContrast(value, EventMode);
    }
    else if ("ThresholdE" == slider->getBiasType())
    {
        m_pCelexSensor->setThreshold(value, EventMode);
    }
    else if ("BrightnessO" == slider->getBiasType())
    {
        m_pCelexSensor->setBrightness(value, FullPic_Event_Mode);
    }
    else if ("ContrastO" == slider->getBiasType())
    {
        m_pCelexSensor->setContrast(value, FullPic_Event_Mode);
    }
    else if ("ThresholdO" == slider->getBiasType())
    {
        m_pCelexSensor->setThreshold(value, FullPic_Event_Mode);
    }
    //------ Frame Length ------
    else if ("FullPic FrameCount" == slider->getBiasType())
    {
        m_pCelexSensor->setFullPicFrameCount(value);
    }
    else if ("Event FrameTime" == slider->getBiasType())
    {
        m_pCelexSensor->setEventFrameLength(value);
    }
    else if ("Optical FrameTime" == slider->getBiasType())
    {
        m_pCelexSensor->setOpticalFrameLength(value);
    }
    //------ Reset Time ------
    else if ("ResetTimeF" == slider->getBiasType())
    {
        m_pCelexSensor->setLoopResetTimePeriod(0, value);
    }
    else if ("ResetTimeE" == slider->getBiasType())
    {
        m_pCelexSensor->setLoopResetTimePeriod(1, value);
    }
    else if ("ResetTimeO" == slider->getBiasType())
    {
        m_pCelexSensor->setLoopResetTimePeriod(2, value);
    }*/
}

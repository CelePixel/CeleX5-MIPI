#include "settingswidget.h"
#include "sliderwidget.h"
#include "cfgslider.h"
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QRadioButton>
#include <iostream>
#include <QDebug>

using namespace std;

SettingsWidget::SettingsWidget(CeleX5 *pCeleX5, QWidget *parent)
    : QWidget()
    , m_pCeleX5(pCeleX5)
    , m_pGroupBControl(NULL)
    , m_pGroupLoop(NULL)
    , m_pGroupAutoISP(NULL)
{
    this->setWindowTitle("Configurations");
    this->setStyleSheet("background-color: lightgray; ");
    //
    createTapWidget1(this, parent);
    this->setGeometry(10, 40, 1350, 950);

//    m_pTabWidget = new QTabWidget;
//    m_pTabWidget->setStyleSheet("QTabWidget::pane {border-width: 1px; border-color: rgb(48, 104, 151);\
//                                                   border-style: outset; background-color: rgb(132, 171, 208);} \
//                                QTabBar::tab {min-height: 30px; font: 18px Calibri;} \
//                                QTabBar::tab:selected{border-color: white; background: #002F6F; color: white;}");

//    QStringList cfgList;
//    cfgList << "Basic Controls" << "Device Info" << "Others";
//    for (int i = 0; i < cfgList.size(); i++)
//    {
//        QWidget* widget = new QWidget();
//        QString tapName = "       " + cfgList[i] + "       ";
//        m_pTabWidget->addTab(widget, tapName);
//        if (i == 0)
//        {
//            createTapWidget1(widget);
//        }
//        else if (i == 1)
//        {
//            createTapWidget2(widget);
//        }
//        else if (i == 2)
//        {
//            createTapWidget3(widget);;
//        }
//    }
//    QHBoxLayout *layout = new QHBoxLayout();
//    layout->addWidget(m_pTabWidget);

//    this->setLayout(layout);
}

void SettingsWidget::setCurrentIndex(int index)
{
    for (int i = 0; i < m_csrTypeList.size(); i++)
        updateCfgParameters(i);
    m_pTabWidget->setCurrentIndex(index);
}

void SettingsWidget::updateCfgs()
{
    QList<CfgSlider*> slider1 = this->findChildren<CfgSlider *>("Clock");
    if (slider1.size() > 0)
        slider1[0]->updateValue(m_pCeleX5->getClockRate());

    QList<CfgSlider*> slider2 = this->findChildren<CfgSlider *>("Threshold");
    if (slider2.size() > 0)
        slider2[0]->updateValue(m_pCeleX5->getThreshold());
}

void SettingsWidget::resizeEvent(QResizeEvent *)
{
    //cout << "SettingsWidget::resizeEvent: width = " << this->width() << ", height = " << this->height() << endl;

    int left1 = 10, top1 = 10;
    int widthGroup = (this->width()-60)/2;
    int heightGroup = (this->height()-60)/2;
    m_pGroupBControl->setGeometry(left1, top1, widthGroup, heightGroup);

    QStringList cfgList1;
    cfgList1 << "Clock" << "Brightness" << "Threshold" << "ISO";
    int sliderHeight = (m_pGroupBControl->height() / 5+5);
    top1 += 30;
    for (int i = 0; i < cfgList1.size(); i++)
    {
        QList<CfgSlider*> slider = this->findChildren<CfgSlider *>(cfgList1[i]);
        if (slider.size() > 0)
            slider[0]->setGeometry(left1+10, top1+i*(sliderHeight+5), m_pGroupBControl->width()-50, sliderHeight);
    }

    int left2 = left1+m_pGroupBControl->width()+20;
    int top2 = 10;
    m_pGroupLoop->setGeometry(left2, top2, widthGroup, heightGroup);
    QStringList cfgList2;
    cfgList2 << "Event Duration" << "FullPic Num" << "S FO Pic Num"  << "M FO Pic Num";
    top2 += 30;
    for (int i = 0; i < cfgList2.size(); i++)
    {
        QList<CfgSlider*> slider = this->findChildren<CfgSlider *>(cfgList2[i]);
        if (slider.size() > 0)
            slider[0]->setGeometry(left2+10, top2+i*(sliderHeight+5), m_pGroupBControl->width()-50, sliderHeight);
    }

    int left3 = 10;
    int top3 = top1+m_pGroupBControl->height();
    m_pGroupAutoISP->setGeometry(left3, top3, widthGroup*2+20, heightGroup);
    QStringList cfgList3;
    cfgList3 << "Brightness 1" << "Brightness 2" << "Brightness 3" << "Brightness 4"
             << "BRT Threshold 1" << "BRT Threshold 2" << "BRT Threshold 3";
    top3 += 30;
    int index = 0;
    for (int i = 0; i < cfgList3.size(); i++)
    {
        if (i == 4)
        {
            left3 = left2;
            top3 = top1+m_pGroupBControl->height()+30;
            index = 0;
        }
        QList<CfgSlider*> slider = this->findChildren<CfgSlider *>(cfgList3[i]);
        if (slider.size() > 0)
            slider[0]->setGeometry(left3+10, top3+index*(sliderHeight+5), m_pGroupBControl->width()-50, sliderHeight);
        index++;
    }
}

void SettingsWidget::updateCfgParameters(int index)
{
    QObjectList objList = m_pTabWidget->widget(index)->children();
    for (int i = 0; i < objList.size(); ++i)
    {
        SliderWidget* pSlider = (SliderWidget*)objList.at(i);
        QString csrType = m_csrTypeList.at(index);
        CeleX5::CfgInfo cfgInfo = m_pCeleX5->getCfgInfoByName(csrType.toStdString(), pSlider->getBiasType(), false);
        pSlider->updateValue(cfgInfo.value);
        cout << "CeleX5Cfg::updateCfgParameters: " << pSlider->getBiasType() << " = " << cfgInfo.value << endl;
    }
}

void SettingsWidget::createTapWidget1(QWidget *widget, QWidget *slotWidget)
{
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

    //----- Group 1 -----
    m_pGroupBControl = new QGroupBox("Sensor Control Parameters: ", widget);
    m_pGroupBControl->setStyleSheet(style1 + style2);

    QStringList cfgList;
    cfgList << "Clock" << "Brightness" << "Threshold" << "ISO";
    int min[5]   = {20,  0,    50,  1};
    int max[5]   = {100, 1023, 511, 6};
    int value[5] = {100, 100,  171, 3};
    int step[5]  = {10,  1,    1,   1};
    for (int i = 0; i < cfgList.size(); i++)
    {
        if ("ISO" == cfgList.at(i))
        {
            value[i] = m_pCeleX5->getISOLevel();
            max[i] = m_pCeleX5->getISOLevelCount();
        }
        else if ("Brightness" == cfgList.at(i))
        {
            value[i] = m_pCeleX5->getBrightness();
        }
        CfgSlider* pSlider = new CfgSlider(widget, min[i], max[i], step[i], value[i], slotWidget);
        pSlider->setBiasType(QString(cfgList.at(i)).toStdString());
        pSlider->setDisplayName(cfgList.at(i));
        pSlider->setObjectName(cfgList.at(i));
    }

    //----- Group 2 -----
    m_pGroupLoop = new QGroupBox("Loop Mode Duration: ", widget);
    m_pGroupLoop->setStyleSheet(style1 + style2);
    QStringList cfgList2;
    cfgList2 << "Event Duration" << "FullPic Num" << "S FO Pic Num"  << "M FO Pic Num";
    int min2[4] = {0};
    int max2[4] = {1023, 255, 255, 255};
    int value2[4] = {20, 1, 1, 3};
    for (int i = 0; i < cfgList2.size(); i++)
    {
        CfgSlider* pSlider = new CfgSlider(widget, min2[i], max2[i], 1, value2[i], slotWidget);
        pSlider->setGeometry(670, 50+i*80, 600, 70);
        pSlider->setBiasType(QString(cfgList2.at(i)).toStdString());
        pSlider->setDisplayName(cfgList2.at(i));
        pSlider->setObjectName(cfgList2.at(i));
    }

    //----- Group 3 -----
    m_pGroupAutoISP = new QGroupBox("Auto ISP Control Parameters: ", widget);
    m_pGroupAutoISP->setStyleSheet(style1 + style2);
    QStringList cfgList3;
    cfgList3 << "Brightness 1" << "Brightness 2" << "Brightness 3" << "Brightness 4"
             << "BRT Threshold 1" << "BRT Threshold 2" << "BRT Threshold 3";
    int min4[7] = {0};
    int max4[7] = {1023, 1023, 1023, 1023, 4095, 4095, 4095};
    int value4[7] = {95, 130, 170, 200, 10, 20, 80};
    for (int i = 0; i < cfgList3.size(); i++)
    {
        CfgSlider* pSlider = new CfgSlider(widget, min4[i], max4[i], 1, value4[i], slotWidget);
        pSlider->setBiasType(QString(cfgList3.at(i)).toStdString());
        pSlider->setDisplayName(cfgList3.at(i));
        pSlider->setObjectName(cfgList3.at(i));
    }
    widget->setFocus();

//    QList<CfgSlider*> slider1 = widget->findChildren<CfgSlider *>("Clock");
//    if (slider1.size() > 0)
//        slider1[0]->updateValue(m_pCeleX5->getClockRate());
}

void SettingsWidget::createTapWidget2(QWidget *widget)
{

}

void SettingsWidget::createTapWidget3(QWidget *widget)
{

}

void SettingsWidget::onValueChanged(uint32_t value, CfgSlider *slider)
{
    cout << "SettingsWidget::onValueChanged: " << slider->getBiasType() << ", " << value << endl;
}

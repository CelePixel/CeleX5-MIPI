#include "celex5cfg.h"
#include "sliderwidget.h"
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>
#include <QDebug>

using namespace std;

CeleX5Cfg::CeleX5Cfg(CeleX5* pCeleX5, QWidget *parent)
    : QWidget(parent)
    , m_pCeleX5(pCeleX5)
{
    this->setWindowTitle("CeleX5 Configuration");
    this->setGeometry(10, 40, 1900, 900);
    this->setStyleSheet("background-color: lightgray; ");

    m_pTabWidget = new QTabWidget;
    m_pTabWidget->setStyleSheet("QTabWidget::pane {border-width: 1px; border-color: rgb(48, 104, 151);\
                                                   border-style: outset; background-color: rgb(132, 171, 208);} \
                                QTabBar::tab {min-height: 30px; font: 18px Calibri;} \
                                QTabBar::tab:selected{border-color: white; background: #002F6F; color: white;}");

    map<std::string, vector<CeleX5::CfgInfo>> mapCfg = m_pCeleX5->getCeleX5Cfg();
    for (auto itr = mapCfg.begin(); itr != mapCfg.end(); itr++)
    {
        cout << "group name: " << itr->first << endl;
        QWidget *widget = new QWidget();
        QString tapName = QString::fromStdString(itr->first);
        m_csrTypeList << tapName;
        tapName = tapName.remove("_Parameters");
        tapName.insert(0, "      ");
        tapName += "       ";
        m_pTabWidget->addTab(widget, tapName);

        int row = 0, col = 0;
        vector<CeleX5::CfgInfo> vecCfg = itr->second;
        int index = 0;
        for (auto itr1 = vecCfg.begin(); itr1 != vecCfg.end(); itr1++)
        {
            CeleX5::CfgInfo cfgInfo = (*itr1);
            bool bVoltage = false;
            if (QString::fromStdString(cfgInfo.name).contains("BIAS_") && itr->first == "Sensor_Core_Parameters")
                bVoltage = true;
            SliderWidget* pSlider = new SliderWidget(widget, cfgInfo.min, cfgInfo.max, cfgInfo.step, cfgInfo.value, this, bVoltage);
            pSlider->setGeometry(10+col*620, 20+row*80, 600, 60);
            pSlider->setBiasType(cfgInfo.name);
            pSlider->setDisplayName(QString::fromStdString(cfgInfo.name));
            pSlider->setObjectName(QString::fromStdString(cfgInfo.name));
            pSlider->setBiasAddr(cfgInfo.high_addr, cfgInfo.middle_addr, cfgInfo.low_addr);

            //cout << "----- Register Name: " << cfgInfo.name << endl;
            row++;
            index++;
            if (col != index/10)
            {
                col = index/10;
                row = 0;
            }
        }
    }

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_pTabWidget);

    this->setLayout(layout);
}

void CeleX5Cfg::setCurrentIndex(int index)
{
    for (int i = 0; i < m_csrTypeList.size(); i++)
        updateCfgParameters(i);
    m_pTabWidget->setCurrentIndex(index);
}

void CeleX5Cfg::updateCfgParameters(int index)
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

void CeleX5Cfg::onValueChanged(uint32_t value, SliderWidget *slider)
{
    cout << "CeleX5Cfg::onValueChanged: " << slider->getBiasType() << ", " << value << endl;
    //Enter CFG Mode
    m_pCeleX5->writeRegister(93, -1, -1, 0);
    m_pCeleX5->writeRegister(90, -1, -1, 1);

    m_pCeleX5->writeRegister(slider->addressH(), slider->addressM(), slider->addressL(), value);

    //Enter Start Mode
    m_pCeleX5->writeRegister(90, -1, -1, 0);
    m_pCeleX5->writeRegister(93, -1, -1, 1);

    m_pCeleX5->modifyCSRParameter("", slider->getBiasType(), value);
    emit valueChanged(slider->getBiasType(), value);
}

void CeleX5Cfg::setTestWidget(TestWidget *pWidget)
{
    m_pTestWidget = pWidget;
    //connect(this, SIGNAL(valueChanged(string,int)), m_pTestWidget, SLOT(onValueChanged(string,int)));
}

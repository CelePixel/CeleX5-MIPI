#include <QTimer>
#include "sliderwidget.h"
#include <QGridLayout>
#include <QIntValidator>
#include <QDebug>

#define SLIDER_DELAY 100

SliderWidget::SliderWidget(QWidget *parent, uint32_t min, uint32_t max, uint32_t step, uint32_t value, QWidget *widgetSlot, bool bShowVoltage)
    : QWidget(parent)
    , m_iAddrH(-1)
    , m_iAddrM(-1)
    , m_iAddrL(-1)
    , m_uiValue(value)
    , m_pLabelVoltage(NULL)
{
    QGridLayout* layout = new QGridLayout(this);
    this->setLayout(layout);
    //layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setColumnStretch(0, 5);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 12);
    layout->setColumnStretch(3, 1);

    m_pLabelName = new QLabel(this);
    m_pLabelName->setStyleSheet("QLabel {background: transparent; color: rgb(0, 0, 0); font: 16px Calibri}");
    m_pLabelName->setAlignment(Qt::AlignRight);

    m_pLabelAddr = new QLabel(this);
    m_pLabelAddr->setAlignment(Qt::AlignRight);
    m_pLabelAddr->setStyleSheet("QLabel {background: transparent; color: rgb(150, 0, 0); font: 16px Calibri}");

    //----------------
    m_pLabelMin = new QLabel(QString::number(min), this);
    m_pLabelMin->setStyleSheet("background: transparent; color: rgb(0, 0, 250); font: 18px Calibri;");
    m_pLabelMin->setAlignment(Qt::AlignCenter);

    m_pLabelMax = new QLabel(QString::number(max), this);
    m_pLabelMax->setStyleSheet("QLabel {background: transparent; color: rgb(0, 0, 250); font: 18px Calibri}");

//    m_pLabelValue = new QLabel(QString::number(value), this);
//    m_pLabelValue->setAlignment(Qt::AlignCenter);
//    m_pLabelValue->setStyleSheet("background: transparent; color: rgb(200, 0, 0); font: 18px Calibri");
//    m_pLabelValue->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QHBoxLayout* pHLayout = new QHBoxLayout;
    if (bShowVoltage)
    {
        m_pLabelVoltage = new QLabel(this);
        m_pLabelVoltage->setStyleSheet("QLabel {background: pink; color: #000000; font: 18px Calibri}");
        //m_pLabelVoltage->setText("   3.6V   ");
        pHLayout->addWidget(m_pLabelVoltage);
    }

    m_pLineEditValue = new QLineEdit(this);
    connect(m_pLineEditValue, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    m_pLineEditValue->setValidator(new QIntValidator(min, max, this));
    m_pLineEditValue->setAlignment(Qt::AlignCenter);
    m_pLineEditValue->setStyleSheet("QLineEdit {background: rgb(255, 255, 222); color: rgb(255, 0, 0); font: 18px Calibri}");
    m_pLineEditValue->setText(QString::number(value));

    pHLayout->addWidget(m_pLineEditValue);

    //----------------
    m_pSlider = new QSlider(Qt::Horizontal, this);
    m_pSlider->setMinimum(min);
    m_pSlider->setMaximum(max);
    m_pSlider->setValue(value);
    m_pSlider->setPageStep(step);
    m_pSlider->setSingleStep(step);
    connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
    connect(m_pSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(this, SIGNAL(valueChanged(uint32_t, SliderWidget*)), widgetSlot, SLOT(onValueChanged(uint32_t, SliderWidget*)));

    m_pSlider->setStyleSheet("QSlider::add-page:Horizontal{background-color: rgb(87, 97, 106); height: 4px; border-radius: 3px; }"
                             "QSlider::sub-page:Horizontal{background-color: rgb(0, 206, 206); height: 4px; border-radius: 3px; }"
                             "QSlider::groove:Horizontal {background: transparent; height: 6px; }"
                             "QSlider::handle:Horizontal {height: 6px; width: 10px; background-color: rgb(255, 255, 255); border-radius: 3px; margin:-4 0}");

    layout->addWidget(m_pLabelName, 1, 0);
    layout->addWidget(m_pLabelMin, 1, 1);
    //layout->addWidget(m_pLabelValue, 1, 2);
    //layout->addWidget(m_pLineEditValue, 1, 2);
    layout->addLayout(pHLayout, 1, 2);
    layout->addWidget(m_pLabelMax, 1, 3);
    //
    layout->addWidget(m_pLabelAddr, 2, 0);
    layout->addWidget(m_pSlider, 2, 2);
}

uint32_t SliderWidget::getValue()
{
    return m_uiValue;
}

void SliderWidget::setBiasType(std::string strType)
{
    m_strBiasType = strType;
    updateVoltage(m_uiValue);
}

void SliderWidget::setBiasAddr(int16_t addrH, int16_t addrM, int16_t addrL)
{
    m_iAddrH = addrH;
    m_iAddrM = addrM;
    m_iAddrL = addrL;
    if (-1 == addrL)
    {
        m_pLabelAddr->setText("[CSR_" + QString::number(addrH) + "]:");
    }
    else
    {
        if (-1 == addrM)
            m_pLabelAddr->setText("[CSR_" + QString::number(addrH) + " / CSR_" + QString::number(addrL) + "]:");
        else
            m_pLabelAddr->setText("[CSR_" + QString::number(addrH) + " / CSR_" + QString::number(addrM) + " / CSR_" + QString::number(addrL) +"]:");
    }
}

std::string SliderWidget::getBiasType()
{
    return m_strBiasType;
}

void SliderWidget::setDisplayName(QString name)
{
    m_pLabelName->setText(name + ":");
}

void SliderWidget::setDisabled(bool disable)
{
    m_pSlider->setDisabled(disable);
    if (disable)
    {
        m_pSlider->setStyleSheet("QSlider::add-page:Horizontal{background-color: rgb(120, 120, 120); height: 4px; border-radius: 3px; }"
                                 "QSlider::sub-page:Horizontal{background-color: rgb(120, 120, 120); height: 4px; border-radius: 3px; }"
                                 "QSlider::groove:Horizontal {background: transparent; height: 6px; }"
                                 "QSlider::handle:Horizontal {height: 6px; width: 10px; background-color: rgb(150, 150, 150); border-radius: 3px; margin:-4 0}");
    }
    else
    {
        m_pSlider->setStyleSheet("QSlider::add-page:Horizontal{background-color: rgb(87, 97, 106); height: 4px; border-radius: 3px; }"
                                 "QSlider::sub-page:Horizontal{background-color: rgb(22, 246, 246); height: 4px; border-radius: 3px; }"
                                 "QSlider::groove:Horizontal {background: transparent; height: 6px; }"
                                 "QSlider::handle:Horizontal {height: 6px; width: 10px; background-color: rgb(255, 255, 255); border-radius: 3px; margin:-4 0}");
    }
}

void SliderWidget::setLineEditText(QString text)
{
    m_pLineEditValue->setText(text);
}

void SliderWidget::updateValue(int value)
{
    m_pSlider->setValue(value);
    m_pLineEditValue->setText(QString::number(value));
    updateVoltage(value);
    m_uiValue = value;
}

void SliderWidget::updateVoltage(int value)
{
    if (m_pLabelVoltage)
    {
        double voltage;
        if (m_strBiasType == "BIAS_EVT_VL" ||
            m_strBiasType == "BIAS_EVT_DC" ||
            m_strBiasType == "BIAS_EVT_VH" ||
            m_strBiasType == "BIAS_C1")
            voltage = (double)value * 1.2 / 1024;
        else
            voltage = (double)value * 2.5 / 1024;
        m_pLabelVoltage->setText("   " + QString::number(voltage, 'f', 2) + " V   ");
    }
}

void SliderWidget::OnSliderValueChanged(int value)
{
    setLineEditText(QString::number(value));
    updateVoltage(m_uiValue);
    m_uiValue = value;
}

void SliderWidget::onSliderReleased()
{
    emit valueChanged(m_uiValue, this);
}

void SliderWidget::unblockSliderSignal()
{
    m_pSlider->blockSignals(false);
}

void SliderWidget::onEditingFinished()
{
    if (m_pLineEditValue->hasFocus())
    {
        this->setFocus();
    }
    else
    {
        qDebug() << "SliderWidget::onEditingFinished";
        //setValue(m_pLineEditValue->text().toInt());
        m_pSlider->setValue(m_pLineEditValue->text().toInt());
        m_uiValue = m_pLineEditValue->text().toInt();
        updateVoltage(m_uiValue);
        emit valueChanged(m_uiValue, this);
    }
}

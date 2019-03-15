#include <QTimer>
#include "hhsliderwidget.h"
#include "hhconstants.h"
#include <QGridLayout>

HHSliderWidget::HHSliderWidget(QWidget *parent, uint32_t min, uint32_t max, uint32_t step, uint32_t value, uint32_t width, QWidget *widgetSlot)
    : QWidget(parent)
    , m_uiValue(value)
{
    QGridLayout* layout = new QGridLayout(this);
    this->setLayout(layout);

    m_pLabelName = new QLabel(this);
    m_pLabelName->setStyleSheet("QLabel {background: transparent; color: rgb(0, 0, 0); font: 16px Calibri;}");
    m_pLabelName->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

    m_pLabelMin = new QLabel(QString::number(min), this);
    m_pLabelMin->setStyleSheet("QLabel {background: transparent; color: black; font: 16px Calibri;}");
    m_pLabelMin->setAlignment(Qt::AlignCenter);

    m_pSlider = new QSlider(Qt::Horizontal, this);
    m_pSlider->setMinimum(min);
    m_pSlider->setMaximum(max);
    m_pSlider->setValue(value);
    m_pSlider->setPageStep(step);
    m_pSlider->setSingleStep(step);
    connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    //connect(m_pSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(this, SIGNAL(valueChanged(uint32_t, HHSliderWidget*)), widgetSlot, SLOT(onValueChanged(uint32_t, HHSliderWidget*)));

    m_pSlider->setStyleSheet("QSlider {background: transparent;}"
                             "QSlider::add-page:Horizontal{background-color: rgb(87, 97, 106); height: 4px; border-radius: 3px; }"
                             "QSlider::sub-page:Horizontal{background-color: rgb(22, 246, 246); height: 4px; border-radius: 3px; }"
                             "QSlider::groove:Horizontal {background: transparent; height: 6px; }"
                             "QSlider::handle:Horizontal {height: 6px; width: 10px; background-color: rgb(255, 255, 255); border-radius: 3px; margin:-4 0}");

    m_pLabelMax = new QLabel(QString::number(max), this);
    m_pLabelMax->setStyleSheet("QLabel {background: transparent; color: black; font: 16px Calibri;}");

    m_pLabelValue = new QLabel(QString::number(value), this);
    m_pLabelValue->setAlignment(Qt::AlignCenter);
    m_pLabelValue->setStyleSheet("QLabel {background: transparent; color: rgb(185, 15, 43); font: 16px Calibri;}");
    m_pLabelValue->move(280, -6);
    m_pLabelValue->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    layout->addWidget(m_pLabelName, 1, 0);
    layout->addWidget(m_pLabelMin, 1, 1);
    layout->addWidget(m_pSlider, 1, 2);
    layout->addWidget(m_pLabelMax, 1, 3);
}

uint32_t HHSliderWidget::getValue()
{
    return m_uiValue;
}

void HHSliderWidget::setBiasType(std::string strType)
{
    m_strBiasType = strType;
}

std::string HHSliderWidget::getBiasType()
{
    return m_strBiasType;
}

void HHSliderWidget::setDisplayName(QString name)
{
    m_pLabelName->setText(name);
}

void HHSliderWidget::setDisabled(bool disable)
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

void HHSliderWidget::setSliderValue(uint32_t value)
{
    m_pSlider->setValue(value);
    m_pLabelValue->setText(QString::number(value));
    m_uiValue = value;
    //emit valueChanged(m_uiValue, this);
}

void HHSliderWidget::setValue(int newValue)
{
    if (newValue == m_uiValue)
        return;

    m_pSlider->blockSignals(true);
    QTimer::singleShot(SLIDER_DELAY, this, SLOT(unblockSliderSignal()));

    m_pLabelValue->setText(QString::number(newValue));
    m_uiValue = newValue;

    emit valueChanged(m_uiValue, this);
}

void HHSliderWidget::onSliderReleased()
{
    emit valueChanged(m_uiValue, this);
}

void HHSliderWidget::unblockSliderSignal()
{
    m_pSlider->blockSignals(false);
}

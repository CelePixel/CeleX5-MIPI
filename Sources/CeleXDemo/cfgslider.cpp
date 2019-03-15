#include <QTimer>
#include "cfgslider.h"
#include <QGridLayout>
#include <QIntValidator>
#include <QDebug>

#define SLIDER_DELAY 100

CfgSlider::CfgSlider(QWidget *parent, uint32_t min, uint32_t max, uint32_t step, uint32_t value, QWidget *widgetSlot)
    : QWidget(parent)
    , m_uiValue(value)
{
    QGridLayout* layout = new QGridLayout(this);
    this->setLayout(layout);
    //layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 12);

    m_pLabelName = new QLabel(this);
    m_pLabelName->setStyleSheet("QLabel {background: transparent; color: rgb(0, 0, 0); font: 18px Calibri}");
    m_pLabelName->setAlignment(Qt::AlignRight);

    //----------------
    QString labelStyle = "QLabel {background: transparent; color: rgb(0, 0, 250); font: 18px Calibri}";
    QHBoxLayout* pHLayout = new QHBoxLayout;
    m_pLabelMin = new QLabel(QString::number(min), this);
    m_pLabelMin->setStyleSheet(labelStyle);
    m_pLabelMin->setAlignment(Qt::AlignLeft);
    pHLayout->addWidget(m_pLabelMin);
    pHLayout->setStretchFactor(m_pLabelMin, 1);

    m_pLineEditValue = new QLineEdit(this);
    connect(m_pLineEditValue, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    m_pLineEditValue->setValidator(new QIntValidator(min, max, this));
    m_pLineEditValue->setAlignment(Qt::AlignCenter);
    m_pLineEditValue->setStyleSheet("QLineEdit {background: rgb(255, 255, 222); color: rgb(255, 0, 0); font: 18px Calibri}");
    m_pLineEditValue->setText(QString::number(value));
    pHLayout->addWidget(m_pLineEditValue);
    pHLayout->setStretchFactor(m_pLineEditValue, 1);

    m_pLabelMax = new QLabel(QString::number(max), this);
    m_pLabelMax->setStyleSheet(labelStyle);
    m_pLabelMax->setAlignment(Qt::AlignRight);
    pHLayout->addWidget(m_pLabelMax);
    pHLayout->setStretchFactor(m_pLabelMax, 1);

    //----------------
    m_pSlider = new QSlider(Qt::Horizontal, this);
    m_pSlider->setRange(min, max);
    m_pSlider->setValue(value);
    m_pSlider->setPageStep(step);
    m_pSlider->setSingleStep(step);
    connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
    connect(m_pSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(this, SIGNAL(valueChanged(uint32_t, CfgSlider*)), widgetSlot, SLOT(onValueChanged(uint32_t, CfgSlider*)));

    m_pSlider->setStyleSheet("QSlider::add-page:Horizontal{background-color: rgb(87, 97, 106); height: 4px; border-radius: 3px; }"
                             "QSlider::sub-page:Horizontal{background-color: rgb(0, 206, 206); height: 4px; border-radius: 3px; }"
                             "QSlider::groove:Horizontal {background: transparent; height: 6px; }"
                             "QSlider::handle:Horizontal {height: 6px; width: 10px; background-color: rgb(255, 255, 255); border-radius: 3px; margin:-4 0}");

    layout->addWidget(m_pLabelName, 1, 0);
    layout->addLayout(pHLayout, 1, 1);
    layout->addWidget(m_pSlider, 2, 1);
}

uint32_t CfgSlider::getValue()
{
    return m_uiValue;
}

void CfgSlider::setBiasType(std::string strType)
{
    m_strBiasType = strType;
}

std::string CfgSlider::getBiasType()
{
    return m_strBiasType;
}

void CfgSlider::setDisplayName(QString name)
{
    m_pLabelName->setText(name + ":  ");
}

void CfgSlider::setDisabled(bool disable)
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

void CfgSlider::setLineEditText(QString text)
{
    m_pLineEditValue->setText(text);
}

void CfgSlider::updateValue(int value)
{
    m_pSlider->setValue(value);
    m_pLineEditValue->setText(QString::number(value));
    m_uiValue = value;
}

void CfgSlider::setMaximum(int value)
{
    m_pSlider->setMaximum(value);
    m_pLabelMax->setText(QString::number(value));
}

int CfgSlider::maximum()
{
    return m_pSlider->maximum();
}

void CfgSlider::OnSliderValueChanged(int value)
{
    setLineEditText(QString::number(value));
    m_uiValue = value;
}

void CfgSlider::onSliderReleased()
{
    emit valueChanged(m_uiValue, this);
}

void CfgSlider::unblockSliderSignal()
{
    m_pSlider->blockSignals(false);
}

void CfgSlider::onEditingFinished()
{
    if (m_pLineEditValue->hasFocus())
    {
        this->setFocus();
    }
    else
    {
        //qDebug() << "SliderWidget::onEditingFinished";
        //setValue(m_pLineEditValue->text().toInt());
        m_pSlider->setValue(m_pLineEditValue->text().toInt());
        m_uiValue = m_pLineEditValue->text().toInt();
        emit valueChanged(m_uiValue, this);
    }
}

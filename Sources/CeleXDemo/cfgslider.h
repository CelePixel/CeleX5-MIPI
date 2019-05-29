#ifndef CFGSLIDER_H
#define CFGSLIDER_H

#include <stdint.h>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>

class CfgSlider : public QWidget
{
    Q_OBJECT
public:
    explicit CfgSlider(QWidget *parent, uint32_t min, uint32_t max, uint32_t step, uint32_t value, QWidget *widgetSlot);

    uint32_t getValue();
    void setBiasType(std::string strType);
    std::string getBiasType();
    void setDisplayName(QString name);
    void setDisabled(bool disable);

    void setLineEditText(QString text);
    void updateValue(int value);

    void setMaximum(int value);
    int maximum();

private:

signals:
    void valueChanged(uint32_t newValue, CfgSlider* slider);

public slots:
    void OnSliderValueChanged(int value);
    void onSliderReleased();
    void unblockSliderSignal();
    void onEditingFinished();

private:
    QLabel*      m_pLabelName;
    QSlider*     m_pSlider;

    QLabel*      m_pLabelMin;
    QLabel*      m_pLabelMax;
    QLabel*      m_pLabelValue;
    QLineEdit*   m_pLineEditValue;

    uint32_t     m_uiValue;
    std::string  m_strBiasType;
};

#endif // CFGSLIDER_H

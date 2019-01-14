#ifndef HHSLIDERCONTROL_H
#define HHSLIDERCONTROL_H

#include <stdint.h>
#include <QWidget>
#include <QLabel>
#include <QSlider>

class HHSliderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HHSliderWidget(QWidget *parent, uint32_t min, uint32_t max, uint32_t step, uint32_t value, uint32_t width, QWidget *widgetSlot);

    uint32_t getValue();
    void setBiasType(std::string strType);
    std::string getBiasType();
    void setDisplayName(QString name);

    void setDisabled(bool disable);
    void setSliderValue(uint32_t value);

signals:
    void valueChanged(uint32_t newValue, HHSliderWidget* slider);

public slots:
    void setValue(int newValue);
    void onSliderReleased();
    void unblockSliderSignal();

private:
    QLabel*      m_pLabelName;
    QSlider*     m_pSlider;
    QLabel*      m_pLabelMin;

    QLabel*      m_pLabelMax;
    QLabel*      m_pLabelValue;

    uint32_t     m_uiValue;
    std::string  m_strBiasType;
};

#endif // HHSLIDERCONTROL_H

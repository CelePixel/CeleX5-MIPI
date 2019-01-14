#ifndef HHSLIDERCONTROL_H
#define HHSLIDERCONTROL_H

#include <stdint.h>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>

class SliderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SliderWidget(QWidget *parent, uint32_t min, uint32_t max, uint32_t step, uint32_t value, QWidget *widgetSlot, bool bShowVoltage);

    uint32_t getValue();
    void setBiasType(std::string strType);
    void setBiasAddr(int16_t addrH, int16_t addrM, int16_t addrL);
    std::string getBiasType();
    void setDisplayName(QString name);
    void setDisabled(bool disable);
    int16_t addressH() { return m_iAddrH; }
    int16_t addressM() { return m_iAddrM; }
    int16_t addressL() { return m_iAddrL; }

    void setLineEditText(QString text);
    void updateValue(int value);

private:
    void updateVoltage(int value);

signals:
    void valueChanged(uint32_t newValue, SliderWidget* slider);

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
    QLabel*      m_pLabelVoltage;
    QLineEdit*   m_pLineEditValue;

    QLabel*      m_pLabelAddr;

    int16_t      m_iAddrH;
    int16_t      m_iAddrM;
    int16_t      m_iAddrL;
    uint32_t     m_uiValue;
    std::string  m_strBiasType;
};

#endif // HHSLIDERCONTROL_H

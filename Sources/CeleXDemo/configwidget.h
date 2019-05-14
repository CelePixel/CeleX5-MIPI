#ifndef CONFIGSETTINGWIDGET_H
#define CONFIGSETTINGWIDGET_H

#include <QWidget>

class QLabel;
class SliderWidget;

class ConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(CelexSensorDLL* pCelexSensor, QWidget *parent = 0);

private:
    void createResetSlider();

signals:

public slots:
    void onValueChanged(uint32_t value, SliderWidget* slider);

private:
    CelexSensorDLL* m_pCelexSensor;
};

#endif // CONFIGSETTINGWIDGET_H

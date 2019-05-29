#ifndef CELEX5CFG_H
#define CELEX5CFG_H

#include <QWidget>
#include "./include/celex5/celex5.h"
#include "sliderwidget.h"

class QTabWidget;
class TestWidget;
class CeleX5Cfg : public QWidget
{
    Q_OBJECT
public:
    explicit CeleX5Cfg(CeleX5* pCeleX5, QWidget *parent = 0);
    void setCurrentIndex(int index);
    void setTestWidget(TestWidget *pWidget);

private:
    void updateCfgParameters(int index);

signals:
    void valueChanged(string cmdName, int value);

public slots:
    void onValueChanged(uint32_t value, SliderWidget* slider);

private:
    CeleX5*            m_pCeleX5;
    QTabWidget*        m_pTabWidget;
    TestWidget*        m_pTestWidget;
    QStringList        m_csrTypeList;
};

#endif // CELEX5CFG_H

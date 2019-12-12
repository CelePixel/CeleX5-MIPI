#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include "./include/celex5/celex5.h"
#include "cfgslider.h"

class QTabWidget;
class TestWidget;
class QGroupBox;
class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(CeleX5* pCeleX5, QWidget *parent = 0);
    void setCurrentIndex(int index);
    void updateCfgs();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateCfgParameters(int index);
    void createTapWidget1(QWidget* widget, QWidget* slotWidget);
    void createTapWidget2(QWidget* widget);
    void createTapWidget3(QWidget* widget);

signals:
    void valueChanged(std::string cmdName, int value);

public slots:
    void onValueChanged(uint32_t value, CfgSlider* slider);

private:
    CeleX5*            m_pCeleX5;
    QTabWidget*        m_pTabWidget;
    QStringList        m_csrTypeList;
    //
    QGroupBox*         m_pGroupBControl;
    QGroupBox*         m_pGroupLoop;
    QGroupBox*         m_pGroupAutoISP;
};

#endif // SETTINGSWIDGET_H

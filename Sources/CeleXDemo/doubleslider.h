#ifndef DOUBLESLIDER_H
#define DOUBLESLIDER_H

#include <QWidget>

class DoubleSlider : public QWidget
{
    Q_OBJECT
public:
    DoubleSlider(QWidget* parent = 0);
    void setRange(unsigned long min, unsigned long max);
    void setSingleStep(int step);

    unsigned long value();
    void setValue(unsigned long value);

    enum State{
        None = 0,
        MinHandle,
        MaxHandle,
        CurHandle};

    unsigned long minValue() const;
    unsigned long maxValue() const;

    unsigned long minRange() const;
    unsigned long maxRange() const;

public slots:
    void setLabel(const QString& label);
    void setMaxValue(unsigned long val);
    void setMinValue(unsigned long val);

signals:
    void minValueChanged(unsigned long);
    void maxValueChanged(unsigned long);
    void valueChanged(unsigned long);
    void sliderPressed();
    void sliderReleased();

private:
    int              m_singleStep;
    unsigned long    m_min;
    unsigned long    m_max;
    unsigned long    m_minValue;
    unsigned long    m_maxValue;
    unsigned long    m_value;
    QRect            m_rectMinHandle;
    QRect            m_rectMaxHandle;
    QRect            m_rectCurHandle;
    State            m_state;
    QString          m_label;

protected:
    void paintEvent(QPaintEvent* event);
    void paintColoredRect(QRect rect, QColor color, QPainter* painter);
    void paintValueLabel(QPainter* painter);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};
#endif // DOUBLESLIDER_H

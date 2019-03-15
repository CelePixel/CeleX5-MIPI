#include "doubleslider.h"

#include "doubleslider.h"
#include <QPainter>
#include <QMouseEvent>

DoubleSlider::DoubleSlider(QWidget* parent)
    : QWidget(parent)
    , m_min(0)
    , m_max(400)
    , m_singleStep(1)
    , m_minValue(0)
    , m_maxValue(300)
    , m_value(0)
    , m_state(None)
{
    setFixedHeight(50);
    setFocusPolicy(Qt::StrongFocus);
}

void DoubleSlider::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    paintValueLabel(&painter);
}

void DoubleSlider::paintColoredRect(QRect rect, QColor color, QPainter* painter)
{
    painter->fillRect(rect, QBrush(color));
}

void DoubleSlider::paintValueLabel(QPainter* painter)
{
    painter->setBrush(Qt::NoBrush);
    int diff = m_max - m_min;
    if (diff <= 0)
        diff = 1;
    int minPos = (m_minValue-m_min ) * width() / diff;
    int maxPos = (m_maxValue-m_min ) * width() / diff;
    int curPos = (m_value-m_min ) * width() / diff;

    if (minPos <= 0)
    {
        minPos = 0;
    }
    else if (minPos >= width() - 8)
    {
        minPos = width() - 8;
    }

    if (maxPos <= 0)
    {
        maxPos = 0;
    }
    else if (maxPos >= width() -8)
    {
        maxPos = width() - 8;
    }

    if (curPos <= 0)
    {
        curPos = 0;
    }
    else if (curPos >= width() - 8)
    {
        curPos = width() - 8;
    }

    //----- paint groove
    paintColoredRect(QRect(4, 15, width() - 8, 6), QColor(87, 97, 106), painter);
    paintColoredRect(QRect(minPos+4, 15, maxPos-minPos, 6), QColor(22, 246, 246), painter);

    //----- handle
    m_rectMinHandle = QRect(minPos, 5, 8, 26);
    m_rectMaxHandle = QRect(maxPos, 5, 8, 26);
    m_rectCurHandle = QRect(curPos, 10, 8, 16);

    //-----paint Handle
    QColor minColor = QColor(255, 97, 106);
    QColor maxColor = QColor(255, 97, 106);
    QColor curColor = QColor(255, 255, 255);
    paintColoredRect(m_rectMinHandle, minColor, painter);
    paintColoredRect(m_rectMaxHandle, maxColor, painter);
    paintColoredRect(m_rectCurHandle, curColor, painter);
}

inline unsigned long getValidValue(unsigned long val, unsigned long min, unsigned long max)
{
    unsigned long tmp = std::max(val, min);
    return std::min(tmp, max);
}

void DoubleSlider::setRange(unsigned long min, unsigned long max)
{
    m_minValue = m_min = min;
    m_maxValue = m_max = max;
}

void DoubleSlider::setSingleStep(int step)
{
    m_singleStep = step;
}

unsigned long DoubleSlider::value()
{
    return m_value;
}

void DoubleSlider::setValue(unsigned long value)
{
    m_value = value;
    update();
}

unsigned long DoubleSlider::minValue() const
{
    return m_minValue;
}
void DoubleSlider::setMinValue(unsigned long val)
{
    m_minValue = val;
    emit minValueChanged(val);
}

unsigned long DoubleSlider::maxValue() const
{
    return m_maxValue;
}
void DoubleSlider::setMaxValue(unsigned long val)
{
    m_maxValue = val;
    emit maxValueChanged(val);
}

void DoubleSlider::setLabel(const QString& label)
{
   m_label = label;
   update();
}

unsigned long DoubleSlider::minRange() const
{
    return m_min;
}
unsigned long DoubleSlider::maxRange() const
{
    return m_max;
}

void DoubleSlider::mousePressEvent(QMouseEvent* event)
{
    if (m_rectCurHandle.contains(event->pos()))
    {
        m_state = CurHandle;
        emit sliderPressed();
    }
    else if (m_rectMinHandle.contains(event->pos()))
    {
        m_state = MinHandle;
    }
    else if (m_rectMaxHandle.contains(event->pos()))
    {
        m_state = MaxHandle;
    }
    else
    {
        m_state = None;
    }
    update();
}

void DoubleSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        int x = event->x();
        if (x < 0) x = 0;
        long move = x * (m_max - m_min) * 1.0/ width() + m_min;
        switch (m_state)
        {
        case MinHandle:
        {
            unsigned long val = getValidValue(move, m_min, maxValue());
            setMinValue(val);
            break;
        }

        case MaxHandle:
        {
            unsigned long val = getValidValue(move, minValue(), m_max);
            setMaxValue(val);
            break;
        }

        case CurHandle:
        {
            unsigned long val = getValidValue(move, m_min, maxValue());
            m_value = val;
            if (m_value < m_minValue)
                m_value = m_minValue;
            if (m_value > m_maxValue)
                m_value = m_maxValue;
            emit valueChanged(m_value);
            break;
        }

        case None:
        default:
            break;
        }
    }
    update();
}

void DoubleSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton)
    {
        if (m_state == CurHandle)
        {
            emit sliderReleased();
        }
    }
}

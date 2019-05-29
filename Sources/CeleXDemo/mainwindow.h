#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "celex5widget.h"
#include <QPushButton>
#include <QTimer>
#include <QTextEdit>
#include <fstream>
#include <QLineEdit>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    //explicit MainWindow(QWidget *parent = 0);
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private:


private slots:
    void showCeleXWidget();

private:
    Ui::MainWindow * ui;
    CeleX5Widget*    m_pCX5Widget;
    QTimer*          m_pStartTimer;
    QTextEdit*       txt;
    QLineEdit*       m_pVersionInfo;
    ifstream         m_fIn;
};

#endif // MAINWINDOW_H

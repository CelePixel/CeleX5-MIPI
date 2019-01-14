#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "celex4widget.h"
#include "celex5widget.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    //explicit MainWindow(QWidget *parent = 0);
    explicit MainWindow(int argc, char *argv[], QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private:

private slots:

private:
    Ui::MainWindow * ui;
    CeleX4Widget*    m_pCX4Widget;
    CeleX5Widget*    m_pCX5Widget;
};

#endif // MAINWINDOW_H

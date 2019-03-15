#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pCX5Widget(NULL)
{
    ui->setupUi(this);
    this->move(10, 10);

    this->setWindowIcon(QIcon(":/images/celepixel.png"));

    this->setWindowFlags(Qt::FramelessWindowHint);
    setGeometry(QRect(720, 360, 480, 360));

    this->setAutoFillBackground(true);
    QPalette palette;
    QPixmap pixmap(":/images/background.png");
    palette.setBrush(QPalette::Window, QBrush(pixmap));
    this->setPalette(palette);

    txt = new QTextEdit(this);
    txt->setStyleSheet("font: 16px Calibri; color: #666666;background:transparent;border-width:0;border-style:outset");
    txt->setGeometry(QRect(250, 50, 250, 250));

    m_pVersionInfo = new QLineEdit(this);
    m_pVersionInfo->setText("Version 1.3.0");
    m_pVersionInfo->setGeometry(QRect(30, 320, 200, 20));
    m_pVersionInfo->setStyleSheet("font: 20px Calibri; color: #FFFFFF;background:transparent;border-width:0;border-style:outset");

    QString strPath = QCoreApplication::applicationDirPath() + "/loading.txt";
    m_fIn.open(strPath.toStdString());
    string line="";
    while(!m_fIn.eof())
    {
        txt->append(QString::fromStdString(line));
        m_fIn >> line;
    }

    m_pStartTimer = new QTimer(this);
    m_pStartTimer->setSingleShot(true);
    connect(m_pStartTimer, SIGNAL(timeout()), this, SLOT(showCeleXWidget()));
    m_pStartTimer->start(100);
}


void MainWindow::showCeleXWidget()
{
    bool bCeleX5Device = true;
    if (bCeleX5Device)
    {
        this->setWindowTitle("CeleX5-Demo");

        if (!m_pCX5Widget)
        {
            m_pCX5Widget = new CeleX5Widget;
        }
        m_pCX5Widget->show();
        m_pCX5Widget->showMaximized();
    }
    this->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    cout << "MainWindow::closeEvent" << endl;
    if (m_pCX5Widget)
        m_pCX5Widget->closeEvent(event);
}

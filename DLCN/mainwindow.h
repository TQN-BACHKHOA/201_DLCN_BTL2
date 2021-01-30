#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtDebug>
#include <QByteArray>
#include <QTextStream>
#include "qcustomplot.h"
#include <iostream>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    union ByteToFloat{
        float myfloat;
        char mybyte[4];
    } m_data, m_data_RX;

private slots:
    void fillPortAction();
    void readData();
    void on_btnExit_clicked();
    void on_btn_SetUart_clicked();
    void on_btn_closeUart_clicked();
    void WaitCanI_SendMySpan();
    void on_SetZero_clicked();
    void on_SenSpan_clicked();


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

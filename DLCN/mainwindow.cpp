#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
QSerialPort *SerialPort;
QTimer *Uart_Timer, *Angle_Timer;
float setpoint, my_latest_angle,my_little_set;
bool flag_PlotTimer;
char CheckSumRX;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("ENCODER người cá");
    fillPortAction();
    SerialPort = new QSerialPort(this);
    Uart_Timer = new QTimer(this);
    Angle_Timer = new QTimer(this);
    ui->RealAngle->setText("0.00");
    ui->RealAngle->setReadOnly(true);
    ui->scrollArea->setBackgroundRole(QPalette::Dark);
  //  Speed_Timer = new QTimer(this);
    connect(Uart_Timer,SIGNAL(timeout()),this,SLOT(readData()));

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::fillPortAction() {
    const auto infos = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : infos){
        ui->Com_Uart->addItem(info.portName());
    }
    ui->baudRate->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRate->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRate->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRate->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->baudRate->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRate->addItem(tr("Custom"));

    ui->dataBits->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBits->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBits->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBits->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBits->setCurrentIndex(3);

    ui->parity->addItem(tr("None"), QSerialPort::NoParity);
    ui->parity->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parity->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parity->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parity->addItem(tr("Space"), QSerialPort::SpaceParity);

    ui->stopBits->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    ui->stopBits->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControl->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControl->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControl->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}


void MainWindow::on_btn_SetUart_clicked()
{
    SerialPort->setPortName(ui->Com_Uart->currentText());
    SerialPort->setBaudRate(ui->baudRate->currentText().toInt());
    SerialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->dataBits->itemData(ui->dataBits->currentIndex()).toInt()));
    SerialPort->setParity(static_cast<QSerialPort::Parity>(ui->parity->itemData(ui->parity->currentIndex()).toInt()));
    SerialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->stopBits->itemData(ui->stopBits->currentIndex()).toInt()));
    SerialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(ui->flowControl->itemData(ui->flowControl->currentIndex()).toInt()));
    SerialPort->open(QIODevice::ReadWrite);
    connect(SerialPort, &QSerialPort::readyRead, this,&MainWindow::readData);
    Uart_Timer->start(20);
}


void MainWindow::on_btn_closeUart_clicked()
{
    SerialPort->close();
    setpoint = 0;
}


void MainWindow::on_btnExit_clicked()
{
    setpoint = 0;
    this->close();
}


void MainWindow::readData()
{
 //ui->m_console->moveCursor(QTextCursor::End);
 QByteArray byte_data = SerialPort->readAll();
 if(!byte_data.isEmpty()){
 if(byte_data.startsWith("$ANGL,")&& byte_data[12]=='\n'&& byte_data[11]=='\r')
 {
 //qDebug() <<byte_data;

 for (int i=0;i<4;i++)
 {
     CheckSumRX+=byte_data[6+i];
 }
     qDebug()<<CheckSumRX<<byte_data[10];
     if(CheckSumRX==byte_data[10]){
         for(int k= 0;k<4;k++){
         m_data_RX.mybyte[3-k] = byte_data[6+k];
         }
         //ui->SpeedError->display(m_data_RX.myfloat-setpoint);
         }
         my_latest_angle=m_data_RX.myfloat;
         QString my_little_data=QString::number(my_latest_angle);
         ui->RealAngle->setText(my_little_data);
         CheckSumRX=0;
         }
 } }

void MainWindow::on_SetZero_clicked()
{
    QByteArray txbuff;
    txbuff="$ZERO,";
    m_data.myfloat = 0;
    for(int k=0;k<4;k++)
    {
    txbuff[6+k]=0;
    }
    txbuff[10]=0;
    txbuff[11]= '\r';
    txbuff[12]= '\n';
    SerialPort->write(txbuff,13);
   // ui->RealAngle->setText("0");
    ui->Set_My_Span->setPlainText("0");
}



void MainWindow::on_SenSpan_clicked()
{
    my_little_set=ui->Set_My_Span->toPlainText().toFloat();
    char Checksum_TX=0;
    QByteArray txbuff;
    txbuff="$SPAN,";
    m_data.myfloat = my_little_set;
    for(int k=0;k<4;k++)
    {
    txbuff[6+k]=m_data.mybyte[3-k];
    Checksum_TX+=txbuff[6+k];
    }
    txbuff[10]=Checksum_TX;
    txbuff[11]= '\r';
    txbuff[12]= '\n';
    SerialPort->write(txbuff,13);
    //WaitCanI_SendMySpan();
}

void MainWindow::WaitCanI_SendMySpan()
{
    my_latest_angle=89.129;
    float Span = my_latest_angle/my_little_set;
    char Checksum_TX=0;
    QByteArray txbuff;
    txbuff="$ANGL,";
    m_data.myfloat = Span;
    for(int k=0;k<4;k++)
    {
    txbuff[6+k]=m_data.mybyte[3-k];
    Checksum_TX+=txbuff[6+k];
    }
    txbuff[10]=Checksum_TX;
    txbuff[11]= '\r';
    txbuff[12]= '\n';
    SerialPort->write(txbuff,13);
}


#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->eSender->setOverwriteMode(false);
    sender.clear();
    receiver.clear();
    ui->eSender->setData(sender);
    ui->eReceiver->setData(receiver);
    baudrate = ui->eBaudRate->currentText().toInt();
    on_bUartRefresh_clicked();

    connect(&resendTimer, &QTimer::timeout, this, &MainWindow::on_bSend_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_serials_activated(int index)
{
    QSerialPortInfo info = ports.at(index);
    curSer = info;
}

void MainWindow::onSerialData()
{
    qDebug() << "onSerialData()";
    QByteArray data = serial.readAll();
    receiver.append(data);
    ui->eReceiver->setData(receiver);
    ui->lNBytes->setText(tr("Read 0x%1 bytes, total 0x%2").arg(data.size(), 4, 16, CH0).arg(receiver.size(), 4, 16, CH0));
}

void MainWindow::on_bClearTransmitter_clicked()
{
    sender.clear();
    ui->eSender->setData(sender);
}

void MainWindow::on_bClearReceiver_clicked()
{
    receiver.clear();
    ui->eReceiver->setData(receiver);
    ui->lNBytes->clear();
}

void MainWindow::on_bSend_clicked()
{
    QByteArray data = ui->eSender->data();
    if (!serial.isOpen()) return;
    serial.write(data);
}

void MainWindow::on_bUartRefresh_clicked()
{
    QString st;
    ports = QSerialPortInfo::availablePorts();
    ui->serials->clear();
    for(int i=0; i<ports.size(); i++) {
        st = ports.at(i).portName() + QString(" - ") + ports.at(i).description();
        ui->serials->addItem(st, i);
    }
}

void MainWindow::on_bConnect_clicked()
{
    qint32 br;
    if (!serial.isOpen()) { // если не подключен
        // устанавливаем новый
        serial.setPort(curSer);
        serial.setDataBits(QSerialPort::Data8);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setParity(QSerialPort::NoParity);

        // подключаем
        if (serial.open(QIODevice::ReadWrite)) {
            connect(&serial, &QIODevice::readyRead, this, &MainWindow::onSerialData);
            serial.setBaudRate(baudrate);
            br = serial.baudRate();
            ui->lOK->setText(tr("Connected %1 @ %2 ").arg(serial.portName()).arg(br));
            receiver.clear();
            ui->eReceiver->setData(receiver);
            ui->bConnect->setText("Отключиться");
        }
        else {
            ui->lOK->setText(tr("Не удалось подключиться к %1").arg(serial.portName()));
            ui->bConnect->setText("Подключиться");
        }
    } else {
        serial.close();
        disconnect(&serial, SIGNAL(readyRead()), nullptr, nullptr);
        ui->bConnect->setText("Подключиться");
        ui->lOK->setText("Отключено");
    }
}

void MainWindow::on_eBaudRate_activated(const QString &arg1)
{
    baudrate = arg1.toInt();
}

void MainWindow::on_bReSend_clicked(bool checked)
{
    if (checked) { // stop
        resendTimer.stop();
        ui->bReSend->setText("Send&&Repeat");
    } else { // start
        resendTimer.start(ui->eReSendTimeMs->value());
        ui->bReSend->setText("Stop");
    }

}

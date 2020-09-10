#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QIcon>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QIcon *icon = new QIcon("ICON/icon1.png");
    ticon = new QSystemTrayIcon(this);
    ui->setupUi(this);
    ticon->setIcon(*icon);
    ticon->show();

    ui->eSender->setOverwriteMode(false);
    sender.clear();
    receiver.clear();
    ui->eSender->setData(sender);
    ui->eReceiver->setData(receiver);
    baudrate = ui->eBaudRate->currentText().toInt();
    on_bUartRefresh_clicked();

    connect(ticon, &QSystemTrayIcon::activated, this, &MainWindow::ticon_activated);
    ticon_activated(QSystemTrayIcon::Trigger);
    connect(&resendTimer, &QTimer::timeout, this, &MainWindow::on_bSend_clicked);
    connect(&refreshTimer, &QTimer::timeout, this, &MainWindow::on_bUartRefresh_clicked);
    refreshTimer.setInterval(5000);
    refreshTimer.start(5000);
}

MainWindow::~MainWindow()
{
    ticon->hide();
    delete ui;
}

void MainWindow::on_serials_activated(int index)
{
    QSerialPortInfo info = ports.at(index);
    curSer = info;
    m_curidx = index;
    qDebug() << "sel idx" << index;
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
    bytesSent = 0;
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

    qDebug() << "send()";

    serial.write(data);
    bytesSent += data.size();
    QString st = ui->lNBytes->text();
    st = tr(" Sent 0x%1 bytes, total 0x%2").arg(data.size()).arg(bytesSent);
    ui->label_3->setText(st);
}

void MainWindow::on_bUartRefresh_clicked()
{
    QString ost2, st2;
    QString st;
    QModelIndex midx;
    //int idx = 0;
    ports = QSerialPortInfo::availablePorts();
    ui->serials->clear();
    // store position
    int idx;
    midx = ui->serials->model()->index(m_curidx, 0);
    ost2 = ui->serials->itemData(m_curidx, Qt::DisplayRole).toString();
    //ost2 = ui->serials->currentText();
    idx = 0;
    for(int i=0; i<ports.size(); i++) {
        st = QString(" - %1:%2 - %3")
                .arg(ports.at(i).vendorIdentifier(), 2, 16, CH0)
                .arg(ports.at(i).productIdentifier(), 2, 16, CH0)
                .arg(ports.at(i).serialNumber());
        st2 = ports.at(i).portName() + st;
        if (st2 == ost2) idx = i;
        ui->serials->addItem(st2, i);
    }
    ui->serials->setCurrentIndex(idx);

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
            ui->lOK->setText(tr("Connected %1-%3:%4 @ %2 ").arg(serial.portName()).arg(br).arg(curSer.vendorIdentifier(), 4, 16, CH0).arg(curSer.productIdentifier(), 4, 16, CH0));
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
    if (!checked) { // stop
        qDebug() << "stop";
        resendTimer.stop();
        ui->bReSend->setText("Send && Repeat");
    } else { // start
        qDebug() << "start repeat";
        resendTimer.start(ui->eReSendTimeMs->value());
        ui->bReSend->setText("Stop");
    }

    //ui->bReSend->setChecked(checked);
}

void MainWindow::ticon_activated(QSystemTrayIcon::ActivationReason reason)
{
    QString st = "Available ports:\n";
    for (int i=0; i<ui->serials->count(); i++) st.append(tr("%1\n").arg(ui->serials->itemText(i)));
    ticon->showMessage("available ports", st, QIcon());
}

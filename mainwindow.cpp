#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QIcon>
#include <QDebug>
#include <QLineEdit>
#include <QCheckBox>

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
    connect(&refreshTimer, &QTimer::timeout, this, &MainWindow::on_TimerUartRefresh);
    refreshTimer.setInterval(5000);
    refreshTimer.start();
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

void MainWindow::onSerialError(QSerialPort::SerialPortError error)
{
    Q_UNUSED(error);
    qDebug() << "Serial Error" << serial.errorString();
    _disconn();
    ui->lOK->setText(serial.errorString());
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

void MainWindow::on_TimerUartRefresh()
{
    if(ui->serials->hasFocus()) return;
    on_bUartRefresh_clicked();
}

void MainWindow::on_bUartRefresh_clicked()
{
    QString ost2, st2;
    QString st;
    QModelIndex midx;
    int nidx=-1, oidx = 0; // new and old index of selected port
    oidx = ui->serials->currentIndex(); // old port index
    QString oPortName = ports.value(oidx).portName(); // old port name
    // refresh ports
    ports = QSerialPortInfo::availablePorts();
    ui->serials->clear();
    //// store position
    //int idx;
    //midx = ui->serials->model()->index(m_curidx, 0);
    //ost2 = ui->serials->itemData(m_curidx, Qt::DisplayRole).toString();
    //ost2 = ui->serials->currentText();
    //idx = 0;
    for(int i=0; i<ports.size(); i++) {
        st = QString(" - %1:%2 - %3")
                .arg(ports.at(i).vendorIdentifier(), 2, 16, CH0)
                .arg(ports.at(i).productIdentifier(), 2, 16, CH0)
                .arg(ports.at(i).serialNumber());
        st2 = ports.at(i).portName() + st;

        if (oPortName == ports.at(i).portName()) nidx = i;
        ui->serials->addItem(st2, i);
    }
    ui->serials->setCurrentIndex(nidx);
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
            connect(&serial, &QSerialPort::errorOccurred, this, &MainWindow::onSerialError);
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
    } else { // отключаем
        _disconn();
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
    Q_UNUSED(reason);
    QString st = "Available ports:\n";
    for (int i=0; i<ui->serials->count(); i++) st.append(tr("%1\n").arg(ui->serials->itemText(i)));
    ticon->showMessage("available ports", st, QIcon());
}

void MainWindow::_disconn()
{
    serial.close();
    disconnect(&serial, 0, 0, 0);
    ui->bConnect->setText("Подключиться");
    ui->lOK->setText("Отключено");

}

void MainWindow::on_bAddScript_clicked()
{
    // create horis layout
    //add lineedit for cmd, name, button send and button del
    QHBoxLayout *lay = new QHBoxLayout;
    QLabel *lNum = new QLabel(this); lNum->setText(tr("%1) ").arg(m_scrButs.size()+1));
    QLineEdit *eName = new QLineEdit(this);
    QLineEdit *eCmd = new QLineEdit(this);
    QCheckBox *cbHex = new QCheckBox(this);
    QPushButton *bSend = new QPushButton(this); bSend->setText("Send");
    QPushButton *bDel = new QPushButton(this); bDel->setText("Del");
    cbHex->setText("Hex");
    lay->addWidget(lNum);lay->addWidget(eName);lay->addWidget(eCmd, 2);lay->addWidget(cbHex);
    lay->addWidget(bSend); lay->addWidget(bDel); lay->addStretch(1);

    connect(bSend, &QPushButton::clicked, this, &MainWindow::onScriptSendButton);
    connect(bDel, &QPushButton::clicked, this, &MainWindow::onScriptDelButton);
    (static_cast<QBoxLayout*>(ui->tabScripts->layout()))->addLayout(lay);
    m_scrButs.append(bSend);
    m_scrDelButs.append(bDel);
    m_scrCommands.append(eCmd);
    m_scrLays.append(lay);
}

void MainWindow::onScriptSendButton()
{
    QPushButton* b = dynamic_cast<QPushButton*>(QObject::sender());
    if (b==nullptr) return;
    int idx = m_scrButs.indexOf(b);
    if (idx<0) return;
    qDebug() << "script send from " << b->text() << "idx" << idx;
    if (!serial.isOpen()) return;
    QLineEdit *le = dynamic_cast<QLineEdit*>(m_scrCommands.value(idx));

    QByteArray dat = QByteArray::fromHex(le->text().toLocal8Bit());
    serial.write(dat);
    bytesSent += dat.size();
    QString st = tr(" Sent 0x%1 bytes, total 0x%2").arg(dat.size()).arg(bytesSent);
    ui->label_3->setText(st);

}

void MainWindow::onScriptDelButton()
{
    QPushButton* b = dynamic_cast<QPushButton*>(QObject::sender());
    if (b==nullptr) return;
    int idx = m_scrDelButs.indexOf(b);
    qDebug() << "delete script" << idx;
    if (idx<0) return;
    QBoxLayout * lay = static_cast<QBoxLayout*>(ui->tabScripts->layout());
    QBoxLayout *lay2 = m_scrLays.value(idx);
    QLayoutItem *itm;
    while ((itm=lay2->takeAt(0))!=nullptr) {
        QWidget *b = itm->widget();
        delete itm;
        if (b==nullptr) continue;
        qDebug() << "delwidget:" << b->objectName();
        disconnect(b, 0,0,0);
        lay2->removeWidget(b);
        b->deleteLater();
        // todo: delete item from lists!
    };
    lay->removeItem(lay2);
    lay2->deleteLater();
}

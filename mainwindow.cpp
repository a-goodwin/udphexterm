#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QIcon>
#include <QDebug>
#include <QLineEdit>
#include <QCheckBox>
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkDatagram>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QCoreApplication::setApplicationName("QUdpHexTerm");
    QCoreApplication::setApplicationVersion(APP_VER_STR);
    QCoreApplication::setOrganizationName("a-goodwin");
    InitConf(set,"");

    ui->setupUi(this);
    this->setWindowTitle(windowTitle().append(" " APP_VER_STR));

    ui->eSender->setOverwriteMode(false);
    sender.clear();
    receiver.clear();
    ui->eSender->setData(sender);
    ui->eReceiver->setData(receiver);
    C2US(set, ui->eAddress);
    C2UI(set, ui->eOurPort);
    C2UI(set, ui->eTargetPort);

    sock = new QUdpSocket(this);

    connect(sock, &QUdpSocket::errorOccurred, this, &MainWindow::on_sockError);
    connect(sock, &QUdpSocket::readyRead, this, &MainWindow::onSockData);

    connect(&resendTimer, &QTimer::timeout, this, &MainWindow::on_bSend_clicked);

    _fetchScriptsFromFile();
}

MainWindow::~MainWindow()
{
    sock->abort();
    sock->disconnectFromHost();
    disconnect(sock, 0,0,0);
    sock->deleteLater();
    delete ui;
}

void MainWindow::onSockData()
{
    QByteArray data;
    qDebug() << "onSockData()";

    // тупое решение, так QUdpSocket будет тупить и пропускать частые
    // пакеты. Здесь надо сокет выделять в отдельный поток!
    while (sock->hasPendingDatagrams()) {
        QNetworkDatagram dgr = sock->receiveDatagram();
        data.append(dgr.data());
    }
    receiver.append(data);
    lastRecvBytes = data.size();
    ui->eReceiver->setData(receiver);
}

void MainWindow::on_bClearTransmitter_clicked()
{
    sender.clear();
    ui->eSender->setData(sender);
    bytesSent = 0;
    lastSendBytes = 0;
    _updateLabels();
}

void MainWindow::on_bClearReceiver_clicked()
{
    receiver.clear();
    ui->eReceiver->setData(receiver);
    ui->lNBytes->clear();
    lastRecvBytes=0;
    _updateLabels();
}

void MainWindow::on_bSend_clicked()
{
    QByteArray data;
    if (sendData.size()==0) data = ui->eSender->data();
    else { data = sendData; sendData.clear();}

    qDebug() << "send()";

    QHostAddress addr(ui->eAddress->text());
    sock->writeDatagram(data, addr, ui->eTargetPort->value());
    lastSendBytes = data.size();
    bytesSent += lastSendBytes;
    _updateLabels();
}

void MainWindow::on_sockError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "socket error" << static_cast<int>(socketError) << ":" << sock->errorString();
    if (sock->state()!=QAbstractSocket::ConnectedState) {
        sock->abort();
    }
}

void MainWindow::on_bConnect_clicked()
{
    sock->bind(QHostAddress::Any, ui->eOurPort->value());
    U2CS(set, ui->eAddress);
    U2CI(set, ui->eOurPort);
    U2CI(set, ui->eTargetPort);
    _updateLabels();
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
}

void MainWindow::_fetchScriptsFromFile()
{
    QFile f;
    f.setFileName("params.ini");
    f.open(QIODevice::ReadOnly);
    QString st;
    QString idxSt, cmdSt;
    int mid=0;
    while(!f.atEnd()) {
        st = f.readLine();
        mid = st.indexOf(',');
        idxSt = st.left(mid);
        if (idxSt.length()==0) continue;
        cmdSt = st.mid(mid+1);
        cmdSt.chop(1); // remove \n
        on_bAddScript_clicked(idxSt, cmdSt);
    }
    f.close();
}

void MainWindow::_saveScriptsToFile()
{
    QFile f;
    f.setFileName("params.ini");
    f.open(QIODevice::WriteOnly);
    QString st;
    QString idxSt, cmdSt;
    //int mid=0;
    int idx = 0;
    while(idx<m_scrNames.count()) {
        st = m_scrNames.value(idx)->text() + "," + m_scrCommands.value(idx)->text()+"\n";
        f.write(st.toLocal8Bit());
        idx++;
    }
    f.close();
}

void MainWindow::_updateLabels()
{
    ui->lOK->setText(tr("Connected %1:%2").arg(sock->peerAddress().toString()).arg(sock->localPort()));
    QString st = ui->lNBytes->text();

    st = tr(" Sent 0x%1 bytes, total 0x%2").arg(lastSendBytes).arg(bytesSent);
    ui->label_3->setText(st);

    ui->lNBytes->setText(tr("Read 0x%1 bytes, total 0x%2").arg(lastRecvBytes, 4, 16, CH0).arg(receiver.size(), 4, 16, CH0));
}

void MainWindow::on_bAddScript_clicked(QString nm, QString cmd)
{
    // create horiz layout
    //add lineedit for cmd, name, button send and button del
    QHBoxLayout *lay = new QHBoxLayout;
    QLabel *lNum = new QLabel(this); lNum->setText(tr("%1) ").arg(m_scrButs.size()+1));
    QLineEdit *eName = new QLineEdit(this); eName->setText(nm);
    QLineEdit *eCmd = new QLineEdit(this); eCmd->setText(cmd);
    //QCheckBox *cbHex = new QCheckBox(this);
    QPushButton *bSend = new QPushButton(this); bSend->setText("Send");
    QPushButton *bDel = new QPushButton(this); bDel->setText("Del");
    //cbHex->setText("Hex");
    lay->addWidget(lNum);lay->addWidget(eName);lay->addWidget(eCmd, 2);//lay->addWidget(cbHex);
    lay->addWidget(bSend); lay->addWidget(bDel); lay->addStretch(0);

    connect(bSend, &QPushButton::clicked, this, &MainWindow::onScriptSendButton);
    connect(bDel, &QPushButton::clicked, this, &MainWindow::onScriptDelButton);
    (static_cast<QBoxLayout*>(ui->tabScripts->layout()))->addLayout(lay);
    m_scrButs.append(bSend);
    m_scrDelButs.append(bDel);
    m_scrNames.append(eName);
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
    if (!sock->isOpen()) return;
    QLineEdit *le = dynamic_cast<QLineEdit*>(m_scrCommands.value(idx));

    sendData = QByteArray::fromHex(le->text().toLocal8Bit());
    on_bSend_clicked();
}

void MainWindow::onScriptDelButton(int defIdx)
{
    int idx;
    if (defIdx<0) {
        QPushButton* b = dynamic_cast<QPushButton*>(QObject::sender());
        if (b==nullptr) return;
        idx = m_scrDelButs.indexOf(b);
    } else idx = defIdx;
    qDebug() << "delete script" << idx;
    if (idx<0) return;
    QBoxLayout * lay = static_cast<QBoxLayout*>(ui->tabScripts->layout());
    QBoxLayout *lay2 = m_scrLays.value(idx);
    QLayoutItem *itm;

    // remove from lists
    m_scrLays.removeAt(idx);
    m_scrButs.removeAt(idx);
    m_scrDelButs.removeAt(idx);
    m_scrNames.removeAt(idx);
    m_scrCommands.removeAt(idx);

    // remove items from layout, and delete them
    while ((itm=lay2->takeAt(0))!=nullptr) {
        QWidget *w = itm->widget();
        delete itm;
        if (w==nullptr) continue;
        qDebug() << "delwidget:" << w->objectName();
        disconnect(w, 0,0,0);
        lay2->removeWidget(w);
        w->deleteLater();
        // todo: delete item from lists!
    };
    // remove layout
    lay->removeItem(lay2);
    lay2->deleteLater();
}

void MainWindow::on_bSaveScripts_clicked()
{
    _saveScriptsToFile();
}

void MainWindow::on_bClearScripts_clicked()
{
    int idx, max;
    max = m_scrButs.count();
    for (idx = 0; idx<max; idx++)
        onScriptDelButton(idx);
}

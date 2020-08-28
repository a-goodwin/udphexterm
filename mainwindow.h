#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QSystemTrayIcon>

#define CH0 QLatin1Char('0')

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSystemTrayIcon *ticon;
protected:
    QByteArray sender, receiver;
    QSerialPort serial;
    QList<QSerialPortInfo> ports;
    qint32 baudrate;
    QSerialPortInfo curSer;
    QTimer resendTimer, refreshTimer;
private slots:
    void on_serials_activated(int index);
    void onSerialData();
    void on_bClearTransmitter_clicked();
    void on_bClearReceiver_clicked();
    void on_bSend_clicked();
    void on_bUartRefresh_clicked();
    void on_bConnect_clicked();
    void on_eBaudRate_activated(const QString &arg1);

    void on_bReSend_clicked(bool checked);
    void ticon_activated(QSystemTrayIcon::ActivationReason reason);
private:
    Ui::MainWindow *ui;

    int bytesSent=0;
    int m_curidx = -1;
};

#endif // MAINWINDOW_H

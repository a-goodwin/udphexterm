#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QUdpSocket>
#include "configutils.h"

#define CH0 QLatin1Char('0')

#define APP_VER_STR "0.01"

namespace Ui {
class MainWindow;
}
class QSettings;
class QPushButton;
class QLineEdit;
class QHBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //QSystemTrayIcon *ticon;
protected:
    QSettings *set;

    QByteArray sender, receiver;
    QUdpSocket *sock;
    //qint32 baudrate;
    QTimer resendTimer;
    QByteArray sendData;
private slots:
    void onSockData();
    void on_bClearTransmitter_clicked();
    void on_bClearReceiver_clicked();
    void on_bSend_clicked();
    void on_sockError(QAbstractSocket::SocketError socketError);

    //void on_TimerUartRefresh();
    //void on_bUartRefresh_clicked();
    void on_bConnect_clicked();
    void on_bReSend_clicked(bool checked);
    //void ticon_activated(QSystemTrayIcon::ActivationReason reason);


    void on_bAddScript_clicked(QString nm="", QString cmd="");
    void onScriptSendButton();
    void onScriptDelButton(int defIdx=-1);
    void on_bSaveScripts_clicked();
    void on_bClearScripts_clicked();

private:
    //void _disconn();
    void _fetchScriptsFromFile();
    void _saveScriptsToFile();
    void _updateLabels();
    Ui::MainWindow *ui;

    int bytesSent=0;
    int m_curidx = -1;
    int lastSendBytes=0;
    int lastRecvBytes=0;
    QList<QPushButton*> m_scrButs;
    QList<QHBoxLayout*> m_scrLays;
    QList<QPushButton*> m_scrDelButs;
    QList<QLineEdit*> m_scrNames;
    QList<QLineEdit*> m_scrCommands;
};

#endif // MAINWINDOW_H

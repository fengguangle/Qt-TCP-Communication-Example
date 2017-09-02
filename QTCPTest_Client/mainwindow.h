#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QTimer>
#include <QTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handleSocketError(QAbstractSocket::SocketError);
    void handleSocketReadyRead();
    void handlSocketDisconnect();
    void serverTimeout();

    void on_pushButton_connect_clicked();

    void on_pushButton_send_clicked();

private:
    Ui::MainWindow *ui;

    QTcpSocket *m_Socket;

    QString m_sLocalIP;
    QString m_sRemoteIP;
    quint16 m_sPort;
    bool isConnect;
    QByteArray m_bufferSend;
    QByteArray m_bufferReceive;

    void log(QString text);

    QTimer *heartbeatTimer;
    QByteArray heartbeatMsg;
};

#endif // MAINWINDOW_H

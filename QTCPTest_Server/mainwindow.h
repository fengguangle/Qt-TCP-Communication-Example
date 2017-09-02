#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QDebug>
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
    void handleNewConnection();
    void handleSocketError(QAbstractSocket::SocketError);
    void handleSocketReadyRead();
    void handlSocketDisconnect();
    void heartbeatTimeout();

    void on_pushButton_listen_clicked();

    void on_pushButton_send_clicked();

private:
    Ui::MainWindow *ui;

    QTcpServer *m_Server;
    QTcpSocket *m_Socket;

    QString m_sLocalIP;
    QString m_sRemoteIP;
    quint16 m_sPort;
    bool isListening;
    bool isConnect;
    QByteArray m_bufferSend;
    QByteArray m_bufferReceive;

    bool listen();
    void log(QString text);

    QTimer *heartbeatTimer;
    char *heartbeatMsg;
};

#endif // MAINWINDOW_H

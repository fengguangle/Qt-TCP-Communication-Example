#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_Socket=new QTcpSocket(this);
    connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleSocketError(QAbstractSocket::SocketError)));
    connect(m_Socket, SIGNAL(readyRead()), this, SLOT(handleSocketReadyRead()));
    connect(m_Socket,SIGNAL(disconnected()),this,SLOT(handlSocketDisconnect()));

    m_sLocalIP="192.168.0.111";
    m_sRemoteIP="192.168.0.109";
    m_sPort=12345;
    isConnect=false;

    ui->lineEdit_serverIP->setText(m_sRemoteIP);
    ui->lineEdit_serverPort->setText(QString("%1").arg(m_sPort));

    heartbeatTimer=new QTimer(this);
    heartbeatTimer->setInterval(5000);
    connect(heartbeatTimer,SIGNAL(timeout()),this,SLOT(serverTimeout()));

    heartbeatMsg.resize(1);
    heartbeatMsg.append(0xFF);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_connect_clicked()
{
    m_sRemoteIP=ui->lineEdit_serverIP->text();
    m_sPort=ui->lineEdit_serverPort->text().toShort();

    if(isConnect){
        m_Socket->close();
        isConnect=false;
        ui->pushButton_connect->setText("连接");
        log(QString("断开连接"));
        return;
    }
    m_Socket->connectToHost(QHostAddress(m_sRemoteIP),m_sPort);
    if (m_Socket->waitForConnected(1000)) {
        log(QString("连接成功 %1 %2")
            .arg(m_Socket->peerAddress().toString())
            .arg(m_Socket->peerPort()));
        ui->pushButton_connect->setText("断开连接");
        isConnect=true;
        heartbeatTimer->start();
        ui->textBrowser_heartbeat->clear();
    }
    else {
        log(QString("连接失败 %1").arg(m_Socket->errorString()));
    }

}

void MainWindow::handleSocketReadyRead()
{
    QByteArray baRead=m_Socket->readAll();

    if(baRead.length()==1&&baRead[0]==0x74)
    {
        ui->textBrowser_heartbeat->append(QTime::currentTime().toString("HH:mm:ss zzz")
                                       +":"+baRead);
        heartbeatTimer->start();
    }
    else
    {
        ui->textBrowser_recive->append(baRead);
    }
}

void MainWindow::handleSocketError(QAbstractSocket::SocketError)
{
    log(QString("Socket错误 %1").arg(m_Socket->errorString()));
}

void MainWindow::log(QString text)
{
    ui->textBrowser_log->append(text);
}

void MainWindow::handlSocketDisconnect()
{
    log(QString("Socket disconnect %1").arg(m_Socket->state()));
    if(isConnect){
        isConnect=false;
        ui->pushButton_connect->setText("连接");
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    QString sSend=ui->lineEdit_send->text();

//    QByteArray block;
//    QDataStream out(&block, QIODevice::WriteOnly);
//    out.setByteOrder(QDataStream::LittleEndian);
//    out.setVersion(QDataStream::Qt_5_7);

//    out << sSend.toLatin1();
//    out.device()->seek(0);
//    out << quint16(block.size() - sizeof(quint16));

    if(isConnect){
        m_Socket->write(sSend.toLatin1());
        if(m_Socket->waitForBytesWritten())
            log("写数据成功");
        else
            log("写数据失败");
    }
}

void MainWindow::serverTimeout()
{
    log("服务器心跳超时,将重新尝试连接...");
    on_pushButton_connect_clicked();
}

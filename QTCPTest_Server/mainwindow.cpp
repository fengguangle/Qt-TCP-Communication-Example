#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_Server=new QTcpServer(this);
    m_Socket=NULL;
    //m_Socket=new QTcpSocket(this);

    m_sLocalIP="192.168.0.109";
    m_sRemoteIP="192.168.0.111";
    m_sPort=12345;
    isListening=false;
    isConnect=false;

    ui->lineEdit_listenIP->setText(m_sLocalIP);
    ui->lineEdit_listenPort->setText(QString("%1").arg(m_sPort));

    heartbeatTimer=new QTimer(this);
    heartbeatTimer->setInterval(4000);
    connect(heartbeatTimer,SIGNAL(timeout()),this,SLOT(heartbeatTimeout()));

    heartbeatMsg=new char('t');

    listen();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::listen()
{
    if(m_Server==NULL||isListening)
        return false;
    if(!m_Server->listen(QHostAddress(m_sLocalIP),m_sPort)){
        ui->pushButton_listen->setText("监听");
        log("开启监听失败:"+m_Server->errorString());
        return false;
    }
    ui->pushButton_listen->setText("停止监听");
    isListening=true;
    connect(m_Server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    log(QString("开始监听 %1 %2")
        .arg(m_Server->serverAddress().toString())
        .arg(m_Server->serverPort()));
    return true;
}

void MainWindow::handleNewConnection()
{
    if(heartbeatTimer->isActive())
        heartbeatTimer->stop();
    m_bufferReceive.clear();
    qDebug()<<m_Socket;
    if(m_Socket!=NULL){
        log(QString("连接前state %1").arg(m_Socket->state()));
        if(m_Socket->state()==QAbstractSocket::SocketState::ConnectedState){
            m_Socket->abort();
        }
        m_Socket->deleteLater();
        delete m_Socket;
        m_Socket=NULL;
    }
    qDebug()<<m_Socket;
    m_Socket = m_Server->nextPendingConnection();

    connect(m_Socket, SIGNAL(readyRead()), this, SLOT(handleSocketReadyRead()));
    connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleSocketError(QAbstractSocket::SocketError)));
    connect(m_Socket,SIGNAL(disconnected()),this,SLOT(handlSocketDisconnect()));

    log(QString("新连接 %1 %2")
        .arg(m_Socket->peerAddress().toString())
        .arg(m_Socket->peerPort()));
    log(QString("连接后state %1").arg(m_Socket->state()));
    isConnect=true;
    heartbeatTimer->start();
    ui->textBrowser_heartbeat->clear();
}

void MainWindow::handleSocketError(QAbstractSocket::SocketError)
{
    log(QString("Socket错误 %1").arg(m_Socket->errorString()));
    log(QString("错误处state %1").arg(m_Socket->state()));
}

void MainWindow::handleSocketReadyRead()
{
    ui->textBrowser_recive->append(m_Socket->readAll());
}

void MainWindow::on_pushButton_listen_clicked()
{
    m_sLocalIP=ui->lineEdit_listenIP->text();
    m_sPort=ui->lineEdit_listenPort->text().toShort();
    if(isListening){
        if(m_Socket!=NULL){
            if(m_Socket->state()==QAbstractSocket::SocketState::ConnectedState){
                m_Socket->disconnectFromHost();
            }
        }
        m_Server->close();
        ui->pushButton_listen->setText("监听");
        disconnect(m_Server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
        log("停止监听");
        isListening=false;
    }
    else{
        listen();
    }
}

void MainWindow::log(QString text)
{
    ui->textBrowser_log->append(text);
}

void MainWindow::handlSocketDisconnect()
{
    log(QString("Socket disconnect %1").arg(m_Socket->state()));
    disconnect(m_Socket, SIGNAL(readyRead()), this, SLOT(handleSocketReadyRead()));
    disconnect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleSocketError(QAbstractSocket::SocketError)));
    disconnect(m_Socket,SIGNAL(disconnected()),this,SLOT(handlSocketDisconnect()));
    isConnect=false;
    if(heartbeatTimer->isActive())
        heartbeatTimer->stop();
}

void MainWindow::heartbeatTimeout()
{
    if(m_Socket!=NULL&&m_Socket->state()==QAbstractSocket::SocketState::ConnectedState){
        //char *data="t";
        m_Socket->write(heartbeatMsg);
        if(m_Socket->waitForBytesWritten()){
            ui->textBrowser_heartbeat->append(QTime::currentTime().toString("HH:mm:ss zzz")
                                           +":"+heartbeatMsg);
        }
        else
            ui->textBrowser_heartbeat->append(QTime::currentTime().toString("HH:mm:ss zzz")
                                           +":写心跳失败");
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    QString sSend=ui->lineEdit_send->text();
    if(isConnect){
        m_Socket->write(sSend.toLatin1());
        if(m_Socket->waitForBytesWritten())
            log("写数据成功");
        else
            log("写数据失败");
    }
    else
        log("未连接");
}

#include "networkuntil.h"
#include <QHostInfo>
NetWorkUntil::NetWorkUntil(QObject *parent)
    : QObject{parent}
{
    //创建tcp服务器
    this->tcpServer=new QTcpServer(this);
    //监听本地ipv4地址和自定端口1314
    int port=1314;
    qDebug()<<"服务器端口号："<<QString::number(port);
    this->tcpServer->listen(this->getLocalHost(),port);
    connect(this->tcpServer,SIGNAL(newConnection()),this,SLOT(handleTcpNewConnected()));
}

NetWorkUntil::~NetWorkUntil()
{
    if(this->tcpServer->isListening())
        this->tcpServer->close();
    for(const auto &tcpSocket:this->tcpSocketList){
        if(tcpSocket!=nullptr&&tcpSocket->state()==QAbstractSocket::ConnectedState)//如果不为空并且还在连接
            tcpSocket->disconnectFromHost();//断开与客户端的连接
    }
}

/**
 * @brief NetWorkUntil::getLocalHost
 * @return
 * 获取本地ipv4地址
 */
QHostAddress NetWorkUntil::getLocalHost()
{
    QString hostName=QHostInfo::localHostName();
    QHostInfo hostInfo=QHostInfo::fromName(hostName);//获取本机网络信息
    QList<QHostAddress> addressList=hostInfo.addresses();//所有本机网络地址信息
    foreach(QHostAddress hostAddress,addressList){
        if(QAbstractSocket::IPv4Protocol==hostAddress.protocol()){
            //打印服务器使用的ip地址
            qDebug()<<"服务器ip地址："<<hostAddress.toString();
            return hostAddress;
        }
    }
    //null没有网络地址，直接报错然后退出程序
    qDebug()<<"没有找到ipv4，网络地址";
    exit(0);
}

/**
 * @brief NetWorkUntil::handleNewConnection
 * 有新连接的处理函数,在server中的一般是获得socket的对象，具体对象处理什么放在socket中的connected信号处理函数中
 */
void NetWorkUntil::handleTcpNewConnected()
{
    qDebug()<<"新连接建立,server";

    //建立一个新socket
    MyTcpSocket *tcpSocket=new MyTcpSocket(this);
    tcpSocket->setSocketDescriptor(this->tcpServer->nextPendingConnection()->socketDescriptor());//获取客户端连接的socket
    this->tcpSocketList.append(tcpSocket);//加入列表中
    //连接信号
    connect(tcpSocket,&MyTcpSocket::connected,tcpSocket,&MyTcpSocket::handleTcpSocketConnected);
    connect(tcpSocket,&MyTcpSocket::disconnected,tcpSocket,&MyTcpSocket::handleTcpSocketDisconnected);
    connect(tcpSocket,&MyTcpSocket::readyRead,tcpSocket,&MyTcpSocket::handleTcpSocketReadyRead);
    connect(tcpSocket,&MyTcpSocket::deleteSelf,this,&NetWorkUntil::deleteTcpSocket);
}

void NetWorkUntil::deleteTcpSocket(MyTcpSocket *tcpsocket)
{
    if(tcpsocket!=nullptr&&tcpsocket->state()==QAbstractSocket::ConnectedState)//如果不为空并且还在连接
        tcpsocket->disconnectFromHost();//断开与客户端的连接
    this->tcpSocketList.removeOne(tcpsocket);//移除元素
    delete tcpsocket;
    tcpsocket=nullptr;
}

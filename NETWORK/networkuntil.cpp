#include "networkuntil.h"
#include "protocol.h"
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

NetWorkUntil &NetWorkUntil::getInstance()
{
    static NetWorkUntil instance;
    return instance;
}

NetWorkUntil::~NetWorkUntil()
{
    if(this->tcpServer->isListening())
        this->tcpServer->close();
}

void NetWorkUntil::addDevice(QString username, MyTcpSocket *tcpSocket)
{
    //如果map中没有这个用户，就新建一个列表
    if(!this->userDevices.contains(username)){
        QList<MyTcpSocket*> list;
        list.append(tcpSocket);
        this->userDevices.insert(username,list);
    }else{//有这个用户，就加入这个列表
        QList<MyTcpSocket*> list=this->userDevices.value(username);
        list.append(tcpSocket);
        this->userDevices.insert(username,list);
    }
}

void NetWorkUntil::deleteDevice(QString username, MyTcpSocket *tcpSocket)
{
    //如果map中没有这个用户，则不管
    if(this->userDevices.contains(username)){
        QList<MyTcpSocket*> list=this->userDevices.value(username);
        //如果list中有这个tcpSocket，则删除
        int index=list.indexOf(tcpSocket);
        if(index!=-1)
            list.removeAt(index);
        if(list.size()==0)
            this->userDevices.remove(username);
        else
            this->userDevices.insert(username,list);
    }
}

void NetWorkUntil::synchronizeDevice(QString username, MyTcpSocket *orgin_socket,QByteArray dbData,QString medifyTime)
{
    //对该用户除orgin_socket的设备发送更新请求
    QList<MyTcpSocket*> list=this->userDevices.value(username);
    QJsonObject jsObj;
    jsObj.insert(MEDIFYTIME,medifyTime);
    QString d=dbData.toBase64();
    jsObj.insert(DB_DATA,d);
    for(auto socket:list){
        if(socket==orgin_socket)
            continue;
        qDebug()<<"发出:"<<MsgTypeMeans.at(UPDATE);
        socket->sendData(createSendData(UPDATE,jsObj));
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
            qDebug()<<"ip:"<<hostAddress;
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
    //连接信号
    connect(tcpSocket,&MyTcpSocket::connected,tcpSocket,&MyTcpSocket::handleTcpSocketConnected);
    connect(tcpSocket,&MyTcpSocket::disconnected,tcpSocket,&MyTcpSocket::handleTcpSocketDisconnected);
    connect(tcpSocket,&MyTcpSocket::readyRead,tcpSocket,&MyTcpSocket::handleTcpSocketReadyRead);
    connect(tcpSocket,&MyTcpSocket::deleteSelf,this,&NetWorkUntil::deleteTcpSocket);
    if(tcpSocket->state()==QAbstractSocket::ConnectedState)
        this->tcpSocketList.append(tcpSocket);//加入列表中
}

void NetWorkUntil::deleteTcpSocket(MyTcpSocket *tcpsocket)
{
    qDebug()<<"socket断开";
    tcpsocket->close();
    this->tcpSocketList.removeOne(tcpsocket);//移除元素
    tcpsocket->deleteLater();
    tcpsocket=nullptr;
}

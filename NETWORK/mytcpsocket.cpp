#include "mytcpsocket.h"
#include "networkuntil.h"
#include "protocol.h"
#include <QThreadPool>
#include <DATA/mydata.h>
#include <DATA/tcptask.h>
MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{

}

MyTcpSocket::~MyTcpSocket()
{
}

/**
 * @brief NetWorkUntil::handleSocketConnected
 * Tcpsocket连接成功的槽函数
 */
void MyTcpSocket::handleTcpSocketConnected()
{
    qDebug()<<"socket连接成功";
}
/**
 * @brief NetWorkUntil::handleSocketDisconnected
 * Tcpsocket断开连接的槽函数
 */
void MyTcpSocket::handleTcpSocketDisconnected()
{
    //删除锁和已登录设备
    MyData::getInstance().deleteLock(this->username);
    NetWorkUntil::getInstance().deleteDevice(this->username,this);
    //发出信号通知server删除自己
    emit deleteSelf(this);
}
/**
 * @brief NetWorkUntil::handleSocketReadyRead
 * Tcpsocket收到数据的槽函数
 * 当服务端发送数据来时，多线程调用处理函数
 */
void MyTcpSocket::handleTcpSocketReadyRead()
{
    while(this->bytesAvailable()){
    //存入缓冲区
    this->m_buffer.append(this->readAll());
    //查看是否收到完整头
    if(this->m_buffer.size()<static_cast<qsizetype>(sizeof(quint32)*2)){//因为协议的头是由两个32int组成的
        qDebug()<<"没接收到完整头";
        return;
    }
    //查看是否接受到完整数据
    PduHearder header=deserializeHeader(this->m_buffer.left(sizeof(quint32)*2));//用两个int32组成头部

    if(this->m_buffer.mid(sizeof(quint32)*2).size()<header.length){//如果当前缓冲区的数据去掉头，长度不与length一致
        qDebug()<<"没接收到完整消息";
        return;
    }
    QByteArray jsData=this->m_buffer.mid(sizeof(quint32)*2,header.length);
    this->m_buffer.remove(0,sizeof(quint32)*2+header.length);//移除缓冲区数据
    qDebug()<<"接收到完整数据，开始处理";
    //组建pdu
    Pdu pdu;
    pdu.header=header;
    pdu.data=QJsonDocument::fromJson(jsData).object();
    //提取用户名
    if(pdu.data.contains(USERNAME))
        this->username=pdu.data.value(USERNAME).toString();
    //把pdu的数据赋予线程去处理
    TcpTask *task=new TcpTask(pdu,this);
    //多线程处理任务
    QThreadPool::globalInstance()->start(task);//task执行完毕后，会自动删除task对象
    }
}

void MyTcpSocket::sendData(const QByteArray &data)
{
    this->write(data);
}

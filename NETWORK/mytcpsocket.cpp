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
    //检测收到的数据是否是完整的json格式
    this->m_buffer.append(this->readAll());
    // qDebug()<<this->m_buffer;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(m_buffer, &error);
    if(error.error!=QJsonParseError::NoError){//说明没接收到完整消息
        qDebug()<<"没有接收到完整消息";
        return;
    }
    //收到完整消息了 释放缓冲区
    this->m_buffer.clear();
    qDebug()<<"接收到完整数据，开始处理";
    PDU pdu(doc);
    if(pdu.data.contains(USERNAME)){
        this->username=pdu.data.value(USERNAME).toString();
    }
    //把pdu的数据赋予线程去处理
    TcpTask *task=new TcpTask(pdu,this);
    //多线程处理任务
    QThreadPool::globalInstance()->start(task);//task执行完毕后，会自动删除task对象
}

void MyTcpSocket::sendData(const QByteArray &data)
{
    this->write(data);
}

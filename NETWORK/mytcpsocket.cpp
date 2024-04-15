#include "mytcpsocket.h"
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
    qDebug()<<"socket断开";
    //发出信号通知server删除自己
    MyData::getInstance().deleteLock(this->username);
    emit deleteSelf(this);
}
/**
 * @brief NetWorkUntil::handleSocketReadyRead
 * Tcpsocket收到数据的槽函数
 * 当服务端发送数据来时，多线程调用处理函数
 */
void MyTcpSocket::handleTcpSocketReadyRead()
{
    qDebug()<<"收到客户端的数据";
    PDU pdu(this->readAll());
    qDebug()<<"消息类型:"<<pdu.msgType;
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
    qDebug()<<"发送数据";
    this->write(data);
}

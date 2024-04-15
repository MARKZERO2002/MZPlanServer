#ifndef NETWORKUNTIL_H
#define NETWORKUNTIL_H

#include "mytcpsocket.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
class NetWorkUntil : public QObject
{
    Q_OBJECT

private:
    QTcpServer *tcpServer;
    QList<MyTcpSocket*> tcpSocketList;
public:
    explicit NetWorkUntil(QObject *parent = nullptr);
    ~NetWorkUntil();
private:
    QHostAddress getLocalHost();//获取本地ipv4地址
public slots:
    void handleTcpNewConnected();//tcpserver传入新连接
    void deleteTcpSocket(MyTcpSocket* tcpsocket);
signals:
};

#endif // NETWORKUNTIL_H

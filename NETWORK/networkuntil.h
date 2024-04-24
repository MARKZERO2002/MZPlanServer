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
    explicit NetWorkUntil(QObject *parent = nullptr);
    QTcpServer *tcpServer;
    QList<MyTcpSocket*> tcpSocketList;
public:
    static NetWorkUntil& getInstance();
    ~NetWorkUntil();
    void addDevice(QString username,MyTcpSocket* tcpSocket);//为一个用户添加一个登陆设备
    void deleteDevice(QString username,MyTcpSocket* tcpSocket);//为一个用户删除一个登陆设备
private:
    QHostAddress getLocalHost();//获取本地ipv4地址
    QMap<QString,QList<MyTcpSocket*>> userDevices;//存储已登录的用户设备 用于发送更新请求
public slots:
    void handleTcpNewConnected();//tcpserver传入新连接
    void addTcpSocket(MyTcpSocket* tcpsocket);
    void deleteTcpSocket();
    void synchronizeDevice(QString username,MyTcpSocket* orgin_socket,QByteArray dbData,QString medifyTime);//对除orgin_socket外的socket发送更新请求
signals:
};

#endif // NETWORKUNTIL_H

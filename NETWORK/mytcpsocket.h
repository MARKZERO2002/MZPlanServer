#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
private:
    QString username;
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    ~MyTcpSocket();
    Q_INVOKABLE void sendData(const QByteArray &data);//子线程处理好了数据 发送消息
public slots:
    void handleTcpSocketConnected();//tcpsocket连接
    void handleTcpSocketDisconnected();//tcpsocket断开连接
    /**
     * @brief NetWorkUntil::handleSocketReadyRead
     * Tcpsocket收到数据的槽函数
     * 当服务端发送数据来时，多线程调用处理函数
     */
    void handleTcpSocketReadyRead();//tcp收到消息
private:
    QByteArray m_buffer;//接收数据缓冲区

signals:
    void deleteSelf(MyTcpSocket *tcpScoket);
};

#endif // MYTCPSOCKET_H

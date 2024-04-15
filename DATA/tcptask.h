#ifndef TCPTASK_H
#define TCPTASK_H
#include <QJsonObject>
#include <QRunnable>
#include "NETWORK/protocol.h"

/**
 * @brief The TcpTask class
 * 处理一个Tcp任务的单个线程对象
 */
class TcpTask : public QRunnable
{
public:
    TcpTask(const PDU &pdu,QObject *obj);
    void run() override;
private:
    void handleLogin();
    void handleRegist();
    void handleCancel();
    void handleSynochronize();
    void sendData(const QByteArray &data);
private:
    MsgType msgType;//消息类型
    QJsonObject data;//数据
    QObject *obj;
};

#endif // TCPTASK_H

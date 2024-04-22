#include "mydata.h"
#include "tcptask.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <qtimer.h>
#include <ENTITY/user.h>
#include <NETWORK/networkuntil.h>
TcpTask::TcpTask(const Pdu &pdu,MyTcpSocket *socket)
{
    this->msgType=pdu.header.msgType;
    this->data=pdu.data;
    this->socket=socket;
    qDebug()<<socket;
}

void TcpTask::run()
{
    if(this->msgType<=MsgTypeMeans.size())
        qDebug()<<"收到："<<MsgTypeMeans.at(this->msgType);
    //根据消息类型判断怎么处理
    switch(this->msgType){
    case LOGIN_REQUEST:
        handleLogin();
        break;
    case REGIST_REQUEST:
        handleRegist();
        break;
    case CANCEL_REQUEST:
        handleCancel();
        break;
    case SYNOCHRONIZE_PLAN_REQUEST:
        handleSynochronize();
        break;
    default:
        qDebug()<<"消息类型"<<this->msgType<<"未找到处理函数";
        break;
    }
}
/**
 * @brief TcpTask::handleLogin
 * 处理登陆
 */
void TcpTask::handleLogin()
{
    QJsonObject jsObj=this->data;
    QString username=jsObj.value(USERNAME).toString(),password=jsObj.value(PASSWORD).toString();//此处可对密码添加解密功能
    QJsonObject ansData;//返回的数据
    //去数据库中查询用户信息
    User user=MyData::getInstance().getUser(username);
    //如果用户存在，对比密码
    if(!user.isNull()){
        if(user.getPassword()==password){
            //密码正确，发送tcp返回给客户端，由客户端断开连接
            ansData.insert(CHECK,true);
            ansData.insert(USERNAME,username);
        }else{
            //密码错误，发送信息给客户端，客户端选择断开连接或再次发送
            ansData.insert(CHECK,false);
            ansData.insert(MSG_STRING,"密码错误");
        }
    }else{
        //发送用户不存在的信息
        ansData.insert(CHECK,false);
        ansData.insert(MSG_STRING,"用户不存在");
    }
    MyData::getInstance().creadLock(username);
    NetWorkUntil::getInstance().addDevice(username,this->socket);//加入登陆设备
    qDebug()<<"发出:"<<MsgTypeMeans.at(LOGIN_RESPONSE);
    this->sendData(createSendData(LOGIN_RESPONSE,ansData));
    // this->tcpSocket->write(ansJsDoc.toJson());//不要在子线程中操作其它线程生成的socket
}
/**
 * @brief TcpTask::handleRegist
 * 处理注册
 */
void TcpTask::handleRegist()
{
    QJsonObject jsObj=this->data;
    QString username=jsObj.value(USERNAME).toString(),password=jsObj.value(PASSWORD).toString();//此处可对密码添加解密功能
    QJsonObject ansData;//返回的数据
    //去数据库中查询用户信息
    User user=MyData::getInstance().getUser(username);
    //如果用户不存在，则可以注册
    if(user.isNull()){
        User user=User(username,password);
        if(MyData::getInstance().insertUser(user)){
            //插入成功
            ansData.insert(CHECK,true);
        }else{
            //插入失败
            ansData.insert(CHECK,false);
            ansData.insert(MSG_STRING,"新增用户失败，未知原因");
        }
    }else{
        //发送用户已存在的信息
        ansData.insert(CHECK,false);
        ansData.insert(MSG_STRING,"新增用户失败，用户已存在");
    }
    qDebug()<<"发出:"<<MsgTypeMeans.at(REGIST_RESPONSE);
    this->sendData(createSendData(REGIST_RESPONSE,ansData));
}
/**
 * @brief TcpTask::handleCancel
 * 处理注销
 */
void TcpTask::handleCancel()
{
    //提取数据
    QJsonObject jsObj=this->data;
    QString username=jsObj.value(USERNAME).toString();
    MyData::getInstance().creadLock(username);//锁住后进行删除操作
    //返回的数据
    QJsonObject ansData;
    //删除用户信息
    if(MyData::getInstance().removeUserAllData(username)){
        ansData.insert(CHECK,true);
    }else{
        ansData.insert(CHECK,false);
    }
    qDebug()<<"发出:"<<MsgTypeMeans.at(CANCEL_RESPONSE);
    this->sendData(createSendData(CANCEL_RESPONSE,ansData));
}
/**
 * @brief TcpTask::handleSynochronize
 * 处理同步，即用户第一次登陆之后或点击同步时发出的信号
 * 要求比对用户发来的数据，若用户发来的数据更新，则替换磁盘数据，返回的信息里needUpdate:false
 * 若一样新，则返回的信息里needUpdate:false
 * 若更旧，则传回数据needUpdate:true plan:... doneplan:...
 */
void TcpTask::handleSynochronize()
{
    //接受的数据
    QJsonObject jsObj=this->data;
    QString username=jsObj.value(USERNAME).toString();
    QByteArray dbData=QByteArray::fromBase64(jsObj.value(DB_DATA).toString().toUtf8());
    QString userMedifyTime=jsObj.value(MEDIFYTIME).toString();
    MyData::getInstance().creadLock(username);
    //返回的数据
    QJsonObject ansData;
    //查询本地修改时间和用户的修改时间是否一致
    QString serverMedifyTime=MyData::getInstance().getMedifyTime(username).toString(DTFORMAT);
    if(serverMedifyTime>userMedifyTime){
        //服务器的新，发送数据给用户
        //返回信息
        ansData.insert(MEDIFYTIME,serverMedifyTime);
        QString d=MyData::getInstance().getDbData(username).toBase64();
        ansData.insert(DB_DATA,d);
        ansData.insert(MSG_STRING,"从服务器收到新计划数据");
    }else if(serverMedifyTime==userMedifyTime){
        //一致，不改变
        ansData.insert(MSG_STRING,"数据一致");
    }else{
        //客户端的新，替换本地的
        MyData::getInstance().writeDb(username,dbData);
        MyData::getInstance().writeMedifyTime(username,userMedifyTime);
        //返回信息
        ansData.insert(MSG_STRING,"更新服务器的数据");
        //发送消息给其它机器
        QTimer::singleShot(100, &NetWorkUntil::getInstance(),[username,this,dbData,userMedifyTime](){
            NetWorkUntil::getInstance().synchronizeDevice(username,this->socket,dbData,userMedifyTime);
        });
    }
    qDebug()<<"发出:"<<MsgTypeMeans.at(SYNOCHRONIZE_PLAN_RESPONSE);
    this->sendData(createSendData(SYNOCHRONIZE_PLAN_RESPONSE,ansData));
}

/**
 * @brief TcpTask::sendData
 * @param data
 * 用元对象发送信号
 */
void TcpTask::sendData(const QByteArray &data)
{
    QMetaObject::invokeMethod(socket, "sendData", Qt::BlockingQueuedConnection,//当前线程将阻塞，直到事件被传递。使用此连接类型在同一线程中的对象之间进行通信将导致死锁。
                              Q_ARG(QByteArray,data));//传参
}

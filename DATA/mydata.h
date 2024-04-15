#ifndef MYDATA_H
#define MYDATA_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <QsqlDatabase>

#include <ENTITY/user.h>

#define CONFIG_FILE_PATH "./config.ini" //配置文件路径和名字
#define MYSQL_IP_STR "mysql_ip"
#define MYSQL_PORT_STR "mysql_port"
#define MYSQL_USER_STR "mysql_user"
#define MYSQL_PASSWORD_STR "mysql_password"
#define DTFORMAT "yyyy/MM/dd hh:mm:ss"
/**
 * @brief The MyData class
 * 磁盘、数据库管理类
 * 由于是被多线程程序调用，要注意线程同步
 */
class MyData
{
private:
    uint mysql_port=3306;
    QString mysql_ip="127.0.0.1";
    QString mysql_user="root";
    QString mysql_password="123456";
    QSqlDatabase db;
    //读写锁，用于保证两个线程不会同时读或写相同文件 用法：以用户名为键，锁为值，用户登录就生成锁，用户退出就释放锁
    QMap<QString,QReadWriteLock*> locks;
    QMutex mutex;//互斥锁，由于销毁锁后也可能有子线程正在等待这个已被销毁的锁，所以需要多加一重判断
private:
    MyData();
    void initConfig();//初始化配置文件
    void loadConfig();//读取配置文件
    void initDataBase();//初始化数据库
    void createUserDir(const QString &username);//创建用户文件夹
    bool lockr(const QString &username);//读锁文件
    bool lockw(const QString &username);//写锁
    void unlock(const QString &username);//解锁文件
public:
    static MyData &getInstance();
    ~MyData();
    User getUser(const QString &username);//从数据库中读取用户信息
    bool insertUser(const User &user);//新增一个用户信息到数据库中
    bool updateUser(const User &user);//更新一个用户信息到数据库中
    QDateTime getMedifyTime(const QString &username);//获取该用户上次更新计划的时间
    void writeMedifyTime(const QString &username,QString dateTime);//更新时间
    bool removeUserAllData(const QString &username);//删除该用户数据库中信息、删除该用户磁盘信息
    void creadLock(const QString &username);//生成锁
    void deleteLock(const QString &username);//释放锁
    QByteArray getDbData(QString username);
    void writeDb(const QString username,QByteArray dbData);//写入整个数据库
};

#endif // MYDATA_H

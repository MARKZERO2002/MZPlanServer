#include "mydata.h"

#include <QFile>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include "NETWORK/protocol.h"
MyData::MyData() {
    this->initConfig();
    this->loadConfig();
    this->initDataBase();
}

MyData &MyData::getInstance()
{
    static MyData instance;
    return instance;
}

MyData::~MyData()
{
    db.close();
    for(auto &l:this->locks){
        delete l;
        l=nullptr;
    }
}

void MyData::initConfig()
{
    QFile configFile;
    if(!configFile.exists()){
        //文件不存在，创建文件
        qDebug()<<"创建config.ini文件";
        //使用QSetting读取
        QSettings *config=new QSettings(CONFIG_FILE_PATH,QSettings::IniFormat);//ini文件
        //向文件中写入内容
        //mysql
        config->setValue(MYSQL_IP_STR,this->mysql_ip);
        config->setValue(MYSQL_PORT_STR,this->mysql_port);
        config->setValue(MYSQL_USER_STR,this->mysql_user);
        config->setValue(MYSQL_PASSWORD_STR,this->mysql_password);
        //写入完成后删除指针
        delete config;
    }
}

void MyData::loadConfig()
{
    QSettings *config=new QSettings(CONFIG_FILE_PATH,QSettings::IniFormat);
    this->mysql_ip=config->value(MYSQL_IP_STR).toString();
    this->mysql_port=config->value(MYSQL_PORT_STR).toUInt();
    this->mysql_user=config->value(MYSQL_USER_STR).toString();
    this->mysql_password=config->value(MYSQL_PASSWORD_STR).toString();
    qDebug()<<"Mysql服务器地址："<<this->mysql_ip<<" 端口号:"<<this->mysql_port;
    delete config;
}

void MyData::initDataBase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    db.setUserName(this->mysql_user);//用户名
    db.setHostName(this->mysql_ip);//主机地址
    db.setPort(this->mysql_port);//端口号
    db.setPassword(this->mysql_password);//密码
    db.setDatabaseName("mzplan");//要连接的数据库名
    db.open();
}

/**
 * @brief MyData::createUserDir
 * @param username
 * 创建用户磁盘信息
 */
void MyData::createUserDir(const QString &username)
{
    QString userDirPath="./data/"+username;
    QString userConfigPath=userDirPath+QDir::separator()+"user.ini";
    //创建文件夹
    QDir dir;
    dir.mkpath(userDirPath);
    //创建文件
    QSettings settings(userConfigPath,QSettings::IniFormat);
    settings.setValue(MEDIFYTIME,"2000/01/01 00:00:00");;
}

User MyData::getUser(const QString &username)
{
    User user;
    //在数据库中读取用户信息
    QSqlQuery query(db);
    QString sql_str="select * from user where username=:username;";
    query.prepare(sql_str);
    query.bindValue(":username",username);
    if(!query.exec()){
        qWarning()<<"查询用户信息时错误:"<<query.lastError();
        return user;
    }
    if(query.next()){//查到了
        QSqlRecord record=query.record();
        user=User(record.value("username").toString(),record.value("password").toString());
    }
    return user;
}

bool MyData::insertUser(const User &user)
{
    qDebug()<<"增加"<<user.toString()<<" 用户";
    QSqlQuery query(db);
    QString sql_str="INSERT INTO user(username,password) VALUES (:username,:password);";
    query.prepare(sql_str);
    query.bindValue(":username",user.getUsername());
    query.bindValue(":password",user.getPassword());
    if(!query.exec()){
        qWarning()<<"增加用户时错误:"<<query.lastError();
        return false;
    }
    //创建用户文件夹
    this->createUserDir(user.getUsername());
    return true;
}

QByteArray MyData::getDbData(QString username)
{
    this->lockr(username);
    //读取用户数据库信息
    QString userDirPath="./data/"+username;
    QString databasePath=userDirPath+"/database.db3";
    QFile file(databasePath);
    file.open(QFile::ReadOnly);
    QByteArray data= file.readAll();
    file.close();
    this->unlock(username);
    return data;
}

void MyData::writeDb(const QString username, QByteArray dbData)
{
    //把数据写入database.db3中
    QString userDirPath="./data/"+username;
    QString databasePath=userDirPath+"/database.db3";
    QFile file(databasePath);
    this->lockw(username);
    if(file.open(QFile::WriteOnly)){
        file.write(dbData);
    }
    file.close();
    this->unlock(username);
}

/**
 * @brief MyData::getMedifyTime
 * @param username
 * @return
 * 读取用户的配置文件中的medifyTime信息
 */
QDateTime MyData::getMedifyTime(const QString &username)
{
    if(!this->lockr(username)){
        throw "读锁上锁失败";
    }
    QString userDirPath="./data/"+username;
    QString userConfigPath=userDirPath+"/user.ini";
    QSettings settings(userConfigPath,QSettings::IniFormat);
    QDateTime ans=QDateTime::fromString(settings.value(MEDIFYTIME).toString(),DTFORMAT);
    this->unlock(username);
    return ans;
}

void MyData::writeMedifyTime(const QString &username, QString dateTime)
{
    this->lockw(username);
    QString userDirPath="./data/"+username;
    QString userConfigPath=userDirPath+"/user.ini";
    QSettings settings(userConfigPath,QSettings::IniFormat);
    settings.setValue(MEDIFYTIME,dateTime);
    this->unlock(username);
}

bool MyData::removeUserAllData(const QString &username)
{
    if(!this->lockw(username)){
        throw "写锁上锁失败";
    }
    qDebug()<<"删除"<<username<<"用户";
    //数据库中删除
    QSqlQuery query(db);
    QString sql_str="DELETE FROM user WHERE username=:username";
    query.prepare(sql_str);
    query.bindValue(":username",username);
    if(!query.exec()){
        qWarning()<<"删除用户时错误:"<<query.lastError();
        this->unlock(username);
        return false;
    }
    //磁盘删除
    QString userDirPath="./data/"+username;
    if(QDir(userDirPath).removeRecursively()){
        this->unlock(username);
        return true;
    }else{
        this->unlock(username);
        return false;
    }
}

/**
 * @brief MyData::creadLock
 * @param username
 * 应当设计成一个用户的第一个设备登陆时，才生成锁。TODO
 * 这里只是用户连接到服务器就生成锁
 */
void MyData::creadLock(const QString &username)
{
    //已经有锁
    if(this->locks.contains(username)){
        return;
    }
    QReadWriteLock *RWlock=new QReadWriteLock(QReadWriteLock::Recursive);//这个参数是可以多次锁定锁，直到同等数量释放锁后才是释放锁
    this->locks.insert(username,RWlock);
}
/**
 * @brief MyData::deleteLock
 * @param username
 * 应当设计成一个用户的最后一个设备退出时，才释放锁。TODO
 * 这里只是用户退出就释放锁
 */
void MyData::deleteLock(const QString &username)
{
    //锁住锁
    mutex.lock();
    //已经无锁
    if(!this->locks.contains(username)){
        mutex.unlock();//返回前一定要释放锁啊
        return ;
    }
    QReadWriteLock *RWlock=this->locks.value(username);
    //等待锁全部解锁后，才释放锁
    RWlock->lockForWrite();
    this->locks.remove(username);
    RWlock->unlock();
    delete RWlock;
    RWlock=nullptr;
    mutex.unlock();
}

/**
 * @brief MyData::lock
 * @param username
 * 锁定一个文件
 */
bool MyData::lockr(const QString &username)
{
    //锁定锁的锁，以免在运行时，锁被释放了
    mutex.lock();
    if(!this->locks.contains(username)){//如果没有锁，说明锁已经被释放，直接忽略用户的这次请求
        return false;//返回false告诉锁定失败，这次操作以失败告终
    }
    QReadWriteLock *rwl=this->locks.value(username);
    mutex.unlock();
    rwl->lockForRead();
    return true;
}

bool MyData::lockw(const QString &username)
{
    //锁定锁的锁，以免在运行时，锁被释放了
    mutex.lock();
    if(!this->locks.contains(username)){//如果没有锁，说明锁已经被释放，直接忽略用户的这次请求
        return false;//返回false告诉锁定失败，这次操作以失败告终
    }
    QReadWriteLock *rwl=this->locks.value(username);
    mutex.unlock();
    rwl->lockForWrite();
    return true;
}

void MyData::unlock(const QString &username)
{
    QReadWriteLock *rwl=this->locks.value(username);
    rwl->unlock();
}

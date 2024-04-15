#ifndef USER_H
#define USER_H
#include <QString>
#include <QList>

/**
 * @brief The User class
 * 用户类，存储用户实体
 */
class User
{
public:
    User(){}
    User(QString username,QString password)
        :username(username),password(password)
    {}
    bool isNull();
    QString getUsername() const{return this->username;}
    QString getPassword() const{return this->password;}
    QString toString() const;
private:
    QString username;
    QString password;
    // QList<QString> isLoginDevices;//当前用户在几个机器上登录，在服务器发送提醒更新时也许有帮助
};

#endif // USER_H

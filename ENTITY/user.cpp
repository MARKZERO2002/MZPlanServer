#include "user.h"


bool User::isNull()
{
    if(this->username.isNull()||this->password.isNull())
        return true;
    return this->username.isEmpty()||this->password.isEmpty();
}

QString User::toString() const
{
    QString ans="username:"+this->username+",password:"+this->password;
    return ans;
}

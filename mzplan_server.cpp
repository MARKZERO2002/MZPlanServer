#include "DATA/mydata.h"
#include "mzplan_server.h"

MZPlan_server &MZPlan_server::getInstance()
{
    static MZPlan_server instance;
    return instance;
}

MZPlan_server::MZPlan_server(QObject *parent)
    : QObject(parent)
{
    SaveLog::Instance()->setName("Mylog");
    SaveLog::Instance()->start();
    MyData::getInstance();
    this->networkUntil=new NetWorkUntil(this);
}

MZPlan_server::~MZPlan_server()
{
    SaveLog::Instance()->stop();
}

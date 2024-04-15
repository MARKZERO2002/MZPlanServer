#ifndef MZPLAN_SERVER_H
#define MZPLAN_SERVER_H

#include "log_module/savelog.h"
#include <QObject>
#include <NETWORK/networkuntil.h>

class MZPlan_server : public QObject
{
    Q_OBJECT
private:
    NetWorkUntil *networkUntil;
public:
    static MZPlan_server &getInstance();
private:
    MZPlan_server(QObject *parent = nullptr);
    ~MZPlan_server();
};
#endif // MZPLAN_SERVER_H

#include "mzplan_server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MZPlan_server::getInstance();
    return a.exec();
}

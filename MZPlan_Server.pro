QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DATA/mydata.cpp \
    DATA/tcptask.cpp \
    ENTITY/user.cpp \
    NETWORK/mytcpsocket.cpp \
    NETWORK/networkuntil.cpp \
    log_module/savelog.cpp \
    main.cpp \
    mzplan_server.cpp

HEADERS += \
    DATA/mydata.h \
    DATA/tcptask.h \
    ENTITY/user.h \
    NETWORK/mytcpsocket.h \
    NETWORK/networkuntil.h \
    NETWORK/protocol.h \
    log_module/savelog.h \
    mzplan_server.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT       += core gui sql serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Login/applogin.cpp \
    Login/appregistration.cpp \
    Login/apprestoration.cpp \
    Login/login.cpp \
    cloudhandeling.cpp \
    commonutility.cpp \
    connectivity.cpp \
    dbroutines.cpp \
    main.cpp \
    homepage.cpp \
    nodeinfowidget.cpp

HEADERS += \
    Login/applogin.h \
    Login/appregistration.h \
    Login/apprestoration.h \
    Login/login.h \
    cloudhandeling.h \
    commonutility.h \
    connectivity.h \
    dbroutines.h \
    homepage.h \
    nodeinfowidget.h

FORMS += \
    Login/applogin.ui \
    Login/appregistration.ui \
    Login/apprestoration.ui \
    Login/login.ui \
    homepage.ui \
    nodeinfowidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources/resources.qrc

QT       += core gui network #添加qt的支持模块

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets  #判断版本并且对应的库

CONFIG += c++11 #使用的c++标准

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#本项目包含的源文件
SOURCES += \
    gover.cpp \
    login.cpp \
    main.cpp \
    paraset.cpp \
    parashow.cpp \
    registe.cpp \
    widget.cpp
#本项目包含的头文件
HEADERS += \
    gover.h \
    login.h \
    paraset.h \
    parashow.h \
    registe.h \
    widget.h
#本项目包含的ui文件
FORMS += \
    gover.ui \
    login.ui \
    paraset.ui \
    parashow.ui \
    registe.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#如果修改生成目标的可执行程序名字，可通过赋值 TARGET = XXX来实现，否则可执行文件默认取值为项目的名字

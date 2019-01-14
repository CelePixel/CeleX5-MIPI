#-------------------------------------------------
#
# Project created by QtCreator 2018-08-14T15:57:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#QMAKE_CXXFLAGS += -std=c++11
CONFIG += console

TARGET = CeleXDemo
TEMPLATE = app

win32 {
    INCLUDEPATH += $$quote(D:/Program Files/opencv/build/include) \
                   $$quote(D:/Program Files/opencv/build/include/opencv) \
                   $$quote(D:/Program Files/opencv/build/include/opencv2)
}
else {

    INCLUDEPATH += /usr/local/include \
                   /usr/local/include/opencv \
                   /usr/local/include/opencv2

    LIBS += /usr/local/lib/libopencv_highgui.so \
            /usr/local/lib/libopencv_core.so    \
            /usr/local/lib/libopencv_imgproc.so \
            /usr/local/lib/libopencv_videoio.so \
            /usr/lib/x86_64-linux-gnu/libusb-1.0.so
}

win32 {
    contains(QT_ARCH, i386) {
        CONFIG(release, debug|release): LIBS += -L$$PWD/lib/Windows/x86/release/ -lCeleX -lopencv_world330 -lavcodec
        else:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/Windows/x86/debug/ -lCeleX -lopencv_world330d
    }
    else {
        CONFIG(release, debug|release): LIBS += -L$$PWD/lib/Windows/x64/release/ -lCeleX -lopencv_world330
        else:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/Windows/x64/debug/ -lCeleX -lopencv_world330d
    }
}
linux-g++ {
    contains(QT_ARCH, i386) {
        LIBS += -L$$PWD/lib/Linux/x86 -lokFrontPanel -lCeleX -lCeleDriver}
    else {
        LIBS += -L$$PWD/lib/Linux/x64 -lokFrontPanel -lCeleX -lCeleDriver}
}

SOURCES += main.cpp\
        mainwindow.cpp \
    sliderwidget.cpp \
    cfgslider.cpp \
    celex5widget.cpp \
    doubleslider.cpp \
    hhsliderwidget.cpp \
    dataqueue.cpp \
    celex5cfg.cpp \
    celex4widget.cpp \
    glwidget.cpp \
    videostream.cpp

HEADERS  += mainwindow.h \
    sliderwidget.h \
    cfgslider.h \
    celex5widget.h \
    include/celex5/celex5.h \
    doubleslider.h \
    hhconstants.h \
    hhsliderwidget.h \
    dataqueue.h \
    celex5cfg.h \
    celex4widget.h \
    glwidget.h \
    videostream.h

FORMS    += mainwindow.ui

win32 {
    CONFIG(release, debug|release) {
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.xml $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.txt $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.bit $$shell_path($$OUT_PWD)\release\
    }
    else:CONFIG(debug, debug|release) {
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.xml $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.txt $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.bit $$shell_path($$OUT_PWD)\debug\
    }
}
linux-g++ {
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/*.xml $$shell_path($$OUT_PWD)/ &
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/*.txt $$shell_path($$OUT_PWD)/ &
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/*.bit $$shell_path($$OUT_PWD)/
}

RESOURCES += \
    images.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/glut/ -lglut64
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/glut/ -lglutd64
else:unix: LIBS += -L$$PWD/glut/ -lglut

win32{
    INCLUDEPATH += $$PWD/glut
    DEPENDPATH += $$PWD/glut
}

unix:!macx: LIBS += -lGL -lGLU -lglut


#win32: LIBS += -L$$PWD/ffmpeg/lib/Windows/ -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale
#unix:!macx: LIBS += -L$$PWD/ffmpeg/lib/Linux/ -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale

win32: LIBS += -L$$PWD/ffmpeg/lib/Windows/ -lavcodec -lavformat -lavutil -lswresample
unix:!macx: LIBS += -L$$PWD/ffmpeg/lib/Linux/ -lavcodec -lavformat -lavutil -lswresample

INCLUDEPATH += $$PWD/ffmpeg/include
DEPENDPATH += $$PWD/ffmpeg/include

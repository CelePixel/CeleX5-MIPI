#-------------------------------------------------
#
# Project created by QtCreator 2018-08-14T15:57:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

#QMAKE_CXXFLAGS += -std=c++11
#CONFIG += console

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
        CONFIG(release, debug|release): LIBS += -L$$PWD/lib/Windows/x86/release/ -lCeleX -lopencv_world330
        else:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/Windows/x86/debug/ -lCeleX -lopencv_world330d
    }
    else {
        CONFIG(release, debug|release): LIBS += -L$$PWD/lib/Windows/x64/release/ -lCeleX -lopencv_world330
        else:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/Windows/x64/debug/ -lCeleX -lopencv_world330d
    }
}
else {
    contains(QT_ARCH, i386) {
        LIBS += -L$$PWD/lib/Linux/x86 -lCeleX}
    else {
        LIBS += -L$$PWD/lib/Linux/x64 -lCeleX}
}

SOURCES += main.cpp\
        mainwindow.cpp \
    sliderwidget.cpp \
    cfgslider.cpp \
    celex5widget.cpp \
    celex5cfg.cpp \
    videostream.cpp \
    settingswidget.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    sliderwidget.h \
    cfgslider.h \
    celex5widget.h \
    include/celex5/celex5.h \
    celex5cfg.h \
    videostream.h \
    settingswidget.h \
    qcustomplot.h

FORMS    += mainwindow.ui

win32 {
    CONFIG(release, debug|release) {
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\cfg_mp $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\cfg_mp_wire $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.txt $$shell_path($$OUT_PWD)\release\
    }
    else:CONFIG(debug, debug|release) {
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\cfg_mp $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\cfg_mp_wire $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.txt $$shell_path($$OUT_PWD)\debug\
    }
}
else {
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/cfg_mp $$shell_path($$OUT_PWD)/ &
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/cfg_mp_wire $$shell_path($$OUT_PWD)/ &
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/*.txt $$shell_path($$OUT_PWD)/
}

RESOURCES += \
    images.qrc


win32 {
    LIBS += -L$$PWD/ffmpeg/lib/Windows/ -lavcodec -lavformat -lavutil -lswresample
}
else {
    LIBS += -L$$PWD/ffmpeg/lib/Linux/ -lavcodec -lavformat -lavutil -lswresample
}

INCLUDEPATH += $$PWD/ffmpeg/include
DEPENDPATH += $$PWD/ffmpeg/include

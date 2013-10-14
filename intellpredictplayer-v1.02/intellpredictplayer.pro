TEMPLATE = app
DEPENDPATH += . build src ui
TARGET = "RenTianTian"

QT += phonon

FORMS += settings.ui
RESOURCES += mediaplayer.qrc

!win32:CONFIG += CONSOLE

SOURCES += main.cpp mediaplayer.cpp \
    thread.cpp
HEADERS += mediaplayer.h \
    thread.h \
    horrorvideoprediction.h \
    NoPornVideo.h

INCLUDEPATH += D:/OpenCV2.4/include \
INCLUDEPATH += D:/OpenCV2.4/build/include

LIBS += -LD:/OpenCV2.4/build/x86/vc10/lib \
    -lopencv_core243 \
    -lopencv_highgui243 \
    -lopencv_imgproc243 \
    -lopencv_features2d243

RC_FILE = icon.rc

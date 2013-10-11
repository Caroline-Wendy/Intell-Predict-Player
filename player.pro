TEMPLATE = app
TARGET = RenTianTian

QT += network \
      xml \
      multimedia \
      multimediawidgets \
      widgets

HEADERS = \
    player.h \
    playercontrols.h \
    videowidget.h \
    histogramwidget.h \
    horrorvideoprediction.h \
    thread.h
SOURCES = main.cpp \
    player.cpp \
    playercontrols.cpp \
    videowidget.cpp \
    histogramwidget.cpp \
    thread.cpp

#maemo* {
#    DEFINES += PLAYER_NO_COLOROPTIONS
#}

#target.path = $$[QT_INSTALL_EXAMPLES]/multimediawidgets/player
#INSTALLS += target

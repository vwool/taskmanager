TARGET = taskmanager

QT = quickcontrols2 websockets

SOURCES += \
        main.cpp \
    taskmanager.cpp \
    procinfo.cpp

HEADERS = taskmanager.h

RESOURCES += qml.qrc

include(app.pri)

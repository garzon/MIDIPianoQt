TEMPLATE = app
TARGET = MIDIPianoQt
QT += core multimedia widgets gui multimediawidgets opengl
CONFIG += static
CONFIG += c++11

DEFINES += QT_DLL QT_MULTIMEDIA_LIB QT_MULTIMEDIAWIDGETS_LIB QT_WIDGETS_LIB

LIBS += winmm.lib

DISTFILES += \
    MIDIPianoQt.pri

RESOURCES += \
    MIDIPianoQt.qrc

FORMS += \
    MIDIPianoQt.ui \
    SettingsDialog.ui \
    visualizationdialog.ui

HEADERS += \
    MIDIPianoQt.h \
    myMIDI.h \
    SettingsDialog.h \
    visualizationdialog.h \
    midifilereader.h \
    midicontroller.h

SOURCES += \
    main.cpp \
    MIDIPianoQt.cpp \
    myMIDI.cpp \
    SettingsDialog.cpp \
    visualizationdialog.cpp \
    midifilereader.cpp \
    midicontroller.cpp

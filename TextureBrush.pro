TEMPLATE = lib
DEFINES += TEXTUREBRUSH_LIBRARY
CONFIG       += plugin
QT           += widgets network

TARGET = texturebrush

CONFIG += c++11

isEmpty(LAIGTER_SRC){
  LAIGTER_SRC=../laigter
}


INCLUDEPATH  += $$LAIGTER_SRC/
INCLUDEPATH  += $$LAIGTER_SRC/src/

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$LAIGTER_SRC/src/image_processor.cpp\
    $$LAIGTER_SRC/src/light_source.cpp\
    $$LAIGTER_SRC/src/sprite.cpp\
    $$LAIGTER_SRC/src/texture.cpp \
    $$LAIGTER_SRC/thirdparty/zip.c \
    downloadmanager.cpp \
    texture_selector.cpp \
    texturebrush.cpp \
    texturebrushgui.cpp
    $$LAIGTER_SRC/src/brush_interface.h

HEADERS += \
    $$LAIGTER_SRC/src/image_processor.h \
    $$LAIGTER_SRC/src/light_source.h\
    $$LAIGTER_SRC/src/sprite.h\
    $$LAIGTER_SRC/src/texture.h \
    $$LAIGTER_SRC/thirdparty/zip.h \
    $$LAIGTER_SRC/thirdparty/miniz.h \
    downloadmanager.h \
    texture_selector.h \
    texturebrush.h \
    texturebrushgui.h

FORMS += \
    texture_selector.ui \
    texturebrushgui.ui

isEmpty(PREFIX){
unix{
 PREFIX = $$system(echo $HOME)/.local/share/laigter/plugins
}
win32{
 PREFIX = $$system(echo %APPDATA%)/laigter/plugins
 LIBS += C:\Qt\Tools\OpenSSL\Win_x64\bin\libcrypto-1_1-x64.dll
 LIBS += C:\Qt\Tools\OpenSSL\Win_x64\bin\libssl-1_1-x64.dll
}
}

target.path = $$PREFIX/

DESTDIR = $$PREFIX
INSTALLS += target

CONFIG += install_ok  # Do not cargo-cult this!
uikit: CONFIG += debug_and_release

RESOURCES += \
  icons.qrc

DISTFILES += \
  metadata.json




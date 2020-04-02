CONFIG += console

INCLUDEPATH = ../thirdparty/terralib/include \
              ../thirdparty/terralib/include/terralib \
              ../thirdparty/terralib/include/terralib/functions \
              ../thirdparty/terralib/include/terralib/kernel \
              ../thirdparty/boost/include \
              ../thirdparty/mseg/include

CONFIG(debug, debug|release) {
    LIBS += -L../thirdparty/terralib/lib/Debug/
    LIBS += -lterralib
    LIBS += -lterralib_shp
    LIBS += -L../thirdparty/mseg/lib/debug/
    LIBS += -lmseg
}

CONFIG(release, debug|release) {
  LIBS += -L../thirdparty/terralib/lib/Release/
  LIBS += -lterralib 
  LIBS += -lterralib_shp
  LIBS += -L../thirdparty/mseg/lib/release/
  LIBS += -lmseg
}

HEADERS += src/ui/MultiSegWidget.h \
           src/ui/Qt4Progress.h

SOURCES += src/Main.cpp \
           src/ui/MultiSegWidget.cpp \
           src/ui/Qt4Progress.cpp

FORMS += src/ui/MultiSegWidgetForm.ui

Debug:UI_DIR = src/ui
Release:UI_DIR = src/ui

RESOURCES += mseg-ui.qrc
RC_FILE = mseg-ui.rc

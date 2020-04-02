CONFIG += console
CONFIG -= core gui

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

SOURCES += src/Main.cpp \
           src/MultiSegOp.cpp \
           src/OpSupportFunctions.cpp \
           src/TerraAidaXML.cpp

HEADERS += src/MultiSegOp.h \
           src/OperatorsBaseDefines.h \
           src/OpSupportFunctions.hpp \
           src/TerraAidaXML.hpp \
           src/Version.hpp

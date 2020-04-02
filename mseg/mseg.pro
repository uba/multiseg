QT -= gui

TEMPLATE = lib

DEFINES += MSEGDLL

INCLUDEPATH = ../thirdparty/terralib/include \
              ../thirdparty/terralib/include/terralib \
              ../thirdparty/terralib/include/terralib/functions \
              ../thirdparty/terralib/include/terralib/kernel \
              ../thirdparty/boost/include

CONFIG(debug, debug|release) {
    LIBS += -L../thirdparty/terralib/lib/Debug/
    LIBS += -lterralib
    LIBS += -lterralib_shp
}

CONFIG(release, debug|release) {
  LIBS += -L../thirdparty/terralib/lib/Release/
  LIBS += -lterralib 
  LIBS += -lterralib_shp
}

HEADERS += src/AbstractMerger.h \
           src/AbstractOutputter.h \
           src/CompositeMerger.h \
           src/CVTable.h \
           src/Config.h \
           src/Enums.h \
           src/EuclideanMerger.h \
           src/FileOutputter.h \
           src/MultiSeg.h \
           src/OpticalCartoonMerger.h \
           src/ParallelMultiSegStrategy.h \
           src/ParallelMultiSegStrategyFactory.h \
           src/Pyramid.h \
           src/RadarCartoonMerger.h \
           src/Region.h \
           src/Utils.h

SOURCES += src/AbstractMerger.cpp \
           src/CompositeMerger.cpp \
           src/CVTable.cpp \
           src/EuclideanMerger.cpp \
           src/FileOutputter.cpp \
           src/MultiSeg.cpp \
           src/OpticalCartoonMerger.cpp \
           src/ParallelMultiSegStrategy.cpp \
           src/ParallelMultiSegStrategyFactory.cpp \
           src/Pyramid.cpp \
           src/RadarCartoonMerger.cpp \
           src/Region.cpp \
           src/Utils.cpp

win32 {
  QMAKE_POST_LINK += copy src\\*.h ..\\thirdparty\\mseg\\include\\mseg &

  CONFIG(debug, debug|release) {
    QMAKE_POST_LINK += copy debug\\mseg.lib ..\\thirdparty\\mseg\\lib\\debug &
    QMAKE_POST_LINK += copy debug\\mseg.dll ..\\thirdparty\\mseg\\bin\\debug
  }

  CONFIG(release, debug|release) {
    QMAKE_POST_LINK += copy release\\mseg.lib ..\\thirdparty\\mseg\\lib\\release &
    QMAKE_POST_LINK += copy release\\mseg.dll ..\\thirdparty\\mseg\\bin\\release
  }
}

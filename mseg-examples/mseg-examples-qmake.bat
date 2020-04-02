set QMAKE_DIR=C:\Qt\4.8.1\bin
set PATH=%QMAKE_DIR%;%PATH%
qmake -tp vc -r "CONFIG += msvc"
pause
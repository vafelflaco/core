@echo off
rem
rem $Id$
rem

if "%HB_BIN_INSTALL%" == "" set HB_BIN_INSTALL=..\..\..\bin
if "%HB_LIB_INSTALL%" == "" set HB_LIB_INSTALL=..\..\..\lib
if "%HB_INC_INSTALL%" == "" set HB_INC_INSTALL=..\..\..\include

set HB_ARCHITECTURE=w32
set HB_COMPILER=msvc
set HB_USER_LIBS=gtwvg.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib oleaut32.lib

call %HB_BIN_INSTALL%\hbmk.bat %1 %2 %3 %4 %5 %6 %7 %8 %9

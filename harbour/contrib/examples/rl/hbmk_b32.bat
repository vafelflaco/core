@echo off
rem
rem $Id$
rem

set HB_DIR_RL=%CLIPPER_DIR%\SOURCE\RL
if exist "%HB_DIR_RL%\RLFRONT.PRG" goto DIR_OK
set HB_DIR_RL=.
if exist "%HB_DIR_RL%\RLFRONT.PRG" goto DIR_OK_LOCAL

echo ---------------------------------------------------------------
echo IMPORTANT: You'll either need to copy the original CA-Cl*pper 
echo            RL sources to this directory, or set the location of 
echo            your original CA-Cl*pper installation (with RL sources 
echo            in SOURCE\RL):
echo            set CLIPPER_DIR=C:\CLIPPER5
echo ---------------------------------------------------------------
goto EXIT

:DIR_OK_LOCAL

if exist hb_rl.dif patch -N -i hb_rl.dif

:DIR_OK

..\..\..\bin\harbour -q -n -l -i..\..\..\include %HB_DIR_RL%\RLBACK.PRG %HB_DIR_RL%\RLDIALG.PRG %HB_DIR_RL%\RLFRONT.PRG

bcc32 -O2 -I..\..\..\include -L..\..\..\lib -erl.exe rlfront.c rlback.c rldialg.c hbdebug.lib hbvm.lib hbrtl.lib gtwin.lib gtwvt.lib hblang.lib hbrdd.lib hbmacro.lib hbpp.lib rddntx.lib rddcdx.lib rddfpt.lib hbsix.lib hbcommon.lib

del *.obj
del rlback.c rldialg.c rlfront.c

rl

:EXIT


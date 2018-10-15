@echo off
set OldPath=%Path%
set Path=%BASEDIR%\src\vdd\dosioctl\bin16;%Path%
set OldInc=%Include%
set Include=%BASEDIR%\inc\ddk\wnet;%Include%

cd dosapp
nmake

cd ..\dosdrvr
nmake

cd ..

Path=%OldPath%
set Include=%OldInc%
set OldPath=
set OldInc=

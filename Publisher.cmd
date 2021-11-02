@set PATH=%PATH%
@set SLNDIR=H:\Labs\Shark\
@set SRCDIR=H:\Labs\Shark\Projects\
@set BINDIR=H:\Labs\Shark\Build\Bins\
@set REDISTDIR=H:\Labs\Shark\Redist\
@set WORKDIR=.\Publisher\

@if not exist %WORKDIR% md %WORKDIR%

@if exist %WORKDIR%AMD64\Sea.exe del /F /Q %WORKDIR%AMD64\Sea.exe
@if exist %WORKDIR%AMD64\Sea.pdb del /F /Q %WORKDIR%AMD64\Sea.pdb
@if exist %WORKDIR%AMD64\Shark.sys del /F /Q %WORKDIR%AMD64\Shark.sys
@if exist %WORKDIR%AMD64\Shark.pdb del /F /Q %WORKDIR%AMD64\Shark.pdb
@if exist %WORKDIR%AMD64\VBoxDrv.sys del /F /Q %WORKDIR%AMD64\VBoxDrv.sys

@if exist %WORKDIR%I386\Sea.exe del /F /Q %WORKDIR%I386\Sea.exe
@if exist %WORKDIR%I386\Sea.pdb del /F /Q %WORKDIR%I386\Sea.pdb
@if exist %WORKDIR%I386\Shark.sys del /F /Q %WORKDIR%I386\Shark.sys
@if exist %WORKDIR%I386\Shark.pdb del /F /Q %WORKDIR%I386\Shark.pdb
@if exist %WORKDIR%I386\VBoxDrv.sys del /F /Q %WORKDIR%I386\VBoxDrv.sys

@if not exist %WORKDIR%AMD64\ md %WORKDIR%AMD64

@if not exist %BINDIR%AMD64\Sea.exe (echo AMD64\Sea.exe not found) else copy /Y %BINDIR%AMD64\Sea.exe %WORKDIR%AMD64\
@if not exist %BINDIR%AMD64\Sea.pdb (echo AMD64\Sea.pdb not found) else copy /Y %BINDIR%AMD64\Sea.pdb %WORKDIR%AMD64\
@if not exist %BINDIR%AMD64\Shark.sys (echo AMD64\Shark.sys not found) else copy /Y %BINDIR%AMD64\Shark.sys %WORKDIR%AMD64\
@if not exist %BINDIR%AMD64\Shark.pdb (echo AMD64\Shark.pdb not found) else copy /Y %BINDIR%AMD64\Shark.pdb %WORKDIR%AMD64\
@if not exist %REDISTDIR%AMD64\VBoxDrv.sys (echo AMD64\VBoxDrv.sys not found) else copy /Y %REDISTDIR%AMD64\VBoxDrv.sys %WORKDIR%AMD64\

@if not exist %WORKDIR%I386\ md %WORKDIR%I386

@if not exist %BINDIR%I386\Sea.exe (echo I386\Sea.exe not found) else copy /Y %BINDIR%I386\Sea.exe %WORKDIR%I386\
@if not exist %BINDIR%I386\Sea.pdb (echo I386\Sea.pdb not found) else copy /Y %BINDIR%I386\Sea.pdb %WORKDIR%I386\
@if not exist %BINDIR%I386\Shark.sys (echo I386\Shark.sys not found) else copy /Y %BINDIR%I386\Shark.sys %WORKDIR%I386\
@if not exist %BINDIR%I386\Shark.pdb (echo I386\Shark.pdb not found) else copy /Y %BINDIR%I386\Shark.pdb %WORKDIR%I386\
@if not exist %REDISTDIR%I386\VBoxDrv.sys (echo I386\VBoxDrv.sys not found) else copy /Y %REDISTDIR%I386\VBoxDrv.sys %WORKDIR%I386\

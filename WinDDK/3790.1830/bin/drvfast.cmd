@echo off
@REM
@REM Sets the env. to use preFAST for Drivers then calls
@REM setenv.bat
@REM
set PREFAST_ROOT=%1%\bin\x86\drvfast
set PATH=%PREFAST_ROOT%\scripts;%path%
call %~dp0\setenv.bat %* no_prefast

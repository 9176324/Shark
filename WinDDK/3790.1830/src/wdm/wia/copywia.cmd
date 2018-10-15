@echo off
rem  Copy all of the WIA DDK binaries to a directory named wiabins. An
rem  optional parameter is a destination directory to prepend to wiabins.

if /I "%BUILD_DEFAULT_TARGETS%" EQU "-386" set cpu2=i386
if /I "%BUILD_DEFAULT_TARGETS%" EQU "-amd64" set cpu2=amd64
if /I "%BUILD_DEFAULT_TARGETS%" EQU "-ia64" set cpu2=ia64

md %1wiabins
md %1wiabins\drivers

copy microdrv\obj%build_alt_dir%\%cpu2%\testmcro.dll   %1wiabins\drivers
copy microdrv\testmcro.inf                             %1wiabins\drivers

copy wiascanr\obj%build_alt_dir%\%cpu2%\wiascanr.dll   %1wiabins\drivers
copy wiascanr\wiascanr.inf %1wiabins\drivers

copy wiacam\obj%build_alt_dir%\%cpu2%\wiacam.dll       %1wiabins\drivers
copy wiacam\wiacam.inf                                 %1wiabins\drivers
copy microcam\obj%build_alt_dir%\%cpu2%\fakecam.dll    %1wiabins\drivers
copy extend\obj%build_alt_dir%\%cpu2%\extend.dll       %1wiabins\drivers

copy %basedir%\tools\wia\%cpu%\wiatest.exe            %1wiabins
copy %basedir%\tools\wia\%cpu%\scanpanl.exe           %1wiabins
copy %basedir%\tools\wia\%cpu%\wiadbgcfg.exe          %1wiabins
copy %basedir%\tools\wia\%cpu%\wialogcfg.exe          %1wiabins

goto end

:Syntax
Echo %0 Drive\path\

:end


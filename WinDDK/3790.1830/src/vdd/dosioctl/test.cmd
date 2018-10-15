REM Before running this batch file, please make sure the following steps are performed.
REM
REM 1. Compile and link the complete set of installed sources.
REM 2. cd %BASEDIR%\src\vdd\dosioctl
REM    run build16.bat
REM 3. md c:\test
REM 4. Copy  %BASEDIR%\src\vdd\dosioct\ioctlvdd\ioctlvdd.dll c:\test
REM    Copy  %BASEDIR%\src\vdd\dosioct\dosdrvr\dosdrvr.sys   c:\test
REM    Copy  %BASEDIR%\src\vdd\dosioct\dosapp\dosapp.exe     c:\test
REM    Copy  %BASEDIR%\src\vdd\dosioct\krnldrvr\krnldrvr.sys c:\test
REM 5. Add the dos device driver dosdrvr.sys to %WINDIR%\system32\config.nt file
REM    device=c:\test\dosdrvr.sys
REM 6. Add a VDD ValueName to the following registry key:
REM    HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\VirtualDeviceDrivers
REM             VDD  REG_MULTI_SZ  c:\test\ioctlvdd.dll
REM

sc create krnldrvr binPath= c:\test\krnldrvr.sys type= kernel
sc start krnldrvr
kill -f ntvdm
c:
cd \test
dosapp
sc stop  krnldrvr

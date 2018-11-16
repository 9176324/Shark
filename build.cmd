@rem set LABS=%~dp0..\
@rem cd /d "%LABS%"
@rem git clone https://github.com/9176324/Shark
@rem git clone https://github.com/9176324/WRK
@rem git clone https://github.com/9176324/WINDDK

@set PATH=C:\Windows;C:\Windows\System32;%~dp0..\WRK\tools\amd64
@set SLND=%~dp0

@if not exist "%SLND%Build\Bins\AMD64" md "%SLND%Build\Bins\AMD64"
@if not exist "%SLND%Build\Objs\Sea\AMD64" md "%SLND%Build\Objs\Sea\AMD64"
@if not exist "%SLND%Build\Objs\Shark\AMD64" md "%SLND%Build\Objs\Shark\AMD64"

@cd /d "%SLND%Projects\Sea"
@NMAKE /NOLOGO REBUILD PLATFORM=x64 PROJ=Sea

@cd /d "%SLND%Projects\Shark"
@NMAKE /NOLOGO REBUILD PLATFORM=x64 PROJ=Shark

@cd /d "%SLND%"

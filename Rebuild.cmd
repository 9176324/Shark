@rem 
@rem 
@rem Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
@rem 
@rem The contents of this file are subject to the Mozilla Public License Version
@rem 2.0 (the "License"); you may not use this file except in compliance with
@rem the License. You may obtain a copy of the License at
@rem http://www.mozilla.org/MPL/
@rem 
@rem Software distributed under the License is distributed on an "AS IS" basis,
@rem WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
@rem for the specific language governing rights and limitations under the
@rem License.
@rem 
@rem The Initial Developer of the Original Code is blindtiger.
@rem 
@rem 

@rem set LABS=%~dp0..\
@rem cd /d "%LABS%"
@rem git clone https://github.com/9176324/Shark
@rem git clone https://github.com/9176324/MiniSDK

@set SLND=%~dp0
@if not exist "%SLND%Build\Bins\AMD64" md "%SLND%Build\Bins\AMD64"
@if not exist "%SLND%Build\Bins\I386" md "%SLND%Build\Bins\I386"

@if not exist "%SLND%Build\Objs\Shark" md "%SLND%Build\Objs\Shark"
@if not exist "%SLND%Build\Objs\Sea" md "%SLND%Build\Objs\Sea"

@echo building x86

:CheckOS
@if exist "%PROGRAMFILES(X86)%" (goto x64x86) ELSE (goto x86x86)

:x86x86
@set path=C:\Windows;C:\Windows\System32;%~dp0\..\MSVC\Hostx86\x86
@goto x86

:x64x86
@set path=C:\Windows;C:\Windows\System32;%~dp0\..\MSVC\Hostx64\x86
@goto x86

:x86
@cd /d "%SLND%Projects\Shark"
@NMAKE /NOLOGO REBUILD PLATFORM=x86 PROJ=Shark

@cd /d "%SLND%Projects\Sea"
@NMAKE /NOLOGO REBUILD PLATFORM=x86 PROJ=Sea

@cd /d "%SLND%"

@echo building x64

:CheckOS
@if exist "%PROGRAMFILES(X86)%" (goto x64x64) ELSE (goto x86x64)

:x86x64
@set path=C:\Windows;C:\Windows\System32;%~dp0\..\MSVC\Hostx86\x64
@goto x64

:x64x64
@set path=C:\Windows;C:\Windows\System32;%~dp0\..\MSVC\Hostx64\x64
@goto x64

:x64
@cd /d "%SLND%Projects\Shark"
@NMAKE /NOLOGO REBUILD PLATFORM=x64 PROJ=Shark

@cd /d "%SLND%Projects\Sea"
@NMAKE /NOLOGO REBUILD PLATFORM=x64 PROJ=Sea

@cd /d "%SLND%"

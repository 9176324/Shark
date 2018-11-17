@rem 
@rem 
@rem Copyright (c) 2018 by blindtiger. All rights reserved.
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
@rem The Initial Developer of the Original e is blindtiger.
@rem 
@rem 

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

# Shark
    Turn off PatchGuard in real time for win7 (7600) ~ later.

# Change Log
    Use decrypt replace hook for 18362 ~ later.

# Create labs
    md X:\Labs
    cd /d X:\Labs
    git clone https://github.com/9176324/Shark
    git clone https://github.com/9176324/WinDDK
    git clone https://github.com/9176324/WRK
    git clone https://github.com/9176324/MSVC

# Build
    Method 1: run FastBuild.cmd or Rebuild.cmd
    Method 2: MSBuild "Shark.sln" -t:Rebuild /p:Platform="x86"
              MSBuild "Shark.sln" -t:Rebuild /p:Platform="x64"
    Method 3: open "Shark.sln" with VisualStudio

# Install
    run Sea.exe (use vbox exploit to load)
    ![win11 disable virus](https://github.com/9176324/Shark/blob/master/Doc/win11 disable virus.png)
    
# Uninstall
    restart windows

# Other projects link
    DisableWin10PatchguardPoc, https://github.com/killvxk/DisableWin10PatchguardPoc
    EfiGuard, https://github.com/Mattiwatti/EfiGuard
    findpg, https://github.com/tandasat/findpg
    PgResarch, https://github.com/tandasat/PgResarch
    UPGDSED, https://github.com/hfiref0x/UPGDSED

# QQ Group : 119651372
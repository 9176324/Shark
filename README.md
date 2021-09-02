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
    Method 1: run Rebuild.cmd
    Method 2: MSBuild "X:\Labs\Shark\Shark.sln" -t:Rebuild /p:Platform="x86|x64"
    Method 3: open "X:\Labs\Shark\Shark.sln" with VisualStudio
    
# Install
    run Sea.exe (The driver has no signature, you must mount windbg, or you can sign it)

# Uninstall
    restart windows

# Other projects link
    DisableWin10PatchguardPoc, https://github.com/killvxk/DisableWin10PatchguardPoc
    EfiGuard, https://github.com/Mattiwatti/EfiGuard
    findpg, https://github.com/tandasat/findpg
    PgResarch, https://github.com/tandasat/PgResarch
    UPGDSED, https://github.com/hfiref0x/UPGDSED

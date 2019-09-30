# Shark
    Turn off PatchGuard in real time for win7 (7600) ~ win10 (18950).

# Create labs
    md X:\Labs
    cd /d X:\Labs
    git clone https://github.com/9176324/Shark
    git clone https://github.com/9176324/WinDDK
    git clone https://github.com/9176324/WRK

# Build
    Method 1: open "X:\Labs\Shark\Shark.sln" with VisualStudio
    Method 2: run Build.cmd
    
# Install
    run Sea.exe (The driver has no signature, you must mount windbg, or you can sign it)

# Uninstall
    restart windows

# References (sort by first letter of the name)
    DisableWin10PatchguardPoc, https://github.com/killvxk/DisableWin10PatchguardPoc
    EfiGuard, https://github.com/Mattiwatti/EfiGuard
    findpg, https://github.com/tandasat/findpg
    PgResarch, https://github.com/tandasat/PgResarch
    UPGDSED, https://github.com/hfiref0x/UPGDSED

# Note
    祖国生日快乐!

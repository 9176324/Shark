# Shark
    Turn off PatchGuard in real time for win7 (7600) ~ later.

# Create labs
    md X:\Labs
    cd /d X:\Labs
    git clone https://github.com/9176324/Shark
    git clone https://github.com/9176324/MiniSDK

# Build
    Method 1: run FastBuild.cmd or Rebuild.cmd
    Method 2: MSBuild "Shark.sln" -t:Rebuild -p:Platform="x86"
              MSBuild "Shark.sln" -t:Rebuild -p:Platform="x64"
    Method 3: open "Shark.sln" with VisualStudio

# Install
    run Sea.exe (use vbox exploit to load)
![win11 disable virus](https://user-images.githubusercontent.com/4614528/139319409-1c3773f7-4c72-48bf-a415-29d018555bee.png)
    
# Uninstall
    restart windows

# Other projects link
    DisableWin10PatchguardPoc, https://github.com/killvxk/DisableWin10PatchguardPoc
    EfiGuard, https://github.com/Mattiwatti/EfiGuard
    findpg, https://github.com/tandasat/findpg
    PgResarch, https://github.com/tandasat/PgResarch
    UPGDSED, https://github.com/hfiref0x/UPGDSED
    
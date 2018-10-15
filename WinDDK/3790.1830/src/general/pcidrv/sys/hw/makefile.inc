mofcomp: pcidrv.bmf

pcidrv.bmf: ..\pcidrv.mof
        mofcomp -B:pcidrv.bmf ..\pcidrv.mof
        wmimofck pcidrv.bmf





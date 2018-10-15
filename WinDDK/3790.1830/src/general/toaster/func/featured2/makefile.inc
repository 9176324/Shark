mofcomp: toaster.bmf

toaster.bmf: toaster.mof
        mofcomp -B:toaster.bmf toaster.mof
        wmimofck toaster.bmf





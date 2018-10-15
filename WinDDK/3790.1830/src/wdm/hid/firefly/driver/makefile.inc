mofcomp: firefly.bmf

firefly.bmf: firefly.mof
        mofcomp -B:firefly.bmf firefly.mof
        wmimofck firefly.bmf





mofcomp: busenum.bmf

busenum.bmf: busenum.mof
        mofcomp -B:busenum.bmf busenum.mof
        wmimofck busenum.bmf





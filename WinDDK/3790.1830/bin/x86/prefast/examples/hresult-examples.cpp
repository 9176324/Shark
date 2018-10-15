//
// PREfast Examples
// Copyright © Microsoft Corporation.  All rights reserved.
//


#include <windows.h>
 
BOOL test()
{
	HRESULT hr = 0;

	hr = S_OK;

	if (FAILED(hr))
	{
		hr = (BOOL) E_FAIL;
	}

	if (hr)
	{
		if (hr > 3)
		{
			if ((BOOL)SUCCEEDED(hr))
			{
				hr = TRUE;
			}
		}
	}

	return hr;
}

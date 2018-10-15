//
// PREfast Examples
// Copyright © Microsoft Corporation.  All rights reserved.
//


int test()
{
	int i = 1;
	int j, k = 3;

	if (k)
	{
		i = j;
		goto CleanUp;
	}
	else
	{
		j = 2;
	}

	i = j + 2;

CleanUp:

    return j;
}

//
// PREfast Examples
// Copyright © Microsoft Corporation.  All rights reserved.
//


static void test(int x, int *y)
{
	// Example:  DEFECT_DESCRIPTION__LogicalOpConst with ||

	if (x || 10) {
		*y += 1;
	}


	// Example:  DEFECT_DESCRIPTION__ConstLogicalOp with ||

	if (10 || x) {
		*y += 1;
	}


	// Example:  DEFECT_DESCRIPTION__LogicalOpConst with &&

	if (x && 10) {
		*y += 1;
	}


	// Example:  DEFECT_DESCRIPTION__ConstLogicalOp with &&

	if (10 && x) {
		*y += 1;
	}
}


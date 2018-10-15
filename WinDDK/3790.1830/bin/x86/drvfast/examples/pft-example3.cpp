//
// PREfast Examples
// Copyright © Microsoft Corporation.  All rights reserved.
//


#include <stdlib.h>
class A {
public:
	int a;
};

bool foo()
{
	A *p1 = new A();

	p1->a = 2;

	A *p2 = new A();

	delete p1;
	delete p2;

	return 1;
}

void bar()
{
	char *p, *q;

	p = (char *) malloc(255);
	q = (char *) malloc(255);

	if (!p)
	{
		return;
	}

	free(p);
	free(q);
	return;
}

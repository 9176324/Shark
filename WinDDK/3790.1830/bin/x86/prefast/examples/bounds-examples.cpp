//
// PREfast Examples
// Copyright © Microsoft Corporation.  All rights reserved.
//


#include <string.h>
#include <stdio.h>

// Calling an unsafe function.  A classic mistake.
void evil_function(void)
{
	char buff[100];
	printf("type a short input line, please\n");
	gets(buff);
}


// failure to validate a string's length before copying it to a local
// array.  If this is a public function, this is the classic exploitable
// hole.
void unchecked_strcpy(char *p)
{
	char buff[80];
	strcpy(buff, p);
}

// A variant in which the string's size is passed as a separate parameter,
// but it isn't checked before using it.  Once again, a potential exploitable
// hole if this is in a public function.  [This is more the RPC style.]
void unchecked_strncpy(char *p, size_t size)
{
	char buff[80];
	strncpy(buff, p, size);
}

// reading a size, and then reading that many bytes.  No good if the
// size has not been validated.
void read_size(FILE *fp)
{
	char buff[80];
	int line_length;
	fread(&line_length, sizeof(int), 1, fp);
	fgets(buff, line_length, fp);
}


// confusing the number of bytes in a buffer with the number of characters;
// loses in a unicode world.  [LoadString is a common place to get this wrong.]
void unicode_screwup(wchar_t *foo)
{
	wchar_t buff[100];
	wcsncpy(buff, foo, sizeof(buff));
}

// a basic error: using the array size as the end of the array.  Not
// in general an exploitable security hole.
void constant_index(void)
{
	int arr[10];
	arr[10] = 0;
}

// Calling an unsafe function on a static.  An error, but usually difficult
// to exploit.
void mildly_evil_function(void)
{
	static char static_buff[100];
	printf("type a short input line, please\n");
	gets(static_buff);
}


// illustrating some cases that we intentionally DON'T warn about

// validating the parameter before usage
void ok_strcpy(char *p)
{
	char buff[80];
	if (strlen(p) < 80)
		strcpy(buff, p);
}


// copying a constant string
void ok_strcpy_2(void)
{
	char buff[80];
	strcpy(buff, "ok");
}


// sprintf("%d") is okay even if the parameter hasn't been checked
void ok_sprintf(int n)
{
	char buff[80];
	sprintf(buff, "%d", n);
}



void unicode_ok(wchar_t *foo)
{
	wchar_t buff[100];
	wcsncpy(buff, foo, 100);
}


void ansi_ok(char *foo)
{
	char buff[100];
	strncpy(buff, foo, sizeof(buff));
}

// reading a size, and checking it before using it.  This is good.
void read_size_ok(FILE *fp)
{
	char buff[80];
	int line_length;
	fread(&line_length, sizeof(int), 1, fp);
	if (line_length > 80)
		return;
	fgets(buff, line_length, fp);
}




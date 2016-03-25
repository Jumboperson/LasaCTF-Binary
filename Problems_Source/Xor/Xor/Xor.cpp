// Xor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "XorStr.h"
#include <stdlib.h>

XorS(str, "lasactf{x0r_1s_a5_k3wl_4s_r3ndom}");

int main(int argc, char* args[])
{
	if (argc < 2)
	{
		puts("Requires 1 parameter.");
		return 1;
	}
	int iVal = 0;
	sscanf_s(args[1], "%i", &iVal);
	puts(str.Decrypt(iVal));
    return 0;
}

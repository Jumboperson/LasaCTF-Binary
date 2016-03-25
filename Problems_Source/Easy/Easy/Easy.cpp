// Easy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

extern "C" volatile char szFlag[] = "lasactf{fun_and_3z_fl4gz}";

int main()
{
	puts("Find the flag!");
    return 0;
	puts(const_cast<char*>(szFlag));
}


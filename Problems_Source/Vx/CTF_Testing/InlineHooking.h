#pragma once
#ifndef _INLINEHOOKING_
#define _INLINEHOOKING_
#include "stdafx.h"
#include <Windows.h>
#include <stdint.h>

void __fastcall Hotpatch(void* ofunc, void* hkfunc);
DWORD __fastcall Detour(unsigned char* ofunc, unsigned char* hkfunc);
DWORD __fastcall Detour(unsigned char* uchOFunction, unsigned char* uchHkFunction, DWORD dwSize);
void* DetourTramp(unsigned char* pOFunction, unsigned char* pHkfunction, int stSize = 5);

void printFunctionBytecodes(BYTE* function);
unsigned int __fastcall GetFuncSize(unsigned char *uchFunction);
void __fastcall NopFunction(unsigned char *uchFunction, unsigned int uiSize);

#endif
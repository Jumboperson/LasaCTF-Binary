// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <iostream>
#include <utility>
#include <Windows.h>
#include <tchar.h>
#include "InlineHooking.h"
#include "CustomTypes.h"
#include "XorStr.h"

extern int g_argc;
extern char** g_args;

extern void JumpOut();
extern void SetupRuntime();

#define TESTING
#define VX_VER 2

#define sym(a, b) a##b
#define SymbolGen(a, b) sym(a, b)

#ifndef TESTING
#define OBFU_ONE() \
	__asm { mov eax, __LINE__ * 0xDEADFA11}; \
	__asm { cmp eax, __LINE__ * 0xF0FFBABE}; \
	__asm { jz SymbolGen(Junk, __LINE__) }; \
	__asm { mov eax, SymbolGen(Real, __LINE__) }; \
	__asm { jmp eax }; \
	__asm { SymbolGen(Junk, __LINE__): }; \
	__asm { _emit(0xd8 + __LINE__ % 8) }; \
	__asm { SymbolGen(Real, __LINE__): };

#define OBFU_TWO() \
	__asm { mov eax, __LINE__ * 0xDEADFA11}; \
	__asm { cmp eax, __LINE__ * 0xF0FFBABE}; \
	__asm { jz SymbolGen(Junk, __LINE__) }; \
	__asm { mov eax, SymbolGen(Real, __LINE__) }; \
	__asm { jmp eax }; \
	__asm { SymbolGen(Junk, __LINE__): }; \
	__asm { _emit(0xb2 + __LINE__ % 8) }; \
	__asm { _emit(0xe0 + __LINE__ % 8) }; \
	__asm { _emit(0xbc + __LINE__ % 8) }; \
	__asm { _emit(0xe4 + __LINE__ % 8) }; \
	__asm { _emit(0xd7 + __LINE__ % 8) }; \
	__asm { _emit(0xaf + __LINE__ % 8) }; \
	__asm { SymbolGen(Real, __LINE__): };

#define OBFU_THR() \
	__asm { mov eax, __LINE__ * 0xDEADFA11}; \
	__asm { cmp eax, __LINE__ * 0xF0FFBABE}; \
	__asm { jz SymbolGen(Junk, __LINE__) }; \
	__asm { mov eax, SymbolGen(Real, __LINE__) }; \
	__asm { jmp eax }; \
	__asm { SymbolGen(Junk, __LINE__): }; \
	__asm { _emit(0xc4 + __LINE__ % 8) }; \
	__asm { _emit(0xe6 + __LINE__ % 8) }; \
	__asm { _emit(0xd3 + __LINE__ % 8) }; \
	__asm { SymbolGen(Real, __LINE__): };

#define OBFU_FOUR() \
	__asm { push eax }; \
	__asm { call SymbolGen(This, __LINE__) }; \
	__asm { SymbolGen(This, __LINE__): }; \
	__asm { pop eax }; \
	__asm { cmp eax, [esp] }; \
	__asm { jz SymbolGen(Fake, __LINE__) }; \
	__asm { lea eax, [eax + 22] }; \
	__asm { sub esp, 4 }; \
	__asm { push eax }; \
	__asm { sub eax, -6 }; \
	__asm { mov [esp + 4], eax }; \
	__asm { ret }; \
	__asm { SymbolGen(Fake, __LINE__): }; \
	__asm { _emit (0xd7 + __LINE__ % 8) }; \
	__asm { mov eax, [esp + 4] }; \
	__asm { ret }; \
	__asm { _emit(0xd8 + __LINE__ % 8) }; \
	__asm { add esp, 4 }; 
#else
#define OBFU_ONE()
#define OBFU_TWO()
#define OBFU_THR()
#define OBFU_FOUR()
#endif

void __forceinline CheckDebugFn()
{
#ifndef TESTING
	bool bExcep = false;
	__try
	{
		OBFU_ONE();
		__asm
		{
			pushfd;
			or dword ptr[esp], 0x100;
			popfd;
			nop;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OBFU_ONE();
		bExcep = true;
	}
	if (!bExcep || IsDebuggerPresent())
	{
		OBFU_ONE();
		TerminateProcess((HANDLE)(-1), 0xF0FFBABE);
		OBFU_ONE();
		exit(0xF0FFBABE);
	}
#endif
}

extern dword g_dwFlag;

#define SetTrapFlag() \
	__asm { pushfd }; \
	__asm { or dword ptr[esp], 0x100 }; \
	__asm { popfd }; \
	__asm { nop };

//CheckDebug();
extern bool g_bExcep;
#ifndef TESTING
#define CheckDebug() g_bExcep = false;\
	if (g_dwFlag) \
	{ \
		__try \
		{ \
			SetTrapFlag(); \
		} \
		__except (EXCEPTION_EXECUTE_HANDLER) \
		{ \
			g_bExcep = true; \
		} \
		if (!g_bExcep || IsDebuggerPresent()) \
		{ \
			OBFU_FOUR(); \
			TerminateProcess((HANDLE)(-1), 0xF0FFBABE + __LINE__); \
			OBFU_ONE(); \
			exit(0xF0FFBABE + __LINE__ % 0x10); \
		} \
	}
#else 
#define CheckDebug()
#endif
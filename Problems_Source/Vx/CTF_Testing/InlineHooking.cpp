#include "stdafx.h"
#include "InlineHooking.h"

//************************************
// Method:    Hotpatch
// FullName:  Hotpatch
// Access:    public 
// Returns:   void __fastcall
// Purpose:   Places a jump at the beginning of a function, redirects flow entirely to 
//             the new function.
// Parameter: void * ofunc, Pointer to the original function.
// Parameter: void * hkfunc, Pointer to the new function.
//************************************
void __fastcall Hotpatch(void* pOFunc, void* pHkFunc)
{
	DWORD dwOldProt = 0;
	BYTE bPatch[5];
	bPatch[0] = 0xE9;
	VirtualProtect((void*)pOFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProt);
	DWORD dwRelativeAddr = ((DWORD)pHkFunc - (DWORD)pOFunc - 5);
	memcpy(&bPatch[1], &dwRelativeAddr, 4);
	memcpy(pOFunc, bPatch, 5);
	VirtualProtect((void*)pOFunc, 5, dwOldProt, NULL);
}

//************************************
// Method:    Detour
// FullName:  Detour
// Access:    public 
// Returns:   DWORD __fastcall
// Purpose:   Detours a function, assumes its a windows API function.
// Parameter: unsigned char * ofunc, Pointer to the original function.
// Parameter: unsigned char * hkfunc, Pointer to the new function.
//************************************
DWORD __fastcall Detour(unsigned char* ofunc, unsigned char* hkfunc)
{
	DWORD dwOldProt = 0;
	BYTE bPatch[5];
	bPatch[0] = 0xE9;
	VirtualProtect((void*)ofunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProt);
	DWORD addr = ((DWORD)hkfunc - (DWORD)ofunc - 5);
	memcpy(&bPatch[1], &addr, 4);
	memcpy(ofunc, bPatch, 5);
	VirtualProtect((void*)ofunc, 5, dwOldProt, NULL);
	return ((DWORD)ofunc + 5);
}

//************************************
// Method:    Detour
// FullName:  Detour
// Access:    public 
// Returns:   DWORD __fastcall
// Purpose:   Detours a function and overwrites all bytes past the jump as specified by 
//             dwSize with NOPs.
// Parameter: unsigned char * uchOFunction, Pointer to the original function.
// Parameter: unsigned char * uchHkFunction, Pointer to the new function.
// Parameter: DWORD dwSize, Size of block being changed.
//************************************
DWORD __fastcall Detour(unsigned char* uchOFunction, unsigned char* uchHkFunction, DWORD dwSize)
{
	if (dwSize<5)
		return (DWORD)uchOFunction;
	DWORD dwOldProt = 0;
	BYTE bPatch[5];
	bPatch[0] = 0xE9;
	VirtualProtect((void*)uchOFunction, 5, PAGE_EXECUTE_READWRITE, &dwOldProt);
	DWORD dwAddress = ((DWORD)uchHkFunction - (DWORD)uchOFunction - 5);
	memcpy(&bPatch[1], &dwAddress, 4);
	memcpy(uchOFunction, bPatch, 5);
	memset(uchOFunction + 5, 0x90, dwSize - 5);
	VirtualProtect((void*)uchOFunction, 5, dwOldProt, 0);
	return (DWORD)uchOFunction + dwSize;
}

#pragma optimize("", off)
void* DetourTramp(unsigned char* pOFunction, unsigned char* pHkfunction, int stSize)
{
	// If you dont give me room for a jmp I don't compensate
	if (stSize<5)
		return (void*)0;
	// Bunch of places to put stuff
	DWORD dwOldProt = 0;
	unsigned char bPatch[5];
	unsigned char* bPrologue = new unsigned char[stSize];
	bPatch[0] = 0xE9;

	// Standard detour function.
	VirtualProtect((void*)pOFunction, 5, PAGE_EXECUTE_READWRITE, &dwOldProt);
	DWORD dwAddress = ((DWORD)pOFunction - (DWORD)pHkfunction - 5);
	memcpy(&bPatch[1], &dwAddress, 4);
	memcpy(bPrologue, pOFunction, stSize);
	memcpy(pOFunction, bPatch, 5);
	memset(pOFunction + 5, 0x90, stSize - 5);
	VirtualProtect((void*)pOFunction, 5, dwOldProt, 0);

	// Trampoline function generation.
	void* pTrampoline = VirtualAlloc(NULL, stSize + 6, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(pTrampoline, bPrologue, stSize); // Copy the old prologue to this function.
	dwAddress = (DWORD)pOFunction + 5; // The place I want the trampoline to jump back to.
	memcpy((void*)((DWORD)pTrampoline + stSize + 2), &dwAddress, 4); // Copy the address to the executable location.
	*(short*)((DWORD)pTrampoline + stSize) = 0x25FF; // Put the far jmp instruction (little endian madafucka).
	delete bPrologue; // Free some mem I allocated.
	return pTrampoline; // Return the function ptr.
}
#pragma optimize("", on)

//************************************
// Method:    printFunctionBytecodes
// FullName:  printFunctionBytecodes
// Access:    public 
// Returns:   void
// Purpose:   Print out the byte codes of a function (imperfect).
// Parameter: BYTE * pFunction, Pointer to the function.
//************************************
void printFunctionBytecodes(BYTE* pFunction)
{
	printf("Function Ptr: %X\n", pFunction);
	int i = 0;
	while (true){
		BYTE* ptr = (BYTE*)(pFunction + i);
		printf("%X ", *ptr);
		if (i % 10 == 0)
			printf("\n");
		if (*ptr == 0xC2 || *ptr == 0xC3)
			break;
		i++;
	}
	printf("\n\n");
}

//************************************
// Method:    GetFuncSize
// FullName:  GetFuncSize
// Access:    public 
// Returns:   unsigned int __fastcall
// Purpose:   Get the size of a function without parsing ASM (imperfect).
// Parameter: unsigned char * uchFunction, Pointer to the function.
//************************************
unsigned int __fastcall GetFuncSize(unsigned char *uchFunction)
{
	register unsigned int uiCount = 0;
	for (; *uchFunction != 0xC2 && *uchFunction != 0xC3; uiCount++, uchFunction++);
	return uiCount;
}

//************************************
// Method:    NopFunction
// FullName:  NopFunction
// Access:    public 
// Returns:   void __fastcall
// Purpose:   Make a function do nothing using NOPs.
// Parameter: unsigned char * uchFunction, Pointer to the function.
// Parameter: unsigned int uiSize, Size of block to NOP, does nothing.
//************************************
void __fastcall NopFunction(unsigned char *uchFunction, unsigned int uiSize = 0)
{
	uiSize = GetFuncSize(uchFunction);
	DWORD dwOldProtection = 0;
	VirtualProtect(uchFunction, uiSize, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	memset(uchFunction, 0x90, uiSize);
	VirtualProtect(uchFunction, uiSize, dwOldProtection, nullptr);
}
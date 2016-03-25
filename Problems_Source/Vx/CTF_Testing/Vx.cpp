#include "stdafx.h"
#include <time.h>
#include "VxAsm.h"

int g_argc;
char** g_args;
bool g_bExcep = false;
dword g_dwFlag = 0x100;

int main(int argc, char** args);

extern "C" int Tester();

typedef long(__cdecl *RtlSetProcessIsCritical) (IN BOOLEAN bNew, OUT BOOLEAN *pbOld, IN BOOLEAN bNeedScb);
RtlSetProcessIsCritical SetCriticalProcess;

__declspec(naked) PVOID __stdcall HkAddVectoredContHandler()
{
	OBFU_FOUR();
	exit(0x0000BABE);
	__asm
	{
		mov eax, 0;
		retn;
	};
}

__declspec(naked) PVOID __stdcall HkAddVectoredExceptionHandler()
{
	OBFU_FOUR();
	exit(0x0000F0FF);
	__asm
	{
		mov eax, 0;
		retn;
	};
}

// Never used due to me being told "No blue screens"
DWORD DsCheckPrivilegesProcess(void)
{
	// Request admin access from the user.
	SetLastError(ERROR_SUCCESS);
	char chFilePath[MAX_PATH];
	if (GetModuleFileNameA(0, chFilePath, sizeof(chFilePath)))
	{
		SHELLEXECUTEINFOA Sei;
		memset(&Sei, 0, sizeof(Sei));
		XorS(sRunas, "runas");
		Sei.cbSize = sizeof(Sei);
		Sei.lpVerb = sRunas.ToggleEncryption();
		Sei.hwnd = 0;
		Sei.nShow = SW_NORMAL;
		Sei.lpFile = chFilePath;
		if (!ShellExecuteExA(&Sei))
			return GetLastError();
	}
	else
	{
		TerminateProcess((HANDLE)(-1), 0xDEADBABE);
	}
	return GetLastError();
}

dword pDecrypt = (dword)HkAddVectoredExceptionHandler;
void* pKiUsr = 0;
byte oKiUsr[6];

#if VX_VER > 1
XorS(ntdll, "ntdll.dll");
XorS(ki_usr_excep, "KiUserExceptionDispatcher");
XorS(hey_user, "Hows that debugger?\n");
#endif

void __forceinline IntializeProtection()
{
	// No BSODs John, waaah

#if VX_VER > 1
	__asm
	{
		push ss
	}
	OBFU_FOUR();
	__asm
	{
		pop ss
		pushf
		nop
		pop eax
		and eax, 0x100
		or eax, eax
		jnz 0
	}

#endif

#if VX_VER == 1
	XorS(ntdll, "ntdll.dll");
#endif
	OBFU_ONE();
	HMODULE hNtdll = GetModuleHandleA(ntdll.ToggleEncryption());
	ntdll.ToggleEncryption();

	OBFU_ONE();

	Hotpatch((byte*)AddVectoredExceptionHandler, (byte*)HkAddVectoredExceptionHandler);
	OBFU_FOUR();
	Hotpatch((byte*)AddVectoredContinueHandler, (byte*)HkAddVectoredContHandler);
#if VX_VER == 1
	XorS(ki_usr_excep, "KiUserExceptionDispatcher");
#endif
	OBFU_TWO();
	byte* pKiUsrExcep = (byte*)GetProcAddress(hNtdll, ki_usr_excep.ToggleEncryption());
	pKiUsr = pKiUsrExcep;
	OBFU_THR();
	ki_usr_excep.ToggleEncryption();
	VirtualProtect((void*)pKiUsrExcep, 6, PAGE_EXECUTE_READWRITE, (PDWORD)oKiUsr);
	memcpy(oKiUsr, pKiUsrExcep, 6);
	
	g_dwFlag = 0;
	
	*(short*)pKiUsrExcep = 0x25ff;
	*(dword*)(pKiUsrExcep + 2) = (dword)&pDecrypt;
}

volatile dword dwActiveThreads = 0;

enum ActiveThreads : dword
{
	TH_MAIN = 1 << 0,
	TH_REAL = 1 << 1,
	TH_ENDENUM
};

#define init_thread(thFn, thEnum) dwActiveThreads |= thEnum; CreateThread(0, 0, thFn, 0, 0, 0);

void SetupRuntime()
{
	g_dwFlag |= 0x100;
	memcpy(pKiUsr, oKiUsr, 6);
}

__declspec(naked) void JumpOut()
{
	dwActiveThreads &= ~ActiveThreads::TH_REAL;
	ExitThread(0);
}

DWORD __stdcall RealThread(LPVOID pParam)
{
	// Do the stuff here...
	// Initalizes the VxCpu runtime.
	__asm
	{
		jmp CodeStuff
	szBadFormat:
		_emit '%'
		_emit 'x'
		_emit '\n'
		_emit 0
	CodeStuff:
		push dword ptr ds : [0]
		push szBadFormat
		call printf_s
		add esp, 8
	}
	dwActiveThreads &= ~ActiveThreads::TH_REAL;
	return 0;
}

XorS(err, "Must have at least 9 parameters.");

int main(int argc, char** args)
{
	OBFU_ONE();

	CheckDebug();

	OBFU_FOUR();

	g_args = args;
	g_argc = argc;

	OBFU_ONE();
	CheckDebug();
	OBFU_ONE();

#if VX_VER == 1
	if (argc < 10)
	{
		OBFU_ONE();
		puts(err.ToggleEncryption());
		OBFU_ONE();
		exit(1);
	}
#endif

	CheckDebug();
	OBFU_FOUR();
	IntializeProtection();

	OBFU_ONE();
	__asm
	{
		mov eax, dword ptr fs : [0x18]
		mov eax, dword ptr ds : [eax + 0x30];
		movzx eax, byte ptr ds : [eax + 2];
		cmp eax, 0;
		jnz DebuggerPres;
		mov eax, dword ptr fs : [0x30];
		movzx eax, byte ptr ds : [eax + 2];
		test eax, eax;
		jnz DebuggerPres;
		mov eax, VxAsm::BeginExecution;
		mov dword ptr ds : [pDecrypt], eax;
	DebuggerPres:
	}

	init_thread(RealThread, ActiveThreads::TH_REAL);
	
	while (dwActiveThreads)
	{
		OBFU_ONE();
		CheckDebug();
	}

	// Destory IDA pseudo-c
	__asm
	{
		sub esp, 0x100
		push 0
		call exit
		sub esp, 0x100
	}

	return 0;
}


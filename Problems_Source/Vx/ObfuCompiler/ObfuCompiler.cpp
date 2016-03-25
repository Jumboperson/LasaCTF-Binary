#include "stdafx.h"
#include "VxAsmCompiler.h"

void DeleteString(char* szTerm)
{
	__try
	{
		delete[] szTerm;
	}
	__except (1)
	{
		return;
	}
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		puts("Usage: ObfuCompiler.exe <Source> <OutInstSet> <OutBinary> [flags]");
		return 1;
	}
	
	// Pre-compile
	InstructionSet::GenerateInstructionSet();
	
	// Compilation
	VxAsmCompiler::Compile(argv[1], argv[3]);

	// Post-compile
	InstructionSet::WriteInstructionSetToFile(argv[2]);

    return 0;
}


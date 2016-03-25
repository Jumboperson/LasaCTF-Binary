#pragma once
#include "stdafx.h"

namespace InstructionSet
{
#include <time.h>
	// Registers
	// 8 registers
	// Unique IDs for each of them each compilation

	// General Instructions
	// + mul
	// + div
	// + add
	// + sub
	// + mov
	// + lea
	// + shl
	// + shr
	// + xor
	// + or
	// + and
	// + cmp
	// + jmp
	// + jg
	// + jl
	// + jle
	// + jge
	// + jz
	// + jnz
	// + call
	// + natv
	// + CreateThread
	// + ret
	// + nop

	enum Id : byte
	{
		ID_MUL,
		ID_DIV,
		ID_ADD,
		ID_SUB,
		ID_MOV,
		ID_LEA,
		ID_SHL,
		ID_SHR,
		ID_XOR,
		ID_OR,
		ID_AND,
		ID_CMP,
		ID_JMP,
		ID_JG,
		ID_JL,
		ID_JLE,
		ID_JGE,
		ID_JZ,
		ID_JNZ,
		ID_PUSH,
		ID_POP,
		ID_CALL,
		ID_NATV, // Native call
		ID_CRTH, // Create thread.
		ID_RET,
		ID_NOP,
		ID_STK,
		ID_ENDENUM
	};

	char* szInstrucs[Id::ID_ENDENUM] =
	{
		"mul",
		"div",
		"add",
		"sub",
		"mov",
		"lea",
		"shl",
		"shr",
		"xor",
		"or",
		"and",
		"cmp",
		"jmp",
		"jg",
		"jl",
		"jle",
		"jge",
		"jz",
		"jnz",
		"push",
		"pop",
		"call",
		"natv",
		"crth",
		"ret",
		"nop",
		"stk"
	};

	enum Order : byte
	{
		ORD_FOR,
		ORD_REV,
		ORD_END
	};

	struct Instruction
	{
		byte m_bId = 0;
		byte m_bParamCount = 0;
		Order m_order = Order::ORD_FOR;
	};

#define InstructionCount Id::ID_ENDENUM

	Instruction instructionSet[InstructionCount];

	void GenerateInstructionSet()
	{
		srand(time(NULL));
		for (int i = 0; i < InstructionCount; ++i)
		{
			byte bId = rand() % 0xFF;
			for (int j = 0; j < i; ++j)
			{
				if (bId == instructionSet[j].m_bId)
				{
					j = -1;
					bId = rand() % 0xFF;
				}
			}
			instructionSet[i].m_bId = bId;

			if (i < Id::ID_JMP)
				instructionSet[i].m_bParamCount = 2;
			else if (i < Id::ID_RET)
				instructionSet[i].m_bParamCount = 1;

			instructionSet[i].m_order = (Order)(rand() % Order::ORD_END);
		}
		instructionSet[Id::ID_STK].m_bParamCount = 2;
	}

	void WriteInstructionSetToFile(char* szName)
	{
		std::ofstream outFile(szName, std::ios::binary);

		for (int i = 0; i < InstructionCount; ++i)
		{
			outFile << instructionSet[i].m_bId << instructionSet[i].m_bParamCount << instructionSet[i].m_order << (byte)(rand() % 256);
		}
	}
}
#pragma once
#include "stdafx.h"
#include "InstructionSet.h"

#define DebugPrint // 
//#define DebugPrint printf_s
namespace VxAsmCompiler
{
	enum SymType : byte
	{
		SYM_FUNC,
		SYM_VAR,
		SYM_LBL,
		SYM_ENDENUM
	};

	template<int argc>
	struct MemInstruction
	{
		byte m_id;
		struct Flags
		{
			byte sf : 1;
			byte flUse : 2;
			byte drf0 : 1;
			byte drf1 : 1;
			byte reg0 : 3;
			byte reg1 : 3;
			byte _next[3];
		} m_flags;
		int args[argc];

		void SetDeref(byte adr0, byte adr1)
		{
			m_flags.drf0 = adr0;
			m_flags.drf1 = adr1;
		}

		void SetReg(byte adr0, byte adr1)
		{
			m_flags.reg0 = adr0;
			m_flags.reg1 = adr1;
		}

		void SetUse(byte use)
		{
			m_flags.flUse = use;
		}

		int GetNext()
		{
			int nxt = 0;
			nxt = *(int*)_next;
			((byte*)&nxt)[3] = 0;
			if (sf)
			{
				nxt *= -1;
			}
			return nxt;
		}

		void SetNext(int iNext)
		{
			if (iNext < 0)
				m_flags.sf = 1;
			else
				m_flags.sf = 0;
			((byte*)&iNext)[3] = 0;
			memcpy(m_flags._next, &iNext, 3);
		}
	};

	struct SymLink
	{
		MemInstruction<0>* m_pInst;
		byte m_argNum;
		bool m_bIsReversed = false;
	};

	struct Symbol
	{
		Symbol(char* szName, SymType type, ul ulPos)
			: m_type(type), m_ulPosition(ulPos)
		{
			strcpy_s(m_szName, szName);
		}
		SymType m_type;
		char m_szName[64];
		ul m_ulPosition;
		std::vector<SymLink> m_vecLinks = std::vector<SymLink>();
	};

	byte arMemory[0x4000];
	bool arUsed[sizeof(arMemory)];
	ul ulLine = 0;

	std::vector<Symbol> vecSymbols = std::vector<Symbol>();
	std::vector<char> vecIAT = std::vector<char>();
	int iImports = 0;

	int IatContains(char* szMod, char* szFunc, int iParam)
	{
		int iFFCnt = 0;
		for (int i = 0; i < vecIAT.size(); ++i)
		{
			if ((byte)vecIAT[i] == 0xFF)
				++iFFCnt;
			
			if (vecIAT[i] == *szMod)
			{
				//printf_s("%s %s -- %s %s -- %i %i -- %i\n", szMod, &vecIAT[i], szFunc, &vecIAT[i] + strlen(szMod) + 1, *(int*)(&vecIAT[i] + strlen(szMod) + 2 + strlen(szFunc)), iParam, iFFCnt);
				if (!strcmp(szMod, &vecIAT[i]) && !strcmp(szFunc, &vecIAT[i] + strlen(szMod) + 1) && *(int*)(&vecIAT[i] + strlen(szMod) + 2 + strlen(szFunc)) == iParam)
					return iFFCnt / 2;
			}
		}
		return -1;
	}

	char* arConventions[] = 
	{
		"cdecl",
		"stdcall",
		"fastcall"
	};

	word GetCallingConvention(char* szConven)
	{
		for (int i = 0; i < sizeof(arConventions) / sizeof(*arConventions); ++i)
		{
			if (!strcmp(arConventions[i], szConven))
				return i;
		}
		return -1;
	}

	char* GetFileContents(char* szFilename, ul& ulSize)
	{
		std::ifstream in;
		char* pContents;
		ulSize = 0;
		pContents = NULL;
		in.open(szFilename, std::ios::in | std::ios::binary);
		if (in.is_open())
		{
			in.seekg(0, std::ios::end);
			ulSize = in.tellg();
			in.seekg(0, std::ios::beg);
			pContents = new char[ulSize + 2];
			in.read((char*)pContents, ulSize);
			pContents[ulSize] = '\n';
			pContents[ulSize + 1] = 0;
			in.close();
		}
		return pContents;
	}

	InstructionSet::Id GetId(char* szLine)
	{
		for (int i = 0; i < InstructionSet::Id::ID_ENDENUM; ++i)
		{
			if (!strcmp(szLine, InstructionSet::szInstrucs[i]))
			{
				return (InstructionSet::Id)i;
			}
		}
		return InstructionSet::Id::ID_ENDENUM;
	}

	bool ValidLocation(ul ulLoc, ul ulSize)
	{
		for (int i = 0; i < ulSize; ++i)
		{
			if (arUsed[ulLoc + i])
				return false;
		}
		return true;
	}

	ul FindNewLocation(ul ulSize)
	{
		ul ulStart = rand() % sizeof(arMemory);
		for (ul u = ulStart, numLooked = 0; numLooked < sizeof(arMemory) - ulSize;
		++numLooked, ++u, (u >(sizeof(arMemory) - ulSize)) ? u = 0 : u = u)
		{
			if (ValidLocation(u, ulSize))
				return u;
		}
		return -1;
	}

	void SetUsed(ul ulPos, ul ulSize)
	{
		for (int i = 0; i < ulSize; ++i)
			arUsed[ulPos + i] = true;
	}

	struct ArgFlags
	{
		byte dr0 : 1;
		byte dr1 : 1;
		byte regUse : 2;
		byte reg0 : 3;
		byte reg1 : 3;

		void SetDeref(byte adr0, byte adr1)
		{
			dr0 = adr0;
			dr1 = adr1;
		}

		void SetRegUse(byte areg0, byte areg1)
		{
			regUse = 0;
			if (areg0)
				regUse |= 1 << 0;
			if (areg1)
				regUse |= 1 << 1;
		}

		void SetReg(byte areg0, byte areg1)
		{
			reg0 = areg0;
			reg1 = areg1;
		}
	};

	template<int argc>
	MemInstruction<argc>* MakeInstruction(byte* pLoc, InstructionSet::Id id, ArgFlags& flag, int arg0 = 0, int arg1 = 0)
	{
		DebugPrint("INST: %i @ pLoc %X %i %i\n", id, pLoc - arMemory, arg0, arg1);
		DebugPrint("OpCode: %X\n", InstructionSet::instructionSet[id].m_bId);
		if (flag.reg0 > 7 || flag.reg1 > 7)
		{
			fprintf_s(stderr, "Cannot use a register greater than r7 @ line %lu.\n", ulLine);
			exit(1);
		}

		if (argc == 2)
		{
			MemInstruction<argc> mem = MemInstruction<argc>();

			mem.SetDeref(flag.dr0, flag.dr1);
			mem.SetUse(flag.regUse);
			mem.SetReg(flag.reg0, flag.reg1);
			mem.m_id = InstructionSet::instructionSet[id].m_bId;

			if (InstructionSet::instructionSet[id].m_order == InstructionSet::Order::ORD_FOR)
			{
				if (flag.regUse & 1)
					mem.args[0] = rand();
				else
					mem.args[0] = arg0;
				
				if (flag.regUse & 2)
					mem.args[1] = rand();
				else
					mem.args[1] = arg1;
			}
			if (InstructionSet::instructionSet[id].m_order == InstructionSet::Order::ORD_REV)
			{
				if (flag.regUse & 1)
					mem.args[1] = rand();
				else
					mem.args[1] = arg0;

				if (flag.regUse & 2)
					mem.args[0] = rand();
				else
					mem.args[0] = arg1;
			}

			memcpy(pLoc, &mem, sizeof(mem));
			SetUsed((ul)(pLoc - arMemory), sizeof(mem));

			if (memcmp(pLoc, &mem, sizeof(mem)))
			{
				fprintf_s(stderr, "Error!!!\n");
				exit(1);
			}
			return (MemInstruction<argc>*)pLoc;
		}

		if (argc == 1)
		{
			MemInstruction<argc> mem = MemInstruction<argc>();
			mem.SetDeref(flag.dr0, flag.dr1);
			mem.SetUse(flag.regUse);

			if(flag.regUse & 1)
				mem.args[0] = rand();
			else
				mem.args[0] = arg0;

			mem.SetReg(flag.reg0, flag.reg1);
			mem.m_id = InstructionSet::instructionSet[id].m_bId;
			memcpy(pLoc, &mem, sizeof(mem));
			SetUsed((ul)(pLoc - arMemory), sizeof(mem));

			if (*pLoc != InstructionSet::instructionSet[id].m_bId)
			{
				fprintf_s(stderr, "Error!!!\n");
				exit(1);
			}
			return (MemInstruction<argc>*)pLoc;
		}

		MemInstruction<argc> mem = MemInstruction<argc>();
		mem.SetDeref(flag.dr0, flag.dr1);
		mem.SetUse(flag.regUse);
		mem.SetReg(flag.reg0, flag.reg1);
		mem.m_id = InstructionSet::instructionSet[id].m_bId;
		memcpy(pLoc, &mem, sizeof(mem));
		SetUsed((ul)(pLoc - arMemory), sizeof(mem));

		if (*pLoc != InstructionSet::instructionSet[id].m_bId)
		{
			fprintf_s(stderr, "Error!!!\n");
			exit(1);
		}
		return (MemInstruction<argc>*)pLoc;
	}

	ul FindSymbol(char* szName, SymType type)
	{
		size_t stNumSym = vecSymbols.size();
		for (int i = 0; i < stNumSym; ++i)
		{
			if ((type != -1 ? vecSymbols[i].m_type == type : true)
				&& !strcmp(szName, vecSymbols[i].m_szName))
			{
				return i;
			}
		}
		return -1;
	}

	void WriteEscapedString(byte* pLoc, char* szString, size_t stLen)
	{
		for (int iStrIter = 0, iMem = 0; iStrIter < stLen; )
		{
			if (szString[iStrIter] != '\\')
			{
				pLoc[iMem++] = szString[iStrIter++];
				continue;
			}
			switch (szString[iStrIter + 1])
			{
			case 'x':
			{
				char parse[3];
				memcpy(parse, &szString[iStrIter + 2], 2);
				int iValue = 0;
				if (sscanf(parse, "%x", &iValue) != 1)
				{
					fprintf_s(stderr, "Invalid hex escape sequence @ line %i.\n", ulLine);
					exit(1);
				}
				iStrIter += 4;
				pLoc[iMem++] = iValue;
				break;
			}
			case 'n':
				iStrIter += 2;
				pLoc[iMem++] = '\n';
				break;
			case 'r':
				iStrIter += 2;
				pLoc[iMem++] = '\r';
				break;
			case 'b':
				iStrIter += 2;
				pLoc[iMem++] = '\b';
				break;
			case '\'':
				iStrIter += 2;
				pLoc[iMem++] = '\'';
				break;
			case '"':
				iStrIter += 2;
				pLoc[iMem++] = '"';
				break;
			case 't':
				iStrIter += 2;
				pLoc[iMem++] = '\t';
				break;
			case '0':
				iStrIter += 2;
				pLoc[iMem++] = '\0';
				break;
			case '\\':
				iStrIter += 2;
				pLoc[iMem++] = '\\';
				break;
			default:
				fprintf_s(stderr, "Invalid escape sequence at line %i\n", ulLine);
				exit(1);
			}
		}
	}

	MemInstruction<0>* pLastInst = 0;
	bool bInsideProc = false;
	bool bFirstInsideFunction = false;
	bool bLabel = false;
	int iLabelIndex = 0;
	bool WriteInstruction(byte*& pByte, char* szInst)
	{
#define FunctionRegisterCheck if(bFirstInsideFunction) { bFirstInsideFunction = false; vecSymbols[vecSymbols.size() - 1].m_ulPosition = pByte - arMemory; DebugPrint("Function at %x\n", pByte - arMemory); } \
	if(bLabel) { bLabel = false; vecSymbols[iLabelIndex].m_ulPosition = pByte - arMemory; DebugPrint("Label at %s at %X\n", vecSymbols[iLabelIndex].m_szName, pByte - arMemory); }
		SSValue sVal = SSValue(szInst);
		puts(szInst);
		size_t stNumVars = sVal.GetSize();
		if (!stNumVars)
			return false;

		char* szInstr = sVal.GetTerm(0);

		InstructionSet::Id id = GetId(szInstr);

		switch (id)
		{
		case InstructionSet::Id::ID_ENDENUM:
		{
			FunctionRegisterCheck;
			int iTemp__ = 0;
			if (!strcmp(szInstr, "dv"))
			{
				if (sVal.GetTerm(1)[0] == 'r'
					&& sscanf_s(sVal.GetTerm(1) + 1, "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Variable can not be named %s at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (sscanf_s(sVal.GetTerm(1), "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Variable %s can not have a numerical prefix at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (stNumVars < 3)
				{
					fprintf_s(stderr, "Not enough arguments for 'dv' on line %lu\n", ulLine);
					exit(1);
				}

				if (FindSymbol(sVal.GetTerm(1), SymType::SYM_VAR) != -1)
				{
					fprintf_s(stderr, "Var %s already exists, line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				int iVal = 0;
				if (*(word*)sVal.GetTerm(2) == 0x7830)
				{
					if (sscanf(sVal.GetTerm(2), "%x", &iVal) != 0)
					{
						ul ulLoc = FindNewLocation(4);
						if (ulLoc == -1)
						{
							fprintf_s(stderr, "Not enough memory to allocate var %s at %lu\n", sVal.GetTerm(1), ulLine);
							exit(1);
						}
						SetUsed(ulLoc, 4);
						*(int*)&arMemory[ulLoc] = iVal;
						Symbol sym = Symbol(sVal.GetTerm(1), SymType::SYM_VAR, ulLoc);
						vecSymbols.push_back(sym);
						DebugPrint("Int %s registered %i @ %X\n", sVal.GetTerm(1), iVal, ulLoc);
						return false;
					}
				}
				else
				{
					if (sscanf(sVal.GetTerm(2), "%i", &iVal) != 0)
					{
						ul ulLoc = FindNewLocation(4);
						if (ulLoc == -1)
						{
							fprintf_s(stderr, "Not enough memory to allocate var %s at %lu\n", sVal.GetTerm(1), ulLine);
							exit(1);
						}
						SetUsed(ulLoc, 4);
						*(int*)&arMemory[ulLoc] = iVal;
						Symbol sym = Symbol(sVal.GetTerm(1), SymType::SYM_VAR, ulLoc);
						vecSymbols.push_back(sym);
						DebugPrint("Int %s registered %i @ %X\n", sVal.GetTerm(1), iVal, ulLoc);
						return false;
					}
				}

				if (sVal.GetTerm(2)[0] == '"')
				{
					//sVal.~SSValue();
					SSValue sValNew = SSValue(szInst, true);
					char* szString = sValNew.GetStringPast('"');
					size_t stLength = strlen(szString);

					if (szString[stLength - 1] == '"' && (szString[stLength - 3] == '\\' ? true : szString[stLength - 2] != '\\'))
						szString[stLength - 1] = '\0';
					else
					{
						fprintf_s(stderr, "Invalid ending for string at line %lu.\n", ulLine);
						exit(1);
					}

					ul ulLoc = FindNewLocation(stLength);
					if (ulLoc == -1)
					{
						fprintf_s(stderr, "Not enough memory to allocate var %s at %lu\n", sValNew.GetTerm(1), ulLine);
						exit(1);
					}
					SetUsed(ulLoc, stLength);
					WriteEscapedString(&arMemory[ulLoc], szString, stLength);
					Symbol sym = Symbol(sValNew.GetTerm(1), SymType::SYM_VAR, ulLoc);
					vecSymbols.push_back(sym);
					DebugPrint("String '%s' registered \"%s\" @%x\n", sValNew.GetTerm(1), szString, ulLoc);
					//delete[] szString;
					return false;
				}

				char szStr[64];
				if (sscanf_s(sVal.GetTerm(2), "%s", szStr) != 0)
				{
					ul ulLoc = FindNewLocation(4);
					ul ulSymIndex = FindSymbol(szStr, (SymType)-1);

					if (ulLoc == -1)
					{
						fprintf_s(stderr, "Not enough memory to allocate var %s at %lu\n", sVal.GetTerm(1), ulLine);
						exit(1);
					}

					if (ulSymIndex == -1)
					{
						fprintf_s(stderr, "Cannot create a var %s of value %s because %s does not exist.\n", sVal.GetTerm(1), sVal.GetTerm(2), sVal.GetTerm(2));
						exit(1);
					}

					SetUsed(ulLoc, 4);
					memcpy(&arMemory[ulLoc], &vecSymbols[ulSymIndex].m_ulPosition, 4);
					Symbol sym = Symbol(sVal.GetTerm(1), SymType::SYM_VAR, ulLoc);
					vecSymbols.push_back(sym);
					DebugPrint("Var '%s' registered.\n", sVal.GetTerm(1), szStr);
					//delete[] szStr;
					return false;
				}

				fprintf_s(stderr, "Var %s is not a valid type, line %lu\n", sVal.GetTerm(1), ulLine);
				exit(1);
			}

			if (!strcmp(szInstr, "da"))
			{
				if (sVal.GetTerm(1)[0] == 'r'
					&& sscanf_s(sVal.GetTerm(1) + 1, "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Array can not be named %s at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (sscanf_s(sVal.GetTerm(1), "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Array %s can not have a numerical prefix at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (stNumVars < 3)
				{
					fprintf_s(stderr, "Not enough arguments for 'da' on line %lu\n", ulLine);
					exit(1);
				}

				if (FindSymbol(sVal.GetTerm(1), SymType::SYM_VAR) != -1)
				{
					fprintf_s(stderr, "Var %s already exists, line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				std::vector<int> vecVars = std::vector<int>();

				for (int i = 2; i < stNumVars; ++i)
				{
					int iVal = 0;
					if (sscanf(sVal.GetTerm(i), "%i", &iVal) != 0)
					{
						vecVars.push_back(iVal);
						continue;
					}
					char szVar[64];
					if (sscanf(sVal.GetTerm(i), "%s", szVar) != 0)
					{
						ul ulSym = FindSymbol(szVar, (SymType)-1);
						if (ulSym == -1)
						{
							fprintf_s(stderr, "Var %s does not exist at line %lu\n", szVar, ulLine);
							exit(1);
						}
						vecVars.push_back(vecSymbols[ulSym].m_ulPosition);
					}

					fprintf_s(stderr, "Var %s is not a valid type, line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
					
				}
				
				ul ulLoc = FindNewLocation(4 * vecVars.size());
				if (ulLoc == -1)
				{
					fprintf_s(stderr, "Not enough memory to allocate var %s at %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}
				SetUsed(ulLoc, 4 * vecVars.size());
				for (int i = 0; i < vecVars.size(); ++i)
					*(int*)&arMemory[ulLoc + (4 * i)] = vecVars[i];
				Symbol sym = Symbol(sVal.GetTerm(1), SymType::SYM_VAR, ulLoc);
				vecSymbols.push_back(sym);
				DebugPrint("Array '%s' registered.\n", sVal.GetTerm(1));
				return false;
			}

			if (!strcmp(szInstr, "proc"))
			{
				if (sVal.GetTerm(1)[0] == 'r'
					&& sscanf_s(sVal.GetTerm(1) + 1, "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Function can not be named %s at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (sscanf_s(sVal.GetTerm(1), "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Function %s can not have a numerical prefix at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (stNumVars < 2)
				{
					fprintf_s(stderr, "Not enough arguments for 'proc' on line %lu\n", ulLine);
					exit(1);
				}

				if (FindSymbol(sVal.GetTerm(1), SymType::SYM_FUNC) != -1)
				{
					fprintf_s(stderr, "Function %s already exists, line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				bInsideProc = true;
				bFirstInsideFunction = true;
				Symbol sym = Symbol(sVal.GetTerm(1), SymType::SYM_FUNC, 0);
				DebugPrint("Function '%s' register'd\n", sVal.GetTerm(1));
				vecSymbols.push_back(sym);
				return false;
			}

			if (!strcmp(szInstr, "lbl"))
			{
				if (sVal.GetTerm(1)[0] == 'r'
					&& sscanf_s(sVal.GetTerm(1) + 1, "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Label can not be named %s at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (sscanf_s(sVal.GetTerm(1), "%i", &iTemp__) > 0)
				{
					fprintf_s(stderr, "Label %s can not have a numerical prefix at line %lu\n", sVal.GetTerm(1), ulLine);
					exit(1);
				}

				if (stNumVars < 2)
				{
					fprintf_s(stderr, "Not enough arguments for 'lbl' on line %lu\n", ulLine);
					exit(1);
				}

				if (FindSymbol(sVal.GetTerm(1), SymType::SYM_LBL) != -1)
				{
					//printf_s("FINDING %s\n", sVal.GetTerm(1));
					bLabel = true;
					iLabelIndex = FindSymbol(sVal.GetTerm(1), SymType::SYM_LBL);
				}
				else
				{
					bLabel = true;
					Symbol sym = Symbol(sVal.GetTerm(1), SymType::SYM_LBL, 0);
					DebugPrint("Label '%s' register'd\n", sVal.GetTerm(1));
					iLabelIndex = vecSymbols.size();
					vecSymbols.push_back(sym);
				}
				return false;
			}

			return false;
		}

		case InstructionSet::Id::ID_NOP:
		{
			FunctionRegisterCheck;
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			if (pLastInst)
			{
				//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			pLastInst = (MemInstruction<0>*)MakeInstruction<0>(pByte, id, flags);
			return true;
		}

		case InstructionSet::Id::ID_RET:
		{
			FunctionRegisterCheck;
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			if (pLastInst)
			{
				//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			MakeInstruction<0>(pByte, id, flags);
			pLastInst = 0;
			bInsideProc = false;
			return true;
		}

		case InstructionSet::Id::ID_CRTH:
		{
			FunctionRegisterCheck;
			if (sVal.GetSize() < 2)
			{
				fprintf_s(stderr, "Not enough arguments for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}
			char* szTerm = sVal.GetTerm(1);
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			if (*szTerm == '[')
			{
				flags.dr0 = 1;
				++szTerm;
			}
			int iRegNum = 0;
			if (sscanf_s(szTerm + 1, "%i", &iRegNum) < 1)
			{
				flags.regUse = 0;
				ul sym = FindSymbol(sVal.GetTerm(1), SymType::SYM_FUNC);
				SymLink link;
				link.m_pInst = (MemInstruction<0>*)pByte;
				link.m_argNum = 0;
				vecSymbols[sym].m_vecLinks.push_back(link);
			}
			else
			{
				flags.regUse = 1;
				flags.reg0 = iRegNum;
			}

			if (pLastInst)
			{
				//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			pLastInst = (MemInstruction<0>*)MakeInstruction<1>(pByte, id, flags);
			return true;
		}

		case InstructionSet::Id::ID_NATV:
		{
			FunctionRegisterCheck;
			if (sVal.GetSize() < 5)
			{
				fprintf_s(stderr, "Not enough arguments for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}
			char* szDll = sVal.GetTerm(1);
			char* szFunc = sVal.GetTerm(2);
			char* szParams = sVal.GetTerm(4);
			char* szCallingConvention = sVal.GetTerm(3);
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			int iParams = atoi(szParams);
			int iRes = IatContains(szDll, szFunc, iParams);
			word wConven = GetCallingConvention(szCallingConvention);
			if (wConven == -1)
			{
				fprintf_s(stderr, "Calling convention invalid for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}
			if (iRes == -1)
			{
				// Todo handle native calls
				vecIAT.push_back('\xFF');
				size_t stMod = strlen(szDll) + 1;
				for (int i = 0; i < stMod; ++i)
				{
					vecIAT.push_back(szDll[i]);
				}

				stMod = strlen(szFunc) + 1;
				for (int i = 0; i < stMod; ++i)
				{
					vecIAT.push_back(szFunc[i]);
				}


				vecIAT.push_back(((char*)&iParams)[0]);
				vecIAT.push_back(((char*)&iParams)[1]);
				vecIAT.push_back(((char*)&wConven)[0]);
				vecIAT.push_back(((char*)&wConven)[1]);

				vecIAT.push_back('\xFF');
				if (pLastInst)
				{
					//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
					pLastInst->SetNext((int)pByte - (int)pLastInst);
				}
				//printf_s("New import %i\n", iImports);
				pLastInst = (MemInstruction<0>*)MakeInstruction<1>(pByte, id, flags, iImports++);
			}
			else
			{
				//printf_s("Reusing import %i\n", iRes);
				if (pLastInst)
				{
					//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
					pLastInst->SetNext((int)pByte - (int)pLastInst);
				}
				pLastInst = (MemInstruction<0>*)MakeInstruction<1>(pByte, id, flags, iRes);
			}
			
			return true;
		}

		case InstructionSet::Id::ID_CALL:
		{
			FunctionRegisterCheck;
			if (sVal.GetSize() < 2)
			{
				fprintf_s(stderr, "Not enough arguments for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}

			char* szTerm = sVal.GetTerm(1);
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			if (*szTerm == '[')
			{
				flags.dr0 = 1;
				++szTerm;
			}
			int iRegNum = 0;
			if (sscanf_s(szTerm + 1, "%i", &iRegNum) < 1)
			{
				flags.regUse = 0;
				ul sym = FindSymbol(sVal.GetTerm(1), SymType::SYM_FUNC);
				SymLink link;
				link.m_pInst = (MemInstruction<0>*)pByte;
				link.m_argNum = 0;
				vecSymbols[sym].m_vecLinks.push_back(link);
			}
			else
			{
				flags.regUse = 1;
				flags.reg0 = iRegNum;
			}

			if (pLastInst)
			{
				//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			pLastInst = (MemInstruction<0>*)MakeInstruction<1>(pByte, id, flags);

			return true;
		}

		case InstructionSet::Id::ID_POP:
		{
			FunctionRegisterCheck;
			if (sVal.GetSize() < 2)
			{
				fprintf_s(stderr, "Not enough arguments for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}

			char* szTerm = sVal.GetTerm(1);
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			int iRegNum = 0;
			if (sscanf_s(szTerm + 1, "%i", &iRegNum) < 1)
			{
				fprintf_s(stderr, "%s takes a register as an argument, line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}
			else
			{
				flags.regUse = 1;
				flags.reg0 = iRegNum;
			}

			if (pLastInst)
			{
				//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			pLastInst = (MemInstruction<0>*)MakeInstruction<1>(pByte, id, flags);

			return true;
		}

		case InstructionSet::Id::ID_PUSH:
		{
			FunctionRegisterCheck;

			if (sVal.GetSize() < 2)
			{
				fprintf_s(stderr, "Not enough arguments for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}

			char* szTerm = sVal.GetTerm(1);
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			int iValue = 0;
			if (*szTerm == 'r')
			{
				int iRegNum = 0;
				if (sscanf_s(szTerm + 1, "%i", &iRegNum) < 1)
				{
					fprintf_s(stderr, "%s takes a register as an argument, line %lu\n", InstructionSet::szInstrucs[id], ulLine);
					exit(1);
				}
				else
				{
					flags.regUse = 1;
					flags.reg0 = iRegNum;
				}
			}
			else
			{
				if (sscanf_s(szTerm, "%i", &iValue) < 1)
				{
					fprintf_s(stderr, "Invalid value at %s, line %lu\n", InstructionSet::szInstrucs[id], ulLine);
					exit(1);
				}
			}

			if (pLastInst)
			{
				//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			pLastInst = (MemInstruction<0>*)MakeInstruction<1>(pByte, id, flags, iValue);

			return true;
		}

		case InstructionSet::Id::ID_JNZ:
		case InstructionSet::Id::ID_JZ:
		case InstructionSet::Id::ID_JGE:
		case InstructionSet::Id::ID_JL:
		case InstructionSet::Id::ID_JG:
		case InstructionSet::Id::ID_JMP:
		{
			FunctionRegisterCheck;
			if (sVal.GetSize() < 2)
			{
				fprintf_s(stderr, "Not enough arguments for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}
			char* szTerm = sVal.GetTerm(1);
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			int iValue = 0;
			if (*szTerm == 'r')
			{
				int iRegNum = 0;
				if (sscanf_s(szTerm + 1, "%i", &iRegNum) < 1)
				{
					fprintf_s(stderr, "%s takes a register as an argument, line %lu\n", InstructionSet::szInstrucs[id], ulLine);
					exit(1);
				}
				else
				{
					flags.regUse = 1;
					flags.reg0 = iRegNum;
				}
			}
			else
			{
				if (sscanf_s(szTerm, "%i", &iValue) < 1)
				{
					ul ulSym = FindSymbol(szTerm, SymType::SYM_LBL);
					
					if (ulSym == -1)
					{
						ulSym = FindSymbol(szTerm, SymType::SYM_FUNC);
					}
					
					if (ulSym == -1)
					{
						ulSym = FindSymbol(szTerm, SymType::SYM_VAR);
					}
					
					if (ulSym == -1)
					{
						Symbol sPlaceHolder = Symbol(szTerm, SymType::SYM_LBL, 0);
						vecSymbols.push_back(sPlaceHolder);
						ulSym = vecSymbols.size() - 1;
					}
					SymLink link;
					link.m_pInst = (MemInstruction<0>*)pByte;
					link.m_argNum = 0;
					link.m_bIsReversed = false;
					vecSymbols[ulSym].m_vecLinks.push_back(link);
				}
			}

			if (pLastInst)
			{
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			pLastInst = (MemInstruction<0>*)MakeInstruction<1>(pByte, id, flags, iValue);

			//DebugPrint("%s, iValue: %i reg: %X %i\n", InstructionSet::szInstrucs[id], iValue, flags.regUse, flags.reg0);
			return true;
		}

		case InstructionSet::Id::ID_CMP:
		case InstructionSet::Id::ID_AND:
		case InstructionSet::Id::ID_OR:
		case InstructionSet::Id::ID_XOR:
		case InstructionSet::Id::ID_SHR:
		case InstructionSet::Id::ID_SHL:
		case InstructionSet::Id::ID_LEA:
		case InstructionSet::Id::ID_MOV:
		case InstructionSet::Id::ID_SUB:
		case InstructionSet::Id::ID_ADD:
		case InstructionSet::Id::ID_DIV:
		case InstructionSet::Id::ID_MUL:
		case InstructionSet::Id::ID_STK:
		{
			FunctionRegisterCheck;
			//puts("inst");
			if (sVal.GetSize() < 3)
			{
				fprintf_s(stderr, "Not enough arguments for %s on line %lu\n", InstructionSet::szInstrucs[id], ulLine);
				exit(1);
			}

			char* szTerm1 = sVal.GetTerm(1);
			char* szTerm2 = sVal.GetTerm(2);
			ArgFlags flags;
			memset(&flags, 0, sizeof(flags));
			int iValue1 = 0;
			int iValue2 = 0;

			if (*szTerm1 == '[')
			{
				flags.dr0 = 1;
				++szTerm1;
			}

			if (*szTerm2 == '[')
			{
				flags.dr1 = 1;
				++szTerm2;
			}

			if (*szTerm1 == 'r')
			{
				int iRegNum = 0;
				if (sscanf_s(szTerm1 + 1, "%i", &iRegNum) < 1)
				{
					goto LABEL_CHECK1;
				}
				else
				{
					flags.regUse |= 1;
					flags.reg0 = iRegNum;
					if (iRegNum > 7)
					{
						fprintf_s(stderr, "Cannot use a register greater than r7 @ line %lu.\n", ulLine);
						exit(1);
					}
				}
			}
			else
			{
			LABEL_CHECK1:
				if (sscanf_s(szTerm1, "%i", &iValue2) < 1)
				{
					ul ulSym = FindSymbol(szTerm1, SymType::SYM_VAR);
					if (ulSym == -1)
					{
						ulSym = FindSymbol(szTerm1, SymType::SYM_FUNC);
					}
					if (ulSym == -1)
					{
						fprintf_s(stderr, "Invalid value %s at %s, line %lu\n", szTerm1, InstructionSet::szInstrucs[id], ulLine);
						exit(1);
					}
					SymLink link;
					link.m_pInst = (MemInstruction<0>*)pByte;
					link.m_argNum = 0;
					if (InstructionSet::instructionSet[id].m_order == InstructionSet::Order::ORD_REV)
						link.m_bIsReversed = true;
					vecSymbols[ulSym].m_vecLinks.push_back(link);
				}
			}

			if (*szTerm2 == 'r')
			{
				int iRegNum = 0;
				if (sscanf_s(szTerm2 + 1, "%i", &iRegNum) < 1)
				{
					goto LABEL_CHECK2;
				}
				else
				{
					flags.regUse |= 2;
					flags.reg1 = iRegNum;
					if (iRegNum > 7)
					{
						fprintf_s(stderr, "Cannot use a register greater than r7 @ line %lu.\n", ulLine);
						exit(1);
					}
				}
			}
			else
			{
			LABEL_CHECK2:
				if (sscanf_s(szTerm2, "%i", &iValue2) < 1)
				{
					ul ulSym = FindSymbol(szTerm2, SymType::SYM_VAR);
					if (ulSym == -1)
					{
						ulSym = FindSymbol(szTerm2, SymType::SYM_FUNC);
					}
					if (ulSym == -1)
					{
						fprintf_s(stderr, "Invalid value %s at %s, line %lu\n", szTerm2, InstructionSet::szInstrucs[id], ulLine);
						exit(1);
					}
					SymLink link;
					link.m_pInst = (MemInstruction<0>*)pByte;
					link.m_argNum = 1;
					if (InstructionSet::instructionSet[id].m_order == InstructionSet::Order::ORD_REV)
						link.m_bIsReversed = true;
					vecSymbols[ulSym].m_vecLinks.push_back(link);
				}
			}

			if (pLastInst)
			{
				//printf_s("NEXT %X\n", (int)pByte - (int)pLastInst);
				pLastInst->SetNext((int)pByte - (int)pLastInst);
			}
			//printf_s("%i %i %x\n", iValue1, iValue2, pByte - arMemory);
			
			pLastInst = (MemInstruction<0>*)MakeInstruction<2>(pByte, id, flags, iValue1, iValue2);

			//DebugPrint("%s %X, iValue1: %i iValue2: %i drf: %i %i reg: %X %i %i\n", InstructionSet::szInstrucs[id], pByte - arMemory, iValue1, iValue2, flags.dr0, flags.dr1, flags.regUse, flags.reg0, flags.reg1);

			return true;
		}
		default:
			return false;
		}
	}

	void Compile(char* szInFile, char* szOutFile)
	{
		ulLine = 1;
		srand(time(0));
		memset(arMemory, 0, sizeof(arMemory));
		memset(arUsed, 0, sizeof(arUsed));
		ul ulSourceSize = 0;
		char* szSource = GetFileContents(szInFile, ulSourceSize);
		if (!szSource)
		{
			fprintf_s(stderr, "Input file '%s' is not a file.\n", szInFile);
			exit(1);
		}
		byte* pStart = arMemory + (rand() % sizeof(arMemory));

		char* line = strtok(szSource, "\n");
		for (; line != 0; line = strtok(NULL, "\n"), ++ulLine)
		{
			printf_s("%i\t", ulLine);
			if (WriteInstruction(pStart, line))
				pStart = arMemory + FindNewLocation(sizeof(MemInstruction<2>));
		}

		ul ulOffMain = 0x2000;
		for (int i = 0; i < vecSymbols.size(); ++i)
		{
			Symbol& sym = vecSymbols[i];
			std::vector<SymLink>& vecLink = sym.m_vecLinks;
			size_t stSize = vecLink.size();
			for (int j = 0; j < stSize; ++j)
			{
				if (vecLink[j].m_bIsReversed)
					vecLink[j].m_pInst->args[vecLink[j].m_argNum == 1 ? 0 : 1] = sym.m_ulPosition;
				else
					vecLink[j].m_pInst->args[vecLink[j].m_argNum] = sym.m_ulPosition;
			}

			if (sym.m_type == SymType::SYM_FUNC
				&& !strcmp("main", sym.m_szName))
				ulOffMain = sym.m_ulPosition;
		}
		
		std::ofstream outFile(szOutFile, std::ios::binary);
		for (int i = 0; i < sizeof(arMemory); ++i)
		{
			if (arUsed[i])
				outFile << (byte)arMemory[i];
			else
				outFile << (byte)(rand() % 256);
		}

		size_t stIATSize = vecIAT.size();
		for (int i = 0; i < stIATSize; ++i)
		{
			outFile << (byte)vecIAT[i];

		}

		DebugPrint("Entry: %X\n", ulOffMain);
		outFile << *((byte*)&ulOffMain) << *((byte*)&ulOffMain + 1) << *((byte*)&ulOffMain + 2) << *((byte*)&ulOffMain + 3);
	}
}
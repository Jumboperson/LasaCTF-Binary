#pragma once
#include <vector>
#include "stdafx.h"
#include <fstream>

// This should probably be better organized...

namespace VxAsm
{
	byte arMemory[0x4000];
	byte arInstSet[0x6C];
	ul ulEntry;

	struct VxCPU;

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

		void GetDeref(byte& adr0, byte& adr1)
		{
			adr0 = m_flags.drf0;
			adr1 = m_flags.drf1;
		}

		void SetReg(byte adr0, byte adr1)
		{
			m_flags.reg0 = adr0;
			m_flags.reg1 = adr1;
		}

		void GetReg(byte& reg0, byte& reg1)
		{
			reg0 = m_flags.reg0;
			reg1 = m_flags.reg1;
		}

		void SetUse(byte use)
		{
			m_flags.flUse = use;
		}

		byte GetUse()
		{
			return m_flags.flUse;
		}

		int GetNext()
		{
			int nxt = 0;
			nxt = *(int*)m_flags._next;
			((byte*)&nxt)[3] = 0;
			if (m_flags.sf)
			{
				((byte*)&nxt)[3] = 0xFF;
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
		byte __buff;
	};

	struct VxCPU
	{
		VxCPU()
		{
			stack = new int[0x100];
			r7 = stack + (0x100);
		}
		int* stack;
		int r0 = 0;
		int r1 = 0;
		int r2 = 0;
		int r3 = 0;
		int r4 = 0;
		int r5 = 0;
		int r6 = 0;
		int* r7;
		struct
		{
			byte zf : 1;
			byte pf : 1;
			byte nf : 1;
			byte of : 1;
		} flags;

		int ip;

		void __forceinline push(int iVal)
		{
			*--r7 = iVal;
		}

		int __forceinline pop()
		{
			return *r7++;
		}

		bool ExecuteInstruction();
	};

	class ThreadManager
	{
	public:
		ThreadManager()
		{ }

		void InitThread(ul ulOffsetInstr, ul ulPriority)
		{
			OBFU_ONE();
			m_vecThreads.push_back(ThreadTracker());
			OBFU_ONE();
			ThreadTracker& thread = m_vecThreads[m_vecThreads.size() - 1];
			thread.ulPriority = ulPriority;
			thread.m_cpu.ip = ulOffsetInstr;
			thread.m_cpu.push(-1);
		}

		void ForcePush(ul ulThread, int iVal)
		{
			OBFU_ONE();
			m_vecThreads[ulThread].m_cpu.push(iVal);
		}

		void Tick()
		{
			CheckDebug();
			size_t stNumThreads = m_vecThreads.size();
			if (stNumThreads == 0)
				return;
			ThreadTracker* pBest = &m_vecThreads[0];
			for (int i = 0; i < stNumThreads; ++i)
			{
				OBFU_ONE();
				m_vecThreads[i].ulCurrentLevel += m_vecThreads[i].ulPriority;
				if (m_vecThreads[i].ulCurrentLevel > pBest->ulCurrentLevel)
				{
					OBFU_FOUR();
					pBest = &m_vecThreads[i];
				}
				CheckDebug();
			}
			if (!pBest->m_cpu.ExecuteInstruction())
			{
				OBFU_ONE();
				CheckDebug();
				pBest->ulPriority = 0;
			}
			pBest->ulCurrentLevel = 0;
		}

		bool GetActive()
		{
			size_t stSize = m_vecThreads.size();
			for (int i = 0; i < stSize; ++i)
			{
				// If any thread still exists
				if (m_vecThreads[i].ulPriority)
					return true;
			}
			return false;
		}
	protected:
		struct ThreadTracker
		{
			VxCPU m_cpu = VxCPU();
			ul ulPriority;
			ul ulCurrentLevel = 0;
		};
		std::vector<ThreadTracker> m_vecThreads = std::vector<ThreadTracker>();
	};

	ThreadManager* g_pManager;

	struct Import
	{
		Import(char* szMod, char* szFunc, int iParams)
			: m_szModule(szMod), m_szFunc(szFunc), m_iTest(iParams)
		{ }
		char* m_szModule;
		char* m_szFunc;
		union
		{
			struct
			{
				word m_iNumParams;
				word m_callType;
			};
			int m_iTest;
		};
		Import* m_pNext = 0;
	};

	Import* pRootImport;

#define GetInstr(x) ((Instruction*)arInstSet)[x]

	typedef bool(__fastcall*InstructionHandler)(VxCPU*, byte*);

	namespace InstructionHandling
	{
#define GetRegValue(regNum) (int)(((&pThis->r0)[regNum] > sizeof(arMemory)) ? *(int*)((int)pThis->r7 + ((&pThis->r0)[regNum] - (int)pThis->r7)) : *(int*)&arMemory[(&pThis->r0)[regNum]])
		bool __fastcall MulHandler(VxCPU* pThis, byte* pInst)
		{
			// This is kinda disgusting, the ID/copy paste following this comment is insane.
#define Id 0
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				if (!drf0)
				{
					OBFU_ONE();
					if (use & 2)
					{
						(&pThis->r0)[reg0] *= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] *= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					OBFU_THR();
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) *= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) *= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				OBFU_THR();
				if (!drf0)
				{
					return true;
				}
				else
				{
					if (use & 2)
					{
						OBFU_ONE();
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] *= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] *= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall DivHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 1
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				OBFU_THR();
				if (!drf0)
				{
					if (use & 2)
					{
						(&pThis->r0)[reg0] /= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] /= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					OBFU_FOUR();
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) /= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) /= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				OBFU_THR();
				if (!drf0)
				{
					return true;
				}
				else
				{
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] /= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						OBFU_ONE();
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] /= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall AddHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 2
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			OBFU_THR();
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				OBFU_ONE();
				if (!drf0)
				{
					OBFU_THR();
					if (use & 2)
					{
						(&pThis->r0)[reg0] += (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] += bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					OBFU_ONE();
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) += drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						OBFU_ONE();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) += bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				OBFU_THR();
				if (!drf0)
				{
					return true;
				}
				else
				{
					if (use & 2)
					{
						OBFU_THR();
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] += drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] += bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall SubHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 3
			OBFU_ONE();
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				OBFU_FOUR();
				if (!drf0)
				{
					if (use & 2)
					{
						OBFU_ONE();
						(&pThis->r0)[reg0] -= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] -= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					if (use & 2)
					{
						OBFU_THR();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) -= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) -= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				OBFU_ONE();
				if (!drf0)
				{
					OBFU_ONE();
					return true;
				}
				else
				{
					if (use & 2)
					{
						OBFU_THR();
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] -= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] -= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall MovHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 4
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_FOUR();
			pInstr->GetDeref(drf0, drf1);
			OBFU_THR();
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				OBFU_ONE();
				if (!drf0)
				{
					if (use & 2)
					{
						(&pThis->r0)[reg0] = (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						OBFU_ONE();
						(&pThis->r0)[reg0] = bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) = drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						OBFU_ONE();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) = bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				OBFU_THR();
				if (!drf0)
				{
					return true;
				}
				else
				{
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] = drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						OBFU_ONE();
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] = bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall LeaHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 5
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_FOUR();
			pInstr->GetDeref(drf0, drf1);
			OBFU_THR();
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				OBFU_ONE();
				if (!drf0)
				{
					if (use & 2)
					{
						(&pThis->r0)[reg0] = (int)(drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (int)&arMemory[(&pThis->r0)[reg1]]);
					}
					else
					{
						OBFU_ONE();
						(&pThis->r0)[reg0] = (bTemp ? (drf1 ? pInstr->args[0] : (int)&arMemory[pInstr->args[0]]) 
							: (drf1 ? pInstr->args[1] : (int)&arMemory[pInstr->args[1]]));
					}
				}
				else
				{
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) = (int)(drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (int)&arMemory[(&pThis->r0)[reg1]]);
					}
					else
					{
						OBFU_ONE();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) = (int)&(bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) 
							: (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]));
					}
				}
			}
			else
			{
				OBFU_THR();
				if (!drf0)
				{
					return true;
				}
				else
				{
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] = (int)(drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (int)&arMemory[(&pThis->r0)[reg1]]);
					}
					else
					{
						OBFU_ONE();
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] = (int)&(bTemp ? pInstr->args[0] : pInstr->args[1]);
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall ShlHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 6
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_THR();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			OBFU_THR();
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				OBFU_ONE();
				if (!drf0)
				{
					if (use & 2)
					{
						OBFU_ONE();
						(&pThis->r0)[reg0] <<= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] <<= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) <<= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) <<= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				OBFU_ONE();
				if (!drf0)
				{
					OBFU_ONE();
					return true;
				}
				else
				{
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] <<= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] <<= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall ShrHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 7
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				if (!drf0)
				{
					OBFU_ONE();
					if (use & 2)
					{
						(&pThis->r0)[reg0] >>= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] >>= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					if (use & 2)
					{
						OBFU_ONE();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) >>= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) >>= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				if (!drf0)
				{
					return true;
				}
				else
				{
					OBFU_FOUR();
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] >>= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] >>= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall XorHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 8
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			OBFU_ONE();
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				if (!drf0)
				{
					OBFU_THR();
					if (use & 2)
					{
						(&pThis->r0)[reg0] ^= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] ^= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) ^= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						OBFU_ONE();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) ^= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				if (!drf0)
				{
					OBFU_FOUR();
					return true;
				}
				else
				{
					OBFU_ONE();
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] ^= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] ^= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall OrHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 9
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			OBFU_FOUR();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			OBFU_ONE();
			byte use = pInstr->GetUse();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				if (!drf0)
				{
					OBFU_THR();
					if (use & 2)
					{
						(&pThis->r0)[reg0] |= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] |= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					OBFU_ONE();
					if (use & 2)
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) |= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) |= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				if (!drf0)
				{
					return true;
				}
				else
				{
					OBFU_ONE();
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] |= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] |= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall AndHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 10
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			byte use = pInstr->GetUse();
			OBFU_ONE();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				if (!drf0)
				{
					OBFU_THR();
					if (use & 2)
					{
						(&pThis->r0)[reg0] &= (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						(&pThis->r0)[reg0] &= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
				else
				{
					if (use & 2)
					{
						OBFU_ONE();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) &= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) &= bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]);
					}
				}
			}
			else
			{
				if (!drf0)
				{
					return true;
				}
				else
				{
					OBFU_FOUR();
					OBFU_ONE();
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] &= drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1];
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] &= bTemp ? pInstr->args[0] : pInstr->args[1];
					}
				}
			}
			return true;
#undef Id
		}

		bool __fastcall CmpHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 11
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			OBFU_THR();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_ONE();
			pInstr->GetDeref(drf0, drf1);
			int iRes = 0;

			byte use = pInstr->GetUse();
			OBFU_ONE();
			byte bTemp = GetInstr(Id).m_order;

			if (use & 1)
			{
				if (!drf0)
				{
					OBFU_THR();
					if (use & 2)
					{
						iRes = ((&pThis->r0)[reg0]) - ((drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]));
					}
					else
					{
						iRes = ((&pThis->r0)[reg0]) - (bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]));
					}
				}
				else
				{
					if (use & 2)
					{
						OBFU_FOUR();
						iRes = ((reg0 != 7 ? GetRegValue(reg0) : *pThis->r7)) - (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						iRes = ((reg0 != 7 ? GetRegValue(reg0) : *pThis->r7)) - (bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1]));
					}
				}
			}
			else
			{
				if (!drf0)
				{
					return true;
				}
				else
				{
					OBFU_ONE();
					OBFU_FOUR();
					if (use & 2)
					{
						iRes = (*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]]) - (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]);
					}
					else
					{
						iRes = (*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]]) - (bTemp ? pInstr->args[0] : pInstr->args[1]);
					}
				}
			}

			if (!iRes)
			{
				pThis->flags.zf = 1;
				pThis->flags.pf = 0;
				pThis->flags.nf = 0;
				return true;
			}
			if (iRes > 0)
			{
				pThis->flags.zf = 0;
				pThis->flags.pf = 1;
				pThis->flags.nf = 0;
				return true;
			}
			if (iRes < 0)
			{
				pThis->flags.zf = 0;
				pThis->flags.pf = 0;
				pThis->flags.nf = 1;
				return true;
			}
#undef Id
		}

		bool __fastcall JmpHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_TWO();
			OBFU_FOUR();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			pThis->ip = pInstr->GetUse() & 1 ? (drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7): (&pThis->r0)[reg0]) : (drf0 ? arMemory[pInstr->args[0]] : pInstr->args[0]);
			return false;
		}

		bool __fastcall JgHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_TWO();
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			if (pThis->flags.pf)
			{
				pThis->ip = pInstr->GetUse() & 1 ? (drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) : (&pThis->r0)[reg0]) : (drf0 ? arMemory[pInstr->args[0]] : pInstr->args[0]);
				return false;
			}
			else
			{
				return true;
			}
		}

		bool __fastcall JlHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_TWO();
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			if (pThis->flags.nf)
			{
				pThis->ip = pInstr->GetUse() & 1 ? (drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) : (&pThis->r0)[reg0]) : (drf0 ? arMemory[pInstr->args[0]] : pInstr->args[0]);
				return false;
			}
			else
			{
				return true;
			}
		}

		bool __fastcall JleHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_TWO();
			OBFU_FOUR();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			if (pThis->flags.nf || pThis->flags.zf)
			{
				pThis->ip = pInstr->GetUse() & 1 ? (drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) : (&pThis->r0)[reg0]) : (drf0 ? arMemory[pInstr->args[0]] : pInstr->args[0]);
				return false;
			}
			else
			{
				return true;
			}
		}

		bool __fastcall JgeHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_TWO();
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			if (pThis->flags.pf || pThis->flags.zf)
			{
				pThis->ip = pInstr->GetUse() & 1 ? (drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) : (&pThis->r0)[reg0]) : (drf0 ? arMemory[pInstr->args[0]] : pInstr->args[0]);
				return false;
			}
			else
			{
				return true;
			}
			return false;
		}

		bool __fastcall JzHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_TWO();
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			pInstr->GetDeref(drf0, drf1);
			if (pThis->flags.zf)
			{
				pThis->ip = pInstr->GetUse() & 1 ? (drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) : (&pThis->r0)[reg0]) : (drf0 ? arMemory[pInstr->args[0]] : pInstr->args[0]);
				return false;
			}
			else
			{
				return true;
			}
			return false;
		}

		bool __fastcall JnzHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_TWO();
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_FOUR();
			pInstr->GetDeref(drf0, drf1);
			if (!pThis->flags.zf)
			{
				pThis->ip = pInstr->GetUse() & 1 ? (drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) : (&pThis->r0)[reg0]) : (drf0 ? arMemory[pInstr->args[0]] : pInstr->args[0]);
				return false;
			}
			else
			{
				return true;
			}
			return false;
		}

		bool __fastcall PushHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_FOUR();
			pInstr->GetDeref(drf0, drf1);
			OBFU_THR();
			if (pInstr->GetUse() & 1)
				pThis->push(drf0 ? (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) : (&pThis->r0)[reg0]);
			else
				pThis->push(drf0 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]);
			return true;
		}

		bool __fastcall PopHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_ONE();
			OBFU_ONE();
			pInstr->GetDeref(drf0, drf1);
			if (pInstr->GetUse() | 1)
			{
				OBFU_THR();
				if (drf0)
					(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) = pThis->pop();
				else
					(&pThis->r0)[reg0] = pThis->pop();
			}
			else
			{
				OBFU_ONE();
				if (drf0)
					*(int*)&arMemory[pInstr->args[0]] = pThis->pop();
			}
			return true;
		}

		bool __fastcall CallHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_THR();
			OBFU_ONE();
			pInstr->GetDeref(drf0, drf1);

			OBFU_THR();
			pThis->push(pInstr->GetNext() + pThis->ip);
			if (pInstr->GetUse() & 1)
			{
				OBFU_FOUR();
				if (drf0)
				{
					pThis->ip = (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7);
				}
				else
				{
					pThis->ip = (&pThis->r0)[reg0];
				}
			}
			else
			{
				OBFU_THR();
				if (drf0)
				{
					pThis->ip = (reg0 != 7 ? GetRegValue(reg0) : *pThis->r7);
				}
				else
				{
					pThis->ip = pInstr->args[0];
				}
			}
			return false;
		}

		bool __fastcall NatvHandler(VxCPU* pThis, byte* pInst)
		{
			//puts("fuk");
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			OBFU_THR();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_ONE();
			pInstr->GetDeref(drf0, drf1);
			Import* pNextImport = pRootImport;
			Import* pTrain = pNextImport;
			int iNumImp = 0;
			OBFU_THR();
			while (iNumImp < pInstr->args[0] && pTrain->m_pNext)
			{
				pNextImport = pTrain->m_pNext;
				pTrain = pTrain->m_pNext;
				++iNumImp;
			}
			OBFU_FOUR();
			if (iNumImp == pInstr->args[0])
			{
				int iTemp = 0;
				OBFU_ONE();
				void* pAddr = GetProcAddress
					(
						strlen(pNextImport->m_szModule) != 1 ? LoadLibraryA(pNextImport->m_szModule) : GetModuleHandleA(0),
						pNextImport->m_szFunc
					);
				OBFU_ONE();
				int i = 0;
				int iEcx = 0;
				int iEdx = 0;
				for (; i < pNextImport->m_iNumParams; ++i)
				{
					OBFU_ONE();
					iTemp = *(pThis->r7 + (pNextImport->m_iNumParams - i - 1));
					if (pNextImport->m_callType > 1)
					{
						OBFU_TWO();
						if (i == 0)
							iEcx = iTemp;
						if (i == 1)
							iEdx = iTemp;
						if (i > 1)
						{
							__asm
							{
								push[iTemp]
							}
						}
					}
					else
					{
						OBFU_THR();
						__asm
						{
							push[iTemp]
						}
					}
				}
				pThis->r7 += i;
				i *= 4;
				OBFU_FOUR();
				int iRet = 0;
				if (pNextImport->m_callType > 1)
				{
					__asm
					{
						push ecx
						push edx
						mov ecx, [iEcx]
						mov edx, [iEdx]
					}
				}
				__asm
				{
					call pAddr
					mov dword ptr ds : [iRet], eax;
				}
				if (pNextImport->m_callType == 0)
				{
					__asm
					{
						add esp, [i]
					}
				}
				if (pNextImport->m_callType > 1)
				{
					__asm
					{
						pop edx
						pop ecx
					}
				}
				pThis->r0 = iRet;
			}

			return true;
		}

		bool __fastcall CrthHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<1>* pInstr = (MemInstruction<1>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			OBFU_ONE();
			pInstr->GetDeref(drf0, drf1);
			if (pInstr->GetUse() | 1)
			{
				OBFU_THR();
				if (drf0)
				{
					g_pManager->InitThread((reg0 != 7 ? GetRegValue(reg0) : *pThis->r7), 1);
				}
				else
				{
					g_pManager->InitThread((&pThis->r0)[reg0], 1);
				}
			}
			else
			{
				OBFU_FOUR();
				if (drf0)
				{
					g_pManager->InitThread(*(int*)&arMemory[pInstr->args[0]], 1);
				}
				else
				{
					g_pManager->InitThread(pInstr->args[0], 1);
				}
			}
			return true;
		}

		bool __fastcall RetHandler(VxCPU* pThis, byte* pInst)
		{
			MemInstruction<0>* pInstr = (MemInstruction<0>*)pInst;
			byte reg0, reg1;
			OBFU_THR();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			OBFU_THR();
			pThis->ip = pThis->pop();
			return false;
		}

		bool __fastcall NopHandler(VxCPU* pThis, byte* pInst)
		{
			OBFU_ONE();
			return true;
		}

		bool __fastcall StkHandler(VxCPU* pThis, byte* pInst)
		{
#define Id 26
			MemInstruction<2>* pInstr = (MemInstruction<2>*)pInst;
			byte reg0, reg1;
			OBFU_ONE();
			pInstr->GetReg(reg0, reg1);
			byte drf0, drf1;
			pInstr->GetDeref(drf0, drf1);
			byte use = pInstr->GetUse();
			OBFU_ONE();
			byte bTemp = GetInstr(Id).m_order;
			if (use & 1)
			{
				if (!drf0)
				{
					OBFU_THR();
					if (use & 2)
					{
						(&pThis->r0)[reg0] = *(int*)(((byte*)pThis->r6) + (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]));
					}
					else
					{
						(&pThis->r0)[reg0] = *(int*)(((byte*)pThis->r6) + (bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1])));
					}
				}
				else
				{
					if (use & 2)
					{
						OBFU_ONE();
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) = *(int*)(((byte*)pThis->r6) + (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]));
					}
					else
					{
						(reg0 != 7 ? GetRegValue(reg0) : *pThis->r7) = *(int*)(((byte*)pThis->r6) + (bTemp ? (drf1 ? *(int*)&arMemory[pInstr->args[0]] : pInstr->args[0]) : (drf1 ? *(int*)&arMemory[pInstr->args[1]] : pInstr->args[1])));
					}
				}
			}
			else
			{
				if (!drf0)
				{
					return true;
				}
				else
				{
					OBFU_FOUR();
					OBFU_ONE();
					if (use & 2)
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] = *(int*)(((byte*)pThis->r7) + (drf1 ? (reg1 != 7 ? GetRegValue(reg1) : *pThis->r7) : (&pThis->r0)[reg1]));
					}
					else
					{
						*(int*)&arMemory[bTemp ? pInstr->args[1] : pInstr->args[0]] = *(int*)(((byte*)pThis->r7) + (bTemp ? pInstr->args[0] : pInstr->args[1]));
					}
				}
			}
			return true;
#undef Id
			return true;
		}

		InstructionHandler arHandlers[27] =
		{
			MulHandler,
			DivHandler,
			AddHandler,
			SubHandler,
			MovHandler,
			LeaHandler,
			ShlHandler,
			ShrHandler,
			XorHandler,
			OrHandler,
			AndHandler,
			CmpHandler,
			JmpHandler,
			JgHandler,
			JlHandler,
			JleHandler,
			JgeHandler,
			JzHandler,
			JnzHandler,
			PushHandler,
			PopHandler,
			CallHandler,
			NatvHandler,
			CrthHandler,
			RetHandler,
			NopHandler,
			StkHandler
		};
	}

	bool VxCPU::ExecuteInstruction()
	{
		if (ip == -1)
		{
			return false;
		}
		OBFU_THR();
		for (int i = 0; i < sizeof(arInstSet) / sizeof(Instruction); ++i)
		{
			byte id = ((MemInstruction<0>*)&arMemory[ip])->m_id;
			byte instId = ((Instruction*)arInstSet)[i].m_bId;
			if (id == instId)
			{
				OBFU_ONE();
				int iValue = 0;
				__try
				{
					__asm
					{
						_emit 0x2d
					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					__asm
					{
						mov [iValue], 1
					}
				}
				if (!iValue)
					puts((char*)0);
				if (InstructionHandling::arHandlers[i](this, &arMemory[ip]))
				{
					OBFU_THR();
					int iNext = ((MemInstruction<0>*)&arMemory[ip])->GetNext();
					ip += iNext;
				}
				break;
			}
		}
		return true;
	}

	XorS(bin, "Vx.bin");
	XorS(inst, "Vx.inst");
	XorS(binErr, "bin file missing.");
	XorS(instErr, "inst file missing.");

	void BeginExecution()
	{
		SetupRuntime();
		OBFU_THR();
		std::ifstream inBin(bin.ToggleEncryption(), std::ios::in | std::ios::binary);
		bin.ToggleEncryption();
		OBFU_ONE();
		std::ifstream inInst(inst.ToggleEncryption(), std::ios::in | std::ios::binary);
		inst.ToggleEncryption();
		OBFU_FOUR();
		if (inBin.is_open())
		{
			OBFU_ONE();
			inBin.seekg(0, std::ios::end);
			ul ulSize = inBin.tellg();
			OBFU_ONE();
			inBin.seekg(0, std::ios::beg);
			inBin.read((char*)arMemory, sizeof(arMemory));
			char* pFooter = new char[ulSize - sizeof(arMemory)];
			inBin.read(pFooter, ulSize - sizeof(arMemory));
			OBFU_THR();
			Import* pNextImport = pRootImport;
			Import* pBase = pRootImport;
			bool bFirst = true;
			int iNumIter = 0;
			while (ulSize - sizeof(arMemory) - 4 > iNumIter)
			{
				char* szModule = &pFooter[iNumIter + 1];
				iNumIter += strlen(szModule) + 1;
				OBFU_FOUR();
				char* szFunc = &pFooter[iNumIter + 1];
				iNumIter += strlen(szFunc) + 1;
				int iParams = *(int*)&pFooter[iNumIter + 1];
				OBFU_ONE();
				// 0, num, ff
				iNumIter += 6;
				pNextImport = new Import(szModule, szFunc, iParams);
				if (bFirst)
				{
					bFirst = false;
					pRootImport = pNextImport;
					pBase = pRootImport;
				}
				OBFU_TWO();
				pBase->m_pNext = pNextImport;
				pBase = pBase->m_pNext;
			}
			ulEntry = *(ul*)(pFooter + ulSize - sizeof(arMemory) - 4);
			inBin.close();
		}
		else
		{
			OBFU_FOUR();
			puts(binErr.ToggleEncryption());
			exit(1);
		}
		OBFU_TWO();
		if (inInst.is_open())
		{
			inInst.seekg(0, std::ios::end);
			OBFU_ONE();
			ul ulSize = inInst.tellg();
			inInst.seekg(0, std::ios::beg);
			OBFU_THR();
			inInst.read((char*)arInstSet, ulSize);
			inBin.close();
		}
		else
		{
			puts(instErr.ToggleEncryption());
			OBFU_ONE();
			exit(1);
		}
		for (int i = 0; i < sizeof(arInstSet) / sizeof(Instruction); ++i)
		{
			*(int*)&GetInstr(i).m_order -= 0x30;
		}

		g_pManager = new ThreadManager();
		g_pManager->InitThread(ulEntry, 2);
		
#if VX_VER == 1
		for (int i = 0; i < 9; ++i)
		{
			OBFU_FOUR();
			int iFin = *(int*)g_args[1 + i];
			g_pManager->ForcePush(0, iFin);
		}
#endif

		OBFU_ONE();
		while (g_pManager->GetActive())
		{
			OBFU_THR();
			g_pManager->Tick();
		}
		
		JumpOut();

		__asm
		{
			mov esp, ebp
			pop ebp
			add esp, 0x100
			ret
		}
	}
}

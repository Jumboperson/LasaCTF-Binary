// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <fstream>
#include <vector>
#include <time.h>
#include "CustomTypes.h"

#define Seed ((__TIME__[7] - '0') * 1  + (__TIME__[6] - '0') * 10  + \
              (__TIME__[4] - '0') * 60   + (__TIME__[3] - '0') * 600 + \
              (__TIME__[1] - '0') * 3600 + (__TIME__[0] - '0') * 36000)

extern void DeleteString(char* szTerm);

class SSValue
{
private:
	bool StrContains(char* szStr, char ch)
	{
		for (int i = 0; i < strlen(szStr); ++i)
		{
			if (szStr[i] == ch)
				return true;
		}
		return false;
	}
public:
	SSValue(char* szTerm, bool bString = false)
	{
		while (*szTerm == '\t')
			++szTerm;
		char* pLast = szTerm;
		char* pV = szTerm;
		m_bStr = bString;
		for (; *pV && *pV != '\r' && *pV != '\n'; ++pV)
		{
			if (*pV == ' ' || *pV == '\t' || (bString ? false : (*pV == ']' || *pV == ',')))
			{
				size_t stSize = pV - pLast + 1;
				char* szNew = new char[stSize];
				memset(szNew, 0, stSize);
				memcpy(szNew, pLast, stSize - 1);
				m_vecTerms.push_back(szNew);
				pLast = pV;
				++pLast;
				while (*pV == ' ' || *pV == '\t' || (bString ? false : (*pV == ']' || *pV == ',')))
					++pV;
				while (*pLast == ' ' || *pLast == '\t' || (bString ? false : (*pLast == ']' || *pLast == ',')))
					++pLast;
			}
		}
		size_t stSize = pV - pLast + 1;
		char* szNew = new char[stSize];
		memset(szNew, 0, stSize);
		memcpy(szNew, pLast, stSize - 1);
		m_vecTerms.push_back(szNew);
		pLast = pV;

		m_szTerm = new char[pV - szTerm + 1];
		memset(m_szTerm, 0, pV - szTerm + 1);
		memcpy(m_szTerm, szTerm, pV - szTerm);
	}

	~SSValue()
	{
		for (int i = 0; i < m_vecTerms.size(); ++i)
		{
			if (!m_bStr)
				DeleteString(m_vecTerms[i]);
		}
		delete[] m_szTerm;
	}

	size_t GetSize()
	{
		return m_vecTerms.size();
	}

	char* GetTerm(size_t stIndex)
	{
		if (stIndex < m_vecTerms.size())
		{
			return m_vecTerms[stIndex];
		}
		return 0;
	}

	char* GetStringPast(char chSep)
	{
		char* pRet = m_szTerm;
		for (; *pRet != chSep && *pRet != 0; ++pRet);
		if (*pRet == 0)
			return 0;
		++pRet;
		return pRet;
	}

	char* CombineTerms(int iStartTerm, int iEndTerm)
	{
		size_t stTotalLength = 0;
		for (int i = iStartTerm; i <= iEndTerm; ++i)
		{
			stTotalLength += strlen(GetTerm(i));
		}
		char* szNew = new char[stTotalLength];
		memset(szNew, 0, stTotalLength);
		char* pBuff = szNew;
		for (int i = iStartTerm; i <= iEndTerm; ++i)
		{
			char* pStr = GetTerm(i);
			size_t stAdd = strlen(pStr);
			memcpy(pBuff, pStr, stAdd);
			if (i != iEndTerm)
				pBuff[stAdd] = ' ';
			pBuff += stAdd + 1;
		}
		return szNew;
	}

protected:
	std::vector<char*> m_vecTerms = std::vector<char*>();
	char* m_szTerm;
	bool m_bStr = false;
private:

};

// TODO: reference additional headers your program requires here

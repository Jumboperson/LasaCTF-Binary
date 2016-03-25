// nested_dump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <fstream>

typedef unsigned char byte;
typedef unsigned __int32 dword;
typedef unsigned __int64 qword;
typedef unsigned long ul;

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

byte* GetFileContents(char* szFilename, size_t& ulSize)
{
	std::ifstream in;
	byte* pContents;
	ulSize = 0;
	pContents = NULL;
	in.open(szFilename, std::ios::in | std::ios::binary);
	if (in.is_open())
	{
		in.seekg(0, std::ios::end);
		ulSize = in.tellg();
		in.seekg(0, std::ios::beg);
		pContents = new byte[ulSize];
		in.read((char*)pContents, ulSize);
		in.close();
	}
	return pContents;
}

enum FileType : dword
{
	FIL_PE,
	FIL_ELF,
	FIL_MACH,
	FIL_END
};

class pe_file
{
public:
	pe_file(byte* pContent, size_t ulSize);
	IMAGE_SECTION_HEADER* GetFirstSection();
	IMAGE_SECTION_HEADER* GetSection(int i);
	dword GetNumSections();
	IMAGE_SECTION_HEADER* CreateSection(char* szName, size_t ulSize);
	bool LoadHeader();
	void* GetFileOffset(size_t ulOffset);
	size_t CalcOffset(byte* pLoc);
	void Export(char* szFileName);
	size_t GetCurrentEntry();
	void SetEntry(size_t ulFileOffset);
	size_t GetImageBase();
	size_t GetSizeDelta();
	bool GetIs64();
	bool GetIsLoaded();

protected:

private:
	std::vector<byte> m_bContent;
	ul m_ulSizeDelta;
	IMAGE_DOS_HEADER* m_dosHeader;
	bool m_b64bit = false;
	bool m_bLoaded = false;
	union
	{
		IMAGE_NT_HEADERS32* m_ntHeader;
		IMAGE_NT_HEADERS64* m_nt64Header;
	};
	IMAGE_SECTION_HEADER* m_sections;
};

pe_file::pe_file(byte* pContent, size_t ulSize)
{
	m_bContent.insert(m_bContent.end(), pContent, pContent + ulSize);
	m_ulSizeDelta = ulSize;
	m_bLoaded = LoadHeader();
	//puts("issue");
}

size_t RoundAlignment(int iValue, size_t ulRound)
{
	return ulRound * ((ulRound + iValue - 1) / ulRound);
}

IMAGE_SECTION_HEADER* pe_file::GetFirstSection()
{
	if (m_b64bit)
		return (IMAGE_SECTION_HEADER*)((size_t)m_nt64Header + sizeof(IMAGE_NT_HEADERS64));
	return (IMAGE_SECTION_HEADER*)((size_t)m_ntHeader + sizeof(IMAGE_NT_HEADERS32));
}

IMAGE_SECTION_HEADER* pe_file::GetSection(int i)
{
	if (m_b64bit)
		return (IMAGE_SECTION_HEADER*)((size_t)m_nt64Header + sizeof(IMAGE_NT_HEADERS64) + (i * sizeof(IMAGE_SECTION_HEADER)));
	return (IMAGE_SECTION_HEADER*)((size_t)m_ntHeader + sizeof(IMAGE_NT_HEADERS32) + (i * sizeof(IMAGE_SECTION_HEADER)));
}

dword pe_file::GetNumSections()
{
	if (m_b64bit)
		return m_nt64Header->FileHeader.NumberOfSections;
	return m_ntHeader->FileHeader.NumberOfSections;
}

bool pe_file::LoadHeader()
{
	bool bValid = false;
	m_dosHeader = (IMAGE_DOS_HEADER*)&m_bContent[0];

	if (m_dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		m_ntHeader = (IMAGE_NT_HEADERS32*)((size_t)&(m_bContent[m_dosHeader->e_lfanew]));
		m_nt64Header = (IMAGE_NT_HEADERS64*)((size_t)&(m_bContent[m_dosHeader->e_lfanew]));

		switch (m_ntHeader->OptionalHeader.Magic)
		{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			m_b64bit = false;
			break;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			m_b64bit = true;
			break;
		default:
			return false;
		}

		if (m_ntHeader->Signature == IMAGE_NT_SIGNATURE)
		{
			m_sections = GetFirstSection();
			bValid = true;
		}
	}
	return bValid;
}

void* pe_file::GetFileOffset(size_t ulOffset)
{
	return &m_bContent[ulOffset];
}

size_t pe_file::CalcOffset(byte* pLoc)
{
	return (size_t)((size_t)pLoc - (size_t)&m_bContent[0]);
}

void pe_file::Export(char * szFileName)
{
	std::ofstream out(szFileName, std::ios::out | std::ios::binary);
	for (int i = 0; i < m_bContent.size(); ++i)
		out << m_bContent[i];
	out.close();
}

size_t pe_file::GetCurrentEntry()
{
	//printf_s("%llX\n", (size_t)m_ntHeader->OptionalHeader.AddressOfEntryPoint + GetImageBase());
	if (m_b64bit)
		return ((size_t)m_nt64Header->OptionalHeader.AddressOfEntryPoint);
	return ((size_t)m_ntHeader->OptionalHeader.AddressOfEntryPoint);
}

void pe_file::SetEntry(size_t ulFileOffset)
{
	if (m_b64bit)
	{
		m_nt64Header->OptionalHeader.AddressOfEntryPoint = ulFileOffset;
		return;
	}

	m_ntHeader->OptionalHeader.AddressOfEntryPoint = ulFileOffset;
}

size_t pe_file::GetImageBase()
{
	if (m_b64bit)
		return (size_t)m_nt64Header->OptionalHeader.ImageBase;
	return (size_t)m_ntHeader->OptionalHeader.ImageBase;
}

size_t pe_file::GetSizeDelta()
{
	return m_ulSizeDelta;
}

bool pe_file::GetIs64()
{
	return m_b64bit;
}

bool pe_file::GetIsLoaded()
{
	return m_bLoaded;
}


int main(int argc, char** argv)
{
	if (argc < 3)
	{
		puts("2 args required babe.\n<in> <out>");
		return 1;
	}

	byte* pDecryptFunction = (byte*)VirtualAlloc(0, 0x100, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	typedef void(__cdecl*DecryptFn_t)(void* pMem, uint32_t uiSize);

	size_t stSize = 0;
	byte* pFile = GetFileContents(argv[1], stSize);
	pe_file file = pe_file(pFile, stSize);

	//byte* pEntry = (byte*)file.GetFileOffset(file.GetCurrentEntry());
	byte* pEntry = 0;
	uint32_t uiTemp = 0x10000;
	for (int i = 0; i < file.GetNumSections(); ++i)
	{
		IMAGE_SECTION_HEADER* pHead = file.GetSection(i);
		if (file.GetCurrentEntry() - pHead->VirtualAddress < uiTemp)
		{
			uiTemp = file.GetCurrentEntry() - pHead->VirtualAddress;
			pEntry = (byte*)file.GetFileOffset(pHead->PointerToRawData) + uiTemp;
		}
		//printf_s("Sec %i: %x\n", i, pHead->VirtualAddress);
	}
	uint32_t uiNextFileSize = *(uint32_t*)(&pEntry[16]);
	uint32_t uiPtr = *(uint32_t*)(&pEntry[21]);
	uint32_t uiRelDecrypt = *(uint32_t*)(&pEntry[26]);

	byte* pDecFunc = &pEntry[30] + uiRelDecrypt;
	printf_s("%x\n%x\n%x\n%x\n%x\n", file.GetCurrentEntry(), *(uint32_t*)pEntry, uiNextFileSize, uiPtr, uiRelDecrypt);
	memcpy(pDecryptFunction, pDecFunc, 0x70);
	DecryptFn_t decrypt_code = (DecryptFn_t)pDecryptFunction;

	IMAGE_SECTION_HEADER* pClosest = file.GetFirstSection();
	for (int i = 0; i < file.GetNumSections(); ++i)
	{
		IMAGE_SECTION_HEADER* pHead = file.GetSection(i);
		uint32_t uiDiff = uiPtr - file.GetImageBase() - (uint32_t)pHead->VirtualAddress;

		if (uiDiff < uiPtr - file.GetImageBase() - (uint32_t)pClosest->VirtualAddress)
		{
			pClosest = pHead;
		}
	}


	byte* pOutFile = (byte*)file.GetFileOffset(pClosest->PointerToRawData) + (uiPtr - file.GetImageBase() - (uint32_t)pClosest->VirtualAddress);

	decrypt_code(pOutFile, uiNextFileSize);
	std::ofstream outFile(argv[2], std::ios::binary);
	for (int i = 0; i < uiNextFileSize; ++i)
	{
		outFile << pOutFile[i];
	}

    return 0;
}


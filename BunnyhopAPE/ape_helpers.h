#include <TlHelp32.h>
#include <winternl.h>

DWORD GetPIDByName(const char* ProcName)
{
	PROCESSENTRY32 pe32;
	HANDLE hSnapshot = NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(hSnapshot, &pe32))
	{
		do{
			if (strcmp(pe32.szExeFile, ProcName) == 0)
				return pe32.th32ProcessID;
		} while (Process32Next(hSnapshot, &pe32));
	}

	if (hSnapshot != INVALID_HANDLE_VALUE)
		CloseHandle(hSnapshot);

	return NULL;
}

DWORD GetModuleHandleExtern(DWORD dwProcessId, LPSTR lpModuleName)
{
	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
	if (!hSnapShot)	return NULL;

	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL bModule = Module32First(hSnapShot, &lpModuleEntry);

	while (bModule)
	{
		if (!strcmp(lpModuleEntry.szModule, lpModuleName))
		{
			CloseHandle(hSnapShot);
			return (DWORD)lpModuleEntry.modBaseAddr;
		}

		bModule = Module32Next(hSnapShot, &lpModuleEntry);
	}

	CloseHandle(hSnapShot);
	return NULL;
}

DWORD FindPatternEx(HANDLE hProc, DWORD base, DWORD len, BYTE* sig, char* mask)
{
	BYTE* buf = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (ReadProcessMemory(hProc, (LPCVOID)base, buf, len, NULL))
	{
		for (DWORD i = 0; i <= (len - strlen(mask)); i++)
		{
			if ((buf[i] == sig[0] && mask[0] == 'x') || (mask[0] == '?'))
			{
				for (int x = 0;; x++)
				{
					if (mask[x] == 'x')
					{
						if (buf[i + x] == sig[x])
							continue;
						else
							break;
					}
					else if (mask[x] == 0x00)
					{
						return (DWORD)(base + i);
					}
				}
			}
		}
	}
	return NULL;
}

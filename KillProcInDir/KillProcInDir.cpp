#include "stdafx.h"

HINSTANCE g_hInstance;
HWND g_hwndParent;

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance=(struct HINSTANCE__ *)hInst;
	return TRUE;
}

bool IsInDir(const char * file, const char * dir)
{
	int filelen = strlen(file), dirlen = strlen(dir);
	if(filelen <= dirlen) return false;
	for(int i = 0; i < dirlen; ++i)
	{
		if(file[i] != dir[i])
			return false;
	}
	return true;	
}

int KILL_PROC_IN_DIR(const char *dirPath)
{
	typedef BOOL (WINAPI *EnumProcessesType)(DWORD* pProcessIds, DWORD cb, DWORD* pBytesReturned);
	typedef DWORD (WINAPI *GetModuleFileNameExType)(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);

	EnumProcessesType EnumProcesses = NULL;
	GetModuleFileNameExType GetModuleFileNameEx = NULL;
	HINSTANCE psapi = NULL;
	psapi = LoadLibraryA("psapi.dll");
	if(psapi)
	{
		EnumProcesses = (EnumProcessesType)GetProcAddress(psapi, "EnumProcesses");
		GetModuleFileNameEx = (GetModuleFileNameExType)GetProcAddress(psapi, "GetModuleFileNameExA");
	}
	else return -1;

	DWORD ids[1024], br, pn;
	if(!EnumProcesses(ids, sizeof(ids), &br)) return -1;
	pn = br / sizeof(DWORD);
	for(unsigned int i = 0; i < pn; ++i)
	{
		if(ids[i] == 0) continue;
		HANDLE hp = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ids[i]);
		if(!hp) continue;
		char procName[1024];
		DWORD length = GetModuleFileNameEx(hp, NULL, procName, 1024);
		if(length > 0 && IsInDir(procName, dirPath))
		{
			CloseHandle(hp);
			hp = OpenProcess(PROCESS_TERMINATE, FALSE, ids[i]);
			if(!TerminateProcess(hp, 0))
			{
				CloseHandle(hp);
				return -1;
			}
		}
		CloseHandle(hp);
	}
	return 0;
}

extern "C" __declspec(dllexport) void KillProcInDir(HWND hwndParent, int string_size, 
	char *variables, stack_t **stacktop)
{
	char parameter[200];
	char temp[10];
	int value;
	g_hwndParent=hwndParent;
	EXDLL_INIT();
	{
		popstring(parameter);
		value=KILL_PROC_IN_DIR(parameter);
		_itoa_s(value, temp, 10);
		setuservariable(INST_R0, temp);
	}
}
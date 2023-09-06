#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

HWND cisco_hwnd = 0;
BOOL __stdcall enum_window_cb(HWND hwnd, LPARAM lParam)
{
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == *((DWORD*)lParam))
	{
		cisco_hwnd = hwnd;
		return 0;
	}
	return 1;
}

HWND find_cisco_hwnd()
{
	auto hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe32{};
	pe32.dwSize = sizeof(PROCESSENTRY32);
	auto hProc = Process32First(hSnap, &pe32);
	while (hProc)
	{
		if (strcmp(pe32.szExeFile, "PacketTracer.exe") == 0)
		{
			CloseHandle(hSnap);
			HWND result = 0;
			EnumWindows(enum_window_cb, (LPARAM)&pe32.th32ProcessID);
			return cisco_hwnd;
		}
		hProc = Process32Next(hSnap, &pe32);
	}
	CloseHandle(hSnap);
	return 0;
}


int main()
{
	printf("[+] Packet tracer no password (Tested on 8.2.1)\n\n");
	
	auto hwnd = find_cisco_hwnd();
	if (!hwnd)
	{
		printf("[-] Failed to find PacketTracer (Run this tool on the main menu of packet tracer)\n");
		system("pause");
		exit(-1);
	}

	DWORD pid = NULL;
	DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
	if (!pid || !tid)
	{
		printf("[-] Failed to get tid(%d) or pid(%d)\n", tid, pid);
		system("pause");
		exit(-1);
	}

	auto hMod = LoadLibraryEx("PacketTracerDll.dll", 0, DONT_RESOLVE_DLL_REFERENCES);
	if (!hMod)
	{
		printf("[-] Failed to get dll (Did you forgot to put it in the same folder as the executable under the name PacketTracerDll.dll)?\n");
		system("pause");
		exit(-1);
	}
	
	auto hook_address = GetProcAddress(hMod, "WindowHook");
	if (!hook_address)
	{
		printf("[-] Dll is missing the WindowHook export\n");
		system("pause");
		exit(-1);
	}

	auto hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)hook_address, hMod, tid);
	PostThreadMessageA(tid, 0, 0, 0);

	printf("[+] Wait for a BEEP then press a key here\n");
	system("pause");

	UnhookWindowsHookEx(hHook);

	printf("[+] It should be patched!\n");
}

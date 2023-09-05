#include <Windows.h>
#include <iostream>

int main()
{
	printf("[+] Packet tracer no password (Tested on 8.2.1)\n\n");
	
	auto hwnd = FindWindowA(0, "PacketTracer");
	if (!hwnd)
	{
		printf("[-] Failed to find PacketTracer\n");
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

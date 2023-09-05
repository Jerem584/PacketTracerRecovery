#include <Windows.h>
#include <iostream>
#include <vector>


extern "C" __declspec(dllexport) int WindowHook(int code, WPARAM wParam, LPARAM lParam) { return CallNextHookEx(NULL, code, wParam, lParam); }

uint8_t* SigScan(const HMODULE module, const std::string& byte_array, int skip = 0) {
	if (!module)
		return nullptr;

	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{};
		const auto start = const_cast<char*>(pattern);
		const auto end = const_cast<char*>(pattern) + std::strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;

				if (*current == '?')
					++current;

				bytes.push_back(-1);
			}
			else {
				bytes.push_back(std::strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	const auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
	const auto nt_headers =
		reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(module) + dos_header->e_lfanew);

	const auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
	const auto pattern_bytes = pattern_to_byte(byte_array.c_str());
	const auto scan_bytes = reinterpret_cast<std::uint8_t*>(module);

	const auto pattern_size = pattern_bytes.size();
	const auto pattern_data = pattern_bytes.data();

	for (auto i = 0ul; i < size_of_image - pattern_size; ++i) {
		bool found = true;

		for (auto j = 0ul; j < pattern_size; ++j) {
			if (scan_bytes[i + j] != pattern_data[j] && pattern_data[j] != -1) {
				found = false;
				break;
			}
		}
		if (found)
			return &scan_bytes[i];
	}

	return nullptr;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		auto jz_addr = SigScan(GetModuleHandle(0), "0F 84 9B 13 00 00"); // 0f 84 9b 13 00 00       jz     addr
		if (!jz_addr)
			exit(-1);

		DWORD old;
		if (!VirtualProtect(jz_addr, 0x8, PAGE_EXECUTE_READWRITE, &old))
			exit(-2);

		*(char*)(jz_addr) = 0xE9;
		*(char*)(jz_addr + 1) = 0x9C;
		*(char*)(jz_addr + 2) = 0x13;
		*(char*)(jz_addr + 3) = 0; // jmp addr


		VirtualProtect(jz_addr, 0x8, old, &old);
		Beep(1000, 1000);

	}
	return TRUE;
}
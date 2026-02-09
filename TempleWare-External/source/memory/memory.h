#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>
#include <cstring>

class Memory {
private:
	std::uintptr_t processId_;
	HANDLE processHandle_;

public:
	Memory(const std::string& processName) noexcept
		: processId_(0), processHandle_(nullptr)
	{
		PROCESSENTRY32 entry = { sizeof(PROCESSENTRY32) };
		auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (snapshot == INVALID_HANDLE_VALUE)
			return;

		if (!Process32First(snapshot, &entry))
		{
			CloseHandle(snapshot);
			return;
		}

		do
		{
			if (_stricmp(entry.szExeFile, processName.c_str()) == 0)
			{
				processId_ = entry.th32ProcessID;
				
				processHandle_ = OpenProcess(PROCESS_ALL_ACCESS, FALSE, static_cast<DWORD>(processId_));
				
				if (processHandle_ == nullptr || processHandle_ == INVALID_HANDLE_VALUE)
				{
					processHandle_ = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, 
						FALSE, static_cast<DWORD>(processId_));
				}
				
				if (processHandle_ != nullptr && processHandle_ != INVALID_HANDLE_VALUE)
					break;
					
				processHandle_ = nullptr;
			}
		} while (Process32Next(snapshot, &entry));

		CloseHandle(snapshot);
	}

	~Memory() noexcept 
	{
		if (processHandle_)
			CloseHandle(processHandle_);
	}

	bool IsValid() const noexcept
	{
		return processHandle_ != nullptr && processHandle_ != INVALID_HANDLE_VALUE;
	}

	std::uintptr_t GetModuleAddress(const std::string& moduleName) const noexcept
	{
		if (!IsValid())
			return 0;

		MODULEENTRY32 entry = { sizeof(MODULEENTRY32) };
		auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, static_cast<DWORD>(processId_));
		std::uintptr_t result = 0;

		if (snapshot == INVALID_HANDLE_VALUE)
			return 0;

		if (!Module32First(snapshot, &entry))
		{
			CloseHandle(snapshot);
			return 0;
		}

		do
		{
			if (_stricmp(entry.szModule, moduleName.c_str()) == 0)
			{
				result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
				break;
			}
		} while (Module32Next(snapshot, &entry));

		CloseHandle(snapshot);
		return result;
	}

	template <typename T>
	T Read(const std::uintptr_t& address) const noexcept 
	{
		T value{};
		if (!IsValid() || address == 0)
			return value;

		SIZE_T bytesRead = 0;
		if (!ReadProcessMemory(processHandle_, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytesRead) ||
			bytesRead != sizeof(T))
		{
			return T{};
		}
		return value;
	}

	template <typename T>
	bool Write(const std::uintptr_t& address, const T& value) const noexcept
	{
		if (!IsValid() || address == 0)
			return false;

		SIZE_T bytesWritten = 0;
		return WriteProcessMemory(processHandle_, reinterpret_cast<LPVOID>(address), &value, sizeof(T), &bytesWritten) &&
			bytesWritten == sizeof(T);
	}
};
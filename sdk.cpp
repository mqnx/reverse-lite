//#include "stdafx.h"
//#include "sdk.h"
//#include "xor.hpp"
//#include "lazyimporter.h"
//#include "memory.h"
//#include <map>
//#include "defs.h"
//#include "globals.h"
//#include "xorstr.hpp"
//#include "weapon.h"
#include "stdafx.h"
#include "sdk.h"
#include "xor.hpp"
#include "lazyimporter.h"
#include "memory.h"
#include <map>
#include "defs.h"
#include "globals.h"
#include "xorstr.hpp"
#include "weapon.h"
#include "mem.h"
#include <fstream>
#pragma comment(lib, "user32.lib")
#define DEBASE(a) ((size_t)a - (size_t)(unsigned long long)GetModuleHandleA(NULL))

uintptr_t dwProcessBase;
uint64_t backup = 0, Online_Loot__GetItemQuantity = 0, stackFix = 0;
NTSTATUS(*NtContinue)(PCONTEXT threadContext, BOOLEAN raiseAlert) = nullptr;

DWORD64 resolveRelativeAddress(DWORD64 instr, DWORD offset, DWORD instrSize) {
	return instr == 0ui64 ? 0ui64 : (instr + instrSize + *(int*)(instr + offset));
}

bool compareByte(const char* pData, const char* bMask, const char* szMask) {
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}

DWORD64 findPattern(DWORD64 dwAddress, DWORD64 dwLen, const char* bMask, const char* szMask) {
	DWORD length = (DWORD)strlen(szMask);
	for (DWORD i = 0; i < dwLen - length; i++)
		if (compareByte((const char*)(dwAddress + i), bMask, szMask))
			return (DWORD64)(dwAddress + i);
	return 0ui64;
}

LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	if (pExceptionInfo && pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
	{
		if (pExceptionInfo->ContextRecord->R11 == 0xDEEDBEEF89898989)
		{
			pExceptionInfo->ContextRecord->R11 = backup;

			if (pExceptionInfo->ContextRecord->Rip > Online_Loot__GetItemQuantity && pExceptionInfo->ContextRecord->Rip < (Online_Loot__GetItemQuantity + 0x1000))
			{
				pExceptionInfo->ContextRecord->Rip = stackFix;
				pExceptionInfo->ContextRecord->Rax = 1;
			}
			NtContinue(pExceptionInfo->ContextRecord, 0);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}


namespace process
{
	HWND hwnd;

	BOOL CALLBACK EnumWindowCallBack(HWND hWnd, LPARAM lParam)
	{
		DWORD dwPid = 0;
		GetWindowThreadProcessId(hWnd, &dwPid);
		if (dwPid == lParam)
		{
			hwnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	HWND get_process_window()
	{
		if (hwnd)
			return hwnd;

		EnumWindows(EnumWindowCallBack, GetCurrentProcessId());

		if (hwnd == NULL)
			Exit();

		return hwnd;
	}
}
namespace radarVars
{
	bool transparent = true;
	bool bEnable2DRadar = true;
	bool bSetRadarSize = true;
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	ImVec2 v2RadarNormalLocation = ImVec2(0, 0);
	ImVec2 v2RadarNormalSize = ImVec2(300, 300);
	ImVec2 v2SetRadarSize;
	ImDrawList* Draw;
}


struct ClientBits
{
	int array[7];
};


namespace g_data
{
	uintptr_t base;
	uintptr_t peb;
	HWND hWind;
	uintptr_t unlocker;
	uintptr_t ddl_loadasset;
	uintptr_t ddl_getrootstate;
	uintptr_t ddl_getdllbuffer;
	uintptr_t ddl_movetoname;
	uintptr_t ddl_setint;
	uintptr_t Dvar_FindVarByName;
	uintptr_t Dvar_SetBoolInternal;
	uintptr_t Dvar_SetInt_Internal;
	uintptr_t Dvar_SetFloat_Internal;
	uintptr_t Camo_Offset_Auto_Test;


	uintptr_t Clantag_auto;

	uintptr_t a_parse;
	uintptr_t ddl_setstring;
	uintptr_t ddl_movetopath;
	uintptr_t ddlgetInth;
	//ClientBits visible_base;
	QWORD current_visible_offset;
	QWORD cached_visible_base;
	QWORD last_visible_offset;
	uintptr_t cached_client_t = 0;

	bool MemCompare(const BYTE* bData, const BYTE* bMask, const char* szMask) {
		for (; *szMask; ++szMask, ++bData, ++bMask) {
			if (*szMask == 'x' && *bData != *bMask) {
				return false;
			}
		}
		return (*szMask == NULL);
	}
	uintptr_t PatternScanEx(uintptr_t start, uintptr_t size, const char* sig, const char* mask)
	{
		BYTE* data = new BYTE[size];
		SIZE_T bytesRead;

		(iat(memcpy).get()(data, (LPVOID)start, size));
		//ReadProcessMemory(hProcess, (LPVOID)start, data, size, &bytesRead);

		for (uintptr_t i = 0; i < size; i++)
		{
			if (MemCompare((const BYTE*)(data + i), (const BYTE*)sig, mask)) {
				return start + i;
			}
		}
		delete[] data;
		return NULL;
	}

	uintptr_t FindOffset(uintptr_t start, uintptr_t size, const char* sig, const char* mask, uintptr_t base_offset, uintptr_t pre_base_offset, uintptr_t rindex, bool addRip = true)
	{
		auto address = PatternScanEx(start, size, sig, mask) + rindex;
		if (!address)
			return 0;
		auto ret = pre_base_offset + *reinterpret_cast<int32_t*>(address + base_offset);

		if (addRip)
		{
			ret = ret + address;
			if (ret)
				return (ret - base);
		}

		return ret;
	}
	address_t find_ida_sig(const char* mod, const char* sig)
	{
		/// Credits: MarkHC, although slightly modified by me and also documented

		static auto pattern_to_byte = [](const char* pattern)
			{
				/// Prerequisites
				auto bytes = std::vector<int>{};
				auto start = const_cast<char*>(pattern);
				auto end = const_cast<char*>(pattern) + strlen(pattern);

				/// Convert signature into corresponding bytes
				for (auto current = start; current < end; ++current)
				{
					/// Is current byte a wildcard? Simply ignore that that byte later
					if (*current == '?')
					{
						++current;

						/// Check if following byte is also a wildcard
						if (*current == '?')
							++current;

						/// Dummy byte
						bytes.push_back(-1);
					}
					else
					{
						/// Convert character to byte on hexadecimal base
						bytes.push_back(strtoul(current, &current, 16));
					}
				}
				return bytes;
			};

		const auto module_handle = GetModuleHandleA(mod);
		if (!module_handle)
			return {};

		/// Get module information to search in the given module
		MODULEINFO module_info;
		GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(module_handle), &module_info, sizeof(MODULEINFO));

		/// The region where we will search for the byte sequence
		const auto image_size = module_info.SizeOfImage;

		/// Check if the image is faulty
		if (!image_size)
			return {};

		/// Convert IDA-Style signature to a byte sequence
		auto pattern_bytes = pattern_to_byte(sig);

		const auto image_bytes = reinterpret_cast<byte*>(module_handle);

		const auto signature_size = pattern_bytes.size();
		const auto signature_bytes = pattern_bytes.data();

		/// Loop through all pages and check the accessable pages
		auto page_information = MEMORY_BASIC_INFORMATION{};
		for (auto current_page = reinterpret_cast<byte*>(module_handle); current_page < reinterpret_cast<byte*>(module_handle + image_size); current_page = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(page_information.BaseAddress) + page_information.RegionSize))
		{
			auto status = VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));

			if (page_information.Protect != PAGE_EXECUTE_READWRITE)
				continue;

			/// Now loop through all bytes and check if the byte sequence matches
			for (auto i = reinterpret_cast<uintptr_t>(page_information.BaseAddress) - reinterpret_cast<uintptr_t>(module_handle); i < page_information.RegionSize; ++i)
			{
				auto byte_sequence_found = true;

				if (i + signature_size == page_information.RegionSize)
				{
					auto status = VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));

					if (page_information.Protect != PAGE_EXECUTE_READ)
						break;
				}

				/// Go through all bytes from the signature and check if it matches
				for (auto j = 0ul; j < signature_size; ++j)
				{
					if (image_bytes[i + j] != signature_bytes[j] /// Bytes don't match
						&& signature_bytes[j] != -1) /// Byte isn't a wildcard either, WHAT THE HECK
					{
						byte_sequence_found = false;
						break;
					}
				}

				/// All good, now return the right address
				if (byte_sequence_found)
					return address_t(uintptr_t(&image_bytes[i]));
			}
		}

		/// Byte sequence wasn't found
		return {};
	}


	void init()
	{
		base = (uintptr_t)(iat(GetModuleHandleA).get()("cod.exe"));


		hWind = process::get_process_window();

		if (globals::streampoof)
		{
			SetWindowDisplayAffinity(hWind, WDA_EXCLUDEFROMCAPTURE);
		}

		peb = __readgsqword(0x60);

		jmp_rbx = find_ida_sig(NULL, xorstr_("FF 23"));
		tpva = find_ida_sig(NULL, xorstr_("E8 ?? ?? ?? ?? 48 8B 9C 24 ?? ?? ?? ?? 8B CD 84 C0")).self_jmp(1);
		R_AddDObjToSceneInternal = find_ida_sig(NULL, xorstr_("E8 ? ? ? ? 48 81 C3 ? ? ? ? 48 83 EE 01 0F 85 ? ? ? ? 48 8B 74 24 ? 48 8B 5C 24 ? 48 8D 7C 24")).self_jmp(1);


		/*	globals::unlockallbase = PatternScanEx(base + 0x5000000, 0x4F00000, xorstr_("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xC8\xC5\xF0\x57\xC9\xC4\xE1\xF3\x2A\xC9\x48\x8B\xCF\xE8\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\xB8\x00\x00\x00\x00\x48\x83\xC4\x20\x5F\xC3"), xorstr_("xxx????x????xxxxxxxxxxxxxxx????xxxx?x????xxxxxx")) - base;
			globals::unlockallbase = globals::unlockallbase + 0x7;
			memcpy((BYTE*)(globals::UnlockBytes), (BYTE*)g_data::base + globals::unlockallbase, 5);
			globals::checkbase = FindOffset(base + 0x2000000, 0x1F00000, xorstr_("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x75\x10\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x84\xC0\x74\x0F"), xorstr_("xxx????x????xxxxxxx????x????xxxx"), 3, 7, 0);
			globals::checkbase = globals::checkbase + -0x8;*/
	}
}
template<typename T> inline auto readMemory(uintptr_t ptr) noexcept -> T {
	if (is_bad_ptr(ptr)) {
		//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
		return {};
	}
	return *reinterpret_cast<T*>(ptr);
}
template<typename T> inline auto writeMemory(uintptr_t ptr, T value) noexcept -> T {
	if (is_bad_ptr(ptr)) {
		//DEBUG_INFO("Attempted to read invalid memory at {:#x}", ptr);
		return {};
	}
	return *reinterpret_cast<T*>(ptr) = value;
}
dvar_s* Dvar_FindVarByName(const char* dvarName)
{
	//[48 83 EC 48 49 8B C8 E8 ?? ?? ?? ?? + 0x7] resolve call.
	return reinterpret_cast<dvar_s * (__fastcall*)(const char* dvarName)>(g_data::base + globals::Dvar_FindVarByName)(dvarName);
}

uintptr_t Dvar_SetBool_Internal(dvar_s* a1, bool a2)
{
	//E8 ? ? ? ? 80 3D ? ? ? ? ? 4C 8D 35 ? ? ? ? 74 43 33 D2 F7 05 ? ? ? ? ? ? ? ? 76 2E
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, bool a2)>(g_data::base + globals::Dvar_SetBoolInternal)(a1, a2);
}

uintptr_t Dvar_SetInt_Internal(dvar_s* a1, unsigned int a2)
{
	//40 53 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 0F B6 41 09 48 8B D9
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, unsigned int a2)>(g_data::base + globals::Dvar_SetInt_Internal)(a1, a2);
}

uintptr_t Dvar_SetBoolByName(const char* dvarName, bool value)
{
	//"48 89 ? ? ? 57 48 81 EC ? ? ? ? 0F B6 ? 48 8B"
	int64(__fastcall * Dvar_SetBoolByName_t)(const char* dvarName, bool value); //48 89 5C 24 ? 57 48 81 EC ? ? ? ? 0F B6 DA
	return reinterpret_cast<decltype(Dvar_SetBoolByName_t)>(globals::Dvar_SetBoolByName)(dvarName, value);
}

uintptr_t Dvar_SetFloat_Internal(dvar_s* a1, float a2)
{
	//E8 ? ? ? ? 45 0F 2E C8 RESOLVE CALL
	return reinterpret_cast<std::ptrdiff_t(__fastcall*)(dvar_s * a1, float a2)>(g_data::base + globals::Dvar_SetFloat_Internal)(a1, a2);
}

uintptr_t Dvar_SetIntByName(const char* dvarname, int value)
{
	uintptr_t(__fastcall * Dvar_SetIntByName_t)(const char* dvarname, int value); //48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B DA 48 8B F9
	return reinterpret_cast<decltype(Dvar_SetIntByName_t)>(globals::Dvar_SetIntByName)(dvarname, value);
}

__int64 Com_DDL_LoadAsset(__int64 file) {
	uintptr_t address = g_data::base + globals::ddl_loadasset;
	return ((__int64 (*)(__int64))address)(file);
}

__int64 DDL_GetRootState(__int64 state, __int64 file) {
	uintptr_t address = g_data::base + globals::ddl_getrootstate;
	return ((__int64 (*)(__int64, __int64))address)(state, file);
}

bool CL_PlayerData_GetDDLBuffer(__int64 context, int controllerindex, int stats_source, unsigned int statsgroup) {
	uintptr_t address = g_data::base + globals::ddl_getdllbuffer;
	return ((bool (*)(__int64, int, int, unsigned int))address)(context, controllerindex, stats_source, statsgroup);
}
bool ParseShit(const char* a, const char** b, const int c, int* d)
{
	uintptr_t address = g_data::base + globals::a_parse;
	return ((bool (*)(const char* a, const char** b, const int c, int* d))address)(a, b, c, d);

}
char DDL_MoveToPath(__int64* fromState, __int64* toState, int depth, const char** path) {
	uintptr_t address = g_data::base + globals::ddl_movetopath;
	return ((char (*)(__int64* fromState, __int64* toState, int depth, const char** path))address)(fromState, toState, depth, path);

}
__int64 DDL_MoveToName(__int64 fstate, __int64 tstate, __int64 path) {
	uintptr_t address = g_data::base + globals::ddl_movetoname;
	return ((__int64 (*)(__int64, __int64, __int64))address)(fstate, tstate, path);
}

char DDL_SetInt(__int64 fstate, __int64 context, unsigned int value) {
	uintptr_t address = g_data::base + globals::ddl_setint;
	return ((char (*)(__int64, __int64, unsigned int))address)(fstate, context, value);
}
int DDL_GetInt(__int64* fstate, __int64* context) {
	uintptr_t address = g_data::base + globals::ddl_getint;
	return ((int (*)(__int64*, __int64*))address)(fstate, context);
}
char DDL_SetString(__int64 fstate, __int64 context, const char* value) {
	uintptr_t address = g_data::base + globals::ddl_setstring;
	return ((char (*)(__int64, __int64, const char*))address)(fstate, context, value);
}
char DDL_SetInt2(__int64* fstate, __int64* context, int value) {
	uintptr_t address = g_data::base + globals::ddl_setint;
	return ((char (*)(__int64*, __int64*, unsigned int))address)(fstate, context, value);
}
namespace sdk
{
	const DWORD nTickTime = 64;//64 ms
	bool bUpdateTick = false;
	std::map<DWORD, velocityInfo_t> velocityMap;




	void enable_uav()
	{


		auto uavptr = *(uint64_t*)(g_data::base + globals::uavbase);
		if (uavptr != 0)
		{
			//*(bool*)(uavptr + auav) = g_Vars->mSettings.b_radar;
			*(int*)(uavptr + 0x41F) = 2;
			//*(int*)(uavptr + uav + 0x5) = 6;
		}


	}
	float valuesRecoilBackup[962][60];
	float valuesSpreadBackup[962][22];
	void no_spread()
	{
		WeaponCompleteDefArr* weapons = (WeaponCompleteDefArr*)(g_data::base + bones::weapon_definitions);
		if (globals::b_spread && in_game)
		{

			for (int count = 0; count < 962; count++)
			{
				if (weapons->weaponCompleteDefArr[count]->weapDef)
				{

					valuesSpreadBackup[count][0] = weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay;
					valuesSpreadBackup[count][1] = weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay;
					valuesSpreadBackup[count][2] = weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay;
					valuesSpreadBackup[count][3] = weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay;
					valuesSpreadBackup[count][4] = weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos;
					valuesSpreadBackup[count][5] = weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount;
					valuesSpreadBackup[count][6] = weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount;
					valuesSpreadBackup[count][7] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed;
					valuesSpreadBackup[count][8] = weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed;
					valuesSpreadBackup[count][9] = weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor;
					valuesSpreadBackup[count][10] = weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor;
					valuesSpreadBackup[count][11] = weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch;
					valuesSpreadBackup[count][12] = weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw;
					valuesSpreadBackup[count][13] = weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch;
					valuesSpreadBackup[count][14] = weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw;
					valuesSpreadBackup[count][15] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime;
					valuesSpreadBackup[count][16] = weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime;
					valuesSpreadBackup[count][17] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin;
					valuesSpreadBackup[count][18] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax;
					valuesSpreadBackup[count][19] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate;
					valuesSpreadBackup[count][20] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd;
					valuesSpreadBackup[count][21] = weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd;
					weapons->weaponCompleteDefArr[22]->weapDef->ballisticInfo.muzzleVelocity;
					// WRITE

					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd = 0.0F;
					weapons->weaponCompleteDefArr[count]->weapDef->ballisticInfo.muzzleVelocity = 0.0f;
				}
			}
		}
		else
		{
			for (int count = 0; count < 962; count++)
			{
				if (weapons->weaponCompleteDefArr[count]->weapDef)
				{
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadDuckedDecay = valuesSpreadBackup[count][0];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipSpreadProneDecay = valuesSpreadBackup[count][1];
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadSprintDecay = valuesSpreadBackup[count][2];
					weapons->weaponCompleteDefArr[count]->weapDef->hipSpreadInAirDecay = valuesSpreadBackup[count][3];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipReticleSidePos = valuesSpreadBackup[count][4];
					weapons->weaponCompleteDefArr[count]->weapDef->fAdsIdleAmount = valuesSpreadBackup[count][5];
					weapons->weaponCompleteDefArr[count]->weapDef->fHipIdleAmount = valuesSpreadBackup[count][6];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleSpeed = valuesSpreadBackup[count][7];
					weapons->weaponCompleteDefArr[count]->weapDef->hipIdleSpeed = valuesSpreadBackup[count][8];
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleCrouchFactor = valuesSpreadBackup[count][9];
					weapons->weaponCompleteDefArr[count]->weapDef->fIdleProneFactor = valuesSpreadBackup[count][10];
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxPitch = valuesSpreadBackup[count][11];
					weapons->weaponCompleteDefArr[count]->weapDef->fGunMaxYaw = valuesSpreadBackup[count][12];
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxPitch = valuesSpreadBackup[count][13];
					weapons->weaponCompleteDefArr[count]->weapDef->fViewMaxYaw = valuesSpreadBackup[count][14];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpStartTime = valuesSpreadBackup[count][15];
					weapons->weaponCompleteDefArr[count]->weapDef->adsIdleLerpTime = valuesSpreadBackup[count][16];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMin = valuesSpreadBackup[count][17];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadMax = valuesSpreadBackup[count][18];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadDecayRate = valuesSpreadBackup[count][19];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadFireAdd = valuesSpreadBackup[count][20];
					weapons->weaponCompleteDefArr[count]->weapDef->slideSpreadTurnAdd = valuesSpreadBackup[count][21];

				}
			}
		}
	}

	void no_recoil()
	{
		unsigned __int64 r12 = get_client_info();
		r12 += player_info::recoil_offset;
		unsigned __int64 rsi = r12 + 0x4;
		DWORD edx = *(unsigned __int64*)(r12 + 0xC);
		DWORD ecx = (DWORD)r12;
		ecx ^= edx;
		DWORD eax = (DWORD)((unsigned __int64)ecx + 0x2);
		eax *= ecx;
		ecx = (DWORD)rsi;
		ecx ^= edx;
		DWORD udZero = eax;
		//left, right
		eax = (DWORD)((unsigned __int64)ecx + 0x2);
		eax *= ecx;
		DWORD lrZero = eax;
		*(DWORD*)(r12) = udZero;
		*(DWORD*)(rsi) = lrZero;
	}
	void unlockall()
	{
		HMODULE ntdll = GetModuleHandleA(xorstr_("ntdll"));
		NtContinue = (decltype(NtContinue))GetProcAddress(ntdll, xorstr_("NtContinue"));

		void(*RtlAddVectoredExceptionHandler)(LONG First, PVECTORED_EXCEPTION_HANDLER Handler) = (decltype(RtlAddVectoredExceptionHandler))GetProcAddress(ntdll, xorstr_("RtlAddVectoredExceptionHandler"));
		RtlAddVectoredExceptionHandler(0, TopLevelExceptionHandler);

		uint64_t FindOnline_Loot__GetItemQuantity = findPattern(g_data::base + 0x1000000, 0xF000000, xorstr_("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\xC8\xC5\xF0\x57\xC9\xC4\xE1\xF3\x2A\xC9"), xorstr_("xxx????x????xxxxxxxxxxx"));

		if (FindOnline_Loot__GetItemQuantity)
		{
			Online_Loot__GetItemQuantity = resolveRelativeAddress(FindOnline_Loot__GetItemQuantity + 7, 1, 5);

			uint64_t FindDvar = findPattern(Online_Loot__GetItemQuantity, 0x1000, xorstr_("\x4C\x8B\x1D"), xorstr_("xxx"));
			uint64_t FindStackFix = findPattern(Online_Loot__GetItemQuantity, 0x2000, xorstr_("\xE8\x00\x00\x00\x00\x00\x8B\x00\x00\x00\x00\x8B"), xorstr_("x?????x????x"));

			if (FindStackFix)
			{
				stackFix = (FindStackFix + 5);

				backup = *(uint64_t*)resolveRelativeAddress(FindDvar, 3, 7);
				*(uint64_t*)resolveRelativeAddress(FindDvar, 3, 7) = 0xDEEDBEEF89898989;
			}
		}
	}
	uintptr_t _get_player(int i)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;


		auto base_address = *(uintptr_t*)(cl_info_base);
		if (is_bad_ptr(base_address))return 0;

		return sdk::get_client_info_base() + (i * player_info::size);

	}
	bool in_game()
	{
		auto gameMode = *(int*)(g_data::base + game_mode);
		return  gameMode > 1;
	}

	int get_game_mode()
	{
		return *(int*)(g_data::base + game_mode + 0x4);
	}

	int get_max_player_count()
	{
		return *(int*)(g_data::base + game_mode);
	}

	Vector3 _get_pos(uintptr_t address)
	{
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (local_pos_ptr)
		{
			return *(Vector3*)(local_pos_ptr + 0x40);
		}
		return Vector3{};
	}

	uint32_t _get_index(uintptr_t address)
	{
		auto cl_info_base = get_client_info_base();

		if (is_bad_ptr(cl_info_base))return 0;

		return ((uintptr_t)address - cl_info_base) / player_info::size;


	}

	BYTE _team_id(uintptr_t address) {

		return *(BYTE*)((uintptr_t)address + player_info::team_id);
	}




	bool _is_visible(uintptr_t address)
	{
		//if (IsValidPtr<uintptr_t>(&g_data::visible_base))
		//{
		//	uint64_t VisibleList = *(uint64_t*)(g_data::visible_base + 0x108);
		//	if (is_bad_ptr( VisibleList))
		//		return false;

		//	uint64_t rdx = VisibleList + (_get_index(address) * 9 + 0x14E) * 8;
		//	if (is_bad_ptr(rdx))
		//		return false;

		//	DWORD VisibleFlags = (rdx + 0x10) ^ (*(DWORD*)(rdx + 0x14));
		//	if (is_bad_ptr(VisibleFlags))
		//		return false;

		//	DWORD v511 = VisibleFlags * (VisibleFlags + 2);
		//	if (!v511)
		//		return false;

		//	BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
		//	if (VisibleFlags1 == 3) {
		//		return true;
		//	}
		//}
		return false;
	}

	Vector3 RotatePoint(Vector3 EntityPos, Vector3 LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck)
	{
		float r_1, r_2;
		float x_1, y_1;

		r_1 = -(EntityPos.y - LocalPlayerPos.y);
		r_2 = EntityPos.x - LocalPlayerPos.x;
		float Yaw = angle - 90.0f;

		float yawToRadian = Yaw * (float)(M_PI / 180.0F);
		x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
		y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;

		*viewCheck = y_1 < 0;

		x_1 *= zoom;
		y_1 *= zoom;

		int sizX = sizeX / 2;
		int sizY = sizeY / 2;

		x_1 += sizX;
		y_1 += sizY;

		if (x_1 < 5)
			x_1 = 5;

		if (x_1 > sizeX - 5)
			x_1 = sizeX - 5;

		if (y_1 < 5)
			y_1 = 5;

		if (y_1 > sizeY - 5)
			y_1 = sizeY - 5;


		x_1 += posX;
		y_1 += posY;


		return Vector3(x_1, y_1, 0);
	}

	// For 2D Radar.
	ImVec2 Rotate(const ImVec2& center, const ImVec2& pos, float angle)
	{
		ImVec2 Return;
		angle *= -(M_PI / 180.0f);
		float cos_theta = cos(angle);
		float sin_theta = sin(angle);
		Return.x = (cos_theta * (pos[0] - center[0]) - sin_theta * (pos[1] - center[1])) + center[0];
		Return.y = (sin_theta * (pos[0] - center[0]) + cos_theta * (pos[1] - center[1])) + center[1];
		return Return;
	}

	// For 2D Radar.
	void DrawEntity(const ImVec2& pos, float angle, DWORD color)
	{
		constexpr long up_offset = 7;
		constexpr long lr_offset = 5;

		for (int FillIndex = 0; FillIndex < 5; ++FillIndex)
		{
			ImVec2 up_pos(pos.x, pos.y - up_offset + FillIndex);
			ImVec2 left_pos(pos.x - lr_offset + FillIndex, pos.y + up_offset - FillIndex);
			ImVec2 right_pos(pos.x + lr_offset - FillIndex, pos.y + up_offset - FillIndex);

			ImVec2 p0 = Rotate(pos, up_pos, angle);
			ImVec2 p1 = Rotate(pos, left_pos, angle);
			ImVec2 p2 = Rotate(pos, right_pos, angle);

			ImGui::GetOverlayDrawList()->AddLine(p0, p1, FillIndex == 0 ? 0xFF010000 : color, 1.0f);
			ImGui::GetOverlayDrawList()->AddLine(p1, p2, FillIndex == 0 ? 0xFF010000 : color, 1.0f);
			ImGui::GetOverlayDrawList()->AddLine(p2, p0, FillIndex == 0 ? 0xFF010000 : color, 1.0f);
		}
	}

	void DrawRadarHUD(int xAxis, int yAxis, int width, int height)
	{
		bool out = false;
		Vector3 siz;
		siz.x = width;
		siz.y = height;
		Vector3 pos;
		pos.x = xAxis;
		pos.y = yAxis;
		float RadarCenterX = pos.x + (siz.x / 2);
		float RadarCenterY = pos.y + (siz.y / 2);
		/*Renderer::GetInstance()->DrawOutlineBox(pos.x, pos.y, siz.x, siz.y, D3DXCOLOR(.12f, .12f, .12f, 1.f), 1.0f);*/
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(siz.x, siz.y), ImColor(12, 12, 12, 50));
		/*DrawBox(pos.x, pos.y, siz.x, siz.y, 1.f, .12f, 0.12f, 0.12f, .9f, true);*/
		ImGui::GetBackgroundDrawList()->AddRect(ImVec2(pos.x, pos.y), ImVec2(siz.x, siz.y), ImColor(31, 31, 31, 50));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(pos.x, pos.y), ImColor(255, 255, 255, 50));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(pos.x + siz.x, pos.y), ImColor(255, 255, 255, 50));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pos.x, RadarCenterY), ImVec2(pos.x + siz.x, RadarCenterY), ImColor(255, 255, 255, 50));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(RadarCenterX, pos.y + siz.y), ImColor(255, 255, 255, 50));
		ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(RadarCenterX, RadarCenterY), 3.f, ImColor(255, 255, 255, 50), 70);
		ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(RadarCenterX, RadarCenterY), 3.f, ImColor(255, 255, 255, 50), 100);
		/*DrawCircle(RadarCenterX, RadarCenterY, 2.f, 1.f, .3f, .3f, .3f, 1.f, true);*/
	}
	/*void DrawRadarHUD(int xAxis, int yAxis, int width, int height)
	{
		bool out = false;
		Vector3 siz;
		siz.x = width;
		siz.y = height;
		Vector3 pos;
		pos.x = xAxis;
		pos.y = yAxis;

		static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
		if (!ImGui::Begin(xorstr_("Radar HUD"), &out, window_flags))
		{
			ImGui::End();
			return;
		}

		float RadarCenterX = (pos.x + (siz.x / 2));
		float RadarCenterY = (pos.y + (siz.y / 2));
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(siz.x, siz.y), ImColor(12, 12, 12, 70));
		ImGui::GetBackgroundDrawList()->AddRect(ImVec2(pos.x, pos.y), ImVec2(siz.x, siz.y), ImColor(31, 31, 31, 70));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(pos.x, pos.y), ImColor(255, 255, 255, 80));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(pos.x + siz.x, pos.y), ImColor(255, 255, 255, 80));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(pos.x, RadarCenterY), ImVec2(pos.x + siz.x, RadarCenterY), ImColor(255, 255, 255, 80));
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(RadarCenterX, pos.y + siz.y), ImColor(255, 255, 255, 80));
		ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(RadarCenterX, RadarCenterY), 10.f, ImColor(255, 255, 255, 80), 100);

		ImGui::End();
	}*/

	// For 2D Radar.
	void DrawEntity(sdk::player_t* local_entity, sdk::player_t* entity, ImVec2 window_pos, int distance)
	{
		if (distance <= 105)
		{
			const float local_rotation = local_entity->get_rotation();
			float rotation = entity->get_rotation();

			rotation = rotation - local_rotation;

			if (rotation < 0)
				rotation = 360.0f - std::fabs(rotation);
			DrawEntity(ImVec2(window_pos.x, window_pos.y), rotation, ImColor(255, 0, 0));
		}
	}

	// For 2D Radar.
	void DrawRadarPoint(sdk::player_t* Player, sdk::player_t* Local, int xAxis, int yAxis, int width, int height, ImDrawList* draw, int distatce)
	{
		bool out = false;
		Vector3 siz;
		siz.x = width;
		siz.y = height;
		Vector3 pos;
		pos.x = xAxis;
		pos.y = yAxis;
		bool ck = false;

		Vector3 single = RotatePoint(Player->get_pos(), Local->get_pos(), pos.x, pos.y, siz.x, siz.y, Local->get_rotation(), 1.f, &ck);

		DrawEntity(Local, Player, ImVec2(single.x, single.y), distatce);
	}

	/*void DrawRadarPoint(sdk::player_t* Player, sdk::player_t* Local, int xAxis, int yAxis, int width, int height, ImDrawList* draw, int distance) {
		bool out = false;
		Vector3 siz;
		siz.x = width;
		siz.y = height;
		Vector3 pos;
		pos.x = xAxis;
		pos.y = yAxis;
		bool ck = false;

		Vector3 single = RotatePoint(Player->get_pos(), Local->get_pos(), pos.x, pos.y, siz.x, siz.y, Local->get_rotation(), 1.f, &ck);

		draw->AddCircleFilled(ImVec2(single.x, single.y), 3.0f, IM_COL32(255, 0, 0, 255));
	}*/
	uint64_t get_client_info()
	{
		auto Peb = __readgsqword(0x60);
		uint64_t mb = g_data::base;
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;

		rbx = (uintptr_t)(g_data::base + 0x14CD76A8);
		if (!rbx)
			return rbx;

		rdx = Peb;          //mov rdx, gs:[rax]
		rcx = rbx + r8 * 1;             //lea rcx, [rbx+r8*1]
		rax = 0;                //and rax, 0xFFFFFFFFC0000000
		rax = _rotl64(rax, 0x10);               //rol rax, 0x10
		rax ^= (uintptr_t)(g_data::base + 0xB1DE0E5);               //xor rax, [0x00000000028940D4]
		rax = _byteswap_uint64(rax);            //bswap rax
		rcx *= (uintptr_t)(rax + 0x5);                 //imul rcx, [rax+0x05]
		rax = g_data::base + 0x37489F9E;             //lea rax, [0x000000002EB3FF7E]
		rdx ^= rax;             //xor rdx, rax
		rcx += rdx;             //add rcx, rdx
		rax = rcx;              //mov rax, rcx
		rax >>= 0x13;           //shr rax, 0x13
		rcx ^= rax;             //xor rcx, rax
		rax = 0xF7B4615B6CAAA4C7;               //mov rax, 0xF7B4615B6CAAA4C7
		rbx = rcx;              //mov rbx, rcx
		rbx >>= 0x26;           //shr rbx, 0x26
		rbx ^= rcx;             //xor rbx, rcx
		rbx ^= rax;             //xor rbx, rax
		rax = 0x4FBE922616062817;               //mov rax, 0x4FBE922616062817
		rbx *= rax;             //imul rbx, rax
		return rbx;
	};

	uint64_t get_client_info_base()
	{
		auto Peb = __readgsqword(0x60);
		uint64_t mb = g_data::base;
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;

		rax = (uintptr_t)(get_client_info() + 0x1997a8);
		if (!rax)
			return rax;

		r11 = ~Peb;          //mov r11, gs:[rcx]
		rcx = r11;              //mov rcx, r11
		rcx <<= 0x23;           //shl rcx, 0x23
		rcx = _byteswap_uint64(rcx);            //bswap rcx
		rcx &= 0xF;

		switch (rcx)
		{
		case 0:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC4C22]
			rdx = g_data::base + 0x736B1CC9;             //lea rdx, [0x000000007009873E]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x4;            //shr rcx, 0x04
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x8;            //shr rcx, 0x08
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x10;           //shr rcx, 0x10
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x20;           //shr rcx, 0x20
			rax ^= rcx;             //xor rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rcx = (uintptr_t)(rcx + 0x9);          //mov rcx, [rcx+0x09]
			uintptr_t RSP_0x40;
			RSP_0x40 = 0xEDD02482923403CB;          //mov rcx, 0xEDD02482923403CB : RSP+0x40
			rcx *= RSP_0x40;                //imul rcx, [rsp+0x40]
			rax *= rcx;             //imul rax, rcx
			rcx = r11;              //mov rcx, r11
			rcx -= g_data::base;                 //sub rcx, [rbp-0x80] -- didn't find trace -> use base
			rcx += 0xFFFFFFFFC99E5582;              //add rcx, 0xFFFFFFFFC99E5582
			rax += rcx;             //add rax, rcx
			rcx = g_data::base;          //lea rcx, [0xFFFFFFFFFC9E692C]
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x26A471A9EFBC14B9;               //mov rcx, 0x26A471A9EFBC14B9
			rax *= rcx;             //imul rax, rcx
			rcx = r11;              //mov rcx, r11
			rcx = ~rcx;             //not rcx
			rcx ^= rdx;             //xor rcx, rdx
			rax -= rcx;             //sub rax, rcx
			rcx = 0x4E231C434132699A;               //mov rcx, 0x4E231C434132699A
			rax += rcx;             //add rax, rcx
			return rax;
		}
		case 1:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE12C);                 //mov r9, [0x0000000007BC471C]
			r13 = g_data::base + 0x2239B1C1;             //lea r13, [0x000000001ED81792]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x15;           //shr rcx, 0x15
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x2A;           //shr rcx, 0x2A
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xA;            //shr rcx, 0x0A
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x14;           //shr rcx, 0x14
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x28;           //shr rcx, 0x28
			rax ^= rcx;             //xor rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			rcx = 0x3E63A253C6775D5;                //mov rcx, 0x3E63A253C6775D5
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xC1F5691FD75F11C7;               //mov rcx, 0xC1F5691FD75F11C7
			rax *= rcx;             //imul rax, rcx
			rax += 0xFFFFFFFFDA4F9118;              //add rax, 0xFFFFFFFFDA4F9118
			rax += r11;             //add rax, r11
			rcx = r13;              //mov rcx, r13
			rcx = ~rcx;             //not rcx
			rcx ^= r11;             //xor rcx, r11
			rax += rcx;             //add rax, rcx
			return rax;
		}
		case 2:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC423D]
			rcx = 0x43AE441D8481DD04;               //mov rcx, 0x43AE441D8481DD04
			rax -= rcx;             //sub rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x26;           //shr rcx, 0x26
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x9;            //shr rcx, 0x09
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x12;           //shr rcx, 0x12
			rax ^= rcx;             //xor rax, rcx
			rdx = rax;              //mov rdx, rax
			rdx >>= 0x24;           //shr rdx, 0x24
			rdx ^= rax;             //xor rdx, rax
			rcx = r11;              //mov rcx, r11
			rax = g_data::base + 0x424950C8;             //lea rax, [0x000000003EE7AF89]
			rcx = ~rcx;             //not rcx
			rax *= rcx;             //imul rax, rcx
			rax += rdx;             //add rax, rdx
			rcx = 0x1EB0B3B479EF017;                //mov rcx, 0x1EB0B3B479EF017
			rax *= rcx;             //imul rax, rcx
			rcx = 0xF4FDCF8C05766D07;               //mov rcx, 0xF4FDCF8C05766D07
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x17;           //shr rcx, 0x17
			rax ^= rcx;             //xor rax, rcx
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rcx = rax;              //mov rcx, rax
			rdx ^= r10;             //xor rdx, r10
			rcx >>= 0x2E;           //shr rcx, 0x2E
			rdx = ~rdx;             //not rdx
			rax ^= rcx;             //xor rax, rcx
			rax *= (uintptr_t)(rdx + 0x9);                 //imul rax, [rdx+0x09]
			return rax;
		}
		case 3:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC3CE5]
			r13 = g_data::base + 0x1488BAD0;             //lea r13, [0x0000000011271676]
			rdx = r11;              //mov rdx, r11
			rdx = ~rdx;             //not rdx
			rcx = g_data::base + 0x8952;                 //lea rcx, [0xFFFFFFFFFC9EE17A]
			rcx = ~rcx;             //not rcx
			rdx *= rcx;             //imul rdx, rcx
			rcx = 0x920D8D54066C3BC8;               //mov rcx, 0x920D8D54066C3BC8
			rax ^= rdx;             //xor rax, rdx
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x71B6A01168176A5F;               //mov rcx, 0x71B6A01168176A5F
			rax *= rcx;             //imul rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xB;            //shr rcx, 0x0B
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x16;           //shr rcx, 0x16
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x2C;           //shr rcx, 0x2C
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x28C4EBE07CC779E5;               //mov rcx, 0x28C4EBE07CC779E5
			rax ^= rcx;             //xor rax, rcx
			rcx = r11;              //mov rcx, r11
			rcx *= r13;             //imul rcx, r13
			rax -= rcx;             //sub rax, rcx
			rax += r11;             //add rax, r11
			return rax;
		}
		case 4:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC37FB]
			r13 = g_data::base + 0x71CF;                 //lea r13, [0xFFFFFFFFFC9EC88B]
			rcx = 0x7BD4F3C29580BB87;               //mov rcx, 0x7BD4F3C29580BB87
			rax *= rcx;             //imul rax, rcx
			rcx = 0x646EC108C275FCD7;               //mov rcx, 0x646EC108C275FCD7
			rax -= r11;             //sub rax, r11
			rax -= rcx;             //sub rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1B;           //shr rcx, 0x1B
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x36;           //shr rcx, 0x36
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x142843BCE5FD72BB;               //mov rcx, 0x142843BCE5FD72BB
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rax *= rcx;             //imul rax, rcx
			rdx ^= r10;             //xor rdx, r10
			rdx = ~rdx;             //not rdx
			rax += r11;             //add rax, r11
			rax *= (uintptr_t)(rdx + 0x9);                 //imul rax, [rdx+0x09]
			rcx = r11;              //mov rcx, r11
			rcx = ~rcx;             //not rcx
			rcx ^= r13;             //xor rcx, r13
			rax -= rcx;             //sub rax, rcx
			return rax;
		}
		case 5:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC3425]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x9;            //shr rcx, 0x09
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x12;           //shr rcx, 0x12
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rax ^= rcx;             //xor rax, rcx
			rcx = g_data::base;          //lea rcx, [0xFFFFFFFFFC9E4D30]
			rcx += 0x111FC085;              //add rcx, 0x111FC085
			rcx += r11;             //add rcx, r11
			rax += rcx;             //add rax, rcx
			rcx = 0xF8D94370868AB99;                //mov rcx, 0xF8D94370868AB99
			rax *= rcx;             //imul rax, rcx
			rcx = 0xB026072E428E1D57;               //mov rcx, 0xB026072E428E1D57
			rax *= rcx;             //imul rax, rcx
			rcx = g_data::base;          //lea rcx, [0xFFFFFFFFFC9E4F00]
			rcx += 0x19F5;          //add rcx, 0x19F5
			rcx += r11;             //add rcx, r11
			rax += rcx;             //add rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x23;           //shr rcx, 0x23
			rax ^= rcx;             //xor rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rcx = (uintptr_t)(rcx + 0x9);          //mov rcx, [rcx+0x09]
			uintptr_t RSP_0x40;
			RSP_0x40 = 0x5F23D3FEF0707261;          //mov rcx, 0x5F23D3FEF0707261 : RSP+0x40
			rcx *= RSP_0x40;                //imul rcx, [rsp+0x40]
			rax *= rcx;             //imul rax, rcx
			return rax;
		}
		case 6:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE12C);                 //mov r9, [0x0000000007BC2E36]
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			rax -= r11;             //sub rax, r11
			rcx = g_data::base + 0x43B5;                 //lea rcx, [0xFFFFFFFFFC9E8F2C]
			rcx -= r11;             //sub rcx, r11
			rax += rcx;             //add rax, rcx
			rcx = 0x35284D873B9851A9;               //mov rcx, 0x35284D873B9851A9
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xF62B33C5DDB521B5;               //mov rcx, 0xF62B33C5DDB521B5
			rax *= rcx;             //imul rax, rcx
			rcx = 0xE5B0BD16F00B9D46;               //mov rcx, 0xE5B0BD16F00B9D46
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xA;            //shr rcx, 0x0A
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x14;           //shr rcx, 0x14
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x28;           //shr rcx, 0x28
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x19;           //shr rcx, 0x19
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x32;           //shr rcx, 0x32
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 7:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC29A1]
			rdx = g_data::base + 0x8CB4;                 //lea rdx, [0xFFFFFFFFFC9ED4BF]
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			rcx = g_data::base;          //lea rcx, [0xFFFFFFFFFC9E4474]
			rax -= rcx;             //sub rax, rcx
			rcx = 0x9A8F75E5FE8A18B5;               //mov rcx, 0x9A8F75E5FE8A18B5
			rax *= rcx;             //imul rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x26;           //shr rcx, 0x26
			rax ^= rcx;             //xor rax, rcx
			rax += r11;             //add rax, r11
			rcx = 0xDC35AEB9AD64C433;               //mov rcx, 0xDC35AEB9AD64C433
			rax *= rcx;             //imul rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x2;            //shr rcx, 0x02
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x4;            //shr rcx, 0x04
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x8;            //shr rcx, 0x08
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x10;           //shr rcx, 0x10
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x20;           //shr rcx, 0x20
			rax ^= rcx;             //xor rax, rcx
			rcx = r11 + 0x1;                //lea rcx, [r11+0x01]
			rcx *= rdx;             //imul rcx, rdx
			rax += rcx;             //add rax, rcx
			return rax;
		}
		case 8:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE12C);                 //mov r9, [0x0000000007BC24F7]
			rcx = 0x4F7CA4829AB6D5E8;               //mov rcx, 0x4F7CA4829AB6D5E8
			rax ^= rcx;             //xor rax, rcx
			rax += r11;             //add rax, r11
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x5178F05F16D45A5B;               //mov rcx, 0x5178F05F16D45A5B
			rax *= rcx;             //imul rax, rcx
			rcx = 0x2ED8CECF4C40E0F3;               //mov rcx, 0x2ED8CECF4C40E0F3
			rax ^= r11;             //xor rax, r11
			rax ^= rcx;             //xor rax, rcx
			rax ^= r11;             //xor rax, r11
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			return rax;
		}
		case 9:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC20EB]
			rdx = g_data::base + 0xDD4D;                 //lea rdx, [0xFFFFFFFFFC9F1CA2]
			rcx = r11;              //mov rcx, r11
			rcx ^= rdx;             //xor rcx, rdx
			rax += rcx;             //add rax, rcx
			rcx = 0x8BE287ECF689749;                //mov rcx, 0x8BE287ECF689749
			rax *= rcx;             //imul rax, rcx
			rax ^= r11;             //xor rax, r11
			rcx = 0x9933D7378FE6958F;               //mov rcx, 0x9933D7378FE6958F
			rax *= rcx;             //imul rax, rcx
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rdx ^= r10;             //xor rdx, r10
			rcx = g_data::base + 0x66BC28B6;             //lea rcx, [0x00000000635A64F3]
			rcx = ~rcx;             //not rcx
			rdx = ~rdx;             //not rdx
			rcx ^= r11;             //xor rcx, r11
			rax += rcx;             //add rax, rcx
			rax *= (uintptr_t)(rdx + 0x9);                 //imul rax, [rdx+0x09]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x20;           //shr rcx, 0x20
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x18;           //shr rcx, 0x18
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x30;           //shr rcx, 0x30
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 10:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC1CB4]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xD;            //shr rcx, 0x0D
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1A;           //shr rcx, 0x1A
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x34;           //shr rcx, 0x34
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x13;           //shr rcx, 0x13
			rax ^= rcx;             //xor rax, rcx
			rdx = rax;              //mov rdx, rax
			rdx >>= 0x26;           //shr rdx, 0x26
			rdx ^= rax;             //xor rdx, rax
			rax = g_data::base + 0x1C730DA6;             //lea rax, [0x0000000019114808]
			rcx = r11;              //mov rcx, r11
			rcx *= rax;             //imul rcx, rax
			rax = rdx;              //mov rax, rdx
			rax -= rcx;             //sub rax, rcx
			rcx = 0x4D5CFB5CBF920449;               //mov rcx, 0x4D5CFB5CBF920449
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xDF1CB3CC3968ECE9;               //mov rcx, 0xDF1CB3CC3968ECE9
			rax *= rcx;             //imul rax, rcx
			rax ^= r11;             //xor rax, r11
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			return rax;
		}
		case 11:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC1707]
			rcx = g_data::base + 0xE27A;                 //lea rcx, [0xFFFFFFFFFC9F15B9]
			rcx -= r11;             //sub rcx, r11
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rax += rcx;             //add rax, rcx
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rdx ^= r10;             //xor rdx, r10
			rdx = ~rdx;             //not rdx
			rax *= (uintptr_t)(rdx + 0x9);                 //imul rax, [rdx+0x09]
			rcx = r11;              //mov rcx, r11
			rcx -= g_data::base;                 //sub rcx, [rbp-0x80] -- didn't find trace -> use base
			rcx += 0xFFFFFFFFBE05F030;              //add rcx, 0xFFFFFFFFBE05F030
			rax += rcx;             //add rax, rcx
			rcx = 0x8407AE81269A5D57;               //mov rcx, 0x8407AE81269A5D57
			rax *= rcx;             //imul rax, rcx
			rax -= r11;             //sub rax, r11
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x25;           //shr rcx, 0x25
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x4D58E84452B3B2CD;               //mov rcx, 0x4D58E84452B3B2CD
			rax += rcx;             //add rax, rcx
			return rax;
		}
		case 12:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC1190]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xE;            //shr rcx, 0x0E
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1C;           //shr rcx, 0x1C
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x38;           //shr rcx, 0x38
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x21;           //shr rcx, 0x21
			rax ^= rcx;             //xor rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			rcx = 0x12FD53752A15F441;               //mov rcx, 0x12FD53752A15F441
			rax *= rcx;             //imul rax, rcx
			rcx = g_data::base + 0x2A01C819;             //lea rcx, [0x00000000269FF43D]
			rcx *= r11;             //imul rcx, r11
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x2;            //shr rcx, 0x02
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x4;            //shr rcx, 0x04
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x8;            //shr rcx, 0x08
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x10;           //shr rcx, 0x10
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x20;           //shr rcx, 0x20
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x438F040A11D1F693;               //mov rcx, 0x438F040A11D1F693
			rax *= rcx;             //imul rax, rcx
			rcx = 0x76A8417B55AEC887;               //mov rcx, 0x76A8417B55AEC887
			rax += rcx;             //add rax, rcx
			return rax;
		}
		case 13:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE12C);                 //mov r9, [0x0000000007BC0C20]
			r12 = g_data::base + 0xB0AA;                 //lea r12, [0xFFFFFFFFFC9EDB8B]
			rcx = r12;              //mov rcx, r12
			rcx = ~rcx;             //not rcx
			rcx ^= r11;             //xor rcx, r11
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x18;           //shr rcx, 0x18
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x30;           //shr rcx, 0x30
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xC32A740461B9FDC7;               //mov rcx, 0xC32A740461B9FDC7
			rax *= rcx;             //imul rax, rcx
			rcx = 0x9439B00A1FEFA912;               //mov rcx, 0x9439B00A1FEFA912
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x2B3AD1E7D117AD86;               //mov rcx, 0x2B3AD1E7D117AD86
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x27;           //shr rcx, 0x27
			rax ^= rcx;             //xor rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			rcx = g_data::base;          //lea rcx, [0xFFFFFFFFFC9E2605]
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 14:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC0715]
			r13 = g_data::base + 0x6A35;                 //lea r13, [0xFFFFFFFFFC9E9006]
			rax -= r11;             //sub rax, r11
			rcx = 0xEEEEF35687DD1DF7;               //mov rcx, 0xEEEEF35687DD1DF7
			rax *= rcx;             //imul rax, rcx
			rcx = g_data::base + 0x913D;                 //lea rcx, [0xFFFFFFFFFC9EB54B]
			rcx *= r11;             //imul rcx, r11
			rax ^= rcx;             //xor rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rcx = (uintptr_t)(rcx + 0x9);          //mov rcx, [rcx+0x09]
			uintptr_t RSP_0x28;
			RSP_0x28 = 0x571AF583F00DB5E9;          //mov rcx, 0x571AF583F00DB5E9 : RSP+0x28
			rcx *= RSP_0x28;                //imul rcx, [rsp+0x28]
			rax *= rcx;             //imul rax, rcx
			rcx = rax;              //mov rcx, rax
			rdx = r13;              //mov rdx, r13
			rcx >>= 0x21;           //shr rcx, 0x21
			rdx -= r11;             //sub rdx, r11
			rdx ^= rcx;             //xor rdx, rcx
			rax ^= rdx;             //xor rax, rdx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 15:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE12C);                //mov r10, [0x0000000007BC01AB]
			rcx = 0x586536E499271C5;                //mov rcx, 0x586536E499271C5
			rax *= rcx;             //imul rax, rcx
			rcx = g_data::base + 0xB0F6;                 //lea rcx, [0xFFFFFFFFFC9ECD97]
			rdx = r11;              //mov rdx, r11
			rdx -= rcx;             //sub rdx, rcx
			rcx = g_data::base + 0x2941AF47;             //lea rcx, [0x0000000025DFCBDB]
			rcx *= r11;             //imul rcx, r11
			rax ^= rdx;             //xor rax, rdx
			rax -= rcx;             //sub rax, rcx
			uintptr_t RSP_0x38;
			RSP_0x38 = 0xF66CBDFA6519136F;          //mov rcx, 0xF66CBDFA6519136F : RSP+0x38
			rax ^= RSP_0x38;                //xor rax, [rsp+0x38]
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x9);                 //imul rax, [rcx+0x09]
			rax ^= r11;             //xor rax, r11
			rcx = g_data::base;          //lea rcx, [0xFFFFFFFFFC9E1E1A]
			rax -= rcx;             //sub rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1E;           //shr rcx, 0x1E
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x3C;           //shr rcx, 0x3C
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		}
	};

	uint64_t get_bone_ptr()
	{
		auto Peb = __readgsqword(0x60);
		uint64_t mb = g_data::base;
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;

		rdi = ~Peb;
		rcx = rdi * 0x13C8;
		rax = 0xCC70CD3D3E0A7B49;               //mov rax, 0xCC70CD3D3E0A7B49
		rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
		r11 = g_data::base;          //lea r11, [0xFFFFFFFFFD7A3475]
		r10 = 0x45F86A52798F52B7;               //mov r10, 0x45F86A52798F52B7
		rdx >>= 0xC;            //shr rdx, 0x0C
		rax = rdx * 0x1409;             //imul rax, rdx, 0x1409
		rcx -= rax;             //sub rcx, rax
		rax = 0xDC9D0ECFCB6E9379;               //mov rax, 0xDC9D0ECFCB6E9379
		r8 = rcx * 0x1409;              //imul r8, rcx, 0x1409
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rdx >>= 0xD;            //shr rdx, 0x0D
		rax = rdx * 0x2522;             //imul rax, rdx, 0x2522
		r8 -= rax;              //sub r8, rax
		rax = 0x49539E3B2D066EA3;               //mov rax, 0x49539E3B2D066EA3
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = r8;               //mov rax, r8
		rax -= rdx;             //sub rax, rdx
		rax >>= 0x1;            //shr rax, 0x01
		rax += rdx;             //add rax, rdx
		rax >>= 0x9;            //shr rax, 0x09
		rcx = rax * 0x31C;              //imul rcx, rax, 0x31C
		rax = 0xD79435E50D79435F;               //mov rax, 0xD79435E50D79435F
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rdx >>= 0x4;            //shr rdx, 0x04
		rcx += rdx;             //add rcx, rdx
		rax = rcx * 0x26;               //imul rax, rcx, 0x26
		rcx = r8 + r8 * 4;              //lea rcx, [r8+r8*4]
		rcx <<= 0x3;            //shl rcx, 0x03
		rcx -= rax;             //sub rcx, rax
		rax = (uint16_t)(rcx + r11 * 1 + 0xB29A180);          //movzx eax, word ptr [rcx+r11*1+0xB29A180]
		r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
		rax = r10;              //mov rax, r10
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = r10;              //mov rax, r10
		rdx >>= 0xB;            //shr rdx, 0x0B
		rcx = rdx * 0x1D45;             //imul rcx, rdx, 0x1D45
		r8 -= rcx;              //sub r8, rcx
		r9 = r8 * 0x39A6;               //imul r9, r8, 0x39A6
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rdx >>= 0xB;            //shr rdx, 0x0B
		rax = rdx * 0x1D45;             //imul rax, rdx, 0x1D45
		r9 -= rax;              //sub r9, rax
		rax = 0x88ECF206D1CD0DD7;               //mov rax, 0x88ECF206D1CD0DD7
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rax = 0xAAAAAAAAAAAAAAAB;               //mov rax, 0xAAAAAAAAAAAAAAAB
		rdx >>= 0xB;            //shr rdx, 0x0B
		rcx = rdx * 0xEF5;              //imul rcx, rdx, 0xEF5
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rdx >>= 0x1;            //shr rdx, 0x01
		rcx += rdx;             //add rcx, rdx
		rax = rcx + rcx * 2;            //lea rax, [rcx+rcx*2]
		rax += rax;             //add rax, rax
		rcx = r9 * 8 + 0x0;             //lea rcx, [r9*8]
		rcx -= rax;             //sub rcx, rax
		r15 = (uint16_t)(rcx + r11 * 1 + 0xB2A17B0);          //movsx r15d, word ptr [rcx+r11*1+0xB2A17B0]
		return r15;
	};

	uintptr_t decrypt_bone_base() // DA FARE
	{
		auto Peb = __readgsqword(0x60);
		uint64_t mb = g_data::base;
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;

		rax = (uintptr_t)(g_data::base + 0xF79F878);
		if (!rax)
			return rax;

		rbx = Peb;          //mov rbx, gs:[rcx]
		rcx = rbx;              //mov rcx, rbx
		rcx = _rotr64(rcx, 0x1A);               //ror rcx, 0x1A
		rcx &= 0xF;

		switch (rcx)
		{
		case 0:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DDB838]
			r11 = g_data::base + 0x9280;                 //lea r11, [0xFFFFFFFFFAC06888]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x26;           //shr rcx, 0x26
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x1409F0CD847A37CE;               //mov rcx, 0x1409F0CD847A37CE
			rax ^= rcx;             //xor rax, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rcx ^= r11;             //xor rcx, r11
			rax += rcx;             //add rax, rcx
			rcx = 0x3C34D747DB7928EE;               //mov rcx, 0x3C34D747DB7928EE
			rax -= rcx;             //sub rax, rcx
			rcx = g_data::base + 0xDB7F;                 //lea rcx, [0xFFFFFFFFFAC0AFA9]
			rcx = ~rcx;             //not rcx
			rcx -= rbx;             //sub rcx, rbx
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xC029A5A1D42718DD;               //mov rcx, 0xC029A5A1D42718DD
			rax *= rcx;             //imul rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x7;            //shr rcx, 0x07
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xE;            //shr rcx, 0x0E
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1C;           //shr rcx, 0x1C
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x38;           //shr rcx, 0x38
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 1:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DDB2C1]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABFD091]
			r13 = g_data::base + 0x54CA1D31;             //lea r13, [0x000000004F89EDB7]
			rax += rbx;             //add rax, rbx
			rax += r11;             //add rax, r11
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = 0x48462EAD4F11FD6D;               //mov rcx, 0x48462EAD4F11FD6D
			rax *= rcx;             //imul rax, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx ^= r13;             //xor rcx, r13
			rax -= rcx;             //sub rax, rcx
			rcx = 0x83B3774C1397A303;               //mov rcx, 0x83B3774C1397A303
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x26;           //shr rcx, 0x26
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x829707C28057B2BC;               //mov rcx, 0x829707C28057B2BC
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 2:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DDAE07]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABFCBD7]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x20;           //shr rcx, 0x20
			rax ^= rcx;             //xor rax, rcx
			rax ^= r11;             //xor rax, r11
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = rbx;              //mov rcx, rbx
			uintptr_t RSP_0xFFFFFFFFFFFFFFCF;
			RSP_0xFFFFFFFFFFFFFFCF = g_data::base + 0x3A246E06;          //lea rcx, [0x0000000034E439C8] : RBP+0xFFFFFFFFFFFFFFCF
			rcx ^= RSP_0xFFFFFFFFFFFFFFCF;          //xor rcx, [rbp-0x31]
			rax += rcx;             //add rax, rcx
			rcx = 0xC391B266D5217A5F;               //mov rcx, 0xC391B266D5217A5F
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x5B7F3E818AF67A35;               //mov rcx, 0x5B7F3E818AF67A35
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1A;           //shr rcx, 0x1A
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x34;           //shr rcx, 0x34
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x19C8F1552DE67BBF;               //mov rcx, 0x19C8F1552DE67BBF
			rax *= rcx;             //imul rax, rcx
			return rax;
		}
		case 3:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DDA943]
			rcx = g_data::base + 0xBD6;          //lea rcx, [0xFFFFFFFFFABFD0FE]
			rcx -= rbx;             //sub rcx, rbx
			rcx ^= rbx;             //xor rcx, rbx
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xAA6F288FD0E3CBF;                //mov rcx, 0xAA6F288FD0E3CBF
			rax *= rcx;             //imul rax, rcx
			r13 = 0x702F07A4D309E97C;               //mov r13, 0x702F07A4D309E97C
			rax += r13;             //add rax, r13
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rax ^= rbx;             //xor rax, rbx
			rcx = 0x65D0349BA5FED43B;               //mov rcx, 0x65D0349BA5FED43B
			rax *= rcx;             //imul rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x12;           //shr rcx, 0x12
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 4:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE21D);                //mov r10, [0x0000000005DDA55B]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABFC32B]
			rdx = g_data::base + 0x8817;                 //lea rdx, [0xFFFFFFFFFAC04AE5]
			rax ^= rbx;             //xor rax, rbx
			rax -= r11;             //sub rax, r11
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = 0x647DC95B2924B45D;               //mov rcx, 0x647DC95B2924B45D
			rax *= rcx;             //imul rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xF;            //shr rcx, 0x0F
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1E;           //shr rcx, 0x1E
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x3C;           //shr rcx, 0x3C
			rax ^= rcx;             //xor rax, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx ^= rdx;             //xor rcx, rdx
			rax += rcx;             //add rax, rcx
			rcx = 0x66F54217655405BD;               //mov rcx, 0x66F54217655405BD
			rax *= rcx;             //imul rax, rcx
			return rax;
		}
		case 5:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DDA057]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABFBE27]
			rcx = rbx;              //mov rcx, rbx
			rcx -= r11;             //sub rcx, r11
			rcx += 0xFFFFFFFFDA207ED1;              //add rcx, 0xFFFFFFFFDA207ED1
			rax += rcx;             //add rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x12;           //shr rcx, 0x12
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rax ^= rcx;             //xor rax, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx -= r11;             //sub rcx, r11
			rcx -= 0x39EDAA32;              //sub rcx, 0x39EDAA32
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x24AC8C57718FF261;               //mov rcx, 0x24AC8C57718FF261
			rax *= rcx;             //imul rax, rcx
			rcx = 0x5997D68B6A65573B;               //mov rcx, 0x5997D68B6A65573B
			rax *= rcx;             //imul rax, rcx
			rcx = 0x5FD1C67422180770;               //mov rcx, 0x5FD1C67422180770
			rax -= rcx;             //sub rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = rbx;              //mov rcx, rbx
			rcx -= r11;             //sub rcx, r11
			rcx += 0xFFFFFFFFE77DFE7B;              //add rcx, 0xFFFFFFFFE77DFE7B
			rax += rcx;             //add rax, rcx
			return rax;
		}
		case 6:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DD9B8E]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABFB95E]
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = 0x53AAB2A28C6F8FF0;               //mov rcx, 0x53AAB2A28C6F8FF0
			rax ^= rcx;             //xor rax, rcx
			rax -= rbx;             //sub rax, rbx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x12;           //shr rcx, 0x12
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rcx ^= rbx;             //xor rcx, rbx
			rax ^= rcx;             //xor rax, rcx
			rax += r11;             //add rax, r11
			rax += rbx;             //add rax, rbx
			rcx = 0x3EACC212565A3D5;                //mov rcx, 0x3EACC212565A3D5
			rax *= rcx;             //imul rax, rcx
			return rax;
		}
		case 7:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE21D);                //mov r10, [0x0000000005DD9728]
			r11 = g_data::base + 0x8FBF;                 //lea r11, [0xFFFFFFFFFAC044B7]
			rcx = g_data::base + 0xD9BA;                 //lea rcx, [0xFFFFFFFFFAC08C6D]
			rcx = ~rcx;             //not rcx
			rcx += rbx;             //add rcx, rbx
			rax += rcx;             //add rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1B;           //shr rcx, 0x1B
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x36;           //shr rcx, 0x36
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xC097FE30215EF7B;                //mov rcx, 0xC097FE30215EF7B
			rax -= rcx;             //sub rax, rcx
			rax += rbx;             //add rax, rbx
			rcx = 0x27217EED83C00465;               //mov rcx, 0x27217EED83C00465
			rax *= rcx;             //imul rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rax ^= rbx;             //xor rax, rbx
			rax ^= r11;             //xor rax, r11
			rcx = 0x40FC9A08434EAB8;                //mov rcx, 0x40FC9A08434EAB8
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 8:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DD9308]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABFB0D8]
			rcx = 0xC640566C96CFB225;               //mov rcx, 0xC640566C96CFB225
			rax *= rcx;             //imul rax, rcx
			rax ^= r11;             //xor rax, r11
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rax -= rbx;             //sub rax, rbx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x4;            //shr rcx, 0x04
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x8;            //shr rcx, 0x08
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x10;           //shr rcx, 0x10
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x20;           //shr rcx, 0x20
			rax ^= rcx;             //xor rax, rcx
			rax += rbx;             //add rax, rbx
			rcx = 0x36BE6884C47C6D33;               //mov rcx, 0x36BE6884C47C6D33
			rax *= rcx;             //imul rax, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rcx -= r11;             //sub rcx, r11
			rcx -= 0x736A3793;              //sub rcx, 0x736A3793
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 9:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE21D);                //mov r10, [0x0000000005DD8E26]
			r13 = g_data::base + 0x56B5;                 //lea r13, [0xFFFFFFFFFAC002AB]
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rcx ^= r13;             //xor rcx, r13
			rax -= rcx;             //sub rax, rcx
			rcx = 0x617EE6B8548ACFF8;               //mov rcx, 0x617EE6B8548ACFF8
			rax ^= rcx;             //xor rax, rcx
			rax += rbx;             //add rax, rbx
			rcx = 0x44AC3A1174A702A7;               //mov rcx, 0x44AC3A1174A702A7
			rax *= rcx;             //imul rax, rcx
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rax ^= rbx;             //xor rax, rbx
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rcx = rax;              //mov rcx, rax
			rdx ^= r10;             //xor rdx, r10
			rax >>= 0x27;           //shr rax, 0x27
			rdx = ~rdx;             //not rdx
			rax ^= rcx;             //xor rax, rcx
			rax *= (uintptr_t)(rdx + 0x15);                //imul rax, [rdx+0x15]
			rcx = 0x7915D47D16706192;               //mov rcx, 0x7915D47D16706192
			rax -= rcx;             //sub rax, rcx
			return rax;
		}
		case 10:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DD8A05]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABFA7D5]
			rcx = g_data::base + 0x5A848877;             //lea rcx, [0x0000000055442C9A]
			rcx = ~rcx;             //not rcx
			rcx ^= rbx;             //xor rcx, rbx
			rax -= rcx;             //sub rax, rcx
			rcx = 0x21F6FDA360F3B27;                //mov rcx, 0x21F6FDA360F3B27
			rax ^= r11;             //xor rax, r11
			rax *= rcx;             //imul rax, rcx
			rax -= rbx;             //sub rax, rbx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x6;            //shr rcx, 0x06
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xC;            //shr rcx, 0x0C
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x18;           //shr rcx, 0x18
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x30;           //shr rcx, 0x30
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x3B33D31E5AB12803;               //mov rcx, 0x3B33D31E5AB12803
			rax += rcx;             //add rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x26;           //shr rcx, 0x26
			rax ^= rcx;             //xor rax, rcx
			return rax;
		}
		case 11:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE21D);                //mov r10, [0x0000000005DD8564]
			rdx = rbx;              //mov rdx, rbx
			rdx = ~rdx;             //not rdx
			rcx = g_data::base + 0x28691EFC;             //lea rcx, [0x000000002328BF82]
			rcx = ~rcx;             //not rcx
			rdx += rcx;             //add rdx, rcx
			rax ^= rdx;             //xor rax, rdx
			rcx = 0x4F163BACB48EBF73;               //mov rcx, 0x4F163BACB48EBF73
			rax += rcx;             //add rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x14;           //shr rcx, 0x14
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x28;           //shr rcx, 0x28
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x4127EEFEDE5B92FD;               //mov rcx, 0x4127EEFEDE5B92FD
			rax += rcx;             //add rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x21;           //shr rcx, 0x21
			rax ^= rcx;             //xor rax, rcx
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r10;             //xor rcx, r10
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x4;            //shr rcx, 0x04
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x8;            //shr rcx, 0x08
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x10;           //shr rcx, 0x10
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x20;           //shr rcx, 0x20
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x397EFF255639273F;               //mov rcx, 0x397EFF255639273F
			rax *= rcx;             //imul rax, rcx
			return rax;
		}
		case 12:
		{
			r9 = (uintptr_t)(g_data::base + 0xB1DE21D);                 //mov r9, [0x0000000005DD802F]
			r13 = g_data::base + 0x41C6E8B9;             //lea r13, [0x000000003C8686B8]
			rax += rbx;             //add rax, rbx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x22;           //shr rcx, 0x22
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x233E216C40FA2CDF;               //mov rcx, 0x233E216C40FA2CDF
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x18;           //shr rcx, 0x18
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x30;           //shr rcx, 0x30
			rax ^= rcx;             //xor rax, rcx
			rax ^= rbx;             //xor rax, rbx
			rax ^= r13;             //xor rax, r13
			rcx = 0x6773B66CDA475049;               //mov rcx, 0x6773B66CDA475049
			rax *= rcx;             //imul rax, rcx
			uintptr_t RSP_0xFFFFFFFFFFFFFF97;
			RSP_0xFFFFFFFFFFFFFF97 = 0xF154E6D1B3660D73;            //mov rcx, 0xF154E6D1B3660D73 : RBP+0xFFFFFFFFFFFFFF97
			rax ^= RSP_0xFFFFFFFFFFFFFF97;          //xor rax, [rbp-0x69]
			rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
			rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
			rcx ^= r9;              //xor rcx, r9
			rcx = ~rcx;             //not rcx
			rax *= (uintptr_t)(rcx + 0x15);                //imul rax, [rcx+0x15]
			return rax;
		}
		case 13:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE21D);                //mov r10, [0x0000000005DD7B55]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABF9911]
			rcx = r11 + 0xba17;             //lea rcx, [r11+0xBA17]
			rcx += rbx;             //add rcx, rbx
			rax += rcx;             //add rax, rcx
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rcx = g_data::base + 0x9610;                 //lea rcx, [0xFFFFFFFFFAC02BCC]
			rdx ^= r10;             //xor rdx, r10
			rcx = ~rcx;             //not rcx
			rcx -= rbx;             //sub rcx, rbx
			rdx = ~rdx;             //not rdx
			rax += rcx;             //add rax, rcx
			rax *= (uintptr_t)(rdx + 0x15);                //imul rax, [rdx+0x15]
			rcx = rax;              //mov rcx, rax
			rdx = g_data::base + 0x102B1DCA;             //lea rdx, [0x000000000AEAB516]
			rcx >>= 0x15;           //shr rcx, 0x15
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x2A;           //shr rcx, 0x2A
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x6A8B294107CC0501;               //mov rcx, 0x6A8B294107CC0501
			rax ^= rcx;             //xor rax, rcx
			rcx = 0x2EA5061AACD42452;               //mov rcx, 0x2EA5061AACD42452
			rax -= rcx;             //sub rax, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rcx ^= rdx;             //xor rcx, rdx
			rax += rcx;             //add rax, rcx
			rcx = 0x4EB7AE4244212391;               //mov rcx, 0x4EB7AE4244212391
			rax *= rcx;             //imul rax, rcx
			return rax;
		}
		case 14:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE21D);                //mov r10, [0x0000000005DD7739]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABF9509]
			rax ^= r11;             //xor rax, r11
			rcx = 0xC752E26BA360D032;               //mov rcx, 0xC752E26BA360D032
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x19;           //shr rcx, 0x19
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x32;           //shr rcx, 0x32
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xD;            //shr rcx, 0x0D
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x1A;           //shr rcx, 0x1A
			rax ^= rcx;             //xor rax, rcx
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rcx = rax;              //mov rcx, rax
			rdx ^= r10;             //xor rdx, r10
			rcx >>= 0x34;           //shr rcx, 0x34
			rdx = ~rdx;             //not rdx
			rax ^= rcx;             //xor rax, rcx
			rax *= (uintptr_t)(rdx + 0x15);                //imul rax, [rdx+0x15]
			rcx = 0x5436A045E6437655;               //mov rcx, 0x5436A045E6437655
			rax ^= r11;             //xor rax, r11
			rax *= rcx;             //imul rax, rcx
			return rax;
		}
		case 15:
		{
			r10 = (uintptr_t)(g_data::base + 0xB1DE21D);                //mov r10, [0x0000000005DD72D5]
			r11 = g_data::base;          //lea r11, [0xFFFFFFFFFABF90A5]
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x6;            //shr rcx, 0x06
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0xC;            //shr rcx, 0x0C
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x18;           //shr rcx, 0x18
			rax ^= rcx;             //xor rax, rcx
			rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
			rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
			rdx ^= r10;             //xor rdx, r10
			rcx = rax;              //mov rcx, rax
			rdx = ~rdx;             //not rdx
			rcx >>= 0x30;           //shr rcx, 0x30
			rax ^= rcx;             //xor rax, rcx
			rax *= (uintptr_t)(rdx + 0x15);                //imul rax, [rdx+0x15]
			rax += r11;             //add rax, r11
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xB6C3A6FE99C92A23;               //mov rcx, 0xB6C3A6FE99C92A23
			rax *= rcx;             //imul rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x9;            //shr rcx, 0x09
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x12;           //shr rcx, 0x12
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x24;           //shr rcx, 0x24
			rax ^= rcx;             //xor rax, rcx
			rcx = 0xD7420EB04571AACF;               //mov rcx, 0xD7420EB04571AACF
			rax *= rcx;             //imul rax, rcx
			rcx = 0x578A3A3D4AF2D633;               //mov rcx, 0x578A3A3D4AF2D633
			rax += rcx;             //add rax, rcx
			return rax;
		}
		}
	};
	uint16_t get_bone_index(uint32_t bone_index) // DA FARE
	{
		uint64_t mb = g_data::base;
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;

		rdi = bone_index;
		rcx = rdi * 0x13C8;
		rax = 0xCC70CD3D3E0A7B49;               //mov rax, 0xCC70CD3D3E0A7B49
		rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
		r11 = g_data::base;          //lea r11, [0xFFFFFFFFFD7A3475]
		r10 = 0x45F86A52798F52B7;               //mov r10, 0x45F86A52798F52B7
		rdx >>= 0xC;            //shr rdx, 0x0C
		rax = rdx * 0x1409;             //imul rax, rdx, 0x1409
		rcx -= rax;             //sub rcx, rax
		rax = 0xDC9D0ECFCB6E9379;               //mov rax, 0xDC9D0ECFCB6E9379
		r8 = rcx * 0x1409;              //imul r8, rcx, 0x1409
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rdx >>= 0xD;            //shr rdx, 0x0D
		rax = rdx * 0x2522;             //imul rax, rdx, 0x2522
		r8 -= rax;              //sub r8, rax
		rax = 0x49539E3B2D066EA3;               //mov rax, 0x49539E3B2D066EA3
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = r8;               //mov rax, r8
		rax -= rdx;             //sub rax, rdx
		rax >>= 0x1;            //shr rax, 0x01
		rax += rdx;             //add rax, rdx
		rax >>= 0x9;            //shr rax, 0x09
		rcx = rax * 0x31C;              //imul rcx, rax, 0x31C
		rax = 0xD79435E50D79435F;               //mov rax, 0xD79435E50D79435F
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rdx >>= 0x4;            //shr rdx, 0x04
		rcx += rdx;             //add rcx, rdx
		rax = rcx * 0x26;               //imul rax, rcx, 0x26
		rcx = r8 + r8 * 4;              //lea rcx, [r8+r8*4]
		rcx <<= 0x3;            //shl rcx, 0x03
		rcx -= rax;             //sub rcx, rax
		rax = (uint16_t)(rcx + r11 * 1 + 0xB29A180);          //movzx eax, word ptr [rcx+r11*1+0xB29A180]
		r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
		rax = r10;              //mov rax, r10
		rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
		rax = r10;              //mov rax, r10
		rdx >>= 0xB;            //shr rdx, 0x0B
		rcx = rdx * 0x1D45;             //imul rcx, rdx, 0x1D45
		r8 -= rcx;              //sub r8, rcx
		r9 = r8 * 0x39A6;               //imul r9, r8, 0x39A6
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rdx >>= 0xB;            //shr rdx, 0x0B
		rax = rdx * 0x1D45;             //imul rax, rdx, 0x1D45
		r9 -= rax;              //sub r9, rax
		rax = 0x88ECF206D1CD0DD7;               //mov rax, 0x88ECF206D1CD0DD7
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rax = 0xAAAAAAAAAAAAAAAB;               //mov rax, 0xAAAAAAAAAAAAAAAB
		rdx >>= 0xB;            //shr rdx, 0x0B
		rcx = rdx * 0xEF5;              //imul rcx, rdx, 0xEF5
		rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
		rdx >>= 0x1;            //shr rdx, 0x01
		rcx += rdx;             //add rcx, rdx
		rax = rcx + rcx * 2;            //lea rax, [rcx+rcx*2]
		rax += rax;             //add rax, rax
		rcx = r9 * 8 + 0x0;             //lea rcx, [r9*8]
		rcx -= rax;             //sub rcx, rax
		r15 = (uint16_t)(rcx + r11 * 1 + 0xB2A17B0);          //movsx r15d, word ptr [rcx+r11*1+0xB2A17B0]
		return r15;
	}

	uint64_t Cbuf_AddText2()
	{
		const uint64_t mb = g_data::base;
		auto Peb = __readgsqword(0x60);
		uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb, rdi = mb, rsi = mb, r8 = mb, r9 = mb, r10 = mb, r11 = mb, r12 = mb, r13 = mb, r14 = mb, r15 = mb;
		rcx = *(uintptr_t*)(g_data::base + 0xE9719D0);
		if (!rcx)
			return rcx;
		rbx = Peb;              //mov rbx, gs:[rdx]
		rdx = rbx;              //mov rdx, rbx
		rdx >>= 0x16;           //shr rdx, 0x16
		rdx &= 0xF;
		switch (rdx) {
		case 0:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CF7AE]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB39651]
			rdi = g_data::base + 0x5FEEE803;              //lea rdi, [0x000000005CA27E48]
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rcx ^= rax;             //xor rcx, rax
			rcx ^= rdi;             //xor rcx, rdi
			rax = 0xDD3817FED3EA23F1;               //mov rax, 0xDD3817FED3EA23F1
			rcx *= rax;             //imul rcx, rax
			rax = 0x7BA2A5D4E0CEEEB;                //mov rax, 0x7BA2A5D4E0CEEEB
			rcx ^= rax;             //xor rcx, rax
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rax = 0x753175DE656DAB8D;               //mov rax, 0x753175DE656DAB8D
			rcx ^= rax;             //xor rcx, rax
			rax = r11 + 0x6522;             //lea rax, [r11+0x6522]
			rax += rbx;             //add rax, rbx
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x11;           //shr rax, 0x11
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x22;           //shr rax, 0x22
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x8;            //shr rax, 0x08
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x10;           //shr rax, 0x10
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x20;           //shr rax, 0x20
			rcx ^= rax;             //xor rcx, rax
			return rcx;
		}
		case 1:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CF394]
			rcx ^= rbx;             //xor rcx, rbx
			rcx += rbx;             //add rcx, rbx
			rcx -= rbx;             //sub rcx, rbx
			rax = 0xCE3BDE74C4EC252D;               //mov rax, 0xCE3BDE74C4EC252D
			rax ^= rcx;             //xor rax, rcx
			rcx = rax;              //mov rcx, rax
			rcx >>= 0x28;           //shr rcx, 0x28
			rcx ^= rax;             //xor rcx, rax
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rcx ^= rbx;             //xor rcx, rbx
			rax = 0x699F610DC7DCA7ED;               //mov rax, 0x699F610DC7DCA7ED
			rcx *= rax;             //imul rcx, rax
			return rcx;
		}
		case 2:
		{
			r10 = *(uintptr_t*)(g_data::base + 0xA896156);          //mov r10, [0x00000000073CF095]
			r14 = g_data::base + 0x1D2ECC72;              //lea r14, [0x0000000019E25BAA]
			r12 = g_data::base + 0x3750392C;              //lea r12, [0x000000003403C858]
			rax = 0x335495AC1570D5CA;               //mov rax, 0x335495AC1570D5CA
			rcx ^= rax;             //xor rcx, rax
			rax = 0xBE6C18EC718145F3;               //mov rax, 0xBE6C18EC718145F3
			rcx *= rax;             //imul rcx, rax
			rcx ^= rbx;             //xor rcx, rbx
			rax = 0x4A2C55D3C67FC0D7;               //mov rax, 0x4A2C55D3C67FC0D7
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x1A;           //shr rax, 0x1A
			rcx ^= rax;             //xor rcx, rax
			rdx = rcx;              //mov rdx, rcx
			rdx >>= 0x34;           //shr rdx, 0x34
			rdx ^= rcx;             //xor rdx, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx = ~rcx;             //not rcx
			rcx *= r12;             //imul rcx, r12
			rcx += rdx;             //add rcx, rdx
			rcx *= *(uintptr_t*)(r10 + 0x7);          //imul rcx, [r10+0x07]
			rcx ^= rbx;             //xor rcx, rbx
			rcx ^= r14;             //xor rcx, r14
			return rcx;
		}
		case 3:
		{
			r10 = *(uintptr_t*)(g_data::base + 0xA896156);          //mov r10, [0x00000000073CEBEA]
			r12 = g_data::base + 0x54DD9551;              //lea r12, [0x0000000051911FDE]
			r13 = g_data::base + 0xFD29;          //lea r13, [0xFFFFFFFFFCB487AA]
			rax = rcx;              //mov rax, rcx
			rax >>= 0xE;            //shr rax, 0x0E
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x1C;           //shr rax, 0x1C
			rcx ^= rax;             //xor rcx, rax
			rdx = rcx;              //mov rdx, rcx
			rdx >>= 0x38;           //shr rdx, 0x38
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rdx ^= rax;             //xor rdx, rax
			rcx ^= rdx;             //xor rcx, rdx
			rcx ^= r13;             //xor rcx, r13
			rdx = rbx;              //mov rdx, rbx
			rdx = ~rdx;             //not rdx
			rdx *= r12;             //imul rdx, r12
			rax = rbx + rcx * 1;            //lea rax, [rbx+rcx*1]
			rcx = rdx;              //mov rcx, rdx
			rcx ^= rax;             //xor rcx, rax
			rax = 0x7005C7E741090CD7;               //mov rax, 0x7005C7E741090CD7
			rcx ^= rax;             //xor rcx, rax
			rax = 0xDE02A9AF298BE023;               //mov rax, 0xDE02A9AF298BE023
			rcx *= rax;             //imul rcx, rax
			rax = 0x5089C442F21734EC;               //mov rax, 0x5089C442F21734EC
			rcx ^= rax;             //xor rcx, rax
			rcx *= *(uintptr_t*)(r10 + 0x7);          //imul rcx, [r10+0x07]
			return rcx;
		}
		case 4:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CE7C8]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB3866B]
			rcx -= r11;             //sub rcx, r11
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rcx -= rbx;             //sub rcx, rbx
			rcx -= rbx;             //sub rcx, rbx
			rax = 0x29D843DAD7832F77;               //mov rax, 0x29D843DAD7832F77
			rcx *= rax;             //imul rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0xD;            //shr rax, 0x0D
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x1A;           //shr rax, 0x1A
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x34;           //shr rax, 0x34
			rcx ^= rax;             //xor rcx, rax
			rax = rbx;              //mov rax, rbx
			rax -= r11;             //sub rax, r11
			rax -= 0x7333;          //sub rax, 0x7333
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x26;           //shr rax, 0x26
			rcx ^= rax;             //xor rcx, rax
			return rcx;
		}
		case 5:
		{
			r10 = *(uintptr_t*)(g_data::base + 0xA896156);          //mov r10, [0x00000000073CE2A3]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB38146]
			r12 = g_data::base + 0x21EC989C;              //lea r12, [0x000000001EA019D6]
			r14 = g_data::base + 0x1F49;          //lea r14, [0xFFFFFFFFFCB3A077]
			rax = rcx;              //mov rax, rcx
			rax >>= 0x12;           //shr rax, 0x12
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x24;           //shr rax, 0x24
			rcx ^= rax;             //xor rcx, rax
			rdx = rcx;              //mov rdx, rcx
			rdx >>= 0x25;           //shr rdx, 0x25
			rdx ^= rcx;             //xor rdx, rcx
			rcx = rbx;              //mov rcx, rbx
			rcx *= r14;             //imul rcx, r14
			rax = 0x1B16FBB771B2D3B2;               //mov rax, 0x1B16FBB771B2D3B2
			rcx += rdx;             //add rcx, rdx
			rcx ^= rbx;             //xor rcx, rbx
			rcx ^= r12;             //xor rcx, r12
			rcx ^= rax;             //xor rcx, rax
			rcx *= *(uintptr_t*)(r10 + 0x7);          //imul rcx, [r10+0x07]
			rax = 0xC2CA1395360DDC9B;               //mov rax, 0xC2CA1395360DDC9B
			rcx *= rax;             //imul rcx, rax
			rax = r11 + 0x32e984fa;                 //lea rax, [r11+0x32E984FA]
			rax += rbx;             //add rax, rbx
			rcx += rax;             //add rcx, rax
			return rcx;
		}
		case 6:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CDE94]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB37D37]
			rdi = g_data::base + 0x87C5;          //lea rdi, [0xFFFFFFFFFCB404F0]
			rax = 0x72F617EE89A68133;               //mov rax, 0x72F617EE89A68133
			rcx -= rax;             //sub rcx, rax
			rax = 0x16FB036F49F86AB5;               //mov rax, 0x16FB036F49F86AB5
			rcx -= rax;             //sub rcx, rax
			rax = 0x3EBD8F78A447DCB3;               //mov rax, 0x3EBD8F78A447DCB3
			rcx *= rax;             //imul rcx, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rax *= rdi;             //imul rax, rdi
			rcx += rax;             //add rcx, rax
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rax = rcx;              //mov rax, rcx
			rax >>= 0x24;           //shr rax, 0x24
			rcx ^= rax;             //xor rcx, rax
			rcx += r11;             //add rcx, r11
			rax = rcx;              //mov rax, rcx
			rax >>= 0x1E;           //shr rax, 0x1E
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x3C;           //shr rax, 0x3C
			rcx ^= rax;             //xor rcx, rax
			return rcx;
		}
		case 7:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CDA54]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB378F7]
			rcx ^= rbx;             //xor rcx, rbx
			rax = 0x56ADC6E1F6215F76;               //mov rax, 0x56ADC6E1F6215F76
			rcx -= rax;             //sub rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x1F;           //shr rax, 0x1F
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x3E;           //shr rax, 0x3E
			rcx ^= rax;             //xor rcx, rax
			rax = 0x418E1A9F43D5F0B7;               //mov rax, 0x418E1A9F43D5F0B7
			rcx -= rax;             //sub rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x23;           //shr rax, 0x23
			rcx ^= rax;             //xor rcx, rax
			rcx += r11;             //add rcx, r11
			rax = 0x870966678336F5D1;               //mov rax, 0x870966678336F5D1
			rcx *= rax;             //imul rcx, rax
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			return rcx;
		}
		case 8:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CD591]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB37434]
			rdi = g_data::base + 0x7674;          //lea rdi, [0xFFFFFFFFFCB3EA9C]
			rax = 0x4D76D2E733EA642D;               //mov rax, 0x4D76D2E733EA642D
			rcx -= rax;             //sub rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x25;           //shr rax, 0x25
			rcx ^= rax;             //xor rcx, rax
			rcx += rbx;             //add rcx, rbx
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rax *= rdi;             //imul rax, rdi
			rcx ^= rax;             //xor rcx, rax
			rax = 0x92775392F2B30283;               //mov rax, 0x92775392F2B30283
			rcx += rax;             //add rcx, rax
			rcx ^= r11;             //xor rcx, r11
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rax = 0x19B2FDFE979714D3;               //mov rax, 0x19B2FDFE979714D3
			rcx *= rax;             //imul rcx, rax
			return rcx;
		}
		case 9:
		{
			r10 = *(uintptr_t*)(g_data::base + 0xA896156);          //mov r10, [0x00000000073CD167]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB3700A]
			r12 = g_data::base + 0x5EAC6B09;              //lea r12, [0x000000005B5FDB07]
			rdx = r12;              //mov rdx, r12
			rdx = ~rdx;             //not rdx
			rdx += rbx;             //add rdx, rbx
			rdx += rcx;             //add rdx, rcx
			rdx *= *(uintptr_t*)(r10 + 0x7);          //imul rdx, [r10+0x07]
			rax = rdx;              //mov rax, rdx
			rax >>= 0x14;           //shr rax, 0x14
			rdx ^= rax;             //xor rdx, rax
			rax = 0xD6D7CC9A8EF21659;               //mov rax, 0xD6D7CC9A8EF21659
			rcx = rdx;              //mov rcx, rdx
			rcx >>= 0x28;           //shr rcx, 0x28
			rcx ^= rdx;             //xor rcx, rdx
			rcx *= rax;             //imul rcx, rax
			rcx ^= rbx;             //xor rcx, rbx
			rax = r11 + 0x8f7a;             //lea rax, [r11+0x8F7A]
			rax += rbx;             //add rax, rbx
			rcx += rax;             //add rcx, rax
			rax = 0x38E425CACF84132E;               //mov rax, 0x38E425CACF84132E
			rcx -= rax;             //sub rcx, rax
			rax = 0xDB978E52E0E2741F;               //mov rax, 0xDB978E52E0E2741F
			rcx ^= rax;             //xor rcx, rax
			return rcx;
		}
		case 10:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CCD73]
			rcx += rbx;             //add rcx, rbx
			rax = rcx;              //mov rax, rcx
			rax >>= 0x23;           //shr rax, 0x23
			rcx ^= rax;             //xor rcx, rax
			rcx ^= rbx;             //xor rcx, rbx
			rax = 0xA74A8E71D70AA17F;               //mov rax, 0xA74A8E71D70AA17F
			rcx *= rax;             //imul rcx, rax
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rax = 0xA0B5882737566D3B;               //mov rax, 0xA0B5882737566D3B
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x20;           //shr rax, 0x20
			rcx ^= rax;             //xor rcx, rax
			rax = 0xB69005B041ED2EBB;               //mov rax, 0xB69005B041ED2EBB
			rcx *= rax;             //imul rcx, rax
			return rcx;
		}
		case 11:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CC8EE]
			r10 = g_data::base + 0xA0FF;          //lea r10, [0xFFFFFFFFFCB40890]
			r11 = g_data::base + 0x3473B3A7;              //lea r11, [0x0000000031271B2C]
			rdi = g_data::base + 0x6766;          //lea rdi, [0xFFFFFFFFFCB3CEDF]
			rax = r10;              //mov rax, r10
			rax = ~rax;             //not rax
			rax *= rbx;             //imul rax, rbx
			rcx += rax;             //add rcx, rax
			rax = 0xF992878CDF1D94B9;               //mov rax, 0xF992878CDF1D94B9
			rcx *= rax;             //imul rcx, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rax *= rdi;             //imul rax, rdi
			rcx += rax;             //add rcx, rax
			rax = 0xBFBF32EDDCBB5245;               //mov rax, 0xBFBF32EDDCBB5245
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rcx ^= rax;             //xor rcx, rax
			rax = 0x5A7B2CB80B808EEE;               //mov rax, 0x5A7B2CB80B808EEE
			rcx -= rax;             //sub rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x1D;           //shr rax, 0x1D
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x3A;           //shr rax, 0x3A
			rcx ^= rax;             //xor rcx, rax
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rax ^= r11;             //xor rax, r11
			rcx += rax;             //add rcx, rax
			return rcx;
		}
		case 12:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CC58E]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB36431]
			rax = 0xF1C89D96095CC45B;               //mov rax, 0xF1C89D96095CC45B
			rcx *= rax;             //imul rcx, rax
			rax = 0x125FBEDDD013474C;               //mov rax, 0x125FBEDDD013474C
			rcx += rax;             //add rcx, rax
			rax = r11 + 0x7bdf7182;                 //lea rax, [r11+0x7BDF7182]
			rax += rbx;             //add rax, rbx
			rcx ^= rax;             //xor rcx, rax
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rcx -= r11;             //sub rcx, r11
			rax = rcx;              //mov rax, rcx
			rax >>= 0x25;           //shr rax, 0x25
			rcx ^= rax;             //xor rcx, rax
			rax = 0xC29333D3E233D9FF;               //mov rax, 0xC29333D3E233D9FF
			rcx ^= r11;             //xor rcx, r11
			rcx *= rax;             //imul rcx, rax
			return rcx;
		}
		case 13:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CC088]
			rdi = g_data::base + 0x2EC5;          //lea rdi, [0xFFFFFFFFFCB38DF0]
			rax = rcx;              //mov rax, rcx
			rax >>= 0x8;            //shr rax, 0x08
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x10;           //shr rax, 0x10
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x20;           //shr rax, 0x20
			rcx ^= rax;             //xor rcx, rax
			rax = 0x2B7582EF01DCBCBB;               //mov rax, 0x2B7582EF01DCBCBB
			rcx *= rax;             //imul rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x16;           //shr rax, 0x16
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x2C;           //shr rax, 0x2C
			rcx ^= rax;             //xor rcx, rax
			rcx += rbx;             //add rcx, rbx
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rax = rbx + 0x1;                //lea rax, [rbx+0x01]
			rax *= rdi;             //imul rax, rdi
			rcx += rax;             //add rcx, rax
			rax = 0x4DDDD579DE7D0C99;               //mov rax, 0x4DDDD579DE7D0C99
			rcx ^= rax;             //xor rcx, rax
			rax = 0x55FBAC3EBDBE48E5;               //mov rax, 0x55FBAC3EBDBE48E5
			rcx += rax;             //add rcx, rax
			return rcx;
		}
		case 14:
		{
			r9 = *(uintptr_t*)(g_data::base + 0xA896156);           //mov r9, [0x00000000073CBB41]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB359E4]
			rdi = g_data::base + 0xD880;          //lea rdi, [0xFFFFFFFFFCB43258]
			rcx += rbx;             //add rcx, rbx
			rax = 0xFE1573C840763E87;               //mov rax, 0xFE1573C840763E87
			rcx *= rax;             //imul rcx, rax
			rax = 0xD43AF299B3EC8497;               //mov rax, 0xD43AF299B3EC8497
			rcx += rax;             //add rcx, rax
			rcx *= *(uintptr_t*)(r9 + 0x7);           //imul rcx, [r9+0x07]
			rax = rbx;              //mov rax, rbx
			rax = ~rax;             //not rax
			rax ^= rdi;             //xor rax, rdi
			rcx -= rax;             //sub rcx, rax
			rax = 0xD10FFEA972FB6D1F;               //mov rax, 0xD10FFEA972FB6D1F
			rcx ^= rax;             //xor rcx, rax
			rcx ^= r11;             //xor rcx, r11
			rax = rcx;              //mov rax, rcx
			rax >>= 0x15;           //shr rax, 0x15
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x2A;           //shr rax, 0x2A
			rcx ^= rax;             //xor rcx, rax
			return rcx;
		}
		case 15:
		{
			r10 = *(uintptr_t*)(g_data::base + 0xA896156);          //mov r10, [0x00000000073CB768]
			r11 = g_data::base;           //lea r11, [0xFFFFFFFFFCB3560B]
			rcx *= *(uintptr_t*)(r10 + 0x7);          //imul rcx, [r10+0x07]
			rax = rcx;              //mov rax, rcx
			rax >>= 0xB;            //shr rax, 0x0B
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x16;           //shr rax, 0x16
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x2C;           //shr rax, 0x2C
			rcx ^= rax;             //xor rcx, rax
			rax = r11 + 0x3c738b03;                 //lea rax, [r11+0x3C738B03]
			rax += rbx;             //add rax, rbx
			rcx = rcx + rax * 2;            //lea rcx, [rcx+rax*2]
			rax = 0xD245D96F966E2C6F;               //mov rax, 0xD245D96F966E2C6F
			rcx ^= rax;             //xor rcx, rax
			rax = rcx;              //mov rax, rcx
			rax >>= 0x21;           //shr rax, 0x21
			rcx ^= rax;             //xor rcx, rax
			rax = 0x6183E02CC4F78DEF;               //mov rax, 0x6183E02CC4F78DEF
			rcx ^= rax;             //xor rcx, rax
			rax = 0xD37B9AFAE096CFDB;               //mov rax, 0xD37B9AFAE096CFDB
			rcx *= rax;             //imul rcx, rax
			return rcx;
		}
		}
	}

	void Cbuf_AddText(const char* text)
	{
		uintptr_t s_cmd_textArray = Cbuf_AddText2();
		if (!s_cmd_textArray)
			return;

		int textLength = strlen(text);
		int cmdsize = readMemory<int>(s_cmd_textArray + 0x10004);

		if (textLength + cmdsize > readMemory<DWORD>(s_cmd_textArray + 0x10000)) // CmdText::maxsize
			return;

		auto writeStringToMemory = [](uintptr_t address, const char* str) -> void {
			for (int i = 0; str[i]; ++i)
				writeMemory<char>(address + i, str[i]);
			};

		writeStringToMemory(s_cmd_textArray + cmdsize, text);
		writeMemory<DWORD>(s_cmd_textArray + 0x10004, readMemory<DWORD>(s_cmd_textArray + 0x10004) + textLength);
	}

	player_t get_player(int i)
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_valid_ptr(decryptedPtr))
		{
			uint64_t client_info = get_client_info_base();

			if (is_valid_ptr(client_info))
			{
				return player_t(client_info + (i * player_info::size));
			}
		}
		return player_t(NULL);
	}

	//player_t player_t

	//player_t get_local_player()
	//{
	//	auto addr = sdk::get_client_info_base() + (get_local_index() * player_info::size);
	//	if (is_bad_ptr(addr)) return 0;
	//	return addr;


	//}

	player_t get_local_player()
	{
		uint64_t decryptedPtr = get_client_info();

		if (is_bad_ptr(decryptedPtr))return player_t(NULL);

		auto local_index = *(uintptr_t*)(decryptedPtr + player_info::local_index);
		if (is_bad_ptr(local_index))return player_t(NULL);
		auto index = *(int*)(local_index + player_info::local_index_pos);
		return get_player(index);


	}

	/*name_t* get_name_ptr(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (bgs)
		{
			name_t* pClientInfo = (name_t*)(bgs + client::name_array_padding + ((i + i * 8) << 4));

			if (is_bad_ptr(pClientInfo))return 0;
			else
			return pClientInfo;

		}
		return 0;
	}*/

	refdef_t* get_refdef()
	{
		uint32_t crypt_0 = *(uint32_t*)(g_data::base + view_port::refdef_ptr);
		uint32_t crypt_1 = *(uint32_t*)(g_data::base + view_port::refdef_ptr + 0x4);
		uint32_t crypt_2 = *(uint32_t*)(g_data::base + view_port::refdef_ptr + 0x8);
		// lower 32 bits
		uint32_t entry_1 = (uint32_t)(g_data::base + view_port::refdef_ptr);
		uint32_t entry_2 = (uint32_t)(g_data::base + view_port::refdef_ptr + 0x4);
		// decryption
		uint32_t _low = entry_1 ^ crypt_2;
		uint32_t _high = entry_2 ^ crypt_2;
		uint32_t low_bit = crypt_0 ^ _low * (_low + 2);
		uint32_t high_bit = crypt_1 ^ _high * (_high + 2);
		auto ret = (refdef_t*)(((QWORD)high_bit << 32) + low_bit);
		if (is_bad_ptr(ret)) return 0;
		else
			return ret;
	}

	Vector3 get_camera_pos()
	{
		Vector3 pos = Vector3{};

		auto camera_ptr = *(uint64_t*)(g_data::base + view_port::camera_ptr);

		if (is_bad_ptr(camera_ptr))return pos;


		pos = *(Vector3*)(camera_ptr + view_port::camera_pos);
		if (pos.IsZero())return {};
		else
			return pos;
	}

	/*std::string get_player_name(int i)
	{
		uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		if (is_bad_ptr(bgs))return NULL;


		if (bgs)
		{
			name_t* clientInfo_ptr = (name_t*)(bgs + client::name_array_padding + (i * 0xD0));
			if (is_bad_ptr(clientInfo_ptr))return NULL;

			int length = strlen(clientInfo_ptr->name);
			for (int j = 0; j < length; ++j)
			{
				char ch = clientInfo_ptr->name[j];
				bool is_english = ch >= 0 && ch <= 127;
				if (!is_english)
					return xorstr_("Player");
			}
			return clientInfo_ptr->name;
		}
		return xorstr_("Player");
	}*/


	bool bones_to_screen(Vector3* BonePosArray, Vector2* ScreenPosArray, const long Count)
	{
		for (long i = 0; i < Count; ++i)
		{
			if (!world(BonePosArray[i], &ScreenPosArray[i]))
				return false;
		}
		return true;
	}



	bool get_bone_by_player_index(int i, int bone_id, Vector3* Out_bone_pos)
	{
		uint64_t decrypted_ptr = decrypt_bone_base();

		if (is_bad_ptr(decrypted_ptr))return false;

		unsigned short index = get_bone_index(i);
		if (index != 0)
		{
			uint64_t bone_ptr = *(uint64_t*)(decrypted_ptr + (index * bones::size) + 0xD8);

			if (is_bad_ptr(bone_ptr))return false;

			Vector3 bone_pos = *(Vector3*)(bone_ptr + (bone_id * 0x20) + 0x10);

			if (bone_pos.IsZero())return false;

			uint64_t client_info = get_client_info();

			if (is_bad_ptr(client_info))return false;



			Vector3 BasePos = *(Vector3*)(client_info + bones::bone_base_pos);

			if (BasePos.IsZero())return false;

			bone_pos.x += BasePos.x;
			bone_pos.y += BasePos.y;
			bone_pos.z += BasePos.z;

			*Out_bone_pos = bone_pos;
			return true;


		}

		return false;
	}

	int get_player_health(int i)
	{
		auto ms_cgameStatics = ((uintptr_t**)(g_data::base + client::name_array))[0];
		if (ms_cgameStatics == nullptr)
			return 0;

		auto clientInfo = (clientinfo_t*)(*(__int64(__fastcall**)(uintptr_t*, __int64))(*(__int64*)(__int64)ms_cgameStatics + 0xE8))(ms_cgameStatics, i);
		return clientInfo->health;

		//uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);

		//if (bgs)
		//{
		//	name_t* pClientInfo = (name_t*)(bgs + client::name_array_padding  +(i * 0xD8));

		//	if (pClientInfo)
		//	{
		//		return pClientInfo->get_health();
		//	}
		//}
		//return 0;
	}

	const char* get_player_name(int entityNum)
	{
		auto ms_cgameStatics = ((uintptr_t**)(g_data::base + client::name_array))[0];
		if (ms_cgameStatics == nullptr)
			return xorstr_("Player");

		auto clientInfo = (clientinfo_t*)(*(__int64(__fastcall**)(uintptr_t*, __int64))(*(__int64*)(__int64)ms_cgameStatics + 0xE8))(ms_cgameStatics, entityNum);
		return clientInfo->name;

		//uint64_t bgs = *(uint64_t*)(g_data::base + client::name_array);
		//if (is_bad_ptr(bgs)) return "";

		//if (bgs)
		//{
		//	name_t* clientInfo_ptr = (name_t*)(bgs + client::name_array_padding + (entityNum * 0xD8));
		//	if (is_bad_ptr(clientInfo_ptr)) return "";

		//	int length = strlen(clientInfo_ptr->name);
		//	for (int j = 0; j < length; ++j)
		//	{
		//		char ch = clientInfo_ptr->name[j];
		//		bool is_english = ch >= 0 && ch <= 127;
		//		if (!is_english)
		//			return xorstr_("Player");
		//	}
		//	return clientInfo_ptr->name;
		//}
		//return xorstr_("Player");
	}

	void start_tick()
	{
		static DWORD lastTick = 0;
		DWORD t = GetTickCount();
		bUpdateTick = lastTick < t;

		if (bUpdateTick)
			lastTick = t + nTickTime;
	}

	void update_vel_map(int index, Vector3 vPos)
	{
		if (!bUpdateTick)
			return;

		velocityMap[index].delta = vPos - velocityMap[index].lastPos;
		velocityMap[index].lastPos = vPos;
	}

	void clear_map()
	{
		if (!velocityMap.empty()) { velocityMap.clear(); }
	}

	Vector3 get_speed(int index)
	{
		return velocityMap[index].delta;
	}

	Vector3 get_prediction(int index, Vector3 source, Vector3 destination)
	{
		auto local_velocity = get_speed(local_index());
		auto target_velocity = get_speed(index);

		const auto distance = source.distance_to(destination);
		const auto travel_time = distance / globals::bullet_speed;
		auto pred_destination = destination + (target_velocity - local_velocity) * travel_time;
		/*position.x += travel_time * final_speed.x;
		position.y += travel_time * final_speed.y;
		position.z += 0.5 * globals::bullet_gravity * travel_time * travel_time;
		return position;*/

		pred_destination.z += 0.5f * std::fabsf(globals::bullet_gravity) * travel_time;

		return pred_destination;
	}


	Result MidnightSolver(float a, float b, float c)
	{
		Result res;

		double subsquare = b * b - 4 * a * c;

		if (subsquare < 0)
		{
			res.hasResult = false;
			return res;
		}
		else
		{
			res.hasResult = true,
				res.a = (float)((-b + sqrt(subsquare)) / (2 * a));
			res.b = (float)((-b - sqrt(subsquare)) / (2 * a));
		}
		return res;
	}

	Vector3 prediction_solver(Vector3 local_pos, Vector3 position, int index, float bullet_speed)
	{
		Vector3 aimPosition = Vector3().Zero();
		auto target_speed = get_speed(index);

		local_pos -= position;

		float a = (target_speed.x * target_speed.x) + (target_speed.y * target_speed.y) + (target_speed.z * target_speed.z) - ((bullet_speed * bullet_speed) * 100);
		float b = (-2 * local_pos.x * target_speed.x) + (-2 * local_pos.y * target_speed.y) + (-2 * local_pos.z * target_speed.z);
		float c = (local_pos.x * local_pos.x) + (local_pos.y * local_pos.y) + (local_pos.z * local_pos.z);

		local_pos += position;

		Result r = MidnightSolver(a, b, c);

		if (r.a >= 0 && !(r.b >= 0 && r.b < r.a))
		{
			aimPosition = position + target_speed * r.a;
		}
		else if (r.b >= 0)
		{
			aimPosition = position + target_speed * r.b;
		}

		return aimPosition;

	}
	int decrypt_visible_flag(int i, QWORD valid_list)
	{
		auto ptr = valid_list + ((static_cast<unsigned long long>(i) * 9 + 0x152) * 8) + 0x10;
		DWORD dw1 = (*(DWORD*)(ptr + 4) ^ (DWORD)ptr);
		DWORD dw2 = ((dw1 + 2) * dw1);
		BYTE dec_visible_flag = *(BYTE*)(ptr) ^ BYTE1(dw2) ^ (BYTE)dw2;

		return (int)dec_visible_flag;
	}

	ClientBits get_visible_base()
	{
		uint64_t cg = get_client_info();
		if (cg)
			return *(ClientBits*)(cg + bones::sightedEnemyFools);

		//for (int32_t j{}; j <= 0x1770; ++j)
		//{
		//	
		//	uint64_t vis_base_ptr = *(uint64_t*)(g_data::base + bones::distribute) + (j * 0x190);
		//	uint64_t cmp_function = *(uint64_t*)(vis_base_ptr + 0x38);

		//	if (!cmp_function)
		//		continue;

		//	//LOGS_ADDR(cmp_function);

		//	uint64_t about_visible = g_data::base + bones::visible;

		//	if (cmp_function == about_visible)
		//	{

		//		g_data::current_visible_offset = vis_base_ptr;
		//		return g_data::current_visible_offset;
		//	}

		//}
		return {};
	}
	bool is_visible(int entityNum) {

		uint64_t cg = get_client_info();
		if (!cg) return false;

		auto& sightedEnemyFools = *(ClientBits*)(cg + bones::sightedEnemyFools);
		auto bitmask = 0x80000000 >> (entityNum & 0x1F);
		return sightedEnemyFools.array[entityNum >> 5] & bitmask;

		//if (!g_data::last_visible_offset)
		//	return false;

		//uint64_t VisibleList = *(uint64_t*)(g_data::last_visible_offset + 0x80);
		//if (!VisibleList)
		//	return false;
		//uint64_t v421 = VisibleList + (entityNum * 9 + 0x152) * 8;
		//if (!v421)
		//	return false;
		//DWORD VisibleFlags = (v421 + 0x10) ^ *(DWORD*)(v421 + 0x14);
		//if (!VisibleFlags)
		//	return false;
		//DWORD v1630 = VisibleFlags * (VisibleFlags + 2);
		//if (!v1630)
		//	return false;
		//BYTE VisibleFlags1 = *(DWORD*)(v421 + 0x10) ^ v1630 ^ BYTE1(v1630);
		//if (VisibleFlags1 == 3) {
		//	return true;
		//}
		//return false;
	}

	void update_last_visible()
	{
		g_data::last_visible_offset = g_data::current_visible_offset;
	}

	// player class methods
	bool player_t::is_valid() {
		if (is_bad_ptr(address))return 0;

		return *(bool*)((uintptr_t)address + player_info::valid);
	}

	bool player_t::is_dead() {
		if (is_bad_ptr(address))return 0;

		auto dead1 = *(bool*)((uintptr_t)address + player_info::dead_1);
		auto dead2 = *(bool*)((uintptr_t)address + player_info::dead_2);
		return !(!dead1 && !dead2 && is_valid());
	}

	BYTE player_t::team_id() {

		if (is_bad_ptr(address))return 0;
		return *(BYTE*)((uintptr_t)address + player_info::team_id);
	}

	int player_t::get_stance() {

		if (is_bad_ptr(address))return 4;
		auto ret = *(int*)((uintptr_t)address + player_info::stance);


		return ret;
	}


	float player_t::get_rotation()
	{
		if (is_bad_ptr(address))return 0;
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (is_bad_ptr(local_pos_ptr))return 0;

		auto rotation = *(float*)(local_pos_ptr + 0x58);

		if (rotation < 0)
			rotation = 360.0f - (rotation * -1);

		rotation += 90.0f;

		if (rotation >= 360.0f)
			rotation = rotation - 360.0f;

		return rotation;
	}

	Vector3 player_t::get_pos()
	{
		if (is_bad_ptr(address))return {};
		auto local_pos_ptr = *(uintptr_t*)((uintptr_t)address + player_info::position_ptr);

		if (is_bad_ptr(local_pos_ptr))return{};
		else
			return *(Vector3*)(local_pos_ptr + 0x48);
		return Vector3{};


	}

	uint32_t player_t::get_index()
	{
		if (is_bad_ptr(this->address))return 0;

		auto cl_info_base = get_client_info_base();
		if (is_bad_ptr(cl_info_base))return 0;


		return ((uintptr_t)this->address - cl_info_base) / player_info::size;


	}

	bool player_t::is_visible()
	{
		//if (is_bad_ptr(g_data::visible_base))return false;

		//if (is_bad_ptr(this->address))return false;
		//
		//	uint64_t VisibleList =*(uint64_t*)(g_data::visible_base + 0x108);
		//	if (is_bad_ptr(VisibleList))
		//		return false;

		//	uint64_t rdx = VisibleList + (player_t::get_index() * 9 + 0x14E) * 8;
		//	if (is_bad_ptr(rdx))
		//		return false;

		//	DWORD VisibleFlags = (rdx + 0x10) ^ *(DWORD*)(rdx + 0x14);
		//	if (!VisibleFlags)
		//		return false;

		//	DWORD v511 = VisibleFlags * (VisibleFlags + 2);
		//	if (!v511)
		//		return false;

		//	BYTE VisibleFlags1 = *(DWORD*)(rdx + 0x10) ^ v511 ^ BYTE1(v511);
		//	if (VisibleFlags1 == 3) {
		//		return true;
		//	}

		return false;
	}




}


#include "stdafx.h"
#include "Menu.h"
#include "imgui/imgui.h"
# include "globals.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "obfuscator.hpp"
#include "xor.hpp"
#include"memory.h"
#include "mem.h"
#include "xorstr.hpp"
#include "configs.hpp"
#include "style.h"

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))
bool b_menu_open = true;
bool b_debug_open = false;
bool boxcheck;
int Selected_Camo_MW = 0;
int Selected_Camo_CW = 0;
int Selected_Camo_VG = 0;
int gameMode2 = 0;
int i_MenuTab = 0;
uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	if (ptr != 0)
	{
		uintptr_t addr = ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			addr = *(uintptr_t*)addr;
			addr += offsets[i];
		}
		return addr;
	}
	else
		return 0;
}


//uint64_t BASEIMAGE2 = reinterpret_cast<uint64_t>(GetModuleHandleA(NULL));

bool b_fov = false;
float f_fov = 1.20f;
float f_map = 1.0f;
bool b_map = false;
bool b_brightmax = false;
bool b_thirdperson = false;
bool b_heartcheat = false;
bool b_norecoil = false;
bool b_no_flashbang = false;

struct unnamed_type_integer
{
	int min;
	int max;
};
struct unnamed_type_integer64
{
	__int64 min;
	__int64 max;
};
struct unnamed_type_enumeration
{
	int stringCount;
	const char* strings;
};
/* 433 */
struct unnamed_type_unsignedInt64
{
	unsigned __int64 min;
	unsigned __int64 max;
};

/* 434 */
struct unnamed_type_value
{
	float min;
	float max;
	float devguiStep;
};

/* 435 */
struct unnamed_type_vector
{
	float min;
	float max;
	float devguiStep;
};




uintptr_t cbuff1;
uintptr_t cbuff2;
char inputtext[50];
int C_TagMOde = 0;


__int64 find_pattern(__int64 range_start, __int64 range_end, const char* pattern) {
	const char* pat = pattern;
	__int64 firstMatch = NULL;
	__int64 pCur = range_start;
	__int64 region_end;
	MEMORY_BASIC_INFORMATION mbi{};
	while (sizeof(mbi) == VirtualQuery((LPCVOID)pCur, &mbi, sizeof(mbi))) {
		if (pCur >= range_end - strlen(pattern))
			break;
		if (!(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READWRITE))) {
			pCur += mbi.RegionSize;
			continue;
		}
		region_end = pCur + mbi.RegionSize;
		while (pCur < region_end)
		{
			if (!*pat)
				return firstMatch;
			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
				if (!firstMatch)
					firstMatch = pCur;
				if (!pat[1] || !pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;
				else
					pat += 2;
			}
			else {
				if (firstMatch)
					pCur = firstMatch;
				pat = pattern;
				firstMatch = 0;
			}
			pCur++;
		}
	}
	return NULL;
}


bool init_once = true;
char input[30];
bool Unlock_once = true;

void KeyBindButton(int& key, int width, int height)
{
	static auto b_get = false;
	static std::string sz_text = xorstr_("Click to bind.");

	if (ImGui::Button(sz_text.c_str(), ImVec2(static_cast<float>(width), static_cast<float>(height))))
		b_get = true;

	if (b_get)
	{
		for (auto i = 1; i < 256; i++)
		{
			if (GetAsyncKeyState(i) & 0x8000)
			{
				if (i != 12)
				{
					key = i == VK_ESCAPE ? -1 : i;
					b_get = false;
				}
			}
		}
		sz_text = xorstr_("Press");
	}
	else if (!b_get && key == -1)
		sz_text = xorstr_("Click to bind.");
	else if (!b_get && key != -1)
	{
		sz_text = xorstr_("Key ") + std::to_string(key);
	}
}

void CL_PlayerData_SetCustomClanTag(int controllerIndex, const char* clanTag) {
	uintptr_t address = g_data::base + globals::clantagbase;
	((void(*)(int, const char*))address)(controllerIndex, clanTag);
}
void ShowToastNotificationAfterUserJoinedParty(const char* message)
{
	uintptr_t address = g_data::base + 0x2512EF0; // CC 48 89 74 24 ? 57 48 83 EC 20 4C 8B 05 ? ? ? ? 33 + 1 
	((void(*)(int, int, int, const char*, int))address)(0, 0, 0, message, 0);
}
void SetCamo(int Class, int Weapon, int Camo)
{
	char context[255];
	char state[255];
	int navStringCount;
	char* navStrings[16]{};
	const char* mode = "";

	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}



	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);

	DDL_GetRootState((__int64)state, ddl_file);
	char buffer[200];
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("squadMembers.loadouts.%i.weaponSetups.%i.camo"), Class, Weapon);
	ParseShit(buffer, (const char**)navStrings, 16, &navStringCount);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
	{
		DDL_SetInt((__int64)state, (__int64)context, Camo);
	}

}

void setWeapon(int loadout, int weapon, int weaponId)
{
	char context[255];
	char state[255];
	int string_count;
	char* str[16]{};
	const char* mode = "";
	char buffer[200];

	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}
	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);
	DDL_GetRootState((__int64)state, ddl_file);
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("squadMembers.loadouts.%i.weaponSetups.%i.weapon"), loadout, weapon);
	ParseShit(buffer, (const char**)str, 255, &string_count);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, string_count, (const char**)str))
	{
		DDL_SetInt((__int64)state, (__int64)context, weaponId);
	}
}
void setOperator(const char operators[255], int operatorid)
{
	char context[255];
	char state[255];
	int string_count;
	char* str[16]{};
	const char* mode = "";
	char buffer[200];

	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}
	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);
	DDL_GetRootState((__int64)state, ddl_file);
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("customizationSetup.operatorCustomization.%s.skin"), operators);
	ParseShit(buffer, (const char**)str, 255, &string_count);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, string_count, (const char**)str))
	{
		DDL_SetInt((__int64)state, (__int64)context, operatorid);
	}
}
void CopyWeapon(int Class)
{
	char context[255];
	char state[255];
	char context2[255];
	char state2[255];
	int navStringCount;
	char* navStrings[16]{};
	int navStringCount2;
	char* navStrings2[16]{};
	const char* mode = "";
	int wep = 0;
	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context, 0, 0, 5);
	}



	__int64 ddl_file = Com_DDL_LoadAsset((__int64)mode);

	DDL_GetRootState((__int64)state, ddl_file);
	char buffer[200];
	memset(buffer, 0, 200);
	sprintf_s(buffer, xorstr_("squadMembers.loadouts.%i.weaponSetups.0.weapon"), Class);
	ParseShit(buffer, (const char**)navStrings, 16, &navStringCount);
	if (DDL_MoveToPath((__int64*)&state, (__int64*)&state, navStringCount, (const char**)navStrings))
	{
		wep = DDL_GetInt((__int64*)&state, (__int64*)&context);

	}
	if (gameMode2 == 0)
	{
		mode = xorstr_("ddl/mp/rankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context2, 0, 0, 3);
	}
	else if (gameMode2 == 1)
	{
		mode = xorstr_("ddl/mp/wzrankedloadouts.ddl");
		CL_PlayerData_GetDDLBuffer((__int64)context2, 0, 0, 5);
	}
	__int64 ddl_file2 = Com_DDL_LoadAsset((__int64)mode);
	DDL_GetRootState((__int64)state2, ddl_file2);
	char buffer2[200];
	memset(buffer2, 0, 200);
	sprintf_s(buffer2, xorstr_("squadMembers.loadouts.%i.weaponSetups.1.weapon"), Class);
	ParseShit(buffer2, (const char**)navStrings2, 16, &navStringCount2);
	if (DDL_MoveToPath((__int64*)&state2, (__int64*)&state2, navStringCount2, (const char**)navStrings2))
	{
		DDL_SetInt2((__int64*)&state2, (__int64*)&context2, wep);
	}


}

int tabs;
int subtabs = 0;

void stylewindow()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = 9.0f;
	style.ChildRounding = 9.0f;
	style.GrabRounding = 9.0f;
	style.ScrollbarSize = 4.0f;

	style.Colors[ImGuiCol_Button] = ImVec4(40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(128.0f / 255.0f, 0.0f / 255.0f, 128.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(45.0f / 255.0f, 45.0f / 255.0f, 45.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 0.2f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 0.5f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(148.0f / 255.0f, 0.f / 255.0f, 211.0f / 255.0f, 0.5f);
}

void MyColorPicker(const char* label, ImColor* color)
{
	ImGui::PushID(label); // Push per identificare univocamente i controlli

	ImGui::Text("%s", label);
	ImGui::SameLine();

	if (ImGui::ColorButton("##ColorButton", *color))
	{
		ImGui::OpenPopup("ColorPickerPopup");
	}

	if (ImGui::BeginPopup("ColorPickerPopup"))
	{
		ImGui::ColorPicker4("##picker", (float*)color);
		ImGui::EndPopup();
	}

	ImGui::PopID();
}

void MyColorPicker2(ImColor* color)
{

	if (ImGui::ColorButton("##ColorButton", *color))
	{
		ImGui::OpenPopup("ColorPickerPopup");
	}

	if (ImGui::BeginPopup("ColorPickerPopup"))
	{
		ImGui::ColorPicker4("##picker", (float*)color);
		ImGui::EndPopup();
	}
}

void Decoration()
{
	auto draw = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetWindowPos();

	draw->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 161, pos.y + 535), ImColor(32, 32, 32, int(255 * ImGui::GetStyle().Alpha)), 10.f, ImDrawCornerFlags_Left); // left bg
	draw->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + 161, pos.y + 535), ImColor(50, 50, 50, int(255 * ImGui::GetStyle().Alpha)), 10.f, ImDrawCornerFlags_Left, 1);  // left bg

	draw->AddRectFilled(ImVec2(pos.x + 160, pos.y), ImVec2(pos.x + 838, pos.y + 535), ImColor(26, 26, 26, int(255 * ImGui::GetStyle().Alpha)), 10.f, ImDrawCornerFlags_Right); // right bg
	draw->AddRect(ImVec2(pos.x + 160, pos.y), ImVec2(pos.x + 838, pos.y + 535), ImColor(50, 50, 50, int(255 * ImGui::GetStyle().Alpha)), 10.f, ImDrawCornerFlags_Right, 1);  // right bg
}

void aimbonesec()
{
	ImGui::Checkbox(xorstr_("Use Bones"), &globals::target_bone);
	if (globals::target_bone)
	{
		static const char* items[] = { "Helmet","Head","Neck","Chest","Mid","Tummy","Pelvis","Right Food","Left Food" };
		static const char* current_item = "Select Bone";


		if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
					current_item = items[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}
		if (current_item == "Helmet")
			globals::b_helmet = true;
		else globals::b_helmet = false;
		if (current_item == "Head")
			globals::b_head = true;
		else globals::b_head = false;
		if (current_item == "Neck")
			globals::b_neck = true;
		else globals::b_neck = false;
		if (current_item == "Chest")
			globals::b_chest = true;
		else globals::b_chest = false;
		if (current_item == "Mid")
			globals::b_mid = true;
		else globals::b_mid = false;
		if (current_item == "Tummy")
			globals::b_tummy = true;
		else globals::b_tummy = false;
		if (current_item == "Pelvis")
			globals::b_pelvis = true;
		else globals::b_pelvis = false;
		if (current_item == "Right Food")
			globals::b_rightfood = true;
		else globals::b_rightfood = false;
		if (current_item == "Left Food")
			globals::b_leftfood = true;
		else globals::b_leftfood = false;
	}
}

void fovtypesec()
{
	ImGui::Checkbox(xorstr_("Show Fov"), &globals::fov_type);
	if (globals::fov_type)
	{
		static const char* items[] = { "Normal","Rainbow" };
		static const char* current_item = "Fov Type";


		if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
					current_item = items[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}
		if (current_item == "Normal")
			globals::b_fov = true;
		else globals::b_fov = false;
		if (current_item == "Rainbow")
			globals::fov_rainbow = true;
		else globals::fov_rainbow = false;
	}

}

void snaplinepos()
{
	if (globals::snapline_type)
	{
		static const char* items[] = { "Center","Bottom", "Top UP" };
		static const char* current_item = "Line Position";


		if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], is_selected))
					current_item = items[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}
		if (current_item == "Center")
			globals::centerpos = true;
		else globals::centerpos = false;
		if (current_item == "Bottom")
			globals::bottompos = true;
		else globals::bottompos = false;
		if (current_item == "Top UP")
			globals::topupos = true;
		else globals::topupos = false;
	}

}

void tabsmain()
{

	ImGui::PushFont(globals::TRIAL2);
	if (ImGui::Button(xorstr_("Aimbot"), ImVec2(150, 50)))
	{
		tabs = 0;
	}
	ImGui::SetCursorPos(ImVec2(7, 200));
	if (ImGui::Button(xorstr_("Visuals"), ImVec2(150, 50)))
	{
		tabs = 1;
	}
	ImGui::SetCursorPos(ImVec2(7, 270));
	if (ImGui::Button(xorstr_("Misc"), ImVec2(150, 50)))
	{
		tabs = 2;
	}
	ImGui::SetCursorPos(ImVec2(7, 340));
	if (ImGui::Button(xorstr_("Settings"), ImVec2(150, 50)))
	{
		tabs = 3;
	}
	ImGui::PopFont();
}

auto vec_files = Settings::GetList();

void UnlockAll()
{
	uintptr_t off_7FF7CBF39610 = g_data::base + 0xBBE8718; //0xBBE8718 //FF 50 08 84 C0 0F 84 ? ? ? ? 85 FF 0F 84
	uintptr_t loc_7FF7C2B97ACD = g_data::base + 0x294CFE7 + 0x2; //  0x294CFE7 //48 8D 15 ? ? ? ? 48 89 54 24 ? 8B D3 C4 61 FB 2C C0

	static BYTE shellcode[] = { 0x48, 0x83, 0xC4, 0x08, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE9, 0x00, 0x00, 0x00, 0x00, 0x01 };

	*(uintptr_t*)(shellcode + 6) = off_7FF7CBF39610 - 0x87;
	*(uintptr_t*)(shellcode + 15) = loc_7FF7C2B97ACD - off_7FF7CBF39610 + 0x83;

	memcpy((void*)(off_7FF7CBF39610 - 0x96), shellcode, sizeof(shellcode));
	*(uintptr_t*)(off_7FF7CBF39610 + 8) = off_7FF7CBF39610 - 0x96;
	*(uintptr_t*)off_7FF7CBF39610 = off_7FF7CBF39610;
}

void mainmenu()
{
	ImGui::PushFont(globals::TRIAL1);
	if (tabs == 0)
	{
		ImGui::SetCursorPos(ImVec2(190, 20));
		ImGui::BeginChild(xorstr_("#subtabsgeneral1"), ImVec2(610, 50), false);

		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(190, 100));
		ImGui::BeginChild(xorstr_("#aimgeneral1"), ImVec2(300, 400), false);
		ImGui::Checkbox(xorstr_("Enable Aim"), &globals::b_lock);
		if (globals::b_lock)
		{
			ImGui::SameLine();
			KeyBindButton(globals::aim_key, 100, 25);
		}
		ImGui::SliderInt(xorstr_("Smooth"), &globals::aim_smooth, 1, 20);
		aimbonesec();
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild(xorstr_("#aimgeneral2"), ImVec2(300, 400), false);
		ImGui::Checkbox(xorstr_("Show Crosshair"), &globals::b_crosshair);
		fovtypesec();
		ImGui::SliderFloat(xorstr_("Aim Fov"), &globals::f_fov_size, 5.f, 500.f);
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Checkbox(xorstr_("Show Aim Hitbox"), &globals::b_aim_point);
		ImGui::EndChild();
	}
	else if (tabs == 1)
	{
		ImGui::SetCursorPos(ImVec2(190, 20));
		ImGui::BeginChild(xorstr_("#subtabsgeneral2"), ImVec2(610, 50), false);
		ImGui::Dummy(ImVec2(5, 5));
		if (ImGui::Button(xorstr_("General"), ImVec2(130, 35)))
		{
			subtabs = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Colors"), ImVec2(130, 35)))
		{
			subtabs = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Other"), ImVec2(130, 35)))
		{
			subtabs = 2;
		}
		ImGui::EndChild();

		if (subtabs == 0)
		{
			ImGui::SetCursorPos(ImVec2(190, 100));
			ImGui::BeginChild(xorstr_("#visualgeneral1"), ImVec2(300, 400), false);
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			ImGui::Checkbox(xorstr_("Enable Charms"), &globals::chams);
			ImGui::Checkbox(xorstr_("Show Box"), &globals::b_box);
			ImGui::Checkbox(xorstr_("Show Corner"), &globals::b_corner);
			ImGui::Checkbox(xorstr_("Show Skeleton"), &globals::b_skeleton);
			ImGui::Checkbox(xorstr_("Show Snapline"), &globals::snapline_type);
			snaplinepos();
			ImGui::Checkbox(xorstr_("Show Healtbar"), &globals::b_health);
			ImGui::Checkbox(xorstr_("Show Distance"), &globals::b_distance);
			ImGui::Checkbox(xorstr_("Show Names"), &globals::b_names);
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::BeginChild(xorstr_("#visualgeneral2"), ImVec2(300, 400), false);
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			ImGui::Text(xorstr_("Charms Thick"));
			ImGui::SliderInt(xorstr_("    "), &globals::chams_width, 1, 10);
			ImGui::Text(xorstr_("Box Thick"));
			ImGui::SliderFloat(xorstr_("         "), &globals::box_thick, 1.f, 3.f);
			ImGui::Text(xorstr_("Corner Thick"));
			ImGui::SliderFloat(xorstr_(" "), &globals::corner_thick, 1.f, 3.f);
			ImGui::Text(xorstr_("Skeleton Thick"));
			ImGui::SliderFloat(xorstr_(""), &globals::skel_thick, 1.f, 3.f);
			ImGui::Text(xorstr_("Snapline Thick"));
			ImGui::SliderFloat(xorstr_("  "), &globals::line_thick, 1.f, 3.f);
			ImGui::Separator();
			ImGui::Checkbox(xorstr_("Box Outline"), &globals::box_outline);
			ImGui::Checkbox(xorstr_("Corner Outline"), &globals::corner_outline);
			ImGui::Checkbox(xorstr_("Skeleton Outline"), &globals::skel_outline);
			ImGui::Checkbox(xorstr_("Snapline Outline"), &globals::line_outline);

			ImGui::EndChild();
		}
		if (subtabs == 1)
		{
			ImGui::SetCursorPos(ImVec2(190, 100));
			ImGui::BeginChild(xorstr_("#colorvisualgeneral1"), ImVec2(300, 400), false);
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			MyColorPicker(xorstr_("Charms Visible"), &color::VisibleColorEnemychams);
			MyColorPicker(xorstr_("Box Visible"), &color::VisibleColorEnemybox);
			MyColorPicker(xorstr_("Corner Visible"), &color::VisibleColorEnemycorner);
			MyColorPicker(xorstr_("Skeleton Visible"), &color::VisibleColorEnemyskeleton);
			MyColorPicker(xorstr_("Snapline Visible"), &color::VisibleColorEnemyline);
			ImGui::Separator();
			MyColorPicker(xorstr_("Team Box Visible"), &color::VisibleColorTeambox);
			MyColorPicker(xorstr_("Team Corner Vis"), &color::VisibleColorTeamcorner);
			MyColorPicker(xorstr_("Team Skeleton Vis"), &color::VisibleColorTeamskeleton);
			MyColorPicker(xorstr_("Team Snapline Vis"), &color::VisibleColorTeamline);
			MyColorPicker(xorstr_("Team Distance Vis"), &color::VisibleColorTeamdistance);
			MyColorPicker(xorstr_("Team Names Vis"), &color::VisibleColorTeamnames);
			ImGui::Separator();
			MyColorPicker(xorstr_("Name Visible"), &color::VisibleColorEnemynames);
			MyColorPicker(xorstr_("Distance Visible"), &color::VisibleColorEnemydistance);

			ImGui::EndChild();
			ImGui::SameLine();

			ImGui::BeginChild(xorstr_("#colorvisualgeneral2"), ImVec2(300, 400), false);
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			MyColorPicker(xorstr_("Charms Invisible"), &color::NotVisibleColorEnemychams);
			MyColorPicker(xorstr_("Box Invisible"), &color::NotVisibleColorEnemybox);
			MyColorPicker(xorstr_("Corner Invisible"), &color::NotVisibleColorEnemydcorner);
			MyColorPicker(xorstr_("Skeleton Invisible"), &color::NotVisibleColorEnemyskeleton);
			MyColorPicker(xorstr_("Snapline Invisible"), &color::NotVisibleColorEnemyline);
			ImGui::Separator();
			MyColorPicker(xorstr_("Team Box Invisible"), &color::NotVisibleColorTeambox);
			MyColorPicker(xorstr_("Team Corner Invis"), &color::NotVisibleColorTeamcorner);
			MyColorPicker(xorstr_("Team Skeleton Invis"), &color::NotVisibleColorTeamskeleton);
			MyColorPicker(xorstr_("Team Snapline Invis"), &color::NotVisibleColorTeamline);
			MyColorPicker(xorstr_("Team Distance Invis"), &color::NotVisibleColorTeamdistance);
			MyColorPicker(xorstr_("Team Names Invis"), &color::NotVisibleColorTeamnames);
			ImGui::Separator();
			MyColorPicker(xorstr_("Name Invisible"), &color::NotVisibleColorEnemynames);
			MyColorPicker(xorstr_("Distance Invisible"), &color::NotVisibleColorEnemydistance);
			ImGui::EndChild();
		}
	}
	else if (tabs == 2)
	{
		ImGui::SetCursorPos(ImVec2(190, 20));
		ImGui::BeginChild(xorstr_("#subtabsgeneral3"), ImVec2(610, 50), false);


		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(190, 100));
		ImGui::BeginChild(xorstr_("#miscgeneral1"), ImVec2(300, 400), false);


		if (ImGui::Button(xorstr_("UNLOCK")))
		{
			UnlockAll();
		}

		ImGui::EndChild();
	}
	else if (tabs == 3)
	{
		ImGui::SetCursorPos(ImVec2(190, 20));
		ImGui::BeginChild(xorstr_("#subtabsgeneral4"), ImVec2(610, 50), false);
		ImGui::Dummy(ImVec2(5, 5));
		if (ImGui::Button(xorstr_("General"), ImVec2(130, 35)))
		{
			subtabs = 3;
		}
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Load / Save"), ImVec2(150, 35)))
		{
			subtabs = 4;
		}
		ImGui::EndChild();

		if (subtabs == 3)
		{
			ImGui::SetCursorPos(ImVec2(190, 100));
			ImGui::BeginChild(xorstr_("#settingsgeneral5"), ImVec2(300, 400), false);
			ImGui::Checkbox(xorstr_("Enable StreamProof ( BETA )"), &globals::streampoof);
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild(xorstr_("#settingsgeneral7"), ImVec2(300, 400), false);
			ImGui::EndChild();
		}
		else if (subtabs == 4)
		{
			ImGui::SetCursorPos(ImVec2(190, 100));
			ImGui::BeginChild(xorstr_("#settingsgeneral1"), ImVec2(300, 400), false);
			if (ImGui::Button(xorstr_("Save Profile 1"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_1");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 2"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_2");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 3"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_3");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 4"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_4");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 5"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_5");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 6"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_6");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 7"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_7");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 8"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_8");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 9"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_9");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Save Profile 10"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_10");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Save_Settings(buf);
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild(xorstr_("#settingsgeneral2"), ImVec2(300, 400), false);
			if (ImGui::Button(xorstr_("Load Profile 1"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_1");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 2"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_2");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 3"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_3");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 4"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_4");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 5"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_5");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 6"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_6");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 7"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_7");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 8"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_8");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 9"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_9");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			if (ImGui::Button(xorstr_("Load Profile 10"), ImVec2(300, 35)))
			{
				std::string buf = xorstr_("Profile_10");

				if (!vec_files.empty() || vec_files.empty())
					Settings::Load_Settings(buf);
			}
			ImGui::EndChild();
		}
	}

	ImGui::PopFont();
}
namespace g_menu
{
	void menu()
	{
		if (GetAsyncKeyState(VK_INSERT) & 0x1)
		{
			b_menu_open = !b_menu_open;

		}
		if (init_once)
		{
			//init_buffer();
			ImGui::SetNextWindowPos(ImVec2(200, 200));
			init_once = false;
		}
		if (b_menu_open)
		{
			ImGui::SetNextWindowSize(ImVec2(838, 535), ImGuiCond_Always);
			ImGui::Begin(xorstr_(""), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			Decoration();
			tabsmain();
			mainmenu();
			stylewindow();
			ImGui::End();
		}
	}
}
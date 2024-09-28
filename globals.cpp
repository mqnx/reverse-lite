#include "globals.h"
#include "sdk.h"

namespace globals
{
	///////////////////\\\\\\\\\\\\\\\\\

	bool box_outline;
	bool line_outline;
	bool skel_outline;
	bool corner_outline;

	bool streampoof;
	bool watermark;
	bool showfps;

	float box_thick = 1.f;
	float line_thick = 1.f;
	float skel_thick = 1.f;
	float corner_thick = 1.f;

	bool no_gun;
	bool no_shield_shake;
	bool bypass_level_4;
	bool skip_wz_into;

	bool fov_type;
	bool fov_rainbow;

	bool fov_slider_enable;
	bool map_resize_enabel;
	bool perm200fov;

	float map_size;

	bool rainbow_box;
	bool rainbow_corner;
	bool rainbow_skeleton;
	bool rianbow_snapline;
	bool rainbow_name;
	bool rainbow_distance;

	bool chams = true;
	bool chams_only_visible;

	bool snapline_type;
	bool centerpos;
	bool bottompos;
	bool topupos;

	int sides = 50;
	int velocity = 5;

	///////////////////\\\\\\\\\\\\\\\\

	UINT64 ShowToastNotificationAfterUserJoinedParty = 0x20403E5;
	UINT64 uavbase = 0x13134F90 + 0x130;
	UINT64 clantagbase = 0; // 0x46C0B40
	uintptr_t Dvar_FindVarByName = 0x43CF6B0;
	uintptr_t Dvar_SetBoolInternal = 0x43D71E0;
	uintptr_t Dvar_SetInt_Internal = 0x43D8E92;
	uintptr_t Dvar_SetBoolByName = 0x43D71E0;
	uintptr_t Dvar_SetFloat_Internal = 0x43D7C50;
	uintptr_t Dvar_RegisterFloat;
	uintptr_t Dvar_SetIntByName;
	uintptr_t ddl_loadasset = 0x32CCCC0;
	uintptr_t ddl_getrootstate = 0x5AEFB60;
	uintptr_t ddl_getdllbuffer = 0x47C3430;
	uintptr_t ddl_movetoname = 0x5B25A70;
	uintptr_t ddl_movetopath = 0x5B25B50;//0x5B25A90; //0x5B25B50
	uintptr_t ddl_setint = 0x5B262D0;
	uintptr_t ddl_setstring = 0x569CC50;//0x5B260E0; //569CC50
	uintptr_t ddl_getint = 0x5B24D10;
	uintptr_t a_parse = 0x56BE9E0;
	UINT64 checkbase;
	UINT64 unlockallbase;
	char UnlockBytes[5] = "";
	bool b_helmet;
	bool b_mid;
	bool b_rightfood;
	bool b_leftfood;
	bool b_head;
	bool b_chest;
	bool b_neck;
	bool b_tummy;
	bool b_pelvis;
	bool b_spread;
	float f_fov = 120.f;
	float b_namesize = 15.0f;
	float b_distancesize = 15.0f;
	bool b_visible_only;
	bool b_unlockall;
	bool b_box = true;
	bool b_corner = false;
	bool b_line;
	bool b_skeleton = true;
	bool b_names = true;
	bool b_distance = true;
	bool b_visible;
	bool b_fov = true;
	bool b_lock = true;
	bool b_crosshair = true;
	bool b_friendly;
	bool b_recoil;
	int aim_key = 2;
	bool b_in_game;
	bool b_health;
	bool local_is_alive;
	bool b_rapid_fire;
	bool b_skip_knocked = false;
	bool b_prediction = false;
	bool b_aim_point = true;
	bool is_aiming;
	bool target_bone;
	bool b_aimbot;
	bool b_UAV;
	bool b_scale_Fov;
	bool third_person;
	bool gamepad;
	bool b_rainbow;
	bool b_unlock;
	bool b_tut;
	bool b_fog;
	//int game_mode_opt = 0;

	int bone_index = 1; //0 ~ 3
	int box_index = 0; //0 ~ 3
	//int lock_key = 1; //0 ~ 3
	int max_distance = 250; //50 ~ 1000
	int aim_smooth = 5; // 1 ~ 30
	int max_player_count = 0;
	int connecte_players = 0;
	int local_team_id;
	int fire_speed = 40;
	int call = 0;

	int player_index;
	float fov = 1.2f;
	Vector3 localpos1;
	Vector3 enemypos1;
	Vector3 enemypos2;
	Vector3 enemypos3;
	float f_fov_size = 90.0f; // 0 ~ 1000
	float aim_speed = 1.f;
	float bullet_speed = 2402.f; // 1 ~ 3000
	float bullet_gravity = 5.f;

	//float gravity = 1.f;
	const char* stance;
	const char* aim_lock_point[] = { "Helmet", "Head", "Neck", "Chest" };
	const char* box_types[] = { "2D Corner Box", "2D Box" };
	const char* theme_choose[] = { "Dark Mode","White Mode"};
	//const char* aim_lock_key[] = { "Left Button", "Right Button", "M4", "M5" };
	uintptr_t local_ptr;
	uintptr_t enemybase;
	sdk::refdef_t* refdefadd;
	ImFont* ESPNAME;
    ImFont* ESPDIST;

	ImFont* NAME;
	ImFont* GENERAL;

	ImFont* TRIAL1;
	ImFont* TRIAL2;
	int theme = 0;

}

namespace color
{
	ImColor Color{ 255,255,255,255 };
	ImColor VisibleColorTeam{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeam{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemy{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemy{ 0.97f, 0.01f, 0.01f, 1.00f };
	ImColor bfov{ 0.97f, 0.01f, 0.01f, 1.00f };
	ImColor draw_crosshair{ 0.f, 0.75f, 1.f, 1.f };
	ImColor nameColor{ 255,255,0,255 };
	ImColor dis_Color{ 255,255,0,255 };
	ImColor healthbar{ 0.15f, 1.f, 0.15f, 1.f };

	ImColor VisibleColorTeambox{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeambox{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemybox{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemybox{ 0.97f, 0.01f, 0.01f, 1.00f };

	ImColor VisibleColorTeamskeleton{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeamskeleton{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemyskeleton{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemyskeleton{ 0.97f, 0.01f, 0.01f, 1.00f };

	ImColor VisibleColorTeamline{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeamline{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemyline{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemyline{ 0.97f, 0.01f, 0.01f, 1.00f };

	ImColor VisibleColorTeamnames{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeamnames{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemynames{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemynames{ 0.97f, 0.01f, 0.01f, 1.00f };

	ImColor VisibleColorTeamdistance{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeamdistance{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemydistance{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemydistance{ 0.97f, 0.01f, 0.01f, 1.00f };

	ImColor VisibleColorTeamcorner{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeamcorner{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemycorner{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemydcorner{ 0.97f, 0.01f, 0.01f, 1.00f };

	ImColor VisibleColorTeamchams{ 0.f, 0.f, 1.f, 1.f };
	ImColor NotVisibleColorTeamchams{ 0.f, 0.75f, 1.f, 1.f };
	ImColor VisibleColorEnemychams{ 0.33f, 0.97f, 0.35f, 1.00f };
	ImColor NotVisibleColorEnemychams{ 0.97f, 0.01f, 0.01f, 1.00f };

}

namespace screenshot
{
	 bool visuals = true;
	 bool* pDrawEnabled = nullptr;
		uint32_t screenshot_counter = 0;
		uint32_t  bit_blt_log = 0;
	 const char* bit_blt_fail;
	 uintptr_t  bit_blt_anotherlog;

		uint32_t	GdiStretchBlt_log = 0;
	 const char* GdiStretchBlt_fail;
	 uintptr_t  GdiStretchBlt_anotherlog;

	uintptr_t	texture_copy_log = 0;




	 uintptr_t virtualqueryaddr = 0;
}


namespace loot
{
	ImColor lootcolor{ 0.f, 0.75f, 1.f, 1.f };
	bool name;
	bool distance;
	bool ar;
	bool smg;
	bool lmg;
	bool sniper;
	bool pistol;
	bool shotgun;

	bool ar_ammo;
	bool smg_ammo;
	bool sniper_ammo;
	bool shotgun_ammo;



}
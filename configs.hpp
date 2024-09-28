#include "globals.h"

namespace Settings
{
	std::string str_config_name = "";

	BOOL WritePrivateProfileInt(LPCSTR lpAppName, LPCSTR lpKeyName, int nInteger, LPCSTR lpFileName) {
		char lpString[1024];
		sprintf(lpString, "%d", nInteger);
		return WritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);
	}
	BOOL WritePrivateProfileFloat(LPCSTR lpAppName, LPCSTR lpKeyName, float nInteger, LPCSTR lpFileName) {
		char lpString[1024];
		sprintf(lpString, "%f", nInteger);
		return WritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);
	}
	float GetPrivateProfileFloat(LPCSTR lpAppName, LPCSTR lpKeyName, FLOAT flDefault, LPCSTR lpFileName)
	{
		char szData[32];

		GetPrivateProfileStringA(lpAppName, lpKeyName, std::to_string(flDefault).c_str(), szData, 32, lpFileName);

		return (float)atof(szData);
	}

	BOOL WritePrivateProfileImColor(LPCSTR lpAppName, LPCSTR lpKeyName, const ImColor& color, LPCSTR lpFileName) {
		char lpString[1024];
		sprintf(lpString, "%.2f %.2f %.2f %.2f", color.Value.x, color.Value.y, color.Value.z, color.Value.w);
		return WritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);
	}

	ImColor GetPrivateProfileImColor(LPCSTR lpAppName, LPCSTR lpKeyName, const ImColor& defaultColor, LPCSTR lpFileName) {
		char szData[1024];
		GetPrivateProfileStringA(lpAppName, lpKeyName, "", szData, 1024, lpFileName);

		float r, g, b, a;
		if (sscanf(szData, "%f %f %f %f", &r, &g, &b, &a) == 4) {
			return ImColor(r, g, b, a);
		}
		else {
			return defaultColor;
		}
	}

	void Save_Settings(std::string fileName)
	{
		char file_path[MAX_PATH];
		sprintf_s(file_path, xorstr_("C:\\Configs\\%s%s"), fileName.c_str(), xorstr_(".ini"));
		// OTHER
		WritePrivateProfileInt(xorstr_("settings"), xorstr_("streamproof"), globals::streampoof, file_path);
		// AIMBOB
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("enable_aim"), globals::b_lock, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("aim_key"), globals::aim_key, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("helmet"), globals::b_helmet, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("head"), globals::b_head, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("neck"), globals::b_neck, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("chest"), globals::b_chest, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("mid"), globals::b_mid, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("tummy"), globals::b_tummy, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("pelvis"), globals::b_pelvis, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("right_food"), globals::b_rightfood, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("left_food"), globals::b_leftfood, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("target_bone"), globals::target_bone, file_path);
		WritePrivateProfileFloat(xorstr_("aimbot"), xorstr_("aim_smooth"), globals::aim_smooth, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("crosshair"), globals::b_crosshair, file_path);
		WritePrivateProfileFloat(xorstr_("aimbot"), xorstr_("aim_fov"), globals::f_fov_size, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("skip_knocked"), globals::b_skip_knocked, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("show_aim_point"), globals::b_aim_point, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("prediction"), globals::b_prediction, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("show_fov"), globals::fov_type, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("normal_fov"), globals::target_bone, file_path);
		WritePrivateProfileInt(xorstr_("aimbot"), xorstr_("rainbow_fov"), globals::target_bone, file_path);

		// VISUALS
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_charms"), globals::chams, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_box"), globals::b_box, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_corner"), globals::b_corner, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_skeleton"), globals::b_skeleton, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_snapline"), globals::snapline_type, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("snapline_center"), globals::centerpos, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("snapline_bottom"), globals::bottompos, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("snapline_topup"), globals::topupos, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_healtbar"), globals::b_health, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_distance"), globals::b_distance, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("enable_names"), globals::b_names, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("box_outline"), globals::box_outline, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("corner_outline"), globals::corner_outline, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("snapline_outline"), globals::line_outline, file_path);
		WritePrivateProfileFloat(xorstr_("visuals"), xorstr_("chams_thick"), globals::chams_width, file_path);
		WritePrivateProfileFloat(xorstr_("visuals"), xorstr_("box_thick"), globals::box_thick, file_path);
		WritePrivateProfileFloat(xorstr_("visuals"), xorstr_("corner_thick"), globals::corner_thick, file_path);
		WritePrivateProfileFloat(xorstr_("visuals"), xorstr_("skeleton_thick"), globals::skel_thick, file_path);
		WritePrivateProfileFloat(xorstr_("visuals"), xorstr_("snapline_thick"), globals::line_thick, file_path);

		// RAINBOW SHIT
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("rainbow_box"), globals::rainbow_box, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("rainbow_corner"), globals::rainbow_corner, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("rainbow_chams"), globals::rainbow_charms, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("rainbow_skeleton"), globals::rainbow_skeleton, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("rainbow_snapline"), globals::rianbow_snapline, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("rainbow_name"), globals::rainbow_name, file_path);
		WritePrivateProfileInt(xorstr_("visuals"), xorstr_("rainbow_distance"), globals::rainbow_distance, file_path);

		WritePrivateProfileFloat(xorstr_("visuals"), xorstr_("rainbow_sides"), globals::sides, file_path);
		WritePrivateProfileFloat(xorstr_("visuals"), xorstr_("rainbow_velocity"), globals::velocity, file_path);

		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("chams_visible"), color::VisibleColorEnemychams, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("box_visible"), color::VisibleColorEnemybox, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("corner_visible"), color::VisibleColorEnemycorner, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("skeleton_visible"), color::VisibleColorEnemyskeleton, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("snapline_visible"), color::VisibleColorEnemyline, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_box_visible"), color::VisibleColorTeambox, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_corner_visible"), color::VisibleColorTeamcorner, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_skeleton_visible"), color::VisibleColorTeamskeleton, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_snapline_visible"), color::VisibleColorTeamline, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_distance_visible"), color::VisibleColorTeamdistance, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_names_visible"), color::VisibleColorTeamnames, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("name_visible"), color::VisibleColorEnemynames, file_path);
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("distance_visible"), color::VisibleColorTeamdistance, file_path);

		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("chams_invisible"), color::NotVisibleColorEnemychams, file_path); // Set the color for invisible enemy chams
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("box_invisible"), color::NotVisibleColorEnemybox, file_path); // Set the color for invisible enemy box
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("corner_invisible"), color::NotVisibleColorEnemydcorner, file_path); // Set the color for invisible enemy corner
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("skeleton_invisible"), color::NotVisibleColorEnemyskeleton, file_path); // Set the color for invisible enemy skeleton
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("snapline_invisible"), color::NotVisibleColorEnemyline, file_path); // Set the color for invisible enemy snapline
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_box_invisible"), color::NotVisibleColorTeambox, file_path); // Set the color for invisible team box
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_corner_invisible"), color::NotVisibleColorTeamcorner, file_path); // Set the color for invisible team corner
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_skeleton_invisible"), color::NotVisibleColorTeamskeleton, file_path); // Set the color for invisible team skeleton
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_snapline_invisible"), color::NotVisibleColorTeamline, file_path); // Set the color for invisible team snapline
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_distance_invisible"), color::NotVisibleColorTeamdistance, file_path); // Set the color for invisible team distance
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("team_names_invisible"), color::NotVisibleColorTeamnames, file_path); // Set the color for invisible team names
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("name_invisible"), color::NotVisibleColorEnemynames, file_path); // Set the color for invisible enemy names
		WritePrivateProfileImColor(xorstr_("visuals"), xorstr_("distance_invisible"), color::NotVisibleColorTeamdistance, file_path); // Set the color for invisible distance

	}

	void Load_Settings(std::string fileName)
	{
		char file_path[MAX_PATH];
		sprintf_s(file_path, xorstr_("C:\\Configs\\%s%s"), fileName.c_str(), xorstr_(".ini"));
		globals::b_lock = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("enable_aim"), globals::b_lock, file_path);
		globals::aim_key = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("aim_key"), globals::aim_key, file_path);
		globals::b_helmet = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("helmet"), globals::b_helmet, file_path);
		globals::b_head = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("head"), globals::b_head, file_path);
		globals::b_neck = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("neck"), globals::b_neck, file_path);
		globals::b_chest = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("chest"), globals::b_chest, file_path);
		globals::b_mid = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("mid"), globals::b_mid, file_path);
		globals::b_tummy = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("tummy"), globals::b_tummy, file_path);
		globals::b_pelvis = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("pelvis"), globals::b_pelvis, file_path);
		globals::b_rightfood = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("right_food"), globals::b_rightfood, file_path);
		globals::b_leftfood = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("left_food"), globals::b_leftfood, file_path);
		globals::target_bone = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("target_bone"), globals::target_bone, file_path);
		globals::b_crosshair = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("crosshair"), globals::b_crosshair, file_path);
		globals::b_skip_knocked = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("skip_knocked"), globals::b_skip_knocked, file_path);
		globals::b_aim_point = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("show_aim_point"), globals::b_aim_point, file_path);
		globals::b_prediction = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("prediction"), globals::b_prediction, file_path);
		globals::fov_type = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("show_fov"), globals::fov_type, file_path);
		globals::b_fov = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("normal_fov"), globals::b_fov, file_path);
		globals::fov_rainbow = GetPrivateProfileIntA(xorstr_("aimbot"), xorstr_("rainbow_fov"), globals::fov_rainbow, file_path);

		globals::aim_smooth = GetPrivateProfileFloat(xorstr_("aimbot"), xorstr_("aim_smooth"), globals::aim_smooth, file_path);

		globals::chams = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_charms"), globals::chams, file_path);
		globals::b_box = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_box"), globals::b_box, file_path);
		globals::b_corner = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_corner"), globals::b_corner, file_path);
		globals::b_skeleton = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_skeleton"), globals::b_skeleton, file_path);
		globals::snapline_type = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_snapline"), globals::snapline_type, file_path);
		globals::centerpos = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("snapline_center"), globals::centerpos, file_path);
		globals::bottompos = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("snapline_bottom"), globals::bottompos, file_path);
		globals::topupos = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("snapline_topup"), globals::topupos, file_path);
		globals::b_health = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_healtbar"), globals::b_health, file_path);
		globals::b_distance = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_distance"), globals::b_distance, file_path);
		globals::b_names = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("enable_names"), globals::b_names, file_path);
		globals::box_outline = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("box_outline"), globals::box_outline, file_path);
		globals::corner_outline = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("corner_outline"), globals::corner_outline, file_path);
		globals::line_outline = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("snapline_outline"), globals::line_outline, file_path);

		globals::rainbow_box = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("rainbow_box"), globals::rainbow_box, file_path);
		globals::rainbow_corner = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("rainbow_corner"), globals::rainbow_corner, file_path);
		globals::rainbow_charms = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("rainbow_chams"), globals::rainbow_charms, file_path);
		globals::rainbow_skeleton = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("rainbow_skeleton"), globals::rainbow_skeleton, file_path);
		globals::rianbow_snapline = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("rainbow_snapline"), globals::rianbow_snapline, file_path);
		globals::rainbow_name = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("rainbow_name"), globals::rainbow_name, file_path);
		globals::rainbow_distance = GetPrivateProfileIntA(xorstr_("visuals"), xorstr_("rainbow_distance"), globals::rainbow_distance, file_path);

		globals::sides = GetPrivateProfileFloat(xorstr_("visuals"), xorstr_("rainbow_sides"), globals::sides, file_path);
		globals::velocity = GetPrivateProfileFloat(xorstr_("visuals"), xorstr_("rainbow_velocity"), globals::velocity, file_path);

		color::VisibleColorEnemychams = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("chams_visible"), color::VisibleColorEnemychams, file_path); // Get the color for visible enemy chams
		color::VisibleColorEnemybox = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("box_visible"), color::VisibleColorEnemybox, file_path); // Get the color for visible enemy box
		color::VisibleColorEnemycorner = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("corner_visible"), color::VisibleColorEnemycorner, file_path); // Get the color for visible enemy corner
		color::VisibleColorEnemyskeleton = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("skeleton_visible"), color::VisibleColorEnemyskeleton, file_path); // Get the color for visible enemy skeleton
		color::VisibleColorEnemyline = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("snapline_visible"), color::VisibleColorEnemyline, file_path); // Get the color for visible enemy snapline
		color::VisibleColorTeambox = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_box_visible"), color::VisibleColorTeambox, file_path); // Get the color for visible team box
		color::VisibleColorTeamcorner = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_corner_visible"), color::VisibleColorTeamcorner, file_path); // Get the color for visible team corner
		color::VisibleColorTeamskeleton = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_skeleton_visible"), color::VisibleColorTeamskeleton, file_path); // Get the color for visible team skeleton
		color::VisibleColorTeamline = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_snapline_visible"), color::VisibleColorTeamline, file_path); // Get the color for visible team snapline
		color::VisibleColorTeamdistance = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_distance_visible"), color::VisibleColorTeamdistance, file_path); // Get the color for visible team distance
		color::VisibleColorTeamnames = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_names_visible"), color::VisibleColorTeamnames, file_path); // Get the color for visible team names
		color::VisibleColorEnemynames = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("name_visible"), color::VisibleColorEnemynames, file_path); // Get the color for visible enemy names
		color::VisibleColorTeamdistance = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("distance_visible"), color::VisibleColorTeamdistance, file_path); // Get the color for visible distance

		color::NotVisibleColorEnemychams = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("chams_invisible"), color::NotVisibleColorEnemychams, file_path); // Get the color for invisible enemy chams
		color::NotVisibleColorEnemybox = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("box_invisible"), color::NotVisibleColorEnemybox, file_path); // Get the color for invisible enemy box
		color::NotVisibleColorEnemydcorner = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("corner_invisible"), color::NotVisibleColorEnemydcorner, file_path); // Get the color for invisible enemy corner
		color::NotVisibleColorEnemyskeleton = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("skeleton_invisible"), color::NotVisibleColorEnemyskeleton, file_path); // Get the color for invisible enemy skeleton
		color::NotVisibleColorEnemyline = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("snapline_invisible"), color::NotVisibleColorEnemyline, file_path); // Get the color for invisible enemy snapline
		color::NotVisibleColorTeambox = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_box_invisible"), color::NotVisibleColorTeambox, file_path); // Get the color for invisible team box
		color::NotVisibleColorTeamcorner = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_corner_invisible"), color::NotVisibleColorTeamcorner, file_path); // Get the color for invisible team corner
		color::NotVisibleColorTeamskeleton = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_skeleton_invisible"), color::NotVisibleColorTeamskeleton, file_path); // Get the color for invisible team skeleton
		color::NotVisibleColorTeamline = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_snapline_invisible"), color::NotVisibleColorTeamline, file_path); // Get the color for invisible team snapline
		color::NotVisibleColorTeamdistance = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_distance_invisible"), color::NotVisibleColorTeamdistance, file_path); // Get the color for invisible team distance
		color::NotVisibleColorTeamnames = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("team_names_invisible"), color::NotVisibleColorTeamnames, file_path); // Get the color for invisible team names
		color::NotVisibleColorEnemynames = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("name_invisible"), color::NotVisibleColorEnemynames, file_path); // Get the color for invisible enemy names
		color::NotVisibleColorTeamdistance = GetPrivateProfileImColor(xorstr_("visuals"), xorstr_("distance_invisible"), color::NotVisibleColorTeamdistance, file_path); // Get the color for invisible distance



		str_config_name = fileName;
	};

	std::vector<std::string> GetList()
	{
		std::vector<std::string> configs;
		WIN32_FIND_DATA ffd;
		LPCSTR directory = xorstr_("C:\\Configs\\*");
		auto hFind = FindFirstFile(directory, &ffd);
		while (FindNextFile(hFind, &ffd))
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string file_name = ffd.cFileName;
				if (file_name.size() < 4) // .cfg
					continue;
				std::string end = file_name;
				end.erase(end.begin(), end.end() - 4);
				if (end != xorstr_(".ini"))
					continue;
				file_name.erase(file_name.end() - 4, file_name.end());
				configs.push_back(file_name);
			}
		}
		return configs;
	}

}
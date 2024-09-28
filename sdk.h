#pragma once
#include "stdafx.h"
#include "vec.h"


#define ror

#define _PTR_MAX_VALUE ((PVOID)0x000F000000000000)
#define BYTEn(x, n)   (*((BYTE*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)

//auto padding
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}

#define is_valid_ptr(p) ((uintptr_t)(p) <= 0x7FFFFFFEFFFF && (uintptr_t)(p) >= 0x1000) 
#define is_bad_ptr(p)	(!is_valid_ptr(p))

template <typename T>
bool IsValidPtr(PVOID ptr)
{
	if (is_bad_ptr(ptr))return false;
	else
		return true;

}


bool world(const Vector3& WorldPos, Vector2* ScreenPos);

//class cg_t
//{
//public:
//	char pad_0000[48]; //0x0000
//	Vector3 vecOrig; //0x0030
//	char pad_003C[416]; //0x003C
//	Vector3 viewAngle; //0x01DC
//	char pad_01E8[112]; //0x01E8
//	int32_t N0000009A; //0x0258
//	int32_t Health; //0x025C
//	char pad_0260[160]; //0x0260
//	int32_t N000000AF; //0x0300
//	int32_t Uav; //0x0304
//	char pad_0308[1408]; //0x0308
//}; //Size: 0x0888


//[<ModernWarfare.exe>+1760F190]
class N0000004E
{
public:
	char pad_0000[1144]; //0x0000
	char N00000A88[16]; //0x0478
	char pad_0488[32]; //0x0488
	char N00000A89[16]; //0x04A8
	char pad_04B8[19416]; //0x04B8
}; //Size: 0x5090
static_assert(sizeof(N0000004E) == 0x5090);

#define game_mode 0x11A693B0

namespace player_info
{
	constexpr auto local_index = 0xCCA50;
	constexpr auto local_index_pos = 0x2F0;
	constexpr auto size = 0x13DF0;
	constexpr auto valid = 0x209;
	constexpr auto team_id = 0xB1D;
	constexpr auto position_ptr = 0x210;
	constexpr auto stance = 0x394;
	constexpr auto recoil_offset = 0x376A8;
	constexpr auto dead_1 = 0x13D74;
	constexpr auto dead_2 = 0x13D74;
	constexpr auto weapon_index = 0xA60;
	//constexpr auto loot_ptr = 0x1EA6A3F0;
}

namespace bones
{
	constexpr auto bone_base_pos = 0xCCAE8;
	constexpr auto size = 0x188;
	//constexpr auto visible = 0x2CBD8F0;
	constexpr auto sightedEnemyFools = 0x9EC68;
	constexpr auto weapon_definitions = 0x14D9CAB0;
	//constexpr auto loot_ptr = 0x1EA6A3F0;
	constexpr auto distribute = 0xD20CC68;
}

namespace view_port
{
	constexpr auto refdef_ptr = 0x14EB55F8;
	constexpr auto camera_ptr = 0x1573DF10;
	constexpr auto camera_view_x = 0x108;
	constexpr auto camera_view_y = 0x118;
	constexpr auto camera_view_z = 0x128;
	constexpr auto camera_pos = 0x204;
	//    constexpr auto loot_ptr = 0x1EA6A3F0;
}

namespace client
{
	//constexpr auto info_ptr = 0x17E24638; //client_info
	//constexpr auto base_offset = 0x9ED78; //client_Base
	constexpr auto name_array = 0x14EB4FE8;
	constexpr auto name_array_padding = 0x2C80;
	//constexpr auto loot_ptr = 0x1E91EC90;
}

namespace direcX
{
	//    constexpr uint32_t command_queue = 0x20B808D8;
}
//////////////////////////////////////////////////////////


	/// Sometimes you need different sizes than your architecture
	/// For example 64 bit addresses on 32 bit programs
template< typename ptr_type = uintptr_t >
struct address_base_t
{
	/// <summary>
	/// Inner pointer
	/// </summary>
	ptr_type m_ptr;

	/// <summary>
	/// Creates a NULL address object
	/// </summary>
	address_base_t() : m_ptr{} {};

	/// <summary>
	/// Creates an address object with the given pointer
	/// </summary>
	/// <param name="ptr">The address on which the object will be based on</param>
	address_base_t(ptr_type ptr) : m_ptr(ptr) {};

	/// <summary>
	/// Creates an address object with the given pointer
	/// </summary>
	/// <param name="ptr">The address on which the object will be based on</param>
	address_base_t(ptr_type* ptr) : m_ptr(ptr_type(ptr)) {};

	/// <summary>
	/// Creates an address object with the given pointer
	/// </summary>
	/// <param name="ptr">The address on which the object will be based on</param>
	address_base_t(void* ptr) : m_ptr(ptr_type(ptr)) {};

	/// <summary>
	/// Creates an address object with the given pointer
	/// </summary>
	/// <param name="ptr">The address on which the object will be based on</param>
	address_base_t(const void* ptr) : m_ptr(ptr_type(ptr)) {};

	/// <summary>
	/// Destroys the address object
	/// </summary>
	~address_base_t() = default;

	/// <summary>
	/// Whenever an address object is being passed into a function but it requires
	/// a uintptr this function will be called
	/// </summary>
	inline operator ptr_type() const
	{
		return m_ptr;
	}

	/// <summary>
	/// Whenever an address object is being passed into a function but it requires
	/// a void* this function will be called
	/// </summary>
	inline operator void* ()
	{
		return reinterpret_cast<void*>(m_ptr);
	}

	/// <summary>
	/// Returns the inner pointer
	/// </summary>
	/// <returns>Inner pointer</returns>
	inline ptr_type get_inner() const
	{
		return m_ptr;
	}

	/// <summary>
	/// Compares the inner pointer with the given address
	/// </summary>
	/// <param name=in>Address that will be compared</param>
	/// <returns>True if the addresses match</returns>
	template< typename t = address_base_t<ptr_type> >
	inline bool compare(t in) const
	{
		return m_ptr == ptr_type(in);
	}

	/// Actions performed on self

	/// <summary>
	/// Deref inner pointer
	/// </summary>
	/// <param name="in">Times the pointer will be deref'd</param>
	/// <returns>Current address object</returns>
	inline address_base_t<ptr_type>& self_get(uint8_t in = 1)
	{
		m_ptr = get<ptr_type>(in);

		return *this;
	}

	/// <summary>
	/// Add offset to inner pointer
	/// </summary>
	/// <param name="in">Offset that will be added</param>
	/// <returns>Current address object</returns>
	inline address_base_t<ptr_type>& self_offset(ptrdiff_t offset)
	{
		m_ptr += offset;

		return *this;
	}

	/// <summary>
	/// Follows a relative JMP instruction
	/// </summary>
	/// <param name="offset">Offset at which the function address is</param>
	/// <returns>Address object</returns>
	template< typename t = address_base_t<ptr_type> >
	inline address_base_t<ptr_type>& self_jmp(ptrdiff_t offset = 0x1)
	{
		m_ptr = jmp(offset);

		return *this;
	}

	/// <summary>
	/// Finds a specific opcode
	/// </summary>
	/// <param name="opcode">Opcode that is being searched for</param>
	/// <param name="offset">Offset that should be added to the resulting address</param>
	/// <returns>Address object</returns>
	inline address_base_t<ptr_type>& self_find_opcode(byte opcode, ptrdiff_t offset = 0x0)
	{
		m_ptr = find_opcode(opcode, offset);

		return *this;
	}

	/// <summary>
	/// Finds a specific opcode sequence
	/// </summary>
	/// <param name="opcodes">Opcodes to be searched</param>
	/// <param name="offset">Offset that should be added to the resulting address</param>
	/// <returns>Address object</returns>
	inline address_base_t<ptr_type>& self_find_opcode_seq(std::vector<byte> opcodes, ptrdiff_t offset = 0x0)
	{
		m_ptr = find_opcode_seq(opcodes, offset);

		return *this;
	}

	/// <summary>
	/// Set inner pointer to given value
	/// </summary>
	/// <param name="in">Offset that will be added</param>
	/// <returns>Current address object</returns>
	template< typename t = address_base_t<ptr_type> >
	inline address_base_t<ptr_type>& set(t in)
	{
		m_ptr = ptr_type(in);

		return *this;
	}

	/// Const actions

	/// <summary>
	/// Returns a casted version of the inner pointer
	/// </summary>
	/// <returns>Current address object</returns>
	template< typename t = ptr_type >
	inline t cast()
	{
		return t(m_ptr);
	}

	/// <summary>
	/// Deref inner pointer
	/// </summary>
	/// <param name="in">Times the pointer will be deref'd</param>
	/// <returns>Current address object</returns>
	template< typename t = address_base_t<ptr_type> >
	inline t get(uint8_t in = 1)
	{
		if (!m_ptr)
			return t{};

		ptr_type dummy = m_ptr;

		while (in--)
			/// Check if pointer is still valid
			if (dummy)
				dummy = *reinterpret_cast<ptr_type*>(dummy);

		return t(dummy);
	}

	/// <summary>
	/// Add offset to inner pointer
	/// </summary>
	/// <param name="in">Offset that will be added</param>
	/// <returns>Address object</returns>
	template< typename t = address_base_t<ptr_type> >
	inline t offset(ptrdiff_t offset)
	{
		return t(m_ptr + offset);
	}

	/// <summary>
	/// Follows a relative JMP instruction
	/// </summary>
	/// <param name="offset">Offset at which the function address is</param>
	/// <returns>Address object</returns>
	template< typename t = address_base_t<ptr_type> >
	inline t jmp(ptrdiff_t offset = 0x1)
	{
		if (!m_ptr)
			return t{};

		/// Example:
		/// E9 ? ? ? ?
		/// The offset has to skip the E9 (JMP) instruction
		/// Then deref the address coming after that to get to the function
		/// Since the relative JMP is based on the next instruction after the address it has to be skipped

		/// Base address is the address that follows JMP ( 0xE9 ) instruction
		ptr_type base = m_ptr + offset;

		/// Store the displacement
		/// Note: Displacement addresses can be signed, thanks d3x
		auto displacement = *reinterpret_cast<int32_t*>(base);

		/// The JMP is based on the instruction after the address
		/// so the address size has to be added
		/// Note: This is always 4 bytes, regardless of architecture, thanks d3x
		base += sizeof(uint32_t);

		/// Now finally do the JMP by adding the function address
		base += displacement;

		return t(base);
	}

	/// <summary>
	/// Finds a specific opcode
	/// </summary>
	/// <param name="opcode">Opcode to be searched</param>
	/// <param name="offset">Offset that should be added to the resulting address</param>
	/// <returns>Address object</returns>
	template< typename t = address_base_t<ptr_type> >
	inline t find_opcode(byte opcode, ptrdiff_t offset = 0x0)
	{
		if (!m_ptr)
			return t{};

		auto base = m_ptr;

		auto opcode_at_address = byte();

		/// Continue looping as long as address is valid
		while (opcode_at_address = *reinterpret_cast<byte*>(base))
		{
			/// Check if we found the opcode we need
			if (opcode == opcode_at_address)
				break;

			/// Continue searching
			base += 1;
		}

		/// Add additional offset
		base += offset;

		return t(base);
	}

	/// <summary>
	/// Finds a specific opcode sequence
	/// </summary>
	/// <param name="opcode">Opcodes to be searched</param>
	/// <param name="offset">Offset that should be added to the resulting address</param>
	/// <returns>Address object</returns>
	template< typename t = address_base_t<ptr_type> >
	inline t find_opcode_seq(std::vector<byte> opcodes, ptrdiff_t offset = 0x0)
	{
		if (!m_ptr)
			return t{};

		auto base = m_ptr;

		auto opcode_at_address = byte();

		/// Continue looping as long as address is valid
		while (opcode_at_address = *reinterpret_cast<byte*>(base))
		{
			/// Check if we found the opcode begin
			if (opcodes.at(0) == opcode_at_address)
			{
				/// Check if all following opcodes match too
				for (auto i = 0u; i < opcodes.size(); i++)
					if (opcodes.at(i) != *reinterpret_cast<byte*>(base + i))
						goto CONT;

				/// We found it!
				break;
			}
		CONT:
			/// Continue searching
			base += 1;
		}

		/// Add additional offset
		base += offset;

		return t(base);
	}
};

/// Adjusted size to architecture
using address_t = address_base_t<uintptr_t>;

/// 32 bit
using address_32_t = address_base_t<uint32_t>;

/// 64 bit
using address_64_t = address_base_t<uint64_t>;

namespace g_data
{
	inline uintptr_t tpva = 0;
	inline uintptr_t jmp_rbx = 0;
	inline uintptr_t R_AddDObjToSceneInternal = 0;

	extern HWND hWind;
	extern uintptr_t base;
	extern uintptr_t peb;
	//extern uintptr_t visible_base;
	extern uintptr_t unlocker;
	extern uintptr_t ddl_loadasset;
	extern uintptr_t ddl_getrootstate;
	extern uintptr_t ddl_getdllbuffer;
	extern uintptr_t ddl_movetoname;
	extern uintptr_t ddl_setint;
	extern uintptr_t Dvar_FindVarByName;
	extern uintptr_t Dvar_SetBoolInternal;
	extern uintptr_t Dvar_SetInt_Internal;
	extern uintptr_t Dvar_SetFloat_Internal;
	extern uintptr_t Camo_Offset_Auto_Test;


	extern uintptr_t Clantag_auto;

	extern uintptr_t a_parse;
	extern uintptr_t ddl_setstring;
	extern uintptr_t ddl_movetopath;
	extern uintptr_t ddlgetInth;
	extern QWORD current_visible_offset;
	extern QWORD cached_visible_base;
	extern QWORD last_visible_offset;
	extern uintptr_t cached_client_t;
	void init();

}
union DvarValue
{
	bool enabled;
	int32_t integer;
	uint32_t unsignedInt;
	float value;
	Vector4 vector;
	const char* string;
	unsigned __int8 color[4];
	uint64_t unsignedInt64;
	int64_t integer64;
};

typedef enum DvarType : uint8_t
{
	DVAR_TYPE_BOOL = 0x0,
	DVAR_TYPE_FLOAT = 0x1,
	DVAR_TYPE_FLOAT_2 = 0x2,
	DVAR_TYPE_FLOAT_3 = 0x3,
	DVAR_TYPE_FLOAT_4 = 0x4,
	DVAR_TYPE_INT = 0x5,
	DVAR_TYPE_INT64 = 0x6,
	DVAR_TYPE_UINT64 = 0x7,
	DVAR_TYPE_ENUM = 0x8,
	DVAR_TYPE_STRING = 0x9,
	DVAR_TYPE_COLOR = 0xA,
	DVAR_TYPE_FLOAT_3_COLOR = 0xB,
	DVAR_TYPE_COUNT = 0xC,
} DvarType;

struct dvar_s
{
	char name[4];
	uint32_t flags;
	BYTE level[1];
	DvarType type;
	bool modified;
	uint32_t checksum;
	char* description;
	char pad2[16];
	unsigned __int16 hashNext;
	DvarValue current;
	DvarValue latched;
	DvarValue reset;

};
class cg_t
{
public:
	char pad_0000[48]; //0x0000
	Vector3 vecOrig; //0x0030
	char pad_003C[416]; //0x003C
	Vector3 viewAngle; //0x01DC
	char pad_01E8[112]; //0x01E8
	int32_t N0000009A; //0x0258
	int32_t Health; //0x025C
	char pad_0260[160]; //0x0260
	int32_t N000000AF; //0x0300
	int32_t Uav; //0x0304
	char pad_0308[1408]; //0x0308
}; //Size: 0x0888
static_assert(sizeof(cg_t) == 0x888);
dvar_s* Dvar_FindVarByName(const char* dvarName);
uintptr_t Dvar_SetBoolByName(const char* dvarName, bool value);
uintptr_t Dvar_SetFloat_Internal(dvar_s* a1, float a2);
uintptr_t Dvar_SetInt_Internal(dvar_s* a1, unsigned int a2);
uintptr_t Dvar_SetBool_Internal(dvar_s* a1, bool a2);
__int64 Com_DDL_LoadAsset(__int64 file);
__int64 DDL_GetRootState(__int64 state, __int64 file);
bool CL_PlayerData_GetDDLBuffer(__int64 context, int controllerindex, int stats_source, unsigned int statsgroup);
bool ParseShit(const char* a, const char** b, const int c, int* d);
char DDL_MoveToPath(__int64* fromState, __int64* toState, int depth, const char** path);
__int64 DDL_MoveToName(__int64 fstate, __int64 tstate, __int64 path);
char DDL_SetInt(__int64 fstate, __int64 context, unsigned int value);
int DDL_GetInt(__int64* fstate, __int64* context);
char DDL_SetInt2(__int64* fstate, __int64* context, int value);
char DDL_SetString(__int64 fstate, __int64 context, const char* value);
void ShowToastNotificationAfterUserJoinedParty(const char* message);


inline const char* dvartype(const char* dvarname)
{
	dvar_s* varbyname = Dvar_FindVarByName(dvarname);
	if (varbyname)
	{
		switch (varbyname->type)
		{
		case DvarType::DVAR_TYPE_BOOL:
		{
			return "DVAR_TYPE_BOOL";
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT:
		{
			return "DVAR_TYPE_FLOAT";
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_2: //vec2
		{
			return "DVAR_TYPE_FLOAT_2";
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_3: //vec3
		{
			return "DVAR_TYPE_FLOAT_3";
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_4: //vec4
		{
			return "DVAR_TYPE_FLOAT_4";
			break;
		}
		case DvarType::DVAR_TYPE_INT:
		{
			return "DVAR_TYPE_INT";
			break;
		}
		case DvarType::DVAR_TYPE_INT64:
		{
			return "DVAR_TYPE_INT64";
			break;
		}
		case DvarType::DVAR_TYPE_UINT64:
		{
			return "DVAR_TYPE_UINT64";
			break;
		}
		case DvarType::DVAR_TYPE_ENUM:
		{
			return "DVAR_TYPE_ENUM";
			break;
		}
		case DvarType::DVAR_TYPE_STRING:
		{
			return "DVAR_TYPE_STRING";
			break;
		}
		case DvarType::DVAR_TYPE_COLOR:
		{
			return "DVAR_TYPE_COLOR";
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_3_COLOR:
		{
			return "DVAR_TYPE_FLOAT_3_COLOR";
			break;
		}
		//case DvarType::DVAR_TYPE_COUNT:
		//	return std::to_string(varbyname->current.);
		//	break;
		default:
			return "[NA]";
			break;
		}
	}
}

static uintptr_t AVAL = NULL;
[[nodiscard]] static BYTE* currentDvarVal(const char* cmnd, uintptr_t& addr = AVAL)
{
	//BYTE* buff = new BYTE[1024];
	//std::unique_ptr < BYTE[] > buff(new BYTE[256]);
	auto buff = std::make_unique<BYTE[]>(256);

	std::string result = "(null)";
	dvar_s* varbyname = Dvar_FindVarByName(cmnd);
	if (varbyname)
	{
		addr = (uintptr_t)varbyname;
		switch (varbyname->type)
		{
		case DvarType::DVAR_TYPE_BOOL:
		{

			result = std::to_string(varbyname->current.enabled);
			//strcpy(buff.get(), result.c_str());
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT:
		{
			result = std::to_string(varbyname->current.value);
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_2: //vec2
		{
			result =
				" x: " + std::to_string(varbyname->current.vector.x) +
				" y: " + std::to_string(varbyname->current.vector.y);

			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_3: //vec3
		{
			result =
				" x: " + std::to_string(varbyname->current.vector.x) +
				" y: " + std::to_string(varbyname->current.vector.y) +
				" z: " + std::to_string(varbyname->current.vector.z);
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_4: //vec4
		{
			result =
				" x: " + std::to_string(varbyname->current.vector.x) +
				" y: " + std::to_string(varbyname->current.vector.y) +
				" z: " + std::to_string(varbyname->current.vector.z) +
				" w: " + std::to_string(varbyname->current.vector.w) + '\0';
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_INT:
		{
			result = std::to_string(varbyname->current.integer);
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_INT64:
		{
			result = std::to_string(varbyname->current.integer64);
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_UINT64:
		{
			result = std::to_string(varbyname->current.unsignedInt64);
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_ENUM:
		{
			result = std::to_string(varbyname->current.integer);
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_STRING:
		{
			result = varbyname->current.string;
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_COLOR:
		{
			result =
				" r: " + std::to_string(varbyname->current.color[0]) +
				" g: " + std::to_string(varbyname->current.color[1]) +
				" b: " + std::to_string(varbyname->current.color[2]) +
				" a: " + std::to_string(varbyname->current.color[3]);

			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_3_COLOR:
		{
			result =
				" r: " + std::to_string(varbyname->current.color[0]) +
				" g: " + std::to_string(varbyname->current.color[1]) +
				" b: " + std::to_string(varbyname->current.color[2]);
			memcpy(buff.get(), (BYTE*)result.c_str() + '\0', result.size() + 1);
			return buff.get();
			break;
		}
		default:
			break;
		}
	}
	addr = NULL;
	//delete[] buff;
	return (BYTE*)result.c_str();
}

template<typename T> inline void dvar_set(const char* dvarName, T value)
{
	dvar_s* dvar = Dvar_FindVarByName(dvarName);
	if (dvar)
	{
		if (dvar->flags > 0)
		{
			dvar->flags = 0;
		}
		switch (dvar->type)
		{
		case DvarType::DVAR_TYPE_BOOL:
		{
			return Dvar_SetBoolByName(dvarName, value);
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT:
		{
			return Dvar_SetFloat_Internal(dvar, value);
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_2: //vec2
		{
			//Dvar_SetVec2_Internal
			return;
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_3: //vec3
		{
			//Dvar_SetVec3_Internal
			return;
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_4: //vec4
		{
			//Dvar_SetVec4_Internal
			return;
			break;
		}
		case DvarType::DVAR_TYPE_INT:
		{
			//Dvar_SetInt_Internal
			return;
			break;
		}
		case DvarType::DVAR_TYPE_INT64:
		{
			//Dvar_SetInt64_Internal
			return;
			break;
		}
		case DvarType::DVAR_TYPE_UINT64:
		{
			//Dvar_SetUInt64_Internal
			return;
			break;
		}
		case DvarType::DVAR_TYPE_ENUM:
		{
			return;
			break;
		}
		case DvarType::DVAR_TYPE_STRING:
		{
			return;
			break;
		}
		case DvarType::DVAR_TYPE_COLOR:
		{
			return;
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT_3_COLOR:
		{
			return;
			break;
		}
		//case DvarType::DVAR_TYPE_COUNT:
		//	return std::to_string(varbyname->current.);
		//	break;
		default:
			return;
			break;
		}
	}
}

inline void dvarFloat(const char* dvarname, float val)
{
	auto DVARFLT_cg_fovScale = Dvar_FindVarByName(dvarname);
	if (DVARFLT_cg_fovScale)
	{
		Dvar_SetFloat_Internal(DVARFLT_cg_fovScale, val);
	}
}

inline uint32_t dvarflag(const char* dvarname)
{
	auto DVARFLT_cg_fovScale = Dvar_FindVarByName(dvarname);
	if (DVARFLT_cg_fovScale)
	{
		return DVARFLT_cg_fovScale->flags;
	}
	return NULL;
}

static void dvar_setBool(const char* dvarname, bool val)
{
	dvar_s* tpv = Dvar_FindVarByName(dvarname);
	if (tpv)
	{
		if (tpv->flags > 0)
		{
			tpv->flags = 0;
		}
		Dvar_SetBoolByName(dvarname, val);
	}
}

template<typename T> inline void dvar_set2(const char* dvarName, T value)
{
	dvar_s* dvar = Dvar_FindVarByName(dvarName);
	if (dvar)
	{
		if (dvar->flags > 0)
		{
			dvar->flags = 0;
		}
		switch (dvar->type)
		{
		case DvarType::DVAR_TYPE_BOOL:
		{
			Dvar_SetBool_Internal(dvar, value);
			//Dvar_SetBoolByName(dvarName, value);
			break;
		}
		case DvarType::DVAR_TYPE_FLOAT:
		{
			Dvar_SetFloat_Internal(dvar, value);
			break;
		}
		case DvarType::DVAR_TYPE_INT:
		{
			Dvar_SetInt_Internal(dvar, value);
			break;
		}
		case DvarType::DVAR_TYPE_STRING:
		{
			break;
		}
		default:
			break;
		}
	}
}

namespace radarVars
{
	extern bool transparent;
	extern bool bEnable2DRadar;
	extern bool bSetRadarSize;
	extern int iScreenWidth;
	extern int iScreenHeight;
	extern ImVec2 v2RadarNormalLocation;
	extern ImVec2 v2RadarNormalSize;
	extern ImVec2 v2SetRadarSize;
	extern ImDrawList* Draw;
}

struct ClientBits;

namespace sdk
{
	
	bool update_visible_addr(int i);
	Vector3 _get_pos(uintptr_t address);
	bool _is_visible(uintptr_t address);
	BYTE _team_id(uintptr_t address);
	uintptr_t _get_player(int i);
	uint32_t _get_index(uintptr_t address);
	uint64_t get_client_info();
	uint64_t get_client_info_base();
	void enable_uav();
	void unlockall();
	void no_recoil();
	void no_spread();
	uint64_t Cbuf_AddText2();
	void Cbuf_AddText(const char* text);
	enum BONE_INDEX : unsigned long
	{

		BONE_POS_HELMET = 8,

		BONE_POS_HEAD = 7,
		BONE_POS_NECK = 6,
		BONE_POS_CHEST = 5,
		BONE_POS_MID = 4,
		BONE_POS_TUMMY = 3,
		BONE_POS_PELVIS = 2,

		BONE_POS_RIGHT_FOOT_1 = 21,
		BONE_POS_RIGHT_FOOT_2 = 22,
		BONE_POS_RIGHT_FOOT_3 = 23,
		BONE_POS_RIGHT_FOOT_4 = 24,

		BONE_POS_LEFT_FOOT_1 = 17,
		BONE_POS_LEFT_FOOT_2 = 18,
		BONE_POS_LEFT_FOOT_3 = 19,
		BONE_POS_LEFT_FOOT_4 = 20,

		BONE_POS_LEFT_HAND_1 = 13,
		BONE_POS_LEFT_HAND_2 = 14,
		BONE_POS_LEFT_HAND_3 = 15,
		BONE_POS_LEFT_HAND_4 = 16,

		BONE_POS_RIGHT_HAND_1 = 9,
		BONE_POS_RIGHT_HAND_2 = 10,
		BONE_POS_RIGHT_HAND_3 = 11,
		BONE_POS_RIGHT_HAND_4 = 12
	};

	enum TYPE_TAG
	{
		ET_GENERAL = 0x0,
		ET_PLAYER = 0x1,
		ET_PLAYER_CORPSE = 0x2,
		ET_ITEM = 0x3,
		ET_MISSILE = 0x4,
		ET_INVISIBLE = 0x5,
		ET_SCRIPTMOVER = 0x6,
		ET_SOUND_BLEND = 0x7,
		ET_FX = 0x8,
		ET_LOOP_FX = 0x9,
		ET_PRIMARY_LIGHT = 0xA,
		ET_TURRET = 0xB,
		ET_HELICOPTER = 0xC,
		ET_PLANE = 0xD,
		ET_VEHICLE = 0xE,
		ET_VEHICLE_COLLMAP = 0xF,
		ET_VEHICLE_CORPSE = 0x10,
		ET_VEHICLE_SPAWNER = 0x11,
		ET_AGENT = 0x12,
		ET_AGENT_CORPSE = 0x13,
		ET_EVENTS = 0x14,
	};

	enum STANCE : int
	{
		STAND = 0,
		CROUNCH = 1,
		PRONE = 2,
		KNOCKED = 3
	};

	//class name_t {
	//public:
	//	uint32_t idx;
	//	//char unk0[0x10];
	//	char name[36];
	//	int32_t get_health()
	//	{
	//		if (!IsValidPtr<name_t>(this))
	//			return 0;

	//		return *reinterpret_cast<int32_t*>((uintptr_t)this + 0x84/*0x8C*/);
	//	}
	//};

	class clientinfo_t
	{
	public:
		char pad_0000[4]; //0x0000
		char name[36]; //0x0004
		char pad_0028[72]; //0x0028
		int32_t game_extrainfo; //0x0070
		char pad_0074[16]; //0x0074
		int32_t health; //0x0084
		char pad_0088[8]; //0x0088
		int32_t squadIndex; //0x0090
		char pad_0094[268]; //0x0094
	}; //Size: 0x01A0

	struct ref_def_view {
		Vector2 tan_half_fov;
		char pad[0xC];
		Vector3 axis[3];
	};

	class refdef_t {
	public:
		int x;
		int y;
		int Width;
		int Height;
		ref_def_view view;
	};

	//class refdef_t
	//{
	//public:
	//	char pad_0000[8]; //0x0000
	//	__int32 Width; //0x0008
	//	__int32 Height; //0x000C
	//	float FovX; //0x0010
	//	float FovY; //0x0014
	//	float Unk; //0x0018
	//	char pad_001C[8]; //0x001C
	//	Vector3 ViewAxis[3]; //0x0024
	//	char pad_0048[52]; //0x0048
	//	Vector3 ViewLocationDelayed0; //0x007C
	//	Vector3 ViewLocationDelayed1; //0x0088
	//	char pad_0094[2808]; //0x0094
	//	Vector3 ViewMatrixTest[3]; //0x0B8C
	//	Vector3 ViewLocation; //0x0BB0
	//};


	class Result
	{
	public:
		bool hasResult;
		float a;
		float b;
	};

	struct velocityInfo_t
	{
		Vector3 lastPos;
		Vector3 delta;
	};

	class player_t
	{
	public:
		player_t(uintptr_t address) {
			this->address = address;
		}
		uintptr_t address{};
		uint32_t get_index();

		bool is_valid();

		bool is_visible();

		bool is_dead();

		BYTE team_id();

		int get_stance();

		Vector3 get_pos();

		float get_rotation();
	};
	void DrawRadarPoint(sdk::player_t* Player, sdk::player_t* Local, int xAxis, int yAxis, int width, int height, ImDrawList* draw, int distatce);
	void DrawRadarHUD(int xAxis, int yAxis, int width, int height);
	bool in_game();
	int get_game_mode();
	int get_max_player_count();
	//name_t* get_name_ptr(int i);
	refdef_t* get_refdef();
	Vector3 get_camera_pos();
	player_t get_player(int i);
	player_t get_local_player();
	int get_local_index_num();
	const char* get_player_name(int i);
	bool get_bone_by_player_index(int i, int bone_index, Vector3* Out_bone_pos);
	
	int get_player_health(int i);
	void start_tick();
	void update_vel_map(int index, Vector3 vPos);
	void clear_map();
	Vector3 get_speed(int index);
	Vector3 prediction_solver(Vector3 local_pos, Vector3 position, int index, float bullet_speed);
	ClientBits get_visible_base();
	bool is_visible(int entityNum);
	void update_last_visible();
	Vector3 get_prediction(int index, Vector3 source, Vector3 destination);
	/*int get_client_count();*/
}

namespace g_radar {

	void draw_entity(sdk::player_t local_entity, sdk::player_t entity, bool IsFriendly, bool IsAlive, DWORD color);
	void show_radar_background();
}


namespace g_game
{
	namespace g_draw
	{
		inline void draw_line(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness);
		inline void draw_box(const float x, const float y, const float width, const float height, const uint32_t color, float thickness);
		void DrawBox(float X, float Y, float W, float H, const ImU32& color, float tickness);
		inline void draw_corned_box(const Vector2& rect, const Vector2& size, uint32_t color, float thickness);
		inline void fill_rectangle(const float x, const float y, const float width, const float hight, const uint32_t color);
	}
}
inline int local_index()
{
	uint64_t decryptedPtr = sdk::get_client_info();

	if (is_valid_ptr(decryptedPtr))
	{
		auto local_index = *(uintptr_t*)(decryptedPtr + player_info::local_index);
		return *(int*)(local_index + player_info::local_index_pos);
	}
	return 0;
}
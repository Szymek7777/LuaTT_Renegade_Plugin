/* Lua Functions
Renegade functions Lua wrapper (for SSGM and DA)
by Stan "sla.ro" Laurentiu Alexandru
Copyright 2010-2022 Sla Studios (http://slastudios.net)

This file is part of the LuaTT
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "General.h"
#include "luatt.h"
#include "engine_tt.h"
#include "engine_io.h"
#include "GameObjManager.h"
#include "engine_player.h"

#define lua_tobooleanCPP(State, Arg) static_cast<bool>(lua_toboolean(State, Arg))
#include "LuaManager.h"
#include "Lua_Class.h"
#include "LuaDialog.h"
#include "LuaThread.h"

#include "HarvesterClass.h"
#include "PhysClass.h"
#include "MoveablePhysClass.h"
#include "PhysicsSceneClass.h"
#include "CombatManager.h"
#include "PowerupGameObj.h"
#include "PowerupGameObjDef.h"
#include "PurchaseSettingsDefClass.h"
#include "weaponmgr.h"
#include "engine_weap.h"
#include "VehicleGameObj.h"
#include "BeaconGameObj.h"
#include "ScriptZoneGameObjDef.h"
#include "cScTextObj.h"

#ifndef DAPLUGIN
#include "gmvehicle.h"
#include "gmgame.h"
#include "gmlog.h"
#else
#include "SpawnerClass.h"
#include "SpawnerDefClass.h"

#if RENEGADE_VERSION == 1038
REF_DEF2(DynamicVectorClass<SpawnerClass*>, SpawnerList, 0, 0x00857D28);
#else
REF_DEF2(DynamicVectorClass<SpawnerClass*>, SpawnerList, 0, 0x008564A8);
#endif

#include "da_vehicle.h"
#include "da_game.h"
#include "da_log.h"
#include "da_plugin.h"
#include "da_ssgm.h"
#include "engine_def.h"
#include "engine_obj.h"
#include "engine_obj2.h"
#include "engine_dmg.h"
#include "engine_game.h"
#include "engine_player.h"
#include "engine_pt.h"
#include "engine_tdb.h"
#include "engine_tt.h"
#include "engine_weap.h"
#include "engine_phys.h"
#include "engine_script.h"

#ifndef luaL_checkint
#define luaL_checkint(L, n) (static_cast<int>(luaL_checkinteger(L,n)))
#endif

// Foward declaration for missing DA exports
SCRIPTS_API const char *Get_Translated_Definition_Name_Ini(const char *preset);
SCRIPTS_API const char *Get_Translated_Preset_Name_Ex(GameObject *obj);
#endif


template<typename Tag, typename Tag::type M>
struct PrivateGetBuilding { friend typename Tag::type get(Tag) { return M; } };
struct GetPrivate_IntBuildingGameObj {
	typedef RefMultiListClass<StaticPhysClass> BuildingGameObj::* type;
};
struct GetPrivate_ExtBuildingGameObj {
	typedef RefMultiListClass<StaticPhysClass> BuildingGameObj::* type;
};
template struct PrivateGetBuilding<GetPrivate_IntBuildingGameObj, &BuildingGameObj::InteriorMeshes>;
template struct PrivateGetBuilding<GetPrivate_ExtBuildingGameObj, &BuildingGameObj::ExteriorMeshes>;


class PhysicsSceneClassLua : public PhysicsSceneClass
{
public:
	static PhysicsSceneClassLua* Get_Instance(void) { return (PhysicsSceneClassLua*)PhysicsSceneClass::Get_Instance(); }
	RefMultiListClass<PhysClass>* getStaticObjList() { return &StaticObjList; }
	RefMultiListClass<PhysClass>* getObjList() { return &ObjList; }
	StaticPhysClass* Get_Static_Object_By_ID(uint32 id)
	{
		RefMultiListIterator<PhysClass> iter = getStaticObjList();
		for (iter.First(); iter.Is_Done() == false; iter.Next())
		{
			PhysClass* o = (PhysClass*)iter.Peek_Obj();
			if (o->Get_ID() == id)
			{
				return o->As_StaticPhysClass();
			}
		}
		return nullptr;
	}
	Vector3 Get_Position(uint32 id)
	{
		StaticPhysClass* obj = Get_Static_Object_By_ID(id);
		if (obj)
			return obj->Get_Transform().Get_Translation();
		else
			return Vector3();
	}

	Vector3 Get_Rotation(uint32 id)
	{
		StaticPhysClass* obj = Get_Static_Object_By_ID(id);
		if (obj)
		{
			Matrix3D transform = obj->Get_Transform();
			return Vector3(transform.Get_X_Rotation(), transform.Get_Y_Rotation(), transform.Get_Z_Rotation());
		}
		else
			return Vector3();
	}
};

class PhysClassLua : PhysClass
{
public:
	const char* Get_Name() { return Model->Get_Name(); }
};

#pragma warning(disable: 4244) /* Ignore the 'loss of data' warning*/

int Get_ID(GameObject *O)
{
	if (O)
	{
		return Commands->Get_ID(O);
	}
	return 0;
}

int Lua_Get_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Commands->Find_Object(lua_tointeger(L, 1))));
	return 1;
}

void Shake_Camera_Player(int PlayerID, Vector3 Position, float Radius, float Intensity, float Duration)
{
	WideStringClass Netcode = StringClass::getFormattedString("j\n30\n%f\n%f\n%f\n%f\n%f\n%f\n",
		Position.X, Position.Y, Position.Z, Radius, Intensity, Duration);
	Send_Client_Text(Netcode, TEXT_MESSAGE_PRIVATE, true, -2, PlayerID, true, true);
}
#endif

/*!\details Inputs a command to FDS Console
*  \par Lua Command
*  \code{.lua}
*			Console_Input(message)
*  \endcode
*  \par		Example
*  \code{.lua}
*			Console_Input("msg Test")
*  \endcode
*  \param message String
*/
int Lua_Console_Input(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Console_Input(lua_tostring(L, 1));
	return 1;
}

/*!\details Writes text in FDS Console
*  \par Lua Command
*  \code{.lua}
*			Console_Output(message)
*  \endcode
*  \par		Example
*  \code{.lua}
*			Console_Input("Hello World\n")
*  \endcode
*  \param message String
*/
int Lua_Console_Output(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Console_Output(lua_tostring(L, 1));
	return 1;
}

/*!\details Gets the name of a definition id
*  \par Lua Command
*  \code{.lua}
*			name = Get_Definition_Name(id)
*  \endcode
*  \par		Example
*  \code{.lua}
*			name = Get_Definition_Name(1000) -- returns: none
*  \endcode
*  \param def_id Number
*  \return name String
*/
int Lua_Get_Definition_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Definition_Name(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Gets the id from a defninition name
*  \par Lua Command
*  \code{.lua}
*			id = Get_Definition_ID(name)
*  \endcode
*  \par		Example
*  \code{.lua}
*			id = Get_Definition_ID("Weapon_StealthTank_Player") -- returns: 409610054
*  \endcode
*  \param def_name String
*  \return def_id Number
*/
int Lua_Get_Definition_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Definition_ID(lua_tostring(L, 1)));
	return 1;
}

/*!\details Gets the definition class id from a defninition name
*  \par Lua Command
*  \code{.lua}
*			id = Get_Definition_Class_ID(name)
*  \endcode
*  \par		Example
*  \code{.lua}
*			id = Get_Definition_Class_ID("Weapon_StealthTank_Player") -- returns: 45057
*  \endcode
*  \param def_name String
*  \return def_id Number
*/
int Lua_Get_Definition_Class_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Definition_Class_ID(lua_tostring(L, 1)));
	return 1;
}

/*!\details Returns true if the preset id is valid
*  \par Lua Command
*  \code{.lua}
*			valid = Is_Valid_Preset_ID(preset_id)
*  \endcode
*  \par		Example
*  \code{.lua}
*			valid = Is_Valid_Preset_ID(1) -- returns: false
*  \endcode
*  \param preset_id Number
*  \return valid Boolean
*/
int Lua_Is_Valid_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Valid_Preset_ID(lua_tointeger(L, 1)));
	return 1;
}

/*!\details Returns true if the preset name is valid
*  \par Lua Command
*  \code{.lua}
*			valid = Is_Valid_Preset(preset_name)
*  \endcode
*  \par		Example
*  \code{.lua}
*			valid = Is_Valid_Preset_ID("Weapon_StealthTank_Player")  -- returns: false
*  \endcode
*  \param preset_name String
*  \return valid Boolean
*/
int Lua_Is_Valid_Preset(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Valid_Preset(lua_tostring(L, 1)));
	return 1;
}

/*!\details Sets max health of an object
*  \par Lua Command
*  \code{.lua}
*			Set_Max_Health(Object, Amount)
*  \endcode
*  \par		Example
*  \code{.lua}
*			Set_Max_Health(Get_GameObj(1), 9000)
*  \endcode
*  \param Object Number
*  \param Amount Number
*/
int Lua_Set_Max_Health(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Max_Health(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 1;
}

/*!\details Sets max shield of an object
*  \par Lua Command
*  \code{.lua}
*			Set_Max_Shield_Strength(Object, Amount)
*  \endcode
*  \par		Example
*  \code{.lua}
*			Set_Max_Shield_Strength(Get_GameObj(1), 9000)
*  \endcode
*  \param Object Number
*  \param Amount Number
*/
int Lua_Set_Max_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Max_Shield_Strength(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 1;
}

int Lua_Get_Shield_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Shield_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Skin(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Skin(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Set_Skin(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	if (!obj) return 0;
	Set_Skin(obj, luaL_checkstring(L, 2));
	return 1;
}

int Lua_Power_Base(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Power_Base(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Set_Can_Generate_Soldiers(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Can_Generate_Soldiers(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}


int Lua_Set_Can_Generate_Vehicles(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Can_Generate_Vehicles(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Destroy_Base(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Destroy_Base(lua_tonumber(L, 1));
	return 1;
}

int Lua_Beacon_Destroyed_Base(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Beacon_Destroyed_Base(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Enable_Base_Radar(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Enable_Base_Radar(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Is_Radar_Enabled(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Radar_Enabled(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Building_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Building_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Building(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_By_Type(lua_tonumber(L, 1), lua_tonumber(L, 2))));
	return 1;
}

int Lua_Find_Base_Defense(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Base_Defense(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Is_Map_Flying(lua_State *L)
{
	lua_pushboolean(L, Is_Map_Flying());
	return 1;
}

int Lua_Find_Harvester(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Harvester(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Is_Base_Powered(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Base_Powered(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Can_Generate_Vehicles(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Can_Generate_Vehicles(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Can_Generate_Soliders(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Can_Generate_Soldiers(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Get_Building_Count_Team(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Building_Count_Team(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Find_Building_By_Team(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_By_Team(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Building_By_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_By_Preset(lua_tonumber(L, 1), lua_tostring(L, 2))));
	return 1;
}

int Lua_Find_Power_Plant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Power_Plant(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Refinery(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Refinery(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Repair_Bay(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Repair_Bay(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Soldier_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Soldier_Factory(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Airstrip(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Vehicle_Factory(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_War_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Vehicle_Factory(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Vehicle_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Vehicle_Factory(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Find_Com_Center(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Com_Center(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Is_Gameplay_Permitted(lua_State *L)
{
	lua_pushboolean(L, Is_Gameplay_Permitted());
	return 1;
}

int Lua_Is_Dedicated(lua_State *L)
{
	lua_pushboolean(L, Is_Dedicated());
	return 1;
}

int Lua_Get_Current_Game_Mode(lua_State *L)
{
	lua_pushnumber(L, Get_Current_Game_Mode());
	return 1;
}

int Lua_Get_Harvester_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Harvester_Preset_ID(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Get_IP_Address(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *IP = Get_IP_Address(lua_tonumber(L, 1));
	if (!IP)
	{
		return 0;
	}
	lua_pushstring(L, IP);
	//delete []IP;
	return 1;
}
int Lua_Get_IP_Port(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *IP = Get_IP_Port(lua_tonumber(L, 1));
	if (!IP)
	{
		return 0;
	}
	lua_pushstring(L, IP);
	//delete []IP;
	return 1;
}
int Lua_Get_Bandwidth(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Bandwidth(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Ping(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Ping(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Kbits(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Kbits(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Object_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Get_Object_Type(obj));
	return 1;
}
int Lua_Set_Object_Type(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Set_Object_Type(obj, lua_tonumber(L, 2));
	return 1;
}

int Lua_Get_Object_Count(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Object_Count(lua_tonumber(L, 1), lua_tostring(L, 2)));
	return 1;
}
int Lua_Find_Random_Preset(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	lua_pushnumber(L, Get_ID(Find_Random_Preset(lua_tostring(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3))));
	return 1;
}
int Lua_Send_Custom_To_Team_Buildings(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Custom_To_Team_Buildings(lua_tonumber(L, 1), Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}
int Lua_Send_Custom_To_Team_Preset(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	Send_Custom_To_Team_Preset(lua_tonumber(L, 1), lua_tostring(L, 2), Commands->Find_Object(lua_tonumber(L, 3)), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));
	return 1;
}
int Lua_Send_Custom_All_Objects(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Send_Custom_All_Objects(lua_tonumber(L, 1), Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3));
	return 1;
}


int Lua_Send_Custom_Event_To_Object(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Custom_Event_To_Object(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}

int Lua_Get_Is_Powerup_Persistant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Get_Is_Powerup_Persistant(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Powerup_Always_Allow_Grant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Get_Powerup_Always_Allow_Grant(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Set_Powerup_Always_Allow_Grant(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Powerup_Always_Allow_Grant(Commands->Find_Object(lua_tonumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Get_Powerup_Grant_Sound(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Powerup_Grant_Sound(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Grant_Powerup(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Grant_Powerup(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2));
	return 1;
}

int Lua_Get_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

int Lua_Grant_Refill(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Grant_Refill(Commands->Find_Object(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Change_Character(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushboolean(L, Change_Character(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}

int Lua_Create_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Create_Vehicle(lua_tostring(L, 1), lua_tonumber(L, 2), Commands->Find_Object(lua_tonumber(L, 3)), lua_tonumber(L, 4));
	return 1;
}

int Lua_Toggle_Fly_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Toggle_Fly_Mode(Commands->Find_Object(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Get_Vehicle_Occupant_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Occupant_Count(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Vehicle_Occupant(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Occupant(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2))));
	return 1;
}

int Lua_Get_Vehicle_Driver(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Driver(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

int Lua_Get_Vehicle_Gunner(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Gunner(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

int Lua_Force_Occupant_Exit(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Force_Occupant_Exit(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}

int Lua_Force_Occupants_Exit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Force_Occupants_Exit(Commands->Find_Object(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Get_Vehicle_Return(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Return(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

int Lua_Get_Fly_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Get_Fly_Mode(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Vehicle_Seat_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Seat_Count(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Soldier_Transition_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Soldier_Transition_Vehicle(Commands->Find_Object(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Vehicle_Mode(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1)
	{
		return 0;
	}
	lua_pushnumber(L, Get_Vehicle_Mode(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Vehicle_Owner(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Owner(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}
int Lua_Force_Occupants_Exit_Team(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Force_Occupants_Exit_Team(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 1;
}
int Lua_Get_Vehicle_Definition_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Definition_Mode(lua_tostring(L, 1)));
	return 1;
}

int Lua_IsInsideZone(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushboolean(L, IsInsideZone(Commands->Find_Object(lua_tonumber(L, 1)), Commands->Find_Object(lua_tonumber(L, 2))));
	return 1;
}

int Lua_Get_Vehicle_Definition_Mode_By_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Definition_Mode_By_ID(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Zone_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Zone_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Vehicle_Gunner_Pos(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Gunner_Pos(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

int Lua_Set_Vehicle_Gunner(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Vehicle_Gunner(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}

int Lua_Get_Model(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;

	if (lua_gettop(L) >= 2 
     && lua_tobooleanCPP(L, 2) == true)
	{
		PhysClassLua* obj = (PhysClassLua*)PhysicsSceneClassLua::Get_Instance()->Get_Static_Object_By_ID(lua_tonumber(L, 1));
		if (obj)
		{
			lua_pushstring(L, obj->Get_Name());
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		lua_pushstring(L, Get_Model(Commands->Find_Object(lua_tonumber(L, 1))));
	}

	return 1;
}

int Lua_Get_Animation_Frame(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Animation_Frame(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/* The Is functions */
#define LUA_IS_OBJ(name, check) int Lua_##name(lua_State *L) \
{\
if (lua_gettop(L) < 1) return 0; \
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1)); \
	lua_pushboolean(L, (obj && check)); \
	return 1; \
}

LUA_IS_OBJ(Is_Attached_To_An_Object, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->Is_Attached_To_An_Object())
LUA_IS_OBJ(Is_Harvester, Is_Harvester(obj))
LUA_IS_OBJ(Is_Projectile, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->Peek_Physical_Object() && obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_ProjectileClass())
LUA_IS_OBJ(Is_TrackedVehicle, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->Peek_Physical_Object() && obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_TrackedVehicleClass())
LUA_IS_OBJ(Is_VTOLVehicle, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->Peek_Physical_Object() && obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_VTOLVehicleClass())
LUA_IS_OBJ(Is_WheeledVehicle, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->Peek_Physical_Object() && obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_WheeledVehicleClass())
LUA_IS_OBJ(Is_Motorcycle, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->Peek_Physical_Object() && obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_MotorcycleClass())
LUA_IS_OBJ(Is_Building, obj->As_BuildingGameObj())
LUA_IS_OBJ(Is_Soldier, obj->As_SoldierGameObj())
LUA_IS_OBJ(Is_Vehicle, obj->As_VehicleGameObj())
LUA_IS_OBJ(Is_ScriptZone, obj->As_ScriptZoneGameObj())
LUA_IS_OBJ(Is_Cinematic, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_CinematicGameObj())
LUA_IS_OBJ(Is_Powerup, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_PowerUpGameObj())
LUA_IS_OBJ(Is_C4, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_C4GameObj())
LUA_IS_OBJ(Is_Beacon, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_BeaconGameObj())
LUA_IS_OBJ(Is_Armed, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_ArmedGameObj())
LUA_IS_OBJ(Is_Simple, obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_SimpleGameObj())
LUA_IS_OBJ(Is_PowerPlant, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_PowerPlantGameObj())
LUA_IS_OBJ(Is_SoldierFactory, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_SoldierFactoryGameObj())
LUA_IS_OBJ(Is_VehicleFactory, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_VehicleFactoryGameObj())
LUA_IS_OBJ(Is_Airstrip, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_AirStripGameObj())
LUA_IS_OBJ(Is_WarFactory, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_WarFactoryGameObj())
LUA_IS_OBJ(Is_Refinery, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_RefineryGameObj())
LUA_IS_OBJ(Is_ComCenter, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_ComCenterGameObj())
LUA_IS_OBJ(Is_RepairBay, obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_RepairBayGameObj())
LUA_IS_OBJ(Is_Scriptable, obj->As_ScriptableGameObj())
LUA_IS_OBJ(Is_Building_Dead, Is_Building_Dead(obj))
LUA_IS_OBJ(Is_Harvester_Preset, Is_Harvester_Preset(obj))
LUA_IS_OBJ(Is_Available_For_Purchase, Is_Available_For_Purchase(obj))
LUA_IS_OBJ(Is_Engine_Enabled, Is_Engine_Enabled(obj))
LUA_IS_OBJ(Is_A_Star, Commands->Is_A_Star(obj))
LUA_IS_OBJ(Is_Stealthed, obj->As_SmartGameObj() && obj->As_SmartGameObj()->Is_Stealthed())
LUA_IS_OBJ(Is_Performing_Pathfind_Action, Commands->Is_Performing_Pathfind_Action(obj))
LUA_IS_OBJ(Is_Delete_Pending, obj->Is_Delete_Pending())
LUA_IS_OBJ(Is_Immovable, obj->As_VehicleGameObj() && obj->As_VehicleGameObj()->Is_Immovable())
LUA_IS_OBJ(Get_Stealth_Active, obj->As_SmartGameObj() && obj->As_SmartGameObj()->Get_Stealth_Active())
LUA_IS_OBJ(Is_In_Elevator, obj->As_SoldierGameObj() && obj->As_SoldierGameObj()->Is_In_Elevator())
LUA_IS_OBJ(Is_Soldier_Busy, obj->As_SoldierGameObj() && Is_Soldier_Busy(obj->As_SoldierGameObj()))

#define LUA_IS_PHYS(name, check) int Lua_##name(lua_State *L)\
{\
	if (lua_gettop(L) < 1) return 0;\
	StaticPhysClass* phys = PhysicsSceneClassLua::Get_Instance()->Get_Static_Object_By_ID(lua_tonumber(L, 1));\
	lua_pushboolean(L, (phys && check));\
	return 1;\
}

LUA_IS_PHYS(Is_Door, phys->As_DoorPhysClass())
LUA_IS_PHYS(Is_Elevator, phys->As_ElevatorPhysClass())
LUA_IS_PHYS(Is_DamageableStaticPhys, phys->As_DamageableStaticPhysClass())
LUA_IS_PHYS(Is_AccessablePhys, phys->As_AccessiblePhysClass())
LUA_IS_PHYS(Is_DecorationPhys, phys->As_DecorationPhysClass())
LUA_IS_PHYS(Is_HumanPhys, phys->As_HumanPhysClass())
LUA_IS_PHYS(Is_MotorVehicle, phys->As_MotorVehicleClass())
LUA_IS_PHYS(Is_Phys3, phys->As_Phys3Class())
LUA_IS_PHYS(Is_RigidBody, phys->As_RigidBodyClass())
LUA_IS_PHYS(Is_StaticAnimPhys, phys->As_StaticAnimPhysClass())
LUA_IS_PHYS(Is_StaticPhys, phys->As_StaticPhysClass())
LUA_IS_PHYS(Is_TimedDecorationPhys, phys->As_TimedDecorationPhysClass())
LUA_IS_PHYS(Is_VehiclePhys, phys->As_TimedDecorationPhysClass())
LUA_IS_PHYS(Is_DynamicAnimPhys, phys->As_DynamicPhysClass())
LUA_IS_PHYS(Is_MoveablePhys, phys->As_MoveablePhysClass())
LUA_IS_PHYS(Is_WheeledVehiclePhys, phys->As_WheeledVehicleClass())
LUA_IS_PHYS(Is_MotorcyclePhys, phys->As_MotorcycleClass())
LUA_IS_PHYS(Is_TrackedVehiclePhys, phys->As_TrackedVehicleClass())
LUA_IS_PHYS(Is_VTOLVehiclePhys, phys->As_VTOLVehicleClass())
LUA_IS_PHYS(Is_LightPhys, phys->As_LightPhysClass())
LUA_IS_PHYS(Is_RenderObjPhys, phys->As_RenderObjPhysClass())
LUA_IS_PHYS(Is_ProjectilePhys, phys->As_ProjectileClass())
/* End of Is functions */

int Lua_Copy_Transform(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Copy_Transform(Commands->Find_Object(lua_tonumber(L, 1)), Commands->Find_Object(lua_tonumber(L, 2)));
	return 0;
}

int Lua_Get_Mass(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Mass(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Htree_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Htree_Name(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Sex(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Sex(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Create_Effect_All_Of_Preset(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Create_Effect_All_Of_Preset(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3), lua_tobooleanCPP(L, 4));
	return 0;
}

int Lua_Get_GameObj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_GameObj(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Player_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Player_ID(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

int Lua_Get_Player_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Player_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Player_Name_By_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Player_Name_By_ID(lua_tonumber(L, 1));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Change_Team(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Change_Team(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}
int Lua_Change_Team_By_ID(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Change_Team_By_ID(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}
int Lua_Get_Player_Count(lua_State *L)
{
	lua_pushnumber(L, Get_Player_Count());
	return 1;
}
int Lua_Get_Team_Player_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team_Player_Count(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Team(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Rank(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Rank(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Kills(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Kills(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Deaths(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Deaths(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Score(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Score(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Money(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Money(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Kill_To_Death_Ratio(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Kill_To_Death_Ratio(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Part_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Rank(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Part_Names(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Part_Names(lua_tostring(L, 1)));
	return 1;
}

int Lua_Get_GameObj_By_Player_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_GameObj_By_Player_Name(lua_tostring(L, 1))));
	return 1;
}
int Lua_Purchase_Item(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Purchase_Item(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Set_Ladder_Points(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Ladder_Points(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}
int Lua_Set_Rung(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Rung(lua_tonumber(L, 1), lua_tonumber(L, 3));
	return 0;
}
int Lua_Set_Money(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Money(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}
int Lua_Set_Score(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Score(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}
int Lua_Find_First_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_First_Player(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Change_Player_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	lua_pushnumber(L, Change_Player_Team(Commands->Find_Object(lua_tonumber(L, 1)), lua_tobooleanCPP(L, 2), lua_tobooleanCPP(L, 3), lua_tobooleanCPP(L, 4)));
	return 1;
}
int Lua_Tally_Team_Size(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Tally_Team_Size(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Team_Score(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team_Score(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Send_Custom_All_Players(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Send_Custom_All_Players(lua_tonumber(L, 1), Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3));
	return 0;
}
int Lua_Steal_Team_Credits(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Steal_Team_Credits(lua_tonumber(L, 1), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Team_Credits(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team_Credits(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Change_Team_2(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Change_Team_2(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}
int Lua_Get_Player_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Player_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Team_Cost(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Team_Cost(lua_tostring(L, 1), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Cost(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Cost(lua_tostring(L, 1)));
	return 1;
}
int Lua_Get_Team_Icon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushstring(L, Get_Team_Icon(lua_tostring(L, 1), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Icon(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Icon(lua_tostring(L, 1)));
	return 1;
}
int Lua_Remove_Script(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Remove_Script(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2));
	return 1;
}
int Lua_Remove_All_Scripts(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Remove_All_Scripts(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}
int Lua_Attach_Script_Preset(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Preset(lua_tostring(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), lua_tonumber(L, 4));
	return 0;
}
int Lua_Attach_Script_Type(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Type(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}
int Lua_Remove_Script_Preset(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Remove_Script_Preset(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}
int Lua_Remove_Script_Type(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Remove_Script_Type(lua_tostring(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}
int Lua_Is_Script_Attached(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushboolean(L, Is_Script_Attached(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Attach_Script_Once(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Attach_Script_Once(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3));
	return 0;
}
int Lua_Attach_Script_Preset_Once(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Preset_Once(lua_tostring(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), lua_tonumber(L, 4));
	return 0;
}
int Lua_Attach_Script_Type_Once(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Type_Once(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}
int Lua_Attach_Script_Building(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Attach_Script_Building(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}
int Lua_Attach_Script_Is_Preset(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Attach_Script_Is_Preset(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3), lua_tostring(L, 4), lua_tonumber(L, 5));

	return 0;
}
int Lua_Attach_Script_Is_Type(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Attach_Script_Is_Type(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2), lua_tostring(L, 3), lua_tostring(L, 4), lua_tonumber(L, 5));
	return 0;
}
int Lua_Attach_Script_Player_Once(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Attach_Script_Player_Once(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 2));
	return 0;
}
int Lua_Remove_Duplicate_Script(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Remove_Duplicate_Script(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2));
	return 0;
}
int Lua_Attach_Script_All_Buildings_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_All_Buildings_Team(lua_tonumber(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), lua_tobooleanCPP(L, 4));
	return 0;
}
int Lua_Attach_Script_All_Turrets_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_All_Turrets_Team(lua_tonumber(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), lua_tobooleanCPP(L, 4));
	return 0;
}
int Lua_Find_Building_With_Script(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_With_Script(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tostring(L, 3), Commands->Find_Object(lua_tonumber(L, 4)))));
	return 1;
}
int Lua_Find_Object_With_Script(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Object_With_Script(lua_tostring(L, 1))));
	return 1;
}
int Lua_Get_Translated_String(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Translated_String(lua_tonumber(L, 1));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Translated_Preset_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Translated_Preset_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Translated_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	const char *ptr = Get_Translated_Weapon(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Current_Translated_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Current_Translated_Weapon(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Team_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Team_Name(lua_tonumber(L, 1));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Vehicle_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Vehicle_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Translated_Definition_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Translated_Definition_Name(lua_tostring(L, 1));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	delete[]ptr;
	return 1;
}
int Lua_Get_Current_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Current_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Current_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Get_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Get_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Get_Current_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Current_Clip_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Clip_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Current_Total_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Total_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Max_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Max_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Get_Max_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Max_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Get_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Get_Position_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Position_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Position_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Position_Total_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Total_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Position_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Position_Clip_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Clip_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Set_Current_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Current_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}
int Lua_Set_Current_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Current_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}
int Lua_Set_Position_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Position_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}
int Lua_Set_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}
int Lua_Set_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	//if (!lua_tonumber(L, 1) || !lua_tonumber(L, 2) || !lua_tonumber(L, 3) || !Commands->Find_Object(lua_tonumber(L, 1))) { return 0; }

	Set_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tonumber(L, 3));
	return 1;
}
int Lua_Get_Powerup_Weapon(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1)
	{
		return 0;
	}
	lua_pushstring(L, Get_Powerup_Weapon(lua_tostring(L, 1)));
	return 1;
}
int Lua_Get_Powerup_Weapon_By_Obj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Powerup_Weapon_By_Obj(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Current_Weapon_Style(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Weapon_Style(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Get_Position_Weapon_Style(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Weapon_Style(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Get_Weapon_Style(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;

	if (!lua_tostring(L, 1)) { return 0; }
	if (!Commands->Find_Object(lua_tonumber(L, 1))) { return 0; }
	if (!lua_tostring(L, 2)) { return 0; }

	lua_pushnumber(L, Get_Weapon_Style(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Disarm_Beacon(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_Beacon(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}
int Lua_Disarm_Beacons(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_Beacons(lua_tonumber(L, 1));
	return 0;
}
int Lua_Disarm_C4(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_C4(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

int Lua_Get_Current_Weapon(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			lua_pushstring(L, Get_Current_Weapon(obj));
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Weapon_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Weapon_Count(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushstring(L, Get_Weapon(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}
int Lua_Has_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Has_Weapon(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}
int Lua_Find_Beacon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Find_Beacon(lua_tonumber(L, 1), lua_tonumber(L, 2))));
	return 1;
}
int Lua_Get_C4_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	if (lua_gettop(L) == 1) {
		lua_pushnumber(L, Get_C4_Count(lua_tonumber(L, 1)));
	}
	else
	{
		lua_pushnumber(L, Get_C4_Count(lua_tonumber(L, 1), lua_tonumber(L, 2)));
	}

	return 1;
}
int Lua_Get_Beacon_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Beacon_Count(lua_tonumber(L, 1)));
	return 1;
}
int Lua_Get_Mine_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Mine_Limit());
	return 1;
}
int Lua_Get_C4_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_C4_Mode(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}
int Lua_Get_C4_Planter(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_C4_Planter(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}
int Lua_Get_C4_Attached(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_C4_Attached(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}
int Lua_Get_Beacon_Planter(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Beacon_Planter(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

int Lua_Create_Object(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->GetX(), a->GetY(), a->GetZ());
	GameObject *obj = Commands->Create_Object(lua_tostring(L, 1), pos);
	if (obj)
	{
		lua_pushnumber(L, Get_ID(obj));
	}
	else
	{
		return 0;
	}
	return 1;
}
int Lua_Destroy_Object(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Destroy_Object(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

int Lua_Get_Preset_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Commands->Get_Preset_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr)
	{
		return 0;
	}
	lua_pushstring(L, ptr);
	return 1;
}

int Lua_Get_Position(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Vector3 pos;

	if (lua_gettop(L) >= 2 
	 && lua_tobooleanCPP(L, 2) == true)
	{
		pos = PhysicsSceneClassLua::Get_Instance()->Get_Position(lua_tonumber(L, 1));
	}
	else
	{
		pos = Commands->Get_Position(Commands->Find_Object(lua_tonumber(L, 1)));
	}

	LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);

	return 1;
}
int Lua_Enable_Vehicle_Transitions(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Enable_Vehicle_Transitions(Commands->Find_Object(lua_tonumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 0;
}

int Lua_Set_Model(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Set_Model(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2));
	return 0;
}

int Lua_Goto_Location(lua_State *L)
{
	if (lua_gettop(L) < 7) return 0;
	ActionParamsStruct aps;
	aps.Set_Movement(Vector3(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tobooleanCPP(L, 7));
	Commands->Action_Goto(Commands->Find_Object(lua_tonumber(L, 1)), aps);
	return 0;
}
int Lua_Goto_Object(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	ActionParamsStruct aps;
	aps.Set_Movement(Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3), lua_tonumber(L, 4));
	Commands->Action_Goto(Commands->Find_Object(lua_tonumber(L, 1)), aps);
	return 0;
}

int Lua_Disable_Physical_Collisions(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Disable_Physical_Collisions(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

int Lua_Enable_Collisions(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Enable_Collisions(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

int Lua_Random_Building(lua_State *L)
{
	GameObject *obj = Find_Building_By_Type(Commands->Get_Random_Int(0, 1), Commands->Get_Random_Int(0, 9));
	while (!obj)
	{
		obj = Find_Building_By_Type(Commands->Get_Random_Int(0, 1), Commands->Get_Random_Int(0, 9));
	}
	lua_pushnumber(L, Commands->Get_ID(obj));
	return 1;
}

int Lua_Get_Current_Map(lua_State *L)
{
	lua_pushstring(L, The_Game()->Get_Map_Name());
	return 1;
}

int Lua_Get_Next_Map(lua_State *L)
{
	const char *map;
	map = Get_Map(Get_Current_Map_Index() + 1);
	if (!map) {
		map = Get_Map(0);
	}
	lua_pushstring(L, map);
	return 1;
}

int Lua_Set_Position(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
			Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
			Commands->Set_Position(obj, pos);
		}
	}
	return 0;
}
int Lua_Apply_Damage(lua_State* L)
{
	int argc = lua_gettop(L);
	if (argc < 3)
	{
		return 0;
	}
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj)
	{
		float amount = (float)lua_tonumber(L, 2);
		const char* type = lua_tostring(L, 3);
		GameObject* obj2 = (argc > 3 ? Commands->Find_Object(lua_tonumber(L, 4)) : 0);
		Commands->Apply_Damage(obj, amount, type, obj2);
	}
	return 0;
}
int Lua_Get_Facing(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	if (lua_gettop(L) >= 2 && lua_tobooleanCPP(L, 2) == true)
	{
		PhysClass* obj = PhysicsSceneClassLua::Get_Instance()->Get_Static_Object_By_ID(lua_tonumber(L, 1));
		if (obj)
			lua_pushnumber(L, obj->Get_Facing());
		else
			return 0;
	}
	else
	{
		GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
		if (obj)
			lua_pushnumber(L, Commands->Get_Facing(obj));
		else
			return 0;
	}

	return 1;
}
int Lua_Set_Facing(lua_State *L)
{
	if (lua_gettop(L) == 2) 
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Commands->Set_Facing(obj, (float)lua_tonumber(L, 2));
		}
	}
	return 0;
}

int Lua_Set_Clouds(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Set_Clouds(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}

int Lua_Set_Ash(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	bool enable = (lua_tonumber(L, 3) > 0 ? 1 : 0);
	Commands->Set_Ash(lua_tonumber(L, 1), lua_tonumber(L, 2), enable);
	return 0;
}

int Lua_Set_Rain(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	bool enable = (lua_tonumber(L, 3) > 0 ? 1 : 0);
	Commands->Set_Rain(lua_tonumber(L, 1), lua_tonumber(L, 2), enable);
	return 0;
}

int Lua_Set_Snow(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	bool enable = (lua_tonumber(L, 3) > 0 ? 1 : 0);
	Commands->Set_Snow(lua_tonumber(L, 1), lua_tonumber(L, 2), enable);
	return 0;
}

int Lua_Set_Wind(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Commands->Set_Wind(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}

int Lua_Attach_Script(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Attach_Script(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3));
	return 0;
}

int Lua_Create_Object_At_Bone(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	lua_pushnumber(L, Commands->Get_ID(Commands->Create_Object_At_Bone(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3))));
	return 1;
}

int Lua_Attach_To_Object_Bone(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Attach_To_Object_Bone(Commands->Find_Object(lua_tonumber(L, 1)), Commands->Find_Object(lua_tonumber(L, 2)), lua_tostring(L, 3));
	return 0;
}

int Lua_Display_Health_Bar(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Display_Health_Bar(Commands->Find_Object(lua_tonumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 0;
}

int Lua_Create_Script_Zone(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc == 7)
	{
		OBBoxClass box;
		Vector3 v;
		v.X = lua_tonumber(L, 2);
		v.Y = lua_tonumber(L, 3);
		v.Z = lua_tonumber(L, 4);
		box.Center = v;//a->box.Center;
		Vector3 v2;
		v2.X = lua_tonumber(L, 5);
		v2.Y = lua_tonumber(L, 6);
		v2.Z = lua_tonumber(L, 7);
		box.Extent = v2;
		lua_pushnumber(L, Commands->Get_ID(Create_Zone(lua_tostring(L, 1), box)));
		return 1;
	}
	else if (argc == 2)
	{
		Box *lbox = LuaBox::GetInstance(L, 2);
		lua_pushnumber(L, Commands->Get_ID(Create_Zone(lua_tostring(L, 1), lbox->box)));
		return 1;
	}
	return 0;
}

int Lua_tClock(lua_State *L)
{
	lua_pushnumber(L, clock());
	return 1;
}

int Lua_Invoke(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;

	LuaManager::CallInvoke(lua_tostring(L, 1), lua_tostring(L, 2));
	return 0;
}


int Lua_Enable_Stealth(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	unsigned int obj = luaL_checknumber(L, 1);
	int stealth = luaL_checknumber(L, 2);
	GameObject *o = Commands->Find_Object(obj);
	if (o)
	{
		Commands->Enable_Stealth(o, stealth == 0 ? false : true);
	}
	else
	{
		luaL_argerror(L, 1, "Not a valid GameObject");
	}
	return 0;
}

int Lua_Get_All_Objects(lua_State *L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<BaseGameObj>* node = GameObjManager::GameObjList.Head(); node; node = node->Next())
	{
		GameObject *o = (GameObject *)node->Data();

		lua_pushnumber(L, Commands->Get_ID(o));
		lua_rawseti(L, buildingTable, index++);
	}
	return 1;
}

int Lua_Set_Shield_Type(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *o = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!o) return 0;

	const char *s = luaL_checkstring(L, 2);
	if (!s) return 0;

	Commands->Set_Shield_Type(o, s);
	return 0;
}

int Lua_FDSMessage(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	const char *m = luaL_checkstring(L, 1);
	const char *h = luaL_checkstring(L, 2);
	SSGMGameLog::Log_Message(m, h);
	return 0;

}

int Lua_Set_Health(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Commands->Set_Health(obj, luaL_checknumber(L, 2));
	return 0;
}

int Lua_Set_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Commands->Set_Shield_Strength(obj, luaL_checknumber(L, 2));
	return 0;

}


int Lua_The_Cnc_Game(lua_State* L)
{
	lua_newtable(L);
	cGameDataCnc *data = The_Cnc_Game();
	if (!data) { return 0; }
	lua_pushboolean(L, data->Base_Destruction_Ends_Game());
	lua_setfield(L, -2, "BaseDestructionEndsGame");
	lua_pushboolean(L, data->Beacon_Placement_Ends_Game());
	lua_setfield(L, -2, "BeaconPlacementEndsGame");
	lua_pushnumber(L, data->Get_Starting_Credits());
	lua_setfield(L, -2, "StartingCredits");
	return 1;
}


int Lua_The_Game(lua_State *L)
{
	lua_newtable(L);
	cGameData *data = The_Game();
	if (!data) { return 0; }
	lua_pushboolean(L, data->Can_Repair_Buildings());
	lua_setfield(L, -2, "CanRepairBuildings");
	lua_pushnumber(L, data->Get_Current_Players());
	lua_setfield(L, -2, "CurrentPlayers");
	lua_pushboolean(L, data->Do_Maps_Loop());
	lua_setfield(L, -2, "DoMapsLoop");
	lua_pushboolean(L, data->Driver_Is_Always_Gunner());
	lua_setfield(L, -2, "DriverIsAlwaysGunner");
	lua_pushnumber(L, data->Get_Game_Duration_S());
	lua_setfield(L, -2, "GameDuration_Seconds");
	char GameTitle[256];
	wcstombs(GameTitle, (const wchar_t *)data->Get_Game_Title(), 256);
	lua_pushstring(L, GameTitle);
	lua_setfield(L, -2, "GameTitle");
	lua_pushnumber(L, data->Get_Intermission_Time_Remaining());
	lua_setfield(L, -2, "IntermissionTimeLeft");
	lua_pushnumber(L, data->Get_Ip_Address());
	lua_setfield(L, -2, "IP");
	lua_pushboolean(L, data->Is_Auto_Restart());
	lua_setfield(L, -2, "IsAutoRestart");
	lua_pushboolean(L, data->Is_Clan_Game());
	lua_setfield(L, -2, "IsClanGame");
	lua_pushboolean(L, data->Is_Dedicated());
	lua_setfield(L, -2, "IsDedicated");
	lua_pushboolean(L, data->Is_Friendly_Fire_Permitted());
	lua_setfield(L, -2, "IsFriendlyFirePermitted");
	lua_pushboolean(L, data->Is_Laddered());
	lua_setfield(L, -2, "IsLaddered");
	lua_pushboolean(L, data->Is_Passworded());
	lua_setfield(L, -2, "IsPassworded");
	lua_pushboolean(L, data->Is_QuickMatch_Server());
	lua_setfield(L, -2, "IsQuickMatch");
	lua_pushboolean(L, data->Is_Team_Changing_Allowed());
	lua_setfield(L, -2, "IsTeamChangingAllowed");
	lua_pushboolean(L, data->Is_Map_Cycle_Over());
	lua_setfield(L, -2, "MapCycleOver");
	lua_pushstring(L, data->Get_Map_Name());
	lua_setfield(L, -2, "MapName");
	lua_pushnumber(L, data->Get_Map_Number());
	lua_setfield(L, -2, "MapNumber");
	lua_pushnumber(L, data->Get_Max_Players());
	lua_setfield(L, -2, "MaxPlayers");
	lua_pushnumber(L, data->Get_Maximum_World_Distance());
	lua_setfield(L, -2, "MaxWorldDistance");
	lua_pushstring(L, data->Get_Mod_Name());
	lua_setfield(L, -2, "ModName");
	char Motd[256];
	wcstombs(Motd, (const wchar_t *)data->Get_Motd(), 256);
	lua_pushstring(L, Motd);
	lua_setfield(L, -2, "Motd");
	lua_pushnumber(L, data->Get_Mvp_Count());
	lua_setfield(L, -2, "MVPCount");
	char MVPName[256];
	wcstombs(MVPName, (const wchar_t *)data->Get_Mvp_Name(), 256);
	lua_pushstring(L, MVPName);
	lua_setfield(L, -2, "MVPName");
	char Owner[256];
	wcstombs(Owner, (const wchar_t *)data->Get_Owner(), 256);
	lua_pushstring(L, Owner);
	lua_setfield(L, -2, "Owner");
	char Password[256];
	wcstombs(Password, (const wchar_t *)data->Get_Password(), 256);
	lua_pushstring(L, Password);
	lua_setfield(L, -2, "Password");
	lua_pushnumber(L, data->Get_Port());
	lua_setfield(L, -2, "Port");
	lua_pushnumber(L, data->Get_Radar_Mode());
	lua_setfield(L, -2, "RadarMode");
	lua_pushboolean(L, data->Is_Remix_Teams());
	lua_setfield(L, -2, "RemixTeams");
	lua_pushboolean(L, data->Spawn_Weapons());
	lua_setfield(L, -2, "SpawnWeapons");
	lua_pushboolean(L, data->Do_String_Versions_Match());
	lua_setfield(L, -2, "StringVersionsMatch");
	lua_pushnumber(L, data->Get_Time_Limit_Minutes());
	lua_setfield(L, -2, "TimeLimit_Minutes");
	lua_pushnumber(L, data->Get_Time_Remaining_Seconds());
	lua_setfield(L, -2, "TimeRemaining_Seconds");
	lua_pushnumber(L, data->Get_Winner_ID());
	lua_setfield(L, -2, "WinnerID");
	lua_pushnumber(L, data->Get_Win_Type());
	lua_setfield(L, -2, "WinType");
	lua_pushboolean(L, data->Is_Intermission());
	lua_setfield(L, -2, "IsIntermission");
	lua_pushnumber(L, data->Get_Intermission_Time_Seconds());
	lua_setfield(L, -2, "IntermissionTime_Seconds");
	return 1;
}


int Lua_cPlayer(lua_State *L)
{

	int ID = luaL_checknumber(L, 1);
	if (ID == 0)
	{
		return 0;
	}

	cPlayer *p = Find_Player(ID);

	if (p)
	{
		lua_newtable(L);
		lua_pushnumber(L, p->Get_Cached_Priority());
		lua_setfield(L, -2, "Cached_Priority");
		lua_pushnumber(L, p->Get_Damage_Scale_Factor());
		lua_setfield(L, -2, "Damage_Scale_Factor");
		lua_pushnumber(L, p->Get_Deaths());
		lua_setfield(L, -2, "Deaths");
		lua_pushnumber(L, p->Get_Fast_Sort_Key());
		lua_setfield(L, -2, "Fast_Sort_Key");
		lua_pushnumber(L, p->Get_Filter_Distance());
		lua_setfield(L, -2, "Filter_Distance");
		lua_pushnumber(L, p->Get_Id());
		lua_setfield(L, -2, "Id");
		lua_pushnumber(L, p->Get_Import_State_Count());
		lua_setfield(L, -2, "Import_State_Count");
		lua_pushboolean(L, p->Get_Invulnerable());
		lua_setfield(L, -2, "Invulnerable");
		lua_pushnumber(L, p->Get_Ip_Address());
		lua_setfield(L, -2, "Ip_Address");
		lua_pushnumber(L, p->Get_Is_Active());
		lua_setfield(L, -2, "Is_Active");
		lua_pushboolean(L, p->Get_Is_In_Game());
		lua_setfield(L, -2, "Is_In_Game");
		lua_pushboolean(L, p->Get_Is_Waiting_For_Intermission());
		lua_setfield(L, -2, "Is_Waiting_For_Intermission");
		lua_pushnumber(L, p->Get_Join_Time());
		lua_setfield(L, -2, "Join_Time");
		lua_pushnumber(L, p->Get_Kills());
		lua_setfield(L, -2, "Kills");
		lua_pushnumber(L, p->Get_Kill_To_Death_Ratio());
		lua_setfield(L, -2, "Kill_To_Death_Ratio");
		lua_pushnumber(L, p->Get_Ladder_Points());
		lua_setfield(L, -2, "Ladder_Points");
		lua_pushnumber(L, p->Get_Last_Object_Id_I_Damaged());
		lua_setfield(L, -2, "Last_Object_Id_I_Damaged");
		lua_pushnumber(L, p->Get_Last_Object_Id_I_Got_Damaged_By());
		lua_setfield(L, -2, "Last_Object_Id_I_Got_Damaged_By");
		lua_pushnumber(L, p->Get_Money());
		lua_setfield(L, -2, "Money");
		lua_pushnumber(L, p->Get_Network_Class_ID());
		lua_setfield(L, -2, "Network_Class_ID");
		lua_pushnumber(L, p->Get_Network_ID());
		lua_setfield(L, -2, "Network_ID");
		lua_pushnumber(L, p->Get_Num_WOL_Games());
		lua_setfield(L, -2, "Num_WOL_Games");
		lua_pushnumber(L, p->Get_Player_Type());
		lua_setfield(L, -2, "Player_Type");
		lua_pushnumber(L, p->Get_Punish_Timer());
		lua_setfield(L, -2, "Punish_Timer");
		lua_pushnumber(L, p->Get_Rung());
		lua_setfield(L, -2, "Rung");
		lua_pushnumber(L, p->Get_Score());
		lua_setfield(L, -2, "Score");
		lua_pushnumber(L, p->Get_Session_Time());
		lua_setfield(L, -2, "Session_Time");
		lua_pushnumber(L, p->Get_Total_Time());
		lua_setfield(L, -2, "Total_Time");
		lua_pushnumber(L, p->Get_Vis_ID());
		lua_setfield(L, -2, "Vis_ID");
		lua_pushnumber(L, p->Fps);
		lua_setfield(L, -2, "Fps");
		return 1;
	}
	return 1;
}

int Lua_LongToIP(lua_State*L)
{
	if (lua_gettop(L) < 1) return 0;
	unsigned long lIP = luaL_checknumber(L, 1);
	unsigned char *IP = (unsigned char *)&lIP;
	lua_pushnumber(L, IP[3]);
	lua_pushnumber(L, IP[2]);
	lua_pushnumber(L, IP[1]);
	lua_pushnumber(L, IP[0]);
	return 4;
}

int Lua_IPToLong(lua_State*L)
{
	if (lua_gettop(L) < 4) return 0;
	unsigned long IP = 0;
	unsigned char *x = (unsigned char *)&IP;
	x[3] = luaL_checknumber(L, 1);
	x[2] = luaL_checknumber(L, 2);
	x[1] = luaL_checknumber(L, 3);
	x[0] = luaL_checknumber(L, 4);
	char str[256];
	sprintf(str, "%lu", IP);
	lua_pushstring(L, str);
	return 1;
}


int Lua_GetTimeRemaining(lua_State *L)
{
	lua_pushnumber(L, The_Game()->Get_Time_Remaining_Seconds());
	//lua_pushnumber(L, The_Game()->Get_Game_Duration_S() > 0 ? The_Game()->Get_Time_Remaining_Seconds() : -1);
	return 1;
}

int Lua_Get_Health(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Health(x));
	return 1;
}

int Lua_Get_Max_Health(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Max_Health(x));
	return 1;
}

int Lua_Get_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Shield_Strength(x));
	return 1;
}

int Lua_Get_Max_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Max_Shield_Strength(x));
	return 1;
}


int Lua_Display_GDI_Player_Terminal_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *o = Commands->Find_Object(luaL_checknumber(L, 1));
	if (o)
	{
		Display_GDI_Player_Terminal_Player(o);
	}
	return 0;
}

int Lua_Display_Nod_Player_Terminal_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *o = Commands->Find_Object(luaL_checknumber(L, 1));
	if (o)
	{
		Display_NOD_Player_Terminal_Player(o);
	}
	return 0;
}

int Lua_Get_IDobj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;
	int n = Commands->Get_ID(x);
	lua_pushnumber(L, n);
	return 1;
}


int Lua_Get_Map_By_Number(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	const char *map;
	map = Get_Map(luaL_checknumber(L, 1));
	if (!map) {
		lua_pushstring(L, "NULL");
		return 1;
	}
	lua_pushstring(L, map);
	return 1;
}

int Lua_Spectate(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (obj)
	{
		Vector3 pos = Commands->Get_Position(obj);
		Remove_Script(obj, "MDB_Weapon_Scope");
		Commands->Clear_Weapons(obj);
		//Change_Team_2(obj, 2);
		Commands->Set_Is_Visible(obj, false);
		Commands->Set_Model(obj, "null");
		Commands->Set_Shield_Type(obj, "Blamo");
		bool flying;
		flying = Get_Fly_Mode(obj);
		if (!flying)
		{
			Toggle_Fly_Mode(obj);
		}
		Commands->Disable_All_Collisions(obj);
		//Set_Obj_Radar_Blip_Shape_Team(1, obj, 0);
		//Set_Obj_Radar_Blip_Shape_Team(0, obj, 0);
		pos.Z += 3.0f;
		Commands->Set_Position(obj, pos);
		return 1;


	}
	return 0;
}


int Lua_Set_Background_Music(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Commands->Set_Background_Music(lua_tostring(L, 1));
	return 1;
}

int Lua_Get_Background_Music(lua_State *L)
{
	lua_pushstring(L, GetCurrentMusicTrack());
	return 1;
}


int Lua_Kill_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	GameObject *obj = Get_GameObj(luaL_checknumber(L, 1));
	if (!obj) { return 0; }

	GameObject *Veh = Get_Vehicle(obj);
	if (Veh)
	{
		if (Get_Vehicle_Driver(Veh) == obj)
		{
			Commands->Apply_Damage(Veh, 99999.0f, "Death", false);
		}
		Commands->Destroy_Object(obj);
	}

	Commands->Apply_Damage(obj, 99999.0f, "Death", false);
	return 1;
}


int Lua_Control_Enable(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			Commands->Control_Enable(obj, lua_tobooleanCPP(L, 2));
			return 1;
		}
	}
	return 0;
}

int Lua_Get_BW_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Get_Bandwidth(luaL_checknumber(L, 1)));
	return 1;
}


int Lua_Get_Vehicles_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Vehicle_Limit());
	return 1;
}

int Lua_Get_Player_Version(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Client_Version(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Kick_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Evict_Client(lua_tonumber(L, 1), lua_tostring(L, 2));
	return 1;
}


int Lua_Add_Console_Hook(lua_State *L)
{
	AddConsoleOutputHook(LuaManager::Call_Console_Output_Hook);
	AddHostHook(LuaManager::Call_Host_Hook);
	return 1;
}


int Lua_Remove_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) {
		return 0;
	}

	Remove_Weapon(obj, lua_tostring(L, 2));
	return 1;
}

int Lua_Remove_All_Weapons(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Clear_Weapons(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}


int Lua_Get_Preset_Name_By_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Translated_Definition_Name_Ini(Get_Definition_Name(luaL_checknumber(L, 1))));
	return 1;
}

int Lua_Get_Client_Serial_Hash(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	int pID = luaL_checknumber(L, 1);
	if (!pID) { return 0; }
	lua_pushstring(L, Get_Client_Serial_Hash(pID));
	return 1;
}

int Lua_Create_Explosion(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;

	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 3));

	Commands->Create_Explosion(lua_tostring(L, 1), pos, obj);
	return 1;
}

int Lua_Create_Explosion_At_Bone(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 2));
	GameObject *creator = Commands->Find_Object(lua_tonumber(L, 4));
	Commands->Create_Explosion_At_Bone(lua_tostring(L, 1), obj, lua_tostring(L, 3), creator);
	return 1;
}

int Lua_Set_Fog_Enable(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	int stealth = luaL_checknumber(L, 1);
	Commands->Set_Fog_Enable(stealth == 0 ? false : true);
	return 1;
}

int Lua_Set_Fog_Range(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Set_Fog_Range(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 1;
}

int Lua_Set_War_Blitz(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	Commands->Set_War_Blitz(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));
	return 1;
}

int Lua_Play_Building_Announcement(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Commands->Play_Building_Announcement(obj, luaL_checkinteger(L,2));
		}
	}
	return 0;
}

int Lua_Shake_Camera(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());

	Commands->Shake_Camera(pos, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 1;
}

int Lua_Add_RadioHook(lua_State *L)
{
	AddRadioHook(LuaManager::RadioHook);
	return 1;
}

int Lua_Is_Crate(lua_State *L) 
{
	if (lua_gettop(L) < 1) return 0;
	// this is how is done in SSGM.
	bool craate = false;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_PowerUpGameObj())
	{
		if (stristr(Commands->Get_Preset_Name(obj), "crate"))
		{
			craate = true;
		}
	}

	lua_pushboolean(L, craate);
	return 1;
}

int Lua_Set_Air_Vehicle_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Set_Air_Vehicle_Limit(lua_tonumber(L, 1));
	return 1;
}

int Lua_Get_Air_Vehicle_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Air_Vehicle_Limit());
	return 1;
}

int Lua_Set_Vehicle_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Set_Vehicle_Limit(lua_tonumber(L, 1));
	return 1;
}


int Lua_Get_Vehicle_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Vehicle_Limit());
	return 1;
}

int Lua_Force_Camera_Look_Player(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
			Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
			Force_Camera_Look_Player(obj, pos);
		}
	}
	return 0;
}

int Lua_Set_Screen_Fade_Opacity_Player(lua_State *L)
{
	if (lua_gettop(L) == 3)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Set_Screen_Fade_Opacity_Player(obj, luaL_checknumber(L,2), luaL_checknumber(L, 3));
		}
	}
	return 0;
}


int Lua_Set_Screen_Fade_Color_Player(lua_State *L)
{
	if (lua_gettop(L) == 5)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Set_Screen_Fade_Color_Player(obj, luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5));
		}
	}
	return 0;
}

int Lua_Enable_Radar_Player(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			int stealth = luaL_checknumber(L, 2);
			Enable_Radar_Player(obj, stealth == 0 ? false : true);
		}
	}
	return 0;
}

int Lua_Get_Build_Time_Multiplier(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Get_Build_Time_Multiplier(lua_tonumber(L, 1)));
	return 1;
}

int Lua_Stop_Background_Music_Player(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Stop_Background_Music_Player(obj);
		}
	}
	return 0;
}

int Lua_Change_Time_Remaining(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Change_Time_Remaining(lua_tonumber(L, 1));
	return 1;
}

int Lua_Change_Time_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Change_Time_Limit(lua_tonumber(L, 1));
	return 1;
}

int Lua_Create_3D_WAV_Sound_At_Bone(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	lua_pushnumber(L, Commands->Create_3D_WAV_Sound_At_Bone(lua_tostring(L, 1), Commands->Find_Object(lua_tonumber(L, 2)), lua_tostring(L, 3)));
	return 1;
}


int Lua_Send_Message(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;

	Send_Message(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tostring(L, 4));
	return 1;
}

int Lua_Select_Weapon(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Commands->Select_Weapon(obj, luaL_checkstring(L,2));
		}
	}
	return 0;
}

int Lua_Create_2D_Sound_Player(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			lua_pushnumber(L, Create_2D_Sound_Player(obj, luaL_checkstring(L, 2)));
			return 1;
		}
	}
	return 0;
}

int Lua_Create_2D_Sound(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Commands->Create_2D_WAV_Sound(lua_tostring(L, 1)));
	return 1;
}

int Lua_Resolve_IP(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	hostent *host;
	in_addr ipHost;
	ipHost.s_addr = inet_addr(lua_tostring(L, 1));
	host = gethostbyaddr((const char*)&ipHost, sizeof(struct in_addr), AF_INET);
	//host = gethostbyname(lua_tostring(L, 1));

	if (host) {
		lua_pushstring(L, host->h_name);
	}
	else {
		lua_pushstring(L, "ERROR");
	}

	return 1;
}

int Lua_Get_Build_Version(lua_State *L)
{
	lua_pushnumber(L, 1);
	return 1;
}

int Lua_Get_Distance(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	LUA_Vector3 *a2 = LuaVector3::GetInstance(L, 1);
	Vector3 pos1 = Vector3(a2->X(), a2->Y(), a2->Z());

	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 pos2 = Vector3(a->X(), a->Y(), a->Z());

	lua_pushnumber(L, Commands->Get_Distance(pos1, pos2));
	return 1;
}

// Windows API


int Lua_Get_File_List(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	char basepath[64];
	char folderlist[256];
	std::vector<std::string> fileslist;

	GetCurrentDirectory(64, basepath);
	sprintf(folderlist, "%s\\%s", basepath, lua_tostring(L, 1));

	GetFiles(std::string(folderlist), &fileslist);
	lua_newtable(L);

	for (std::string x : fileslist)
	{
		lua_pushstring(L, x.c_str());
		lua_setfield(L, -2, x.c_str());
	}

	return 1;
}

// Lua reload


int Lua_Reload_Lua(lua_State *L)
{
	for (auto L : LuaManager::Lua)
	{
		lua_getglobal(L, "UnloadingLua");
		if (lua_isfunction(L, -1))
		{
			LuaManager::Report_Errors(L, lua_pcall(L, 0, 0, 0));
		}
		else
		{
			lua_setglobal(L, "UnloadingLua");
		}
	}
	LuaManager::Reload_Flag = true;
	return 1;
}

int Lua_Set_Animation(lua_State *L)
{
	if (lua_gettop(L) < 7) return 0;

	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) { return 0; }
	const char *anim = lua_tostring(L, 2);
	bool loop = lua_tobooleanCPP(L, 3);
	const char *subobj = lua_tostring(L, 4);
	float start = lua_tonumber(L, 5);
	float end = lua_tonumber(L, 6);
	bool blend = lua_tobooleanCPP(L, 7);

	if (strcmp(subobj, "") == 0) {
		subobj = 0;
	}

	if (blend != 0 && blend != 1) { blend = 0; }
	if (loop != 0 && loop != 1) { loop = 0; }

	Commands->Set_Animation(obj, anim, loop, subobj, start, end, blend);
	return 1;
}

int Lua_Add_TTDamageHook(lua_State *L)
{
	AddTtDamageHook(LuaManager::TT_Damage_Hook);
	return 1;
}

int Lua_Restore_Building(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Restore_Building(obj);
		}
	}
	return 0;
}

int Lua_Revive_Building(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Restore_Building(obj);
			float max = Commands->Get_Max_Health(obj);
			Commands->Set_Health(obj, max);

			if (Is_Base_Powered(Get_Object_Type(obj)))
			{
				Commands->Set_Building_Power(obj, true);
			}
			else
			{
				Commands->Set_Building_Power(obj, false);
			}
		}
	}
	return 0;
}

int Lua_Create_Building(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;

	LUA_Vector3 *a2 = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a2->X(), a2->Y(), a2->Z());
	const char *pres = lua_tostring(L, 1);

	GameObject * obj = Create_Building(pres, pos);
	lua_pushnumber(L, Commands->Get_ID(obj));
	return 1;
}


int Lua_Damage_Occupants(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	int ob = lua_tonumber(L, 1);
	Damage_Occupants(Commands->Find_Object(ob), lua_tonumber(L, 2), lua_tostring(L, 3));
	return 1;
}


int Lua_Set_Kills(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Kills(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 1;
}


int Lua_Set_cPlayer(lua_State *L)
{
	int pID = luaL_checknumber(L, 1);
	if(!lua_istable(L, 2))
	{
		luaL_error(L, "Invalid argument #2 to Set_cPlayer. Expected a table.");
		return 0;
	}

	cPlayer *p = Find_Player(pID);
	if (!p) return 0;

	lua_getfield(L, -1, "Name");
	if (lua_type(L, -1) == LUA_TSTRING) 
	{
		auto str = CharToWideChar(lua_tostring(L, -1));
		p->Set_Name(str);
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "PlayerId");
	if (lua_type(L, -1) == LUA_TNUMBER) p->Set_Id(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Kills");
	if (lua_type(L, -1) == LUA_TNUMBER) p->Set_Kills(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Deaths");
	if (lua_type(L, -1) == LUA_TNUMBER) p->Set_Deaths(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "PlayerType");
	if (lua_type(L, -1) == LUA_TNUMBER) p->Set_Player_Type(lua_tonumber(L, -1));
	lua_pop(L, 1);

#ifdef DAPLUGIN
	lua_getfield(L, -1, "AlliesKilled");
	if(lua_type(L, -1) > 0) p->AlliesKilled = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ArmHit");
	if(lua_type(L, -1) > 0) p->ArmHit = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ArmShots");
	if(lua_type(L, -1) > 0) p->ArmShots = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "BuildingDestroyed");
	if(lua_type(L, -1) > 0) p->BuildingDestroyed = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "CreditGrant");
	if(lua_type(L, -1) > 0) p->CreditGrant = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "CrotchHit");
	if(lua_type(L, -1) > 0) p->CrotchHit = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "CrotchShots");
	if(lua_type(L, -1) > 0) p->CrotchShots = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "DamageScaleFactor");
	if(lua_type(L, -1) > 0) p->DamageScaleFactor = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "EnemiesKilled");
	if(lua_type(L, -1) > 0) p->EnemiesKilled = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "GameTime");
	if(lua_type(L, -1) > 0) p->GameTime = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "HeadHit");
	if(lua_type(L, -1) > 0) p->HeadHit = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "HeadShots");
	if(lua_type(L, -1) > 0) p->HeadShots = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "KillsFromVehicle");
	if(lua_type(L, -1) > 0) p->KillsFromVehicle = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "LegHit");
	if(lua_type(L, -1) > 0) p->LegHit = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "LegShots");
	if(lua_type(L, -1) > 0) p->LegShots = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Money");
	if(lua_type(L, -1) > 0) p->Money = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Ping");
	if(lua_type(L, -1) > 0) p->Ping = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "PowerupsCollected");
	if(lua_type(L, -1) > 0) p->PowerupsCollected = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Rung");
	if(lua_type(L, -1) > 0) p->Rung = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Score");
	if(lua_type(L, -1) > 0) p->Score = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "SessionTime");
	if(lua_type(L, -1) > 0) p->SessionTime = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ShotsFired");
	if(lua_type(L, -1) > 0) p->ShotsFired = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Squishes");
	if(lua_type(L, -1) > 0) p->Squishes = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "TorsoHit");
	if(lua_type(L, -1) > 0) p->TorsoHit = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "TorsoShots");
	if(lua_type(L, -1) > 0) p->TorsoShots = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "TotalTime");
	if(lua_type(L, -1) > 0) p->TotalTime = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "VehiclesDestroyed");
	if(lua_type(L, -1) > 0) p->VehiclesDestroyed = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "VehicleTime");
	if(lua_type(L, -1) > 0) p->VehicleTime = lua_tonumber(L, -1);
	lua_pop(L, 1);
#endif

	p->Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
	return 1;
}

/* can't be fixed. only if TT will unblock them. they are protected.
int Lua_Set_The_Game(lua_State *L)
{
if(!lua_istable(L, -1))
{
luaL_error(L, "Invalid argument #1 to Set_The_Game. Expected table.");
return 0;
}
cGameData *data = The_Game();
lua_getfield(L, -1, "CanRepairBuildings");
if(lua_type(L, -1) > 0) data->Can_Repair_Buildings = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "DoMapsLoop");
if(lua_type(L, -1) > 0) data->Do_Maps_Loop = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "DriverIsAlwaysGunner");
if(lua_type(L, -1) > 0) data->DriverIsAlwaysGunner = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "GameplayPermitted");
if(lua_type(L, -1) > 0) data->GameplayPermitted = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "GameTitle");
if(lua_type(L, -1) > 0) data->GameTitle.Convert_From(lua_tostring(L, -1));
lua_pop(L, 1);

lua_getfield(L, -1, "GrantWeapons");
if(lua_type(L, -1) > 0) data->GrantWeapons = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IntermissionTime_Seconds");
if(lua_type(L, -1) > 0) data->IntermissionTime_Seconds = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IntermissionTimeLeft");
if(lua_type(L, -1) > 0) data->IntermissionTimeLeft = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsAutoRestart");
if(lua_type(L, -1) > 0) data->IsAutoRestart = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsClanGame");
if(lua_type(L, -1) > 0) data->IsClanGame = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsDedicated");
if(lua_type(L, -1) > 0) data->IsDedicated = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsFriendlyFirePermitted");
if(lua_type(L, -1) > 0) data->IsFriendlyFirePermitted = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsLaddered");
if(lua_type(L, -1) > 0) data->IsLaddered = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsPassworded");
if(lua_type(L, -1) > 0) data->IsPassworded = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsQuickMatch");
if(lua_type(L, -1) > 0) data->IsQuickMatch = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "IsTeamChangingAllowed");
if(lua_type(L, -1) > 0) data->IsTeamChangingAllowed = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "MapCycleOver");
if(lua_type(L, -1) > 0) data->MapCycleOver = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "MapNumber");
if(lua_type(L, -1) > 0) data->MapNumber = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "MaxPlayers");
if(lua_type(L, -1) > 0) data->MaxPlayers = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "MaxWorldDistance");
if(lua_type(L, -1) > 0) data->MaxWorldDistance = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "MinQualifyingTime_Minutes");
if(lua_type(L, -1) > 0) data->MinQualifyingTime_Minutes = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "Motd");
if(lua_type(L, -1) > 0) data->Motd.Convert_From(lua_tostring(L, -1));
lua_pop(L, 1);

lua_getfield(L, -1, "MVPCount");
if(lua_type(L, -1) > 0) data->MVPCount = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "MVPName");
if(lua_type(L, -1) > 0) data->MVPName.Convert_From(lua_tostring(L, -1));
lua_pop(L, 1);

lua_getfield(L, -1, "Password");
if(lua_type(L, -1) > 0) data->Password.Convert_From(lua_tostring(L, -1));
lua_pop(L, 1);

lua_getfield(L, -1, "RadarMode");
if(lua_type(L, -1) > 0) data->RadarMode = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "RemixTeams");
if(lua_type(L, -1) > 0) data->RemixTeams = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "SettingsDescription");
if(lua_type(L, -1) > 0) data->SettingsDescription.Convert_From(lua_tostring(L, -1));
lua_pop(L, 1);

lua_getfield(L, -1, "SpawnWeapons");
if(lua_type(L, -1) > 0) data->SpawnWeapons = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "StringVersionsMatch");
if(lua_type(L, -1) > 0) data->StringVersionsMatch = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "TimeLimit_Minutes");
if(lua_type(L, -1) > 0) data->TimeLimit_Minutes = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "TimeRemaining_Seconds");
if(lua_type(L, -1) > 0) data->TimeRemaining_Seconds = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "UseLagReduction");
if(lua_type(L, -1) > 0) data->UseLagReduction = lua_tobooleanCPP(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "WinnerID");
if(lua_type(L, -1) > 0) data->WinnerID = lua_tonumber(L, -1);
lua_pop(L, 1);

lua_getfield(L, -1, "WinType");
if(lua_type(L, -1) > 0) data->WinType = lua_tonumber(L, -1);
lua_pop(L, 1);

return 0;
}

*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
void inline MemoryWrite(void *address, unsigned char c)
{
	DWORD old;
	VirtualProtect((void *)address, 1, PAGE_EXECUTE_READWRITE, &old);
	memcpy((void *)address, &c, 1);
	VirtualProtect((void *)address, 1, old, &old);
}

void inline MemoryRead(void *address, unsigned char *c)
{
	DWORD old;
	VirtualProtect((void *)address, 1, PAGE_EXECUTE_READWRITE, &old);
	memcpy(c, address, 1);
	VirtualProtect((void *)address, 1, old, &old);
}
#endif

int Lua_Memory_Write(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	unsigned int Address = luaL_checknumber(L, 1);
	if (Address == 0)
	{
		return 0;
	}

	if (!lua_istable(L, 2))
	{
		luaL_typerror(L, 2, lua_typename(L, LUA_TTABLE));
		return 0;
	}

	for (int i = 1;; i++)
	{
		lua_rawgeti(L, 2, i);
		if (lua_type(L, -1) <= 0)
		{
			lua_pop(L, 1);
			break;
		}
		else
		{
			unsigned char c = lua_tonumber(L, -1);
			MemoryWrite((void *)(Address + i - 1), c);
			lua_pop(L, 1);
		}
	}
	return 0;
}

int Lua_Memory_Read(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	unsigned int Address = luaL_checknumber(L, 1);
	int Count = luaL_checknumber(L, 2);

	if (Address == 0 || Count == 0)
	{
		return 0;
	}

	lua_newtable(L);

	for (int i = 0; i < Count; i++)
	{
		unsigned char c;
		MemoryRead((void *)(Address + i), &c);
		lua_pushnumber(L, c);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}



int Lua_Is_Vehicle_Owner(lua_State *L)
{

	if (lua_gettop(L) < 2) return 0;

	GameObject *o = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject *os = Commands->Find_Object(lua_tonumber(L, 2));
#ifdef DAPLUGIN
	cPlayer *owner = DAVehicleManager::Get_Vehicle_Owner(o);
	if (Get_Player_ID(os) > 0
		&& owner != nullptr
		&& owner->PlayerId == Get_Player_ID(os))
	{
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushboolean(L, false);
	}

#else
	SSGM_Vehicle *scr = (SSGM_Vehicle *)Find_Script_On_Object(o, "SSGM_Vehicle");
	if (scr)
	{
		if (Commands->Find_Object(scr->OwnerID) == os)
		{
			lua_pushboolean(L, true);
		}
		else {
			lua_pushboolean(L, false);
		}
	}
#endif
	return 1;
}


int Lua_Warp_Soldier(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;

	GameObject *Soldier = Commands->Find_Object(lua_tonumber(L, 1));
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 WarpPos = Vector3(a->GetX(), a->GetY(), a->GetZ());
	Vector3 WarpPos2 = WarpPos;
	PhysClass *Phys = Soldier->As_PhysicalGameObj()->Peek_Physical_Object();

	if (Phys && Phys->As_MoveablePhysClass())
	{
		if (Phys->As_MoveablePhysClass()->Find_Teleport_Location(WarpPos, 0.5f, &WarpPos)) { // check..
			Commands->Set_Position(Soldier, WarpPos2); // warp
			lua_pushboolean(L, true);
			return 1;
		}
	}
	lua_pushboolean(L, false);
	return 1;
}

int Lua_Set_Max_Speed(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	obj->As_SoldierGameObj()->Set_Max_Speed((float)lua_tonumber(L, 2));
	return 0;
}

int Lua_Get_Max_Speed(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, obj->As_SoldierGameObj()->Get_Max_Speed());
	return 1;
}

//SCRIPTS_API uint Send_Object_Update(NetworkObjectClass* object, int remoteHostId);

int Lua_Hide_Object_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Get_GameObj(luaL_checknumber(L, 2));
	if (!obj) return 0;
	Set_Object_Visibility_For_Player(obj, luaL_checknumber(L, 1), false);
	return 1;
}

int Lua_Set_Is_Powerup_Persistant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	((PowerUpGameObjDef &)(obj->As_PhysicalGameObj()->As_PowerUpGameObj()->Get_Definition())).Persistent = lua_tobooleanCPP(L, 2);
	return 1;
}

int Lua_Is_WOL_User(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	bool n = Is_WOL_User(WideStringClass(lua_tostring(L, 1)).Peek_Buffer());
	lua_pushboolean(L, n);

	return 1;
}

enum OColision_Error
{
	OC_ERROR_NO_GROUND,
	OC_ERROR_BUILDING,
	OC_ERROR_NO_SPACE
};

int Lua_OColision(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	int result = 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	LUA_Vector3 *ab = LuaVector3::GetInstance(L, 2);
	GameObject *Player = Commands->Find_Object(luaL_checknumber(L, 3));
	float Facing = luaL_checknumber(L, 4);
	if (!Player) return 0;

	
	Vector3 Pos = Vector3(a->X(), a->Y(), a->Z());
	Vector3 Size = Vector3(ab->X(), ab->Y(), ab->Z());
	PhysClass *PObj = Player ? (Player->As_PhysicalGameObj() ? Player->As_PhysicalGameObj()->Peek_Physical_Object() : 0) : 0; // Get our PhysClass obj if any.
	
	if (PObj)
		PObj->Inc_Ignore_Counter(); // Ignore that in our ray/box casting for a second cause we dont want to include that.

	// Make sure we got the correct Height distance from the ground..
	Vector3 RealPos = Pos;
	RealPos.Z = -9999;
	LineSegClass Ray1(Pos, RealPos);
	CastResultStruct Result1;
	Result1.ComputeContactPoint = true;
	PhysRayCollisionTestClass Ray_Test1(Ray1, &Result1, TERRAIN_ONLY_COLLISION_GROUP, COLLISION_TYPE_ALL);
	PhysicsSceneClass::Get_Instance()->Cast_Ray(Ray_Test1);

	if (Result1.ContactPoint == Vector3(0, 0, 0)) // No ground here..
	{
		result = OC_ERROR_NO_GROUND;
	}
	else
	{
		CastResultStruct Result2;
		PhysRayCollisionTestClass Ray_Test2(Ray1, &Result2, DEFAULT_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);
		Ray_Test2.CheckDynamicObjs = false;
		PhysicsSceneClass::Get_Instance()->Cast_Ray(Ray_Test2);

		if (Ray_Test2.CollidedPhysObj && Ray_Test2.CollidedPhysObj->Get_Observer() && ((CombatPhysObserverClass *)Ray_Test2.CollidedPhysObj->Get_Observer())->As_BuildingGameObj()) // Hmm we collided with a building.. no good.
		{
			result = OC_ERROR_BUILDING;
		}
		else
		{
			RealPos.Z = Result1.ContactPoint.Z + (Size.Z / 2) + 0.1f; // Now calculate the real Z position so it will be at the ground level but add 0.1 for error so the box test will go well without contact with the ground
			// Now compute the terrain stuff;
			// Get the ground rotation.
			Matrix3D Mat;
			Vector3 Contact = Result1.ContactPoint;
			Mat.Look_At(Contact, Contact - Result1.Normal, 0);

			// Apply our facing.
			Matrix3 Rotation = Matrix3(Mat);
			Rotation.Rotate_Z(0);
			Rotation.Rotate_Z(Facing * (3.14159265f / 180));

			// Create an OBBoxClass.. We need that one cause AABoxClass does not support rotations.
			OBBoxClass Box(RealPos, Size / 2, Rotation);

			// Cast that shit.
			CastResultStruct Result3;
			Result3.ComputeContactPoint = true;
			PhysOBBoxCollisionTestClass Col_Test(Box, RealPos, &Result3, DEFAULT_COLLISION_GROUP, COLLISION_TYPE_PHYSICAL);
			PhysicsSceneClass::Get_Instance()->Cast_OBBox(Col_Test);

			if (Result3.StartBad) // Apparently we couldn't create it here..
			{
				result = OC_ERROR_NO_SPACE;
			}
		}
	}

	if (PObj)
		PObj->Dec_Ignore_Counter(); // Make it count in our physics tests again.

	lua_pushnumber(L, result);
	return 1;
}

int Lua_ShowTexture(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Info_Texture(obj, lua_tostring(L, 2));
	return 1;
}

int Lua_HideTexture(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Clear_Info_Texture(obj);
	return 1;
}


// LuaTT.lua ported here..

int Lua_Get_All_Players(lua_State *L)
{
	lua_newtable(L);
	int rotationTable = lua_gettop(L);
	int totalofplayersfound = 0;
	if (Get_Player_Count() == 0) return 1;

	for (int i = 0; i<999; i++)
	{

		if (Find_Player(i))
		{
			lua_pushnumber(L, i);
			totalofplayersfound++;
			lua_rawseti(L, rotationTable, totalofplayersfound);
		}

		if (totalofplayersfound >= Get_Player_Count())
			return 1;
	}

	return 1;
}

int Lua_Get_Rotation(lua_State *L)
{
	lua_newtable(L);
	int rotationTable = lua_gettop(L);
	for (int i = 0; i<999; i++)
	{
		if (Get_Map(i))
			return 1;
		lua_pushstring(L, Get_Map(i));
		lua_rawseti(L, rotationTable, i + 1);
	}


	return 1;
}


int Lua_Get_All_Vehicles(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;
	for (SLNode<VehicleGameObj>* n = GameObjManager::VehicleGameObjList.Head(); n; n = n->Next())
	{
		VehicleGameObj* Soldier = n->Data();
		if (Soldier) {
			lua_pushnumber(L, Commands->Get_ID(Soldier));
			lua_rawseti(L, buildingTable, index++);
		}
	}
	return 1;
}

int Lua_Get_All_Buildings(lua_State *L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<BuildingGameObj>* node = GameObjManager::BuildingGameObjList.Head(); node; node = node->Next())
	{
		GameObject *o = (GameObject *)node->Data();
		lua_pushnumber(L, Commands->Get_ID(o));
		lua_rawseti(L, buildingTable, index++);
	}

	return 1;
}

// 1.5 by sla.ro

int Lua_Set_Damage_Points(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Damage_Points(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checknumber(L, 2));
	return 1;
}

int Lua_Set_Death_Points(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Death_Points(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checknumber(L, 2));
	return 1;
}

int Lua_Get_Damage_Points(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Damage_Points(Commands->Find_Object(luaL_checknumber(L, 1))));
	return 1;
}

int Lua_Get_Death_Points(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Death_Points(Commands->Find_Object(luaL_checknumber(L, 1))));
	return 1;
}

int Lua_Damage_Objects_Half(lua_State *L)
{
	Damage_Objects_Half();
	return 1;
}

int Lua_Kill_Occupants(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Kill_Occupants(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Damage_All_Objects_Area(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 3);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	Damage_All_Objects_Area(luaL_checknumber(L, 1), luaL_checkstring(L, 2), pos, luaL_checknumber(L, 4), Commands->Find_Object(luaL_checknumber(L, 5)), Commands->Find_Object(luaL_checknumber(L, 6)));
	return 1;
}

int Lua_Damage_All_Buildings_By_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Damage_All_Buildings_By_Team(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checkstring(L, 3), Commands->Find_Object(luaL_checknumber(L, 4)));
	return 1;
}

int Lua_Damage_All_Vehicles_Area(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 3);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	Damage_All_Vehicles_Area(luaL_checknumber(L, 1), luaL_checkstring(L, 2), pos, luaL_checknumber(L, 4), Commands->Find_Object(luaL_checknumber(L, 5)), Commands->Find_Object(luaL_checknumber(L, 6)));
	return 1;
}

int Lua_Set_Info_Texture(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Info_Texture(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2));
	return 1;
}

int Lua_Clear_Info_Texture(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Clear_Info_Texture(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Set_Naval_Vehicle_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Naval_Vehicle_Limit(luaL_checknumber(L, 1));
	return 1;
}

int Lua_Get_Naval_Vehicle_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Naval_Vehicle_Limit());
	return 1;
}

int Lua_Send_Message_Player(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Message_Player(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checkstring(L, 5));
	return 1;
}
/*
int Lua_Set_Wireframe_Mode(lua_State *L)
{
if(lua_gettop(L) < 1) return 0;
Set_Wireframe_Mode(luaL_checknumber(L, 1));
return 1;
}*/

int Lua_Load_New_HUD_INI(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Load_New_HUD_INI(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2));
	return 1;
}

int Lua_Change_Radar_Map(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Change_Radar_Map(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4));
	return 1;
}

int Lua_Set_Currently_Building(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Currently_Building(lua_tobooleanCPP(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Is_Currently_Building(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, (Is_Currently_Building(luaL_checknumber(L, 1))));
	return 1;
}

int Lua_Set_Fog_Color(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Fog_Color(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	return 1;
}

int Lua_Display_Security_Dialog(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Display_Security_Dialog(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Do_Objectives_Dlg(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Do_Objectives_Dlg(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2));
	return 1;
}

int Lua_Set_Player_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Player_Limit(luaL_checknumber(L, 1));
	return 1;
}

int Lua_Get_Player_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Player_Limit());
	return 1;
}

int Lua_Set_GDI_Soldier_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_GDI_Soldier_Name(luaL_checkstring(L, 1));
	return 1;
}

int Lua_Set_Nod_Soldier_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Nod_Soldier_Name(luaL_checkstring(L, 1));
	return 1;
}

int Lua_Set_Moon_Is_Earth(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Moon_Is_Earth(lua_tobooleanCPP(L, 1));
	return 1;
}

int Lua_Get_Revision(lua_State *L)
{
	lua_pushnumber(L, Get_Revision(0));
	return 1;
}

int Lua_Can_Team_Build_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Can_Team_Build_Vehicle(luaL_checknumber(L, 1));
	return 1;
}

int Lua_Find_Naval_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Find_Naval_Factory(luaL_checknumber(L, 1));
	return 1;
}

int Lua_Vehicle_Preset_Is_Air(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Vehicle_Preset_Is_Air(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Vehicle_Preset_Is_Naval(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Vehicle_Preset_Is_Naval(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Hide_Preset_By_Name(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Hide_Preset_By_Name(luaL_checknumber(L, 1), luaL_checkstring(L, 2), lua_tobooleanCPP(L, 3));
	return 1;
}

int Lua_Busy_Preset_By_Name(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Busy_Preset_By_Name(luaL_checknumber(L, 1), luaL_checkstring(L, 2), lua_tobooleanCPP(L, 3));
	return 1;
}

int Lua_Attach_Script_Occupants(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Attach_Script_Occupants(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2), luaL_checkstring(L, 3));
	return 1;
}

int Lua_Destroy_Lua_Thread(lua_State *L)
{
	if (lua_gettop(L) < 1)
		return 0;

	int id = luaL_checknumber(L, 1);

	if (!LuaManager::Lua[id])
	{
		lua_pushboolean(L, false);
		return 1;
	}
	lua_close(LuaManager::Lua[id]);
	LuaManager::Lua[id] = nullptr;
	LuaManager::Lua.erase(LuaManager::Lua.begin() + id);
	lua_pushboolean(L, true);
	return 1;
}

#ifdef LUATT_EXPORTS
int Lua_Load_Lua(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	std::string whatfind = std::string(luaL_checkstring(L, 1));
	LuaManager::LoadLua(whatfind.c_str());
	return 1;
}

int Lua_Get_Lua_By_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	std::string whatfind = std::string(luaL_checkstring(L, 1));
	int id = 0;
	for (std::string lua_it : LuaManager::LuaNames)
	{
		if (lua_it == whatfind)
		{
			lua_pushnumber(L, id);
			return 1;
		}
		id++;
	}


	return 0;
}
#endif

// New Revision: 16.09.2013->

int Lua_Get_Current_Map_Index(lua_State *L)
{
	lua_pushnumber(L, Get_Current_Map_Index());
	return 1;
}

int Lua_Set_Map(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;

	if (strcmp(luaL_checkstring(L, 1), "NULL") == 0) {
		Set_Map("", luaL_checknumber(L, 2));
	}
	else {
		Set_Map(luaL_checkstring(L, 1), luaL_checknumber(L, 2));
	}

	return 1;
}

int Lua_Get_Client_Revision(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Client_Revision(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Set_Is_Visible(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Set_Is_Visible(Commands->Find_Object(luaL_checknumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Clear_Weapons(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Clear_Weapons(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Set_Is_Rendered(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Set_Is_Rendered(Commands->Find_Object(luaL_checknumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Get_Vehicle_Lock_Owner(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (obj->As_VehicleGameObj())
	{
		lua_pushnumber(L, Commands->Get_ID(obj->As_VehicleGameObj()->Get_Lock_Owner()));

	}
	return 1;
}

int Lua_Can_Teleport(lua_State *L)
{
	// Get physical game object and moveable phys class references
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	if (!obj)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	MoveablePhysClass* mphys = (obj->As_PhysicalGameObj()) ? obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_MoveablePhysClass() : NULL;

	// Can we move to this position without getting stuck?
	if (mphys)
	{
		lua_pushboolean(L, mphys->Can_Teleport(Matrix3D(location)));
		return 1;
	}
	else
	{
		lua_pushboolean(L, false);
		return 1;
	}


}

int Lua_Set_Deaths(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Deaths(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 1;
}

int Lua_Get_Animation_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj)
		lua_pushstring(L, Get_Animation_Name(obj));

	return 1;
}

int Lua_Set_HUD_Help_Text_Player_Text(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_HUD_Help_Text_Player_Text(obj, lua_tonumber(L, 2), lua_tostring(L, 3), Vector3(lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6)));
	return 1;
}

int Lua_Set_HUD_Help_Text_Player(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_HUD_Help_Text_Player(obj, lua_tonumber(L, 2), Vector3(lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5)));
	return 1;
}

int Lua_Force_Vehicle_Entry(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject *obj2 = Commands->Find_Object(lua_tonumber(L, 2));
	// we will do the check from here instead from Lua.
	if (Get_Vehicle(obj))
	{
		lua_pushboolean(L, false);
		return 1;
	}
	if (Get_Vehicle_Occupant_Count(obj2) >= Get_Vehicle_Seat_Count(obj2))
	{
		lua_pushboolean(L, false);
		return 1;
	}
	Force_Vehicle_Entry(obj, obj2);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Set_Camera_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_Camera_Player(obj, lua_tostring(L, 2));

	return 1;
}

int Lua_Enable_HUD_Pokable_Indicator(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Commands->Enable_HUD_Pokable_Indicator(obj, lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Grant_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Grant_Weapon(obj, lua_tostring(L, 2), lua_tobooleanCPP(L, 3), lua_tonumber(L, 4), lua_tobooleanCPP(L, 5));
	return 1;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
BuildingGameObj *Get_Closest_Building(const Vector3 &Position) {
	float ClosestDist = FLT_MAX;
	BuildingGameObj *Closest = 0;
	for (SLNode<BuildingGameObj> *z = GameObjManager::BuildingGameObjList.Head(); z; z = z->Next()) {
		float Dist = 0.0f;
		z->Data()->Find_Closest_Poly(Position, &Dist);
		if (Dist < ClosestDist) {
			ClosestDist = Dist;
			Closest = z->Data();
		}
	}
	return Closest;
}

PhysicalGameObj *Get_Closest_Fake_Building(const Vector3 &Position) {
	float ClosestDist = FLT_MAX;
	PhysicalGameObj *Closest = 0;
	for (SLNode<BaseGameObj> *z = GameObjManager::GameObjList.Head(); z; z = z->Next()) {
		if (z->Data()->As_PhysicalGameObj()) {
			PhysicalGameObj *Phys = (PhysicalGameObj*)z->Data();
			if (Phys->Get_Definition().Get_Encyclopedia_Type() == 3 && Phys->Get_Defense_Object()->Get_Health() && Phys->Peek_Physical_Object()->Peek_Model()) {
				float Dist = Commands->Get_Distance(Position, Phys->Get_Position());
				if (Dist < ClosestDist) {
					ClosestDist = Dist;
					Closest = Phys;
				}
			}
		}
	}
	return Closest;
}
#endif

int Lua_Get_Closest_Building(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	GameObject *obj = Get_Closest_Building(location);
	lua_pushnumber(L, Get_ID(obj));
	return 1;
}

int Lua_Get_Closest_Fake_Building(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	GameObject *obj = Get_Closest_Fake_Building(location);
	lua_pushnumber(L, Get_ID(obj));
	return 1;
}


int Lua_Get_Closest_Poly(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	BuildingGameObj *building = obj->As_BuildingGameObj();
	if (!building)
	{
		return 0;
	}
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	float Distance;
	building->Find_Closest_Poly(location, &Distance);
	lua_pushnumber(L, Distance);
	return 1;
}

int Lua_Get_String_ID_By_Desc(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Get_String_ID_By_Desc(lua_tostring(L, 1)));

	return 1;
}

int Lua_Change_String_Player(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Change_String_Player(obj, lua_tonumber(L, 2), lua_tostring(L, 3));

	return 1;
}

int Lua_Get_Ammo_Definition(lua_State *L)
{
	const AmmoDefinitionClass *def = nullptr;
	if (lua_islightuserdata(L, 1) == true)
	{
		def = (const AmmoDefinitionClass*)(lua_topointer(L, 1));
	}
	else
	{
		if (lua_gettop(L) < 2) return 0;
		def = Get_Weapon_Ammo_Definition(lua_tostring(L, 1), lua_tobooleanCPP(L, 2));
		if (lua_tobooleanCPP(L, 3) == true)
		{
			lua_pushlightuserdata(L, (void*)def);
			return 1;
		}
	}

	if (def == nullptr)
	{
		return 1; // Invalid ammo?
	}

	// I don't think will work as TT kinda blocked some stuff from accessing with plugins... it has to be hard coded in scripts.dll.....
	lua_newtable(L);
	lua_pushnumber(L, def->AliasedSpeed);
	lua_setfield(L, -2, "AliasedSpeed");
	lua_pushnumber(L, def->AmmoType);
	lua_setfield(L, -2, "AmmoType");
	lua_pushnumber(L, def->AquireTime);
	lua_setfield(L, -2, "AquireTime");
	lua_pushnumber(L, def->BeaconDefID);
	lua_setfield(L, -2, "BeaconDefID");
	lua_pushnumber(L, def->BeamEnabled);
	lua_setfield(L, -2, "BeamEnabled");
	lua_pushnumber(L, def->BurstDelayTime);
	lua_setfield(L, -2, "BurstDelayTime");
	lua_pushnumber(L, def->BeamWidth);
	lua_setfield(L, -2, "BeamWidth");
	lua_pushnumber(L, def->C4Animation);
	lua_setfield(L, -2, "C4Animation");
	lua_pushnumber(L, def->C4Preset);
	lua_setfield(L, -2, "C4Preset");
	lua_pushnumber(L, def->ChargeTime);
	lua_setfield(L, -2, "ChargeTime");
	lua_pushnumber(L, def->DisableHitscan);
	lua_setfield(L, -2, "DisableHitscan");
	lua_pushnumber(L, def->EffectiveRange);
	lua_setfield(L, -2, "EffectiveRange");
	lua_pushnumber(L, def->Velocity);
	lua_setfield(L, -2, "Velocity");
	lua_pushnumber(L, def->Damage);
	lua_setfield(L, -2, "Damage");
	lua_pushnumber(L, def->Elasticity);
	lua_setfield(L, -2, "Elasticity");
	lua_pushnumber(L, def->ExplosionDefID);
	lua_setfield(L, -2, "ExplosionDefID");
	lua_pushnumber(L, def->RateOfFire);
	lua_setfield(L, -2, "RateOfFire");
	lua_pushnumber(L, def->Range);
	lua_setfield(L, -2, "Range");
	lua_pushnumber(L, def->SprayCount);
	lua_setfield(L, -2, "SprayCount");
	lua_pushnumber(L, def->SprayBulletCost);
	lua_setfield(L, -2, "SprayBulletCost");
	lua_pushnumber(L, def->Velocity);
	lua_setfield(L, -2, "Velocity");
	lua_pushnumber(L, def->Warhead);
	lua_setfield(L, -2, "Warhead");
	lua_pushnumber(L, def->TurnRate);
	lua_setfield(L, -2, "TurnRate");
	lua_pushstring(L, def->Get_Name());
	lua_setfield(L, -2, "Name");

	return 1;
}

int Lua_Action_AttackLocation(lua_State *L)
{
	// Action_AttackLocation(obj, attackpos, priority, range, deviation)
	if (lua_gettop(L) < 5) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));

	ActionParamsStruct params;
	//int priority = lua_tonumber(L, 3);
	float range = lua_tonumber(L, 4);
	float deviation = lua_tonumber(L, 5);
	bool primary = true;

	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());

	params.Set_Attack(pos, range, deviation, primary);
	params.AttackCheckBlocked = false;
	Commands->Action_Attack(obj, params);

	return 1;
}

int Lua_Action_AttackMoveLocation(lua_State *L)
{
	// Action_AttackMoveLocation(obj, attackpos, priority, range, deviation, location, speed, distance)
	if (lua_gettop(L) < 8) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));

	ActionParamsStruct params;
	//int priority = lua_tonumber(L, 3);
	float range = lua_tonumber(L, 4);
	float deviation = lua_tonumber(L, 5);
	bool primary = true;

	LUA_Vector3 *a2 = LuaVector3::GetInstance(L, 6);
	Vector3 pos2 = Vector3(a2->X(), a2->Y(), a2->Z());

	//GameObject *GotoObject = Commands->Find_Object(Get_Int_Parameter("Move_ID"));
	params.Set_Movement(pos2, lua_tonumber(L, 7), lua_tonumber(L, 8));
	//params.MoveFollow = true;
	//params.Set_Basic(this, (float)priority, 40016);

	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());

	params.Set_Attack(pos, range, deviation, primary);
	params.AttackCheckBlocked = false;
	Commands->Action_Attack(obj, params);
	Commands->Action_Goto(obj, params);
	return 1;
}

int Lua_Action_MoveLocation(lua_State *L)
{
	// Action_MoveLocation(obj, location, speed, distance)
	if (lua_gettop(L) < 8) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct params;

	LUA_Vector3 *a2 = LuaVector3::GetInstance(L, 2);
	Vector3 pos2 = Vector3(a2->X(), a2->Y(), a2->Z());
	//params.Set_Basic(Get_ID(obj), 9999.0f, 9000000);

	params.Set_Movement(pos2, lua_tonumber(L, 3), lua_tonumber(L, 4));
	//params.MoveFollow = true;
	Commands->Action_Goto(obj, params);

	return 1;
}

int Lua_Action_FollowObject(lua_State *L)
{
	// Action_MoveLocation(obj, location, speed, distance)
	if (lua_gettop(L) < 8) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject *obj2 = Commands->Find_Object(lua_tonumber(L, 2));
	ActionParamsStruct params;

	//params.Set_Basic(Get_ID(obj), 70, 1);

	params.Set_Movement(obj2, 1.0f, 1.0f);
	params.MoveFollow = true;
	Commands->Action_Goto(obj, params);

	return 1;
}


int Lua_Smart_EnableBot(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));

	Commands->Set_Innate_Is_Stationary(obj, false);
	SmartGameObj *Smart = obj->As_SmartGameObj();
	Smart->Set_Enemy_Seen_Enabled(true);
	Commands->Innate_Enable(obj);
	Commands->Innate_Soldier_Enable_Bullet_Heard(obj, true);
	Commands->Innate_Soldier_Enable_Gunshot_Heard(obj, true);
	Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, true);
	Commands->Enable_Hibernation(obj, false);

	return 1;
}

int Lua_Set_Net_Update_Rate_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_Net_Update_Rate_Player(obj, lua_tonumber(L, 2));
	return 1;
}

int Lua_Get_All_Scripts_Object(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_newtable(L);
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr)
	{
		return 1; // Invalid object!
	}
	auto scriptableObj = obj->As_ScriptableGameObj();
	if (scriptableObj == nullptr)
	{
		return 1; // Invalid object type
	}
	const SimpleDynVecClass<GameObjObserverClass*> observers = scriptableObj->Get_Observers();
	const int x = observers.Count();
	if (x == 0)
	{
		return 1; // No scripts attached
	}
	ScriptImpClass* scriptObj = nullptr;
	for (int i = 0; i < x; i++)
	{
		try {
			scriptObj = dynamic_cast<ScriptImpClass*>((observers[i]));
			if (scriptObj != nullptr)
			{
				const char* text = scriptObj->Get_Name();
				lua_pushnumber(L, i);
				lua_pushstring(L, text);
				lua_rawset(L, -3);
			}
		}
		catch (...)
		{
			/* Not a script */
		}
	}

	return 1;
}

int Lua_Get_All_Observers_Object(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_newtable(L);
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr)
	{
		return 1; // Invalid object!
	}
	auto scriptableObj = obj->As_ScriptableGameObj();
	if (scriptableObj == nullptr)
	{
		return 1; // Invalid object type
	}
	const SimpleDynVecClass<GameObjObserverClass*> observers = scriptableObj->Get_Observers();
	const int x = observers.Count();
	if (x == 0)
	{
		return 1; // No scripts attached
	}
	for (int i = 0; i < x; i++)
	{
		try {
			ScriptImpClass* scriptObj = dynamic_cast<ScriptImpClass*>(observers[i]);
		}
		catch (...)
		{
			/* We found an observer */
			const char* text = observers[i]->Get_Name();
			lua_pushnumber(L, i);
			lua_pushstring(L, text);
			lua_rawset(L, -3);
		}
	}

	return 1;
}

int Lua_Get_Script_Parameters(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr)
	{
		return 0; // Invalid object!
	}

	std::vector<ScriptImpClass *> scriptList = LuaScriptManager::Get_ScriptByName(obj, lua_tostring(L, 2));
	if (scriptList.empty())
	{
		return 0;
	}

	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (auto script : scriptList)
	{
		if (script->Get_ID() == 0)
		{
			continue;
		}

		std::string str;

		int count = script->Get_Parameter_Count();
		int i = 0;

		if (count > 0)
		{
			while (i < count)
			{
				if (i > 0)
				{
					str += ",";
				}

				const char* param = script->Get_Parameter(i);
				if (param)
				{
					str += param;
				}
				i++;
			}
		}
		lua_pushstring(L, str.c_str());
		lua_rawseti(L, buildingTable, index++);
	}
	return 1;
}


int Lua_Set_Velocity(lua_State* L) // replace existing one with this sla
{
	if (lua_gettop(L) < 2) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_Velocity(obj, pos);
	return 0;
}

int Lua_Set_Ammo_IsTracking(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	AmmoDefinitionClass *def = nullptr;
	if (lua_islightuserdata(L, 1) == true)
	{
		def = (AmmoDefinitionClass*)(lua_topointer(L, 1));
	}
	else
	{
		if (lua_gettop(L) < 2) return 0;
		def = (AmmoDefinitionClass*)Get_Weapon_Ammo_Definition(lua_tostring(L, 1), lua_tobooleanCPP(L, 2));
		if (lua_tobooleanCPP(L, 3) == true)
		{
			lua_pushlightuserdata(L, (void*)def);
			return 1;
		}
	}

	if (def == nullptr)
	{
		return 1; // Invalid ammo?
	}

	def->IsTracking = lua_tobooleanCPP(L, 3);

	return 1;
}

int Lua_Disable_Innate(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));

	Commands->Set_Innate_Is_Stationary(obj, true);
	SmartGameObj *Smart = obj->As_SmartGameObj();
	Smart->Set_Enemy_Seen_Enabled(false);
	Commands->Innate_Disable(obj);
	Commands->Innate_Soldier_Enable_Bullet_Heard(obj, false);
	Commands->Innate_Soldier_Enable_Gunshot_Heard(obj, false);
	Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, false);
	return 0;
}


int Lua_Find_Closest_Zone(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject *obj = Find_Closest_Zone(pos, ZoneConstants::TYPE_BEACON);

	if (obj == nullptr)
	{
		return 0;
	}

	lua_pushnumber(L, Get_ID(obj));

	return 1;
}


int Lua_Get_All_ScriptZones(lua_State *L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<ScriptZoneGameObj>* node = GameObjManager::ScriptZoneGameObjList.Head(); node; node = node->Next())
	{
		GameObject *o = (GameObject *)node->Data();

		lua_pushnumber(L, Commands->Get_ID(o));
		lua_rawseti(L, buildingTable, index++);
	}
	return 1;
}



int Lua_Get_Game_Definitions(lua_State *L)
{
	lua_newtable(L);
	int index = 1;

	HashTemplateClass<StringClass, GameDefinition> Defs;
	Get_Game_Definitions(Defs);
	for (int i = 0; i < Defs.Get_Size(); ++i)
	{
		GameDefinition Definition = Defs.getValueByIndex(i);
		lua_pushnumber(L, index);
		index++;
		lua_newtable(L);
		lua_pushstring(L, Definition.GetDisplayName());
		lua_setfield(L, -2, "DisplayName");
		lua_pushstring(L, Definition.mapName);
		lua_setfield(L, -2, "mapName");
		lua_pushstring(L, Definition.name);
		lua_setfield(L, -2, "name");
		lua_pushstring(L, Definition.serverPresetsFile);
		lua_setfield(L, -2, "serverPresetsFile");
		lua_newtable(L);
		int table = lua_gettop(L);
		SimpleVecClass<uint32> packagelist = Definition.packages;
		for (int cur = 0; cur < packagelist.Length(); cur++)
		{
			lua_pushnumber(L, packagelist[cur]);
			lua_rawseti(L, table, cur + 1);
		}
		lua_setfield(L, -2, "packages");
		lua_settable(L, -3);
	}
	return 1;
}

int Lua_Get_Player_List(lua_State *L)
{
	lua_newtable(L);
	int playerTable = lua_gettop(L);
	int index = 1;

	for (SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
		cPlayer* Player = z->Data();

		if (Player->Get_Is_Active())
		{
			lua_pushnumber(L, Player->Get_Id());
			lua_rawseti(L, playerTable, index++);
		}
	}
	return 1;
}

int Lua_Create_2D_WAV_Sound_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Player(obj, luaL_checkstring(L, 2)));
	return 1;
}

int Lua_Get_Map_Count(lua_State *L)
{
	lua_pushnumber(L, Get_Map_Count());
	return 1;
}

int Lua_Set_Fog_Enable_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	bool enable = lua_tobooleanCPP(L, 2);
	Set_Fog_Enable_Player(obj, enable);
	return 0;
}

int Lua_Set_Fog_Range_Player(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	float start = luaL_checknumber(L, 2);
	float end = luaL_checknumber(L, 3);
	float trans = luaL_checknumber(L, 4);

	Set_Fog_Range_Player(obj, start, end, trans);
	return 0;
}

int Lua_Set_Background_Music_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	const char* music = luaL_checkstring(L, 2);
	Set_Background_Music_Player(obj, music);
	return 0;
}

int Lua_Fade_Background_Music_Player(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	const char* music = luaL_checkstring(L, 2);
	int x = luaL_checkint(L, 3);
	int y = luaL_checkint(L, 4);

	Fade_Background_Music_Player(obj, music, x, y);
	return 0;
}

int Lua_Enable_HUD_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	bool enable = lua_tobooleanCPP(L, 2);
	Enable_HUD_Player(obj, enable);

	return 0;
}

int Lua_Create_Sound_Player(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	const char* sound = luaL_checkstring(L, 2);
	LUA_Vector3* a = LuaVector3::GetInstance(L, 3);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 4));
	if (!obj && !obj2) return 0;

	lua_pushnumber(L, Create_Sound_Player(obj, sound, pos, obj2));
	return 1;
}

int Lua_Create_3D_WAV_Sound_At_Bone_Player(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	const char* sound = luaL_checkstring(L, 2);
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 3));
	const char* bone = luaL_checkstring(L, 4);
	if (!obj && !obj2) return 0;
	lua_pushnumber(L, Create_3D_WAV_Sound_At_Bone_Player(obj, sound, obj2, bone));
	return 1;
}

int Lua_Create_3D_Sound_At_Bone_Player(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	const char* sound = luaL_checkstring(L, 2);
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 3));
	const char* bone = luaL_checkstring(L, 4);
	if (!obj && !obj2) return 0;
	lua_pushnumber(L, Create_3D_Sound_At_Bone_Player(obj, sound, obj2, bone));
	return 1;
}

int Lua_Set_Fog_Color_Player(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Fog_Color_Player(obj, luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	return 0;
}


int Lua_Set_Fog_Mode_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	unsigned int mode = luaL_checkint(L, 2);
	if (!obj) return 0;
	if (mode < 1 || mode > 3) return 0;
	Set_Fog_Mode_Player(obj, mode);
	return 1;
}

int Lua_Set_Fog_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	unsigned int mode = luaL_checkint(L, 1);
	if (mode < 1 || mode > 3) return 0;
	Set_Fog_Mode(mode);
	return 1;
}

int Lua_Send_HUD_Number(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	unsigned int mode = luaL_checkint(L, 2);
	if (!obj) return 0;
	Send_HUD_Number(obj, mode);
	return 1;
}

int Lua_Set_Fog_Density_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	float density = luaL_checknumber(L, 2);
	if (!obj) return 0;

	Set_Fog_Density_Player(obj, density);
	return 1;
}

int Lua_Set_Fog_Density(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	float density = luaL_checknumber(L, 1);
	Set_Fog_Density(density);
	return 1;
}

int Lua_GetExplosionObj(lua_State *L)
{
	lua_pushnumber(L, Get_ID(GetExplosionObj()));
	return 1;
}

int Lua_SetExplosionObj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	SetExplosionObj(obj);
	return 1;
}


int Lua_Get_Fog_Range(lua_State *L)
{
	float start = 0.f;
	float end = 0.f;
	Get_Fog_Range(start, end); // is this correct way ?
	lua_pushnumber(L, start);
	lua_pushnumber(L, end);
	return 2;
}

int Lua_Get_GDI_Soldier_Name(lua_State *L)
{
	lua_pushstring(L, Get_GDI_Soldier_Name());
	return 1;
}

int Lua_Get_Nod_Soldier_Name(lua_State *L)
{
	lua_pushstring(L, Get_Nod_Soldier_Name());
	return 1;
}

int Lua_Set_Fog_Override_Player(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	float start = luaL_checknumber(L, 1);
	float end = luaL_checknumber(L, 2);
	if (!obj) return 0;
	Set_Fog_Override_Player(obj, start, end); // unsure if this is even working correctly.. Tons of fog regardless of vaules set..
	return 1;
}

int Lua_Clear_Fog_Override_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Clear_Fog_Override_Player(obj);
	return 1;
}

int Lua_Display_Game_Hint(lua_State *L)
{
	if (lua_gettop(L) < 7) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Display_Game_Hint(obj, luaL_checknumber(L, 2), luaL_checkstring(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6), luaL_checknumber(L, 7)); // untested
	return 1;
}

int Lua_Display_Game_Hint_Image(lua_State *L)
{
	if (lua_gettop(L) < 8) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Display_Game_Hint_Image(obj, luaL_checknumber(L, 2), luaL_checkstring(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6), luaL_checknumber(L, 7), luaL_checkstring(L, 8)); // untested
	return 1;
}

int Lua_Set_Global_Stealth_Disable(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Global_Stealth_Disable(lua_tobooleanCPP(L, 1));
	return 1;
}

int Lua_Get_Tech_Level(lua_State *L)
{
	lua_pushnumber(L, Get_Tech_Level()); // unable to confirm if works. maybe not ren supported?
	return 1;
}

int Lua_Set_Tech_Level(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Tech_Level(luaL_checknumber(L, 1)); // unable to confirm if works. maybe not ren supported?
	return 0;
}

int Lua_Create_Explosion_Extended(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 3));
	if (!obj) return 0;
	Create_Explosion_Extended(luaL_checkstring(L, 1), pos, obj);
	return 0;
}

int Lua_Get_Clouds(lua_State *L)
{
	float cloud = 0.f;
	float gloom = 0.f;
	Get_Clouds(cloud, gloom);
	lua_pushnumber(L, cloud);
	lua_pushnumber(L, gloom);
	return 2;
}

int Lua_Get_Lightning(lua_State *L)
{
	float intensity = 0.f;
	float start = 0.f;
	float end = 0.f;
	float heading = 0.f;
	float distribution = 0.f;
	Get_Lightning(intensity, start, end, heading, distribution);
	lua_pushnumber(L, intensity);
	lua_pushnumber(L, start);
	lua_pushnumber(L, end);
	lua_pushnumber(L, heading);
	lua_pushnumber(L, distribution);
	return 5;
}

int Lua_Get_Wind(lua_State *L)
{
	float heading = 0.f;
	float speed = 0.f;
	float variability = 0.f;
	Get_Wind(heading, speed, variability);
	lua_pushnumber(L, heading);
	lua_pushnumber(L, speed);
	lua_pushnumber(L, variability);
	return 3;
}

int Lua_Get_Rain(lua_State *L)
{
	float density = -99.f;
	Get_Rain(density);
	lua_pushnumber(L, (double)density);
	return 1;
}

int Lua_Get_Snow(lua_State *L)
{
	float density = -99.f;
	Get_Snow(density);
	lua_pushnumber(L, (double)density);
	return 1;
}

int Lua_Get_Ash(lua_State *L)
{
	float density = -99.f;
	Get_Ash(density);
	lua_pushnumber(L, (double)density);
	return 1;
}

int Lua_Update_Game_Options(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Update_Game_Options(luaL_checknumber(L, 1)); // wth does this even do?
	return 1;
}

int Lua_GetTTVersion(lua_State *L)
{
	lua_pushnumber(L, GetTTVersion());
	return 1;
}

int Lua_GetTTRevision(lua_State *L)
{
	lua_pushnumber(L, GetTTRevision());
	return 1;
}

int Lua_Force_Position_Update(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Force_Position_Update(obj);
	return 1;
}

int Lua_Create_2D_Wave_Sound_Dialog_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Create_2D_Wave_Sound_Dialog_Player(obj, luaL_checkstring(L, 2)));
	return 1;
}

int Lua_Force_Position_Update_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj || !obj2) return 0;
	Force_Position_Update_Player(obj, obj2);
	return 0;
}

int Lua_Set_Background_Music_Player_Offset(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Background_Music_Player_Offset(obj, luaL_checkstring(L, 2), luaL_checknumber(L, 3));
	return 0;
}

int Lua_Set_Definition_TranslationID_Player(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Definition_TranslationID_Player(obj, luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	return 0;
}

int Lua_Force_Orientation_Update(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Force_Orientation_Update(obj);
	return 0;
}


int Lua_Force_Orientation_Update_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj || !obj2) return 0;
	Force_Orientation_Update_Player(obj, obj2);
	return 0;
}

int Lua_Create_2D_Wave_Sound_Cinematic_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Create_2D_Wave_Sound_Cinematic_Player(obj, luaL_checkstring(L, 2)));
	return 1;
}


int Lua_Force_Turret_Update(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Force_Turret_Update(obj);
	return 0;
}

int Lua_Force_Turret_Update_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj || !obj2) return 0;
	Force_Turret_Update_Player(obj, obj2);
	return 0;
}

int Lua_Force_Velocity_Update(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Force_Velocity_Update(obj);
	return 0;
}

int Lua_Force_Velocity_Update_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj || !obj2) return 0;
	Force_Velocity_Update_Player(obj, obj2);
	return 0;
}

int Lua_Set_Camera_Host_Network(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Camera_Host_Network(obj);
	return 0;
}

int Lua_Send_Message_With_Team_Color(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Send_Message_With_Team_Color(luaL_checknumber(L, 1), luaL_checkstring(L, 2));
	return 0;
}

int Lua_Send_Message_With_Obj_Color(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Send_Message_With_Obj_Color(obj, luaL_checkstring(L, 2));
	return 0;
}

int Lua_Send_Message_Team(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Message_Team(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checkstring(L, 5));
	return 0;
}

int Lua_Send_Translated_Message_Team(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;
	Send_Translated_Message_Team(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), lua_tobooleanCPP(L, 6));
	return 0;
}

int Lua_Set_Occupants_Fade(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Occupants_Fade(obj, luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5));
	return 0;
}

int Lua_Enable_Team_Radar(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Enable_Team_Radar(luaL_checknumber(L, 1), lua_tobooleanCPP(L, 2));
	return 0;
}

int Lua_Create_Sound_Team(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 3));
	if (!obj) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	if (!a) return 0;
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	lua_pushnumber(L, Create_Sound_Team(luaL_checkstring(L, 1), pos, obj, luaL_checknumber(L, 4)));
	return 1;
}

int Lua_Create_2D_Sound_Team(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_Sound_Team(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}

int Lua_Create_2D_WAV_Sound_Team(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Team(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}

int Lua_Create_2D_WAV_Sound_Team_Dialog(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Team_Dialog(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}

int Lua_Create_2D_WAV_Sound_Team_Cinematic(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Team_Cinematic(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}


int Lua_Create_3D_WAV_Sound_At_Bone_Team(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj) return 0;
	lua_pushnumber(L, Create_3D_WAV_Sound_At_Bone_Team(luaL_checkstring(L, 1), obj, luaL_checkstring(L, 3), luaL_checknumber(L, 4)));
	return 1;
}


int Lua_Create_3D_Sound_At_Bone_Team(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj) return 0;
	lua_pushnumber(L, Create_3D_Sound_At_Bone_Team(luaL_checkstring(L, 1), obj, luaL_checkstring(L, 3), luaL_checknumber(L, 4)));
	return 1;
}

int Lua_Kill_All_Buildings_By_Team(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Kill_All_Buildings_By_Team(luaL_checknumber(L, 1));
	return 0;
}

int Lua_Send_Custom_Event_To_Objects_With_Script(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Custom_Event_To_Objects_With_Script(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}

int Lua_Set_Animation_Frame(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Set_Animation_Frame(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2), luaL_checknumber(L, 3));
	return 0;
}

int Lua_Get_Translated_Name_ID(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) { return 0; }
	lua_pushnumber(L, obj->As_DamageableGameObj()->Get_Translated_Name_ID());
	return 1;
}

int Lua_Get_Bone_Position(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Vector3 pos = Commands->Get_Bone_Position(obj, lua_tostring(L, 2));
	LUA_Vector3 *a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);

	return 1;
}

int Lua_Action_Reset(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Action_Reset(obj, lua_tonumber(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Give_Money(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Give_Money(obj, lua_tonumber(L, 2), lua_tobooleanCPP(L, 3));
	lua_pushboolean(L, 1);
	return 1;
}


int Lua_Set_Alt_Definition(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	int Type = lua_tonumber(L, 1);
	int Team = lua_tonumber(L, 2);
	PurchaseSettingsDefClass *PT = PurchaseSettingsDefClass::Find_Definition((PurchaseSettingsDefClass::TYPE)Type, (PurchaseSettingsDefClass::TEAM)Team);
	if (!PT) return 0;

	int Slot = lua_tonumber(L, 3);
	int Alt = lua_tonumber(L, 4);
	int PresetID = lua_tonumber(L, 5);
	PT->Set_Alt_Definition(Slot, Alt, PresetID);
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Set_Definition(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	int Type = lua_tonumber(L, 1);
	int Team = lua_tonumber(L, 2);
	PurchaseSettingsDefClass *PT = PurchaseSettingsDefClass::Find_Definition((PurchaseSettingsDefClass::TYPE)Type, (PurchaseSettingsDefClass::TEAM)Team);
	if (!PT) return 0;

	int Slot = lua_tonumber(L, 3);
	int PresetID = lua_tonumber(L, 4);
	PT->Set_Definition(Slot, PresetID);
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Lock_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject *obj2 = Commands->Find_Object(lua_tonumber(L, 2));
	int timer = lua_tonumber(L, 3);
	if (!obj || !obj->As_VehicleGameObj()) return 0;
	obj->As_VehicleGameObj()->Lock_Vehicle(obj2, timer);
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Get_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Commands->Get_Preset_ID(obj));
	return 1;
}

int Lua_Trigger_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 3);
	if (!a) return 0;
	Vector3 target = Vector3(a->GetX(), a->GetY(), a->GetZ());

	Commands->Trigger_Weapon(obj, lua_tobooleanCPP(L, 2), target, lua_tobooleanCPP(L, 4));
	return 1;
}

int Lua_Get_Action_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Commands->Get_Action_ID(obj));
	return 1;
}

int Lua_Send_Custom_Event(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	Commands->Send_Custom_Event(Commands->Find_Object(lua_tonumber(L, 1)), Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}

int Lua_Give_Points(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Give_Points(obj, lua_tonumber(L, 2), lua_tobooleanCPP(L, 3));
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Enable_Spawner(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Enable_Spawner(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 0;
}

int Lua_Find_Object_By_Preset(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Find_Object_By_Preset(lua_tonumber(L, 1), lua_tostring(L, 2))));
	return 1;
}

int Lua_Enable_Enemy_Seen(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Enable_Enemy_Seen(obj, lua_tobooleanCPP(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Enable_Hibernation(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Enable_Hibernation(obj, lua_tobooleanCPP(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Innate_Enable(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Innate_Enable(obj);
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Get_Translated_Preset_Name_Ex(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushstring(L, Get_Translated_Preset_Name_Ex(obj));
	return 1;
}

int Lua_Is_Obj_Visible(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(lua_tonumber(L, 2));
	if (!obj || !obj->As_SmartGameObj()) return 0;
	if (!obj2 || !obj2->As_PhysicalGameObj()) return 0;


	lua_pushboolean(L, obj->As_SmartGameObj()->Is_Obj_Visible(obj2->As_PhysicalGameObj()));
	return 1;
}

int Lua_ActionParamsStruct(lua_State *L)
{
	LuaActionParams *a = new LuaActionParams;
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "ActionParams");
	lua_setmetatable(L, -2);
	return 1;
}

int Lua_Get_A_Star(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	if (!a) return 0;
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());

	GameObject *astar = Commands->Get_A_Star(pos);
	if (astar)
	{
		lua_pushnumber(L, astar->Get_ID());
		return 1;
	}
	else
	{
		return 0;
	}

}

int Lua_Action_Goto(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 2);
	if (!obj || !a) return 0;

	Commands->Action_Goto(obj, *a);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Action_Attack(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 2);
	if (!obj || !a) return 0;

	Commands->Action_Attack(obj, *a);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Action_Play_Animation(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 2);
	if (!obj || !a) return 0;

	Commands->Action_Play_Animation(obj, *a);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Action_Enter_Exit(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 2);
	if (!obj || !a) return 0;

	Commands->Action_Enter_Exit(obj, *a);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Action_Face_Location(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 2);
	if (!obj || !a) return 0;

	Commands->Action_Face_Location(obj, *a);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Action_Dock(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 2);
	if (!obj || !a) return 0;

	Commands->Action_Dock(obj, *a);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Action_Follow_Input(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 2);
	if (!obj || !a) return 0;

	Commands->Action_Follow_Input(obj, *a);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Modify_Action(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	int id = lua_tonumber(L, 2);
	ActionParamsStruct *a = LuaActionParams::GetInstance(L, 3);
	bool mod_move = lua_tobooleanCPP(L, 4);
	bool mod_attack = lua_tobooleanCPP(L, 5);
	if (!obj || !a) return 0;

	Commands->Modify_Action(obj, id, *a, mod_move, mod_attack);
	lua_pushboolean(L, true);
	return 1;
}

int Lua_Get_Action_Params(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;

	ActionParamsStruct *a = new ActionParamsStruct;
	Commands->Get_Action_Params(obj, *a); // this must be tested!

	if (a == nullptr)
		return 0;

	lua_boxpointer(L, a);
	luaL_getmetatable(L, "ActionParamsStruct");
	lua_setmetatable(L, -2);

	return 1;
}


int Lua_Has_Key(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;

	lua_pushboolean(L, Commands->Has_Key(obj, luaL_checknumber(L, 2)));
	return 1;
}

int Lua_Get_Building_Power(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushboolean(L, Commands->Get_Building_Power(obj));
	return 1;
}

int Lua_Set_Building_Power(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Set_Building_Power(obj, lua_tobooleanCPP(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

int Lua_Disable_All_Collisions(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Disable_All_Collisions(obj);
	return 0;
}

int Lua_Enable_Engine(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Enable_Engine(obj, lua_tobooleanCPP(L, 2));
	return 0;
}

int Lua_Is_Object_Visible(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* looker = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 2));
	if (!obj || !looker) return 0;
	lua_pushboolean(L, Commands->Is_Object_Visible(looker, obj));
	return 1;
}

int Lua_Innate_Disable(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Innate_Disable(obj);
	return 1;
}

int Lua_Get_Bullseye_Position(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	Vector3 pos = obj->As_SoldierGameObj()->Get_Bullseye_Position();
	LUA_Vector3 *a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);
	return 1;
}

int Lua_Is_Enemy(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj1 = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(lua_tonumber(L, 2));
	if (!obj1 || !obj2 || !obj1->As_DamageableGameObj() || !obj2->As_DamageableGameObj()) return 0;
	lua_pushboolean(L, obj1->As_DamageableGameObj()->Is_Enemy(obj2->As_DamageableGameObj()));
	return 1;
}

int Lua_Set_Player_Type(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Set_Player_Type(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 1;
}

int Lua_Give_PowerUp(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Give_PowerUp(obj, luaL_checkstring(L, 2), lua_tobooleanCPP(L, 3));
	return 1;
}

int Lua_Expire_Powerup(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Expire_Powerup(obj);
	return 1;
}

int Lua_Set_Scripts_Can_Fire(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	obj->As_VehicleGameObj()->Set_Scripts_Can_Fire(lua_tobooleanCPP(L, 2));
	return 1;
}


int Lua_Set_Collision_Group(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	obj->As_PhysicalGameObj()->Set_Collision_Group(luaL_checknumber(L, 2));
	return 1;
}

int Lua_Get_Collision_Group(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, obj->As_PhysicalGameObj()->Get_Collision_Group());
	return 1;
}

int Lua_Set_Immovable(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	obj->As_VehicleGameObj()->Set_Immovable(lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Get_All_Stars(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<SoldierGameObj>* node = GameObjManager::StarGameObjList.Head(); node; node = node->Next())
	{
		GameObject* o = (GameObject*)node->Data();

		lua_pushnumber(L, Commands->Get_ID(o));
		lua_rawseti(L, buildingTable, index++);
	}
	return 1;
}

int Lua_Get_All_Beacons(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<BeaconGameObj>* node = GameObjManager::BeaconGameObjList.Head(); node; node = node->Next())
	{
		GameObject* o = (GameObject*)node->Data();

		lua_pushnumber(L, Commands->Get_ID(o));
		lua_rawseti(L, buildingTable, index++);
	}
	return 1;
}

int Lua_Create_2D_WAV_Sound(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Commands->Create_2D_WAV_Sound(lua_tostring(L, 1)));
	return 1;
}


int Lua_Lock_Soldier_Collision_Group(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	Lock_Soldier_Collision_Group(obj, static_cast<Collision_Group_Type>(lua_tointeger(L, 2)) );
	return 1;
}

int Lua_Set_Allow_Stealth_While_Empty(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_VehicleGameObj()) return 0;
	obj->As_VehicleGameObj()->Set_Allow_Stealth_While_Empty(lua_tobooleanCPP(L, 2));
	return 1;
}


int Lua_Get_All_Bone_Names(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;

	if (obj->As_PhysicalGameObj())
	{
		int boneCount = obj->As_PhysicalGameObj()->Peek_Model()->Get_Num_Bones();
		lua_newtable(L);
		int Table = lua_gettop(L);
		int index = 1;
		for (int i = 0; i < boneCount; i++)
		{
			lua_pushstring(L, obj->As_PhysicalGameObj()->Peek_Model()->Get_Bone_Name(i));
			lua_rawseti(L, Table, index++);
		}
	}
	else
	{
		return 0;
	}
	return 1;
}

int Lua_Set_Can_Drive_Vehicles(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	obj->As_SoldierGameObj()->Set_Can_Drive_Vehicles(lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Stop_Sound(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Stop_Sound(luaL_checknumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Start_Sound(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Start_Sound(luaL_checknumber(L, 1));
	return 1;
}

int Lua_Monitor_Sound(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Monitor_Sound(obj, luaL_checknumber(L, 2));
	return 1;
}

int Lua_Create_Sound(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 3));
	if (!obj) return 0;
	lua_pushnumber(L, Commands->Create_Sound(luaL_checkstring(L, 1), pos, obj));
	return 1;
}

int Lua_Get_Defense_Object_Health(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_BuildingGameObj()) { return 0; }
	lua_pushnumber(L, obj->As_BuildingGameObj()->Get_Defense_Object()->Get_Health());
	return 1;
}

int Lua_Set_Gravity_Multiplier(lua_State* L)
{
	if (lua_gettop(L) == 2) 
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_PhysicalGameObj())
		{
			PhysClass* Phys = obj->As_PhysicalGameObj()->Peek_Physical_Object();
			if (Phys && Phys->As_MoveablePhysClass())
			{
				Set_Gravity_Multiplier(obj, luaL_checknumber(L, 2));
				return 1;
			}
		}
	}
	return 0;
}

int Lua_Get_Gravity_Multiplier(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_PhysicalGameObj())
		{
			PhysClass* Phys = obj->As_PhysicalGameObj()->Peek_Physical_Object();
			if (Phys && Phys->As_MoveablePhysClass())
			{
				lua_pushnumber(L, Phys->As_MoveablePhysClass()->Get_Gravity_Multiplier());
				return 1;
			}
		}
	}
	return 0;
}

int Lua_Get_Player_Data(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	SoldierGameObj* o = obj->As_SoldierGameObj();
	if (!o || !o->Get_Player_Data())
	{
		return 0;
	}
#ifdef DAPLUGIN
	lua_newtable(L);
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->Get_Score());
	lua_setfield(L, -2, "Score");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->Get_Money());
	lua_setfield(L, -2, "Money");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->Get_Punish_Timer());
	lua_setfield(L, -2, "PunishTimer");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->GameTime);
	lua_setfield(L, -2, "GameTime");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->Get_Session_Time());
	lua_setfield(L, -2, "SessionTime");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->EnemiesKilled);
	lua_setfield(L, -2, "EnemiesKilled");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->AlliesKilled);
	lua_setfield(L, -2, "AlliesKilled");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->ShotsFired);
	lua_setfield(L, -2, "ShotsFired");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->HeadShots);
	lua_setfield(L, -2, "HeadShots");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->TorsoShots);
	lua_setfield(L, -2, "TorsoShots");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->ArmShots);
	lua_setfield(L, -2, "ArmShots");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->LegShots);
	lua_setfield(L, -2, "LegShots");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->CrotchShots);
	lua_setfield(L, -2, "CrotchShots");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->FinalHealth);
	lua_setfield(L, -2, "FinalHealth");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->PowerupsCollected);
	lua_setfield(L, -2, "PowerupsCollected");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->VehiclesDestroyed);
	lua_setfield(L, -2, "VehiclesDestroyed");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->VehicleTime);
	lua_setfield(L, -2, "VehicleTime");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->KillsFromVehicle);
	lua_setfield(L, -2, "KillsFromVehicle");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->Squishes);
	lua_setfield(L, -2, "Squishes");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->CreditGrant);
	lua_setfield(L, -2, "CreditGrant");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->BuildingDestroyed);
	lua_setfield(L, -2, "BuildingDestroyed");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->HeadHit);
	lua_setfield(L, -2, "HeadHit");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->TorsoHit);
	lua_setfield(L, -2, "TorsoHit");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->ArmHit);
	lua_setfield(L, -2, "ArmHit");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->LegHit);
	lua_setfield(L, -2, "LegHit");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->CrotchHit);
	lua_setfield(L, -2, "CrotchHit");
	lua_pushnumber(L, ((PlayerDataClass*)o->Get_Player_Data())->WeaponFired);
	lua_setfield(L, -2, "WeaponFired");
	// dont know how to add these ones
	//  ReferencerClass Owner; // 000C
	//  SimpleDynVecClass<int> WeaponsFiredID; // 0080
	//  SimpleDynVecClass<int> WeaponsFiredCount; // 0090
#endif
	return 1;
}


int Lua_Shake_Camera_Player(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	cPlayer* x = Find_Player(luaL_checknumber(L, 1));
	if (!x) { return 0; }
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->GetX(), a->GetY(), a->GetZ());
	Shake_Camera_Player(luaL_checknumber(L, 1), pos, luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5));
	return 1;
}

int Lua_Set_Delete_Pending(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			obj->Set_Delete_Pending();
		}
	}
	return 0;
}

int Lua_Set_Is_Delete_Pending(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			obj->Set_Is_Delete_Pending(lua_tobooleanCPP(L, 2));
		}
	}
	return 0;
}

int Lua_Disarm_All_C4_Beacons(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_All_C4_Beacons(lua_tonumber(L, 1));
	return 1;
}


int Lua_Update_Network_Object_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	Update_Network_Object_Player(obj, lua_tonumber(L, 2));
	return 1;
}

int Lua_Clear_Object_Dirty_Bits(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	obj->Clear_Object_Dirty_Bits();
	return 1;
}

int Lua_Get_Warhead_Type(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, ArmorWarheadManager::Get_Warhead_Type(lua_tostring(L, 1)));
	return 1;
}

int Lua_Are_Transitions_Enabled(lua_State* L)
{
#ifdef DAPLUGIN
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj->As_VehicleGameObj())
	{
		lua_pushboolean(L, obj->As_VehicleGameObj()->Are_Transitions_Enabled());
	}
#endif
	return 1;
}

int Lua_Get_All_C4(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<C4GameObj>* node = GameObjManager::C4GameObjList.Head(); node; node = node->Next())
	{
		GameObject* o = (GameObject*)node->Data();

		lua_pushnumber(L, Commands->Get_ID(o));
		lua_rawseti(L, buildingTable, index++);
	}
	return 1;
}

int Lua_Has_Turret(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_VehicleGameObj()) { return 0; }
	lua_pushboolean(L, obj->As_VehicleGameObj()->Has_Turret());
	return 1;
}

int Lua_Is_Valid_Skin(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, ArmorWarheadManager::Get_Armor_Type(luaL_checkstring(L, 1)));
	return 1;
}


int Lua_IsStealthUnit(lua_State * L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject * obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SmartGameObj()) { return 0; }
	lua_pushboolean(L, obj->As_SmartGameObj()->Get_Definition().Is_Stealthed());
	return 1;
}


int Lua_Is_Stealth_Enabled(lua_State* L) // replace existing one with this. existing one only worked on soldier.
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SmartGameObj()) { return 0; }
#ifdef DAPLUGIN
	lua_pushboolean(L, obj->As_SmartGameObj()->Is_Stealth_Enabled());
#endif
	return 1;
}

int Lua_Set_Is_Scripts_Visible(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_VehicleGameObj()) { return 0; }
	obj->As_VehicleGameObj()->Set_Is_Scripts_Visible(lua_tobooleanCPP(L, 2));
	return 1;
}

int Lua_Get_PT_Soldiers(lua_State* L)
{
	lua_newtable(L);
	int Table = lua_gettop(L);
	int index = 1;

	for (int Team = 0; Team < 2; Team++)
	{
		for (int Type = 0; Type < 7; Type++)
		{
#ifdef DAPLUGIN
			PurchaseSettingsDefClass* PT = PurchaseSettingsDefClass::Find_Definition((PurchaseSettingsDefClass::TYPE)Type, (PurchaseSettingsDefClass::TEAM)Team);
			if (PT)
			{
				for (int i = 0; i < 10; i++)
				{
					SoldierGameObjDef* Def = (SoldierGameObjDef*)Find_Definition(PT->Get_Definition(i));
					if (Def && Def->Get_Class_ID() == CID_Soldier)
					{
						lua_pushnumber(L, Def->Get_ID());
						lua_rawseti(L, Table, index++);
						Def = (SoldierGameObjDef*)Find_Definition(PT->Get_Alt_Definition(i, 0));
						if (Def && Def->Get_Class_ID() == CID_Soldier)
						{
							lua_pushnumber(L, Def->Get_ID());
							lua_rawseti(L, Table, index++);
						}
						Def = (SoldierGameObjDef*)Find_Definition(PT->Get_Alt_Definition(i, 1));
						if (Def && Def->Get_Class_ID() == CID_Soldier)
						{
							lua_pushnumber(L, Def->Get_ID());
							lua_rawseti(L, Table, index++);
						}
						Def = (SoldierGameObjDef*)Find_Definition(PT->Get_Alt_Definition(i, 2));
						if (Def && Def->Get_Class_ID() == CID_Soldier)
						{
							lua_pushnumber(L, Def->Get_ID());
							lua_rawseti(L, Table, index++);
						}
					}
				}
			}
#endif
		}
	}

	return 1;
}

int Lua_Get_PT_Vehicles(lua_State* L)
{
	lua_newtable(L);
	int Table = lua_gettop(L);
	int index = 1;

	for (int Team = 0; Team < 2; Team++)
	{
		for (int Type = 0; Type < 7; Type++)
		{
			PurchaseSettingsDefClass* PT = PurchaseSettingsDefClass::Find_Definition((PurchaseSettingsDefClass::TYPE)Type, (PurchaseSettingsDefClass::TEAM)Team);
			if (PT)
			{
				for (int i = 0; i < 10; i++)
				{
					VehicleGameObjDef* Def = (VehicleGameObjDef*)Find_Definition(PT->Get_Definition(i));
					if (Def && Def->Get_Class_ID() == CID_Vehicle)
					{
						lua_pushnumber(L, Def->Get_ID());
						lua_rawseti(L, Table, index++);
						Def = (VehicleGameObjDef*)Find_Definition(PT->Get_Alt_Definition(i, 0));
						if (Def && Def->Get_Class_ID() == CID_Vehicle)
						{
							lua_pushnumber(L, Def->Get_ID());
							lua_rawseti(L, Table, index++);
						}
						Def = (VehicleGameObjDef*)Find_Definition(PT->Get_Alt_Definition(i, 1));
						if (Def && Def->Get_Class_ID() == CID_Vehicle)
						{
							lua_pushnumber(L, Def->Get_ID());
							lua_rawseti(L, Table, index++);
						}
						Def = (VehicleGameObjDef*)Find_Definition(PT->Get_Alt_Definition(i, 2));
						if (Def && Def->Get_Class_ID() == CID_Vehicle)
						{
							lua_pushnumber(L, Def->Get_ID());
							lua_rawseti(L, Table, index++);
						}
					}
				}
			}
		}
	}
	return 1;
}


int Lua_Get_Weapon_Definition_ID_By_Definition_ID(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
#ifdef DAPLUGIN
	SmartGameObjDef* Def = (SmartGameObjDef*)Find_Definition(luaL_checknumber(L, 1));
	if (Def)
	{
		WeaponDefinitionClass* Weapon = (WeaponDefinitionClass*)Find_Definition(Def->WeaponDefID);
		if (Weapon)
		{
			lua_pushnumber(L, Weapon->Get_ID());
			return 1;
		}
	}
#endif
	return 0;
}

int Lua_Get_Weapon_Bag_Count(lua_State* L)
{
#ifdef DAPLUGIN
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			WeaponBagClass* Bag = obj->As_SmartGameObj()->Get_Weapon_Bag();
			if (Bag)
			{
				lua_pushnumber(L, Bag->Get_Count());
				return 1;
			}
		}
	}
#endif
	return 0;
}

int Lua_Get_Harvester_Vehicle(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
		if (obj && obj->As_BuildingGameObj()->As_RefineryGameObj())
		{
#ifdef DAPLUGIN
			lua_pushnumber(L, Get_ID(obj->As_BuildingGameObj()->As_RefineryGameObj()->Get_Harvester_Vehicle()));
#endif
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Harvester_Def_ID(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
		if (obj && obj->As_BuildingGameObj()->As_RefineryGameObj())
		{
			lua_pushnumber(L, obj->As_BuildingGameObj()->As_RefineryGameObj()->Get_Harvester_Def_ID());
			return 1;
		}
	}
	return 0;
}

int Lua_Is_Valid_Model(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	StringClass Model = luaL_checkstring(L, 1);
	FileClass* File = Get_Data_File(Model + ".w3d");
	if (File->Is_Available())
	{
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushboolean(L, false);
	}
	Close_Data_File(File);
	return 1;
}

int Lua_Get_Velocity(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Vector3 pos = Get_Velocity(Commands->Find_Object(lua_tonumber(L, 1)));
	LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);
	return 1;
}

int Lua_Get_Warhead_Name(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, ArmorWarheadManager::Get_Warhead_Name(luaL_checknumber(L, 1)));
	return 1;
}

int Lua_Set_MCTSkin(lua_State* L)
{
#ifdef DAPLUGIN
	if (lua_gettop(L) == 2)
	{
		BuildingGameObjDef* def = (BuildingGameObjDef*)Find_Definition(luaL_checknumber(L, 1));
		if (def)
		{
			def->MCTSkin = ArmorWarheadManager::Get_Armor_Type(luaL_checkstring(L, 2));
			return 1;
		}
	}
#endif
	return 0;
}

int Lua_Get_MCTSkin(lua_State* L)
{
#ifdef DAPLUGIN
	if (lua_gettop(L) == 1)
	{
		BuildingGameObjDef* def = (BuildingGameObjDef*)Find_Definition(luaL_checknumber(L, 1));
		if (def)
		{
			lua_pushnumber(L, def->MCTSkin);
			return 1;
		}
	}
#endif
	return 0;
}

int Lua_Set_Lightning(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;
	Commands->Set_Lightning(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6));
	return 1;
}

int Lua_Is_On_Ladder(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SoldierGameObj()) { return 0; }
	lua_pushboolean(L, obj->As_SoldierGameObj()->Is_On_Ladder());
	return 1;
}

int Lua_Set_Position_Bullets(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Position_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}

int Lua_Find_Closest_Object_With_Script(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	GameObject* obj = Find_Closest_Object_With_Script(luaL_checkstring(L, 1), location);
	lua_pushnumber(L, Get_ID(obj));
	return 1;
}


int Lua_Get_All_Spawners(lua_State* L)
{
#ifdef DAPLUGIN
	lua_newtable(L);
	int Table = lua_gettop(L);
	int index = 1;

	for (int i = 0; i < SpawnerList.Count(); i++)
	{
		const SpawnerDefClass* def = SpawnerList[i]->definition;
		if (def)
		{
			lua_pushnumber(L, def->Get_ID());
			lua_rawseti(L, Table, index++);
		}

	}
#endif
	return 1;
}

int Lua_Get_Spawn_Definition_ID_List(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
#ifdef DAPLUGIN
		const SpawnerDefClass* def = (SpawnerDefClass*)Find_Definition(luaL_checknumber(L, 1));
		if (def)
		{
			auto List = def->Get_Spawn_Definition_ID_List();
			int count = List.Count();
			if (count)
			{
				lua_newtable(L);
				int Table = lua_gettop(L);
				int index = 1;
				for (int i = 0; i < count; i++)
				{
					lua_pushnumber(L, List[i]);
					lua_rawseti(L, Table, index++);
				}
				return 1;
			}
		}
#endif
	}
	return 0;
}


int Lua_Get_All_Spawners_currentObject(lua_State* L)
{
#ifdef DAPLUGIN
	lua_newtable(L);
	int Table = lua_gettop(L);
	int index = 1;
	for (int i = 0; i < SpawnerList.Count(); i++)
	{
		GameObject* SpawnedObject = SpawnerList[i]->currentObject.Get_Ptr();
		if (SpawnedObject)
		{
			lua_pushnumber(L, Commands->Get_ID(SpawnedObject));
			lua_rawseti(L, Table, index++);
		}

	}
#endif
	return 1;
}

int Lua_Get_All_Spawners_Data(lua_State* L)
{
#ifdef DAPLUGIN
	lua_newtable(L);
	int index = 1;
	for (int i = 0; i < SpawnerList.Count(); i++)
	{
		const SpawnerDefClass* def = SpawnerList[i]->definition;
		if (def)
		{
			lua_pushnumber(L, index);
			index++;
			lua_newtable(L);

			lua_pushnumber(L, def->Get_ID());
			lua_setfield(L, -2, "Definition");

			Vector3 vec(SpawnerList[i]->transform.Get_Translation());
			LUA_Vector3* a = new LUA_Vector3(vec.X, vec.Y, vec.Z);
			lua_boxpointer(L, a);
			luaL_getmetatable(L, "Vector3");
			lua_setmetatable(L, -2);
			lua_setfield(L, -2, "Position");

			lua_pushstring(L, def->Get_Name());
			lua_setfield(L, -2, "Name");

			lua_pushboolean(L, def->GotoSpawnerPos);
			lua_setfield(L, -2, "GotoSpawnerPos");

			lua_pushboolean(L, def->IsSoldierStartup);
			lua_setfield(L, -2, "IsSoldierStartup");

			lua_pushboolean(L, SpawnerList[i]->enabled);
			lua_setfield(L, -2, "Enabled");

			lua_pushnumber(L, SpawnerList[i]->spawnCount);
			lua_setfield(L, -2, "SpawnCount");

			lua_pushnumber(L, def->Get_Player_Type());
			lua_setfield(L, -2, "Team");

			lua_pushnumber(L, def->Get_Parameter_Count());
			lua_setfield(L, -2, "ParameterCount");

			lua_pushboolean(L, def->IsMultiplayWeaponSpawner);
			lua_setfield(L, -2, "IsMultiplayWeaponSpawner");

			lua_pushnumber(L, def->SpawnDelay);
			lua_setfield(L, -2, "SpawnDelay");

			lua_pushnumber(L, def->SpawnDelayVariation);
			lua_setfield(L, -2, "SpawnDelayVariation");

			lua_pushnumber(L, def->SpawnMax);
			lua_setfield(L, -2, "SpawnMax");

			lua_pushboolean(L, def->StartsDisabled);
			lua_setfield(L, -2, "StartsDisabled");

			lua_pushboolean(L, def->TeleportFirstSpawn);
			lua_setfield(L, -2, "TeleportFirstSpawn");

			lua_pushnumber(L, SpawnerList[i]->spawnTimeRemaining);
			lua_setfield(L, -2, "SpawnTimeRemaining");

			lua_newtable(L);
			DynamicVectorClass<int> spawnlist = def->Get_Spawn_Definition_ID_List();
			for (int cur = 0; cur < spawnlist.Count(); cur++)
			{
				lua_pushnumber(L, cur + 1);
				lua_pushnumber(L, spawnlist[cur]);
				lua_settable(L, -3);
			}
			lua_setfield(L, -2, "SpawnDefinitionList");

			GameObject* obj = SpawnerList[i]->currentObject.Get_Ptr();
			if (obj != nullptr)
			{
				lua_pushnumber(L, Get_ID(obj));
			}
			else
			{
				lua_pushnumber(L, 0);
			}
			lua_setfield(L, -2, "GameObject");

			lua_settable(L, -3);
		}
	}
#endif
	return 1;
}

int Lua_Get_MCT_Position(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	BuildingGameObj* building = obj->As_BuildingGameObj();
	if (!building) return 0;
	BuildingAggregateClass* ag = building->Find_MCT();
	if (!ag) return 0;
	Vector3 pos;
	ag->Get_Position(&pos);
	LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);

	return 1;
}

int Lua_Get_MCT_Facing(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	BuildingGameObj* building = obj->As_BuildingGameObj();
	if (!building) return 0;
	BuildingAggregateClass* ag = building->Find_MCT();
	if (!ag) return 0;
	float rot = ag->Get_Facing() * (180.0f / 3.14159265f);
	lua_pushnumber(L, rot);

	return 1;
}


int Lua_Set_Team_Score(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Team_Score(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return 1;
}

int Lua_Get_All_Static_Objects(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;
	PhysicsSceneClassLua* inst = PhysicsSceneClassLua::Get_Instance();
	RefMultiListIterator<PhysClass> iter = inst->getStaticObjList();
	for (iter.First(); iter.Is_Done() == false; iter.Next())
	{
		PhysClass* o = iter.Peek_Obj();
		lua_pushnumber(L, o->Get_ID());
		lua_rawseti(L, buildingTable, index++);
	}
	return 1;
}

int Lua_Get_Bounding_Sphere_Radius(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));

	PhysicalGameObj *phys = obj->As_PhysicalGameObj();
	if (!phys)
	{
		return 0;
	}

	RenderObjClass *roc = phys->Peek_Model();
	if (!roc)
	{
		return 0;
	}

	lua_pushnumber(L, roc->Get_Bounding_Sphere().Radius);
	return 1;
}

int Lua_Set_Friendly_Fire_Permitted(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
#ifdef DAPLUGIN
	CombatManager::Set_Friendly_Fire_Permitted(lua_tobooleanCPP(L, 1));
#endif
	return 1;
}

int Lua_Is_Friendly_Fire_Permitted(lua_State* L)
{
#ifdef DAPLUGIN
	lua_pushboolean(L, CombatManager::Is_Friendly_Fire_Permitted());
#endif
	return 1;
}


int Lua_Enable_Letterbox_Player(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			bool ret = lua_tobooleanCPP(L, 2);
			float sec = luaL_checknumber(L, 3);
			Enable_Letterbox_Player(obj, ret, sec);
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Time_Scale(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		float scale = luaL_checknumber(L, 1);
		Set_Time_Scale(scale);
		return 1;
	}
	return 0;
}


int Lua_Is_On_Enemy_Pedestal(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_PhysicalGameObj()->As_BeaconGameObj())
		{
			lua_pushboolean(L, Is_On_Enemy_Pedestal(obj->As_PhysicalGameObj()->As_BeaconGameObj()));
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Bot_Tag(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			const char* str = luaL_checkstring(L, 2);
			obj->As_SoldierGameObj()->Set_Bot_Tag(str);
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Bot_Tag(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			StringClass botName = obj->As_SoldierGameObj()->Get_Bot_Tag();
			lua_pushstring(L, botName.Peek_Buffer());
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Block_Action_Key(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			bool ret = lua_tobooleanCPP(L, 2);
			obj->As_SoldierGameObj()->Set_Block_Action_Key(ret);
			return 1;
		}
	}
	return 0;
}


int Lua_Block_Action_Key(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushboolean(L, obj->As_SoldierGameObj()->Block_Action_Key());
			return 1;
		}
	}
	return 0;
}


int Lua_Set_Scale_Across_Network(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			float scale = luaL_checknumber(L, 2);
			obj->As_SoldierGameObj()->Set_Scale_Across_Network(scale);
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Scale_Across_Network(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushnumber(L, obj->As_SoldierGameObj()->Get_Scale_Across_Network());
			return 1;
		}
	}
	return 0;
}

int Lua_Can_Play_Damage_Animations(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushboolean(L, obj->As_SoldierGameObj()->Can_Play_Damage_Animations());
			return 1;
		}
	}
	return 0;
}


int Lua_Set_Can_Play_Damage_Animations(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			bool ret = lua_tobooleanCPP(L, 2);
			obj->As_SoldierGameObj()->Set_Can_Play_Damage_Animations(ret);
			return 1;
		}
	}
	return 0;
}

int Lua_Can_Drive_Vehicles(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushboolean(L, obj->As_SoldierGameObj()->Can_Drive_Vehicles());
			return 1;
		}
	}
	return 0;
}

int Lua_Can_Steal_Vehicles(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushboolean(L, obj->As_SoldierGameObj()->Can_Steal_Vehicles());
			return 1;
		}
	}
	return 0;
}


int Lua_Set_Can_Steal_Vehicles(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			bool ret = lua_tobooleanCPP(L, 2);
			obj->As_SoldierGameObj()->Set_Can_Steal_Vehicles(ret);
			return 1;
		}
	}
	return 0;
}

int Lua_Create_Yes_No_Dialog(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;

	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	WideStringClass title(lua_tostring(L, 2));
	WideStringClass label(lua_tostring(L, 3));
	int yes_id;
	int no_id;
	Vector3 color(lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));

	ScriptedDialogClass* dialog = Create_Yes_No_Dialog(obj, title, label, yes_id, no_id, color);
	if (dialog)
	{
		LuaScriptedDialogClass::Wrap(L, dialog);
		lua_pushnumber(L, yes_id);
		lua_pushnumber(L, no_id);
		return 3;
	}
	return 0;
}

int Lua_Show_Dialog(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	auto dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		Show_Dialog(dialog);
	}
	return 0;
}

int Lua_Create_Menu_Dialog(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;

	int id = lua_tonumber(L, 1);
	ScriptedDialogClass* dialog = Create_Menu_Dialog(id);
	if (dialog)
	{
		LuaScriptedDialogClass::Wrap(L, dialog);
		return 1;
	}
	return 0;
}

int Lua_Create_Popup_Dialog(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;

	int id = lua_tonumber(L, 1);
	ScriptedDialogClass* dialog = Create_Popup_Dialog(id);
	if (dialog)
	{
		LuaScriptedDialogClass::Wrap(L, dialog);
		return 1;
	}
	return 0;
}

int Lua_Delete_Dialog(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	auto dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		Delete_Dialog(dialog);
	}
	return 0;
}

int Lua_Hide_Dialog(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	auto dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		Hide_Dialog(dialog);
	}
	return 0;
}

int Lua_Find_Dialog(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	int id = lua_tonumber(L, 1);

	ScriptedDialogClass* dialog = Find_Dialog(id);
	if (dialog)
	{
		LuaScriptedDialogClass::Wrap(L, dialog);
		return 1;
	}
	return 0;
}

int Lua_Create_Yes_No_Cancel_Dialog(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;

	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	WideStringClass title(lua_tostring(L, 2));
	WideStringClass label(lua_tostring(L, 3));
	int yes_id;
	int no_id;
	int cancel_id;
	Vector3 color(lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));

	ScriptedDialogClass* dialog = Create_Yes_No_Cancel_Dialog(obj, title, label, yes_id, no_id, cancel_id, color);
	if (dialog)
	{
		LuaScriptedDialogClass::Wrap(L, dialog);
		lua_pushnumber(L, yes_id);
		lua_pushnumber(L, no_id);
		lua_pushnumber(L, cancel_id);
		return 4;
	}
	return 0;
}

int Lua_Create_User_Prompt(lua_State* L)
{
	if (lua_gettop(L) < 8) return 0;

	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	WideStringClass title(lua_tostring(L, 2));
	WideStringClass label(lua_tostring(L, 3));
	int limit = lua_tonumber(L, 4);
	WideStringClass initialText(lua_tostring(L, 5));
	
	int yes_id;
	int textbox_id;
	int cancel_id;
	Vector3 color(lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

	ScriptedDialogClass* dialog = Create_User_Prompt(obj, title, label, limit, textbox_id, yes_id, cancel_id, initialText, color);
	if (dialog)
	{
		LuaScriptedDialogClass::Wrap(L, dialog);
		lua_pushnumber(L, textbox_id);
		lua_pushnumber(L, yes_id);
		lua_pushnumber(L, cancel_id);
		return 4;
	}
	return 0;
}

int Lua_Create_Centered_Popup(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;

	int client = lua_tonumber(L, 1);
	int width = lua_tonumber(L, 2);
	int height = lua_tonumber(L, 3);
	WideStringClass title(lua_tostring(L, 4));
	ScriptedPopupDialogClass* dialog = Create_Centered_Popup(client, width, height, title);
	if (dialog)
	{
		LuaScriptedDialogClass::Wrap(L, dialog);
		return 1;
	}
	return 0;
}

int Lua_Set_Control_Bounds(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;

	ScriptedControlClass *control = LuaScriptedControlClass::GetInstance(L, 1);
	if (control)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		Set_Control_Bounds(control, x, y, width, height);
	}

	return 0;
}

int Lua_Create_ProgressBar_Control(lua_State* L)
{
	if (lua_gettop(L) < 8) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		int value = lua_tonumber(L, 6);
		int min = lua_tonumber(L, 7);
		int max = lua_tonumber(L, 8);
		auto obj = Create_ProgressBar_Control(dialog, x, y, width, height, value, min, max);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_Slider_Control(lua_State* L)
{
	if (lua_gettop(L) < 8) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		int value = lua_tonumber(L, 6);
		int min = lua_tonumber(L, 7);
		int max = lua_tonumber(L, 8);
		auto obj = Create_Slider_Control(dialog, x, y, width, height, value, min, max);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_ComboBox_Control(lua_State* L)
{
	if (lua_gettop(L) < 7) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		DynamicVectorClass<WideStringClass> items;

		/* table is in the stack at index 't' */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, 6) != 0) 
		{
			WideStringClass str(lua_tostring(L, -1));
			items.Add(str);
			lua_pop(L, 1);
		}

		int selected = lua_tonumber(L, 7);
		auto obj = Create_ComboBox_Control(dialog, x, y, width, height, &items, selected);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_CheckBox_Control(lua_State* L)
{
	if (lua_gettop(L) < 7) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		WideStringClass text(lua_tostring(L, 6));
		bool check = lua_tobooleanCPP(L, 7);
		auto obj = Create_CheckBox_Control(dialog, x, y, width, height, text, check);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_TextArea_Control(lua_State* L)
{
	if (lua_gettop(L) < 7) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		WideStringClass text(lua_tostring(L, 6));
		int limit = lua_tonumber(L, 7);
		auto obj = Create_TextArea_Control(dialog, x, y, width, height, text, limit);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_Title_Button_Control(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		WideStringClass text(lua_tostring(L, 6));
		auto obj = Create_Title_Button_Control(dialog, x, y, width, height, text);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_Image_Button_Control(lua_State* L)
{
	if (lua_gettop(L) < 7) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		auto obj = Create_Image_Button_Control(dialog, x, y, width, height, lua_tostring(L, 6), lua_tostring(L, 7));
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_Bordered_Button_Control(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		WideStringClass text(lua_tostring(L, 6));
		auto obj = Create_Bordered_Button_Control(dialog, x, y, width, height, text);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_Image_Control(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		auto obj = Create_Image_Control(dialog, x, y, width, height, lua_tostring(L, 6));
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}

int Lua_Create_Label_Control(lua_State* L)
{
	if (lua_gettop(L) < 10) return 0;

	ScriptedDialogClass *dialog = LuaScriptedDialogClass::GetInstance(L, 1);
	if (dialog)
	{
		int x = lua_tonumber(L, 2);
		int y = lua_tonumber(L, 3);
		int width = lua_tonumber(L, 4);
		int height = lua_tonumber(L, 5);
		WideStringClass text(lua_tostring(L, 6));
		int style = lua_tonumber(L, 7);
		Vector3 color(lua_tonumber(L, 8), lua_tonumber(L, 9), lua_tonumber(L, 10));
		auto obj = Create_Label_Control(dialog, x, y, width, height, text, static_cast<TextStyle>(style), color);
		if (obj)
		{
			LuaScriptedControlClass::Wrap(L, obj);
			return 1;
		}
	}

	return 0;
}


int Lua_Get_Control_Owner(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			lua_pushnumber(L, obj->As_SmartGameObj()->Get_Control_Owner());
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Control_Owner(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			obj->As_SmartGameObj()->Set_Control_Owner(luaL_checknumber(L, 2));
			return 1;
		}
	}
	return 0;
}

int Lua_Set_GameObj(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		cPlayer* p = Find_Player(luaL_checknumber(L, 1));
		if (p && p->Is_Active())
		{
			GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 2));
			if (obj && obj->As_SoldierGameObj())
			{
				p->Set_GameObj(obj->As_SoldierGameObj());
				return 1;
			}
		}
		
	}
	return 0;
}

int Lua_Get_Bounding_Sphere_Center(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L,1));
		if (obj)
		{
			PhysicalGameObj* phys = obj->As_PhysicalGameObj();
			if (phys)
			{
				RenderObjClass* roc = phys->Peek_Model();
				if (roc)
				{
					Vector3 pos;
					pos = roc->Get_Bounding_Sphere().Center;
					LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
					lua_boxpointer(L, a);
					luaL_getmetatable(L, "Vector3");
					lua_setmetatable(L, -2);
					return 1;
				}
			}
		}
	}
	return 0;
}

int Lua_Get_Bounding_Sphere_Volume(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			PhysicalGameObj* phys = obj->As_PhysicalGameObj();
			if (phys)
			{
				RenderObjClass* roc = phys->Peek_Model();
				if (roc)
				{
					lua_pushnumber(L, roc->Get_Bounding_Sphere().Volume());
					return 1;
				}
			}
		}
	}
	return 0;
}

int Lua_Get_Bounding_Box_Extent(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			PhysicalGameObj* phys = obj->As_PhysicalGameObj();
			if (phys)
			{
				RenderObjClass* roc = phys->Peek_Model();
				if (roc)
				{
					Vector3 pos;
					pos = roc->Get_Bounding_Box().Extent;
					LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
					lua_boxpointer(L, a);
					luaL_getmetatable(L, "Vector3");
					lua_setmetatable(L, -2);
					return 1;
				}
			}
		}
	}
	return 0;
}

int Lua_Get_Bounding_Box_Center(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			PhysicalGameObj* phys = obj->As_PhysicalGameObj();
			if (phys)
			{
				RenderObjClass* roc = phys->Peek_Model();
				if (roc)
				{
					Vector3 pos;
					pos = roc->Get_Bounding_Box().Center;
					LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
					lua_boxpointer(L, a);
					luaL_getmetatable(L, "Vector3");
					lua_setmetatable(L, -2);
					return 1;
				}
			}
		}
	}
	return 0;
}

int Lua_Get_Bounding_Box_Volume(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			PhysicalGameObj* phys = obj->As_PhysicalGameObj();
			if (phys)
			{
				RenderObjClass* roc = phys->Peek_Model();
				if (roc)
				{
					lua_pushnumber(L, roc->Get_Bounding_Box().Volume());
					return 1;
				}
			}
		}
	}
	return 0;
}

int Lua_Set_Player_Name(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		cPlayer* p = Find_Player(luaL_checknumber(L, 1));
		if (p)
		{
			const char* str_char = luaL_checkstring(L, 2);
			std::string str(str_char);
			if (str.size() > 0)
			{
				p->Set_Name(str_char);
				return 1;
			}
		}

	}
	return 0;
}


int Lua_Set_Override_Weapon_Hold_Style(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			obj->As_SoldierGameObj()->Set_Override_Weapon_Hold_Style(luaL_checknumber(L, 2));
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Override_Weapon_Hold_Style(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushnumber(L, obj->As_SoldierGameObj()->Get_Override_Weapon_Hold_Style());
			return 1;
		}
	}
	return 0;
}

int Lua_AddRefillHook(lua_State* L)
{
	AddRefillHook(LuaManager::Refill_Hook);
	return 1;
}

int Lua_Print_Client_Console(lua_State* L)
{
	if (lua_gettop(L) == 5)
	{
		Print_Client_Console(luaL_checkstring(L, 1), Vector4(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5)));
		return 1;
	}
	return 0;
}

int Lua_Print_Client_Console_Player(lua_State* L)
{
	if (lua_gettop(L) == 6)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && Commands->Is_A_Star(obj))
		{
			Print_Client_Console_Player(obj, luaL_checkstring(L, 2), Vector4(luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6)));
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Repository_URL(lua_State* L)
{
	lua_pushstring(L, Get_Repository_URL());
	return 1;
}

int Lua_Set_Repository_URL(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Set_Repository_URL(luaL_checkstring(L, 1));
		return 1;
	}
	return 0;
}

int Lua_Get_Screenshot_URL(lua_State* L)
{
	lua_pushstring(L, Get_Screenshot_URL());
	return 1;
}


int Lua_Set_Screenshot_URL(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Set_Screenshot_URL(luaL_checkstring(L, 1));
		return 1;
	}
	return 0;
}

int Lua_Take_Screenshot(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cPlayer* p = Find_Player(luaL_checknumber(L, 1));
		if (p)
		{
			Take_Screenshot(p->Get_Id());
			return 1;
		}
	}
	return 0;
}

int Lua_Is_In_Pathfind_Sector(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		LUA_Vector3* a = LuaVector3::GetInstance(L, 1);
		Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
		lua_pushboolean(L, Is_In_Pathfind_Sector(pos, luaL_checknumber(L, 2)));
		return 1;
	}
	return 0;
}

int Lua_Is_Gameplay_Allowed(lua_State* L)
{
	lua_pushboolean(L, Is_Gameplay_Allowed());
	return 1;
}

int Lua_Set_Gameplay_Allowed(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Set_Gameplay_Allowed(lua_tobooleanCPP(L, 1));
		return 1;
	}
	return 0;
}

int Lua_Get_Team_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int Red = 0, Green = 0, Blue = 0;
		Get_Team_Color(luaL_checknumber(L, 1), &Red, &Green, &Blue);
		lua_pushnumber(L, Red);
		lua_pushnumber(L, Green);
		lua_pushnumber(L, Blue);
		return 3;
	}
	return 0;
}

int Lua_Create_Logical_Sound(lua_State* L)
{
	if (lua_gettop(L) == 4)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		LUA_Vector3* a = LuaVector3::GetInstance(L, 3);
		Vector3 pos = Vector3(a->GetX(), a->GetY(), a->GetZ());
		lua_pushnumber(L, Commands->Create_Logical_Sound(obj,luaL_checknumber(L, 2),pos, luaL_checknumber(L, 4)));
		return 1;
	}
	return 0;
}

int Lua_Innate_Soldier_Enable_Bullet_Heard(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			Commands->Innate_Soldier_Enable_Bullet_Heard(obj,lua_tobooleanCPP(L,2));
		}
	}
	return 0;
}

int Lua_Innate_Soldier_Enable_Gunshot_Heard(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			Commands->Innate_Soldier_Enable_Gunshot_Heard(obj, lua_tobooleanCPP(L, 2));
		}
	}
	return 0;
}

int Lua_Innate_Soldier_Enable_Footsteps_Heard(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			Commands->Innate_Soldier_Enable_Footsteps_Heard(obj, lua_tobooleanCPP(L, 2));
		}
	}
	return 0;
}

int Lua_Get_All_Soldiers(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;
	for (SLNode<SoldierGameObj>* n = GameObjManager::SoldierGameObjList.Head(); n; n = n->Next()) 
	{
		SoldierGameObj* Soldier = n->Data();
		if (Soldier) {
			lua_pushnumber(L, Commands->Get_ID(Soldier));
			lua_rawseti(L, buildingTable, index++);
		}
	}
	return 1;
}

int Lua_Set_Freeze(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			obj->As_SoldierGameObj()->Set_Freeze(lua_tobooleanCPP(L,2));
			return 1;
		}
	}
	return 0;
}

int Lua_Is_Frozen(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushboolean(L, obj->As_SoldierGameObj()->Is_Frozen());
			return 1;
		}
	}
	return 0;
}

int Lua_Get_All_PowerUps(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<BaseGameObj>* node = GameObjManager::GameObjList.Head(); node; node = node->Next())
	{
		GameObject* o = (GameObject*)node->Data();
		if (o->As_PhysicalGameObj() && o->As_PhysicalGameObj()->As_PowerUpGameObj())
		{
			lua_pushnumber(L, Commands->Get_ID(o));
			lua_rawseti(L, buildingTable, index++);
		}
	}
	return 1;
}

int Lua_Is_Pathfind_Generated(lua_State* L)
{
	lua_pushboolean(L, Is_Pathfind_Generated());
	return 1;
}

int Lua_Stop_Sound_Team(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	Stop_Sound_Team(luaL_checknumber(L, 1), lua_tobooleanCPP(L, 2), luaL_checknumber(L, 3));
	return 1;
}

int Lua_Stop_Sound_Player(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
		if (obj)
		{
			Stop_Sound_Player(obj, luaL_checknumber(L, 2), lua_tobooleanCPP(L, 3));
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Player_ID_By_Name(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		lua_pushnumber(L, Get_Player_ID_By_Name(luaL_checkstring(L, 1)));
		return 1;
	}
	return 0;
}

int Lua_Reload_Level(lua_State* L)
{
	Reload_Level();
	return 1;
}

int Lua_Force_Client_Reload(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cPlayer* p = Find_Player(luaL_checknumber(L, 1));
		if (p)
		{
			Force_Client_Reload(p->Get_Id());
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Targeting_Pos(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			Vector3 pos = obj->As_SmartGameObj()->Get_Targeting_Pos();
			LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
			lua_boxpointer(L, a);
			luaL_getmetatable(L, "Vector3");
			lua_setmetatable(L, -2);
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Base_Destruction_Ends_Game(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameDataCnc* data = The_Cnc_Game();
		if (data) 
		{
			data->Set_Base_Destruction_Ends_Game(lua_tobooleanCPP(L, 1));
			Update_Game_Options(-1);
		}
	}
	return 0;
}

int Lua_Set_Beacon_Placement_Ends_Game(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameDataCnc* data = The_Cnc_Game();
		if (data) 
		{
			data->Set_Beacon_Placement_Ends_Game(lua_tobooleanCPP(L, 1));
			Update_Game_Options(-1);
		}
	}
	return 0;
}

int Lua_Set_Starting_Credits(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameDataCnc* data = The_Cnc_Game();
		if (data) 
		{
			data->Set_Starting_Credits(luaL_checkinteger(L, 1));
			Update_Game_Options(-1);
			return 1;
		}
	}
	return 0;
}

int Lua_Reset_Time_Remaining_Seconds(lua_State* L)
{
	cGameData* data = The_Game();
	if (data) 
	{
		data->Reset_Time_Remaining_Seconds();
		Update_Game_Options(-1);
	}
	return 0;
}

int Lua_Set_Time_Remaining_Seconds(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			data->Set_Time_Remaining_Seconds(luaL_checknumber(L, 1));
			Update_Game_Options(-1);
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Motd(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			WideStringClass title(luaL_checkstring(L, 1));
			data->Set_Motd(title);
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Map_Name(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			data->Set_Map_Name(luaL_checkstring(L, 1));
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Radar_Mode(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			data->Set_Radar_Mode(static_cast<RadarModeEnum>(luaL_checkinteger(L, 1)));
			Update_Game_Options(-1);
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Is_Team_Changing_Allowed(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			data->Set_Is_Team_Changing_Allowed(lua_tobooleanCPP(L, 1));
			Update_Game_Options(-1);
		}
	}
	return 0;
}

int Lua_Set_Remix_Teams(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			data->Set_Remix_Teams(lua_tobooleanCPP(L, 1));
			Update_Game_Options(-1);
		}
	}
	return 0;
}

int Lua_Set_Password(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			WideStringClass title(luaL_checkstring(L, 1));
			data->Set_Password(title);
			Update_Game_Options(-1);
		}
	}
	return 0;
}

int Lua_Set_Game_Title(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cGameData* data = The_Game();
		if (data) 
		{
			WideStringClass title(luaL_checkstring(L, 1));
			data->Set_Game_Title(title);
			Update_Game_Options(-1);
		}
	}
	return 0;
}

int Lua_Set_Intermission_Time_Seconds(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
#ifdef DAPLUGIN
		cGameDataCnc* data = The_Cnc_Game();
		if (data) 
		{
			data->IntermissionTime_Seconds = luaL_checkinteger(L, 1);
			Update_Game_Options(-1);
		}
#endif
	}
	return 0;
}

int Lua_Set_Object_Visibility(lua_State* L)
{
	if (lua_gettop(L) == 2) 
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Set_Object_Visibility(obj->Get_ID(), lua_tobooleanCPP(L, 2));
		}
	}
	return 0;
}

int Lua_Set_Object_Visibility_For_Player(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && Commands->Is_A_Star(obj))
		{
			GameObject* obj2 = Commands->Find_Object(luaL_checkinteger(L, 2));
			if (obj2) 
			{
				Set_Object_Visibility_For_Player(obj,obj2->Get_ID(), lua_tobooleanCPP(L, 3));
			}
		}
	}
	return 0;
}

int Lua_Set_Team_Visibility(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_PhysicalGameObj())
		{
			obj->As_PhysicalGameObj()->Set_Team_Visibility(static_cast<TeamVisibilitySetting>(luaL_checkinteger(L, 2)));
		}
	}
	return 0;
}

int Lua_Get_Client_Hardware_Identifier(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		lua_pushstring(L, Get_Client_Hardware_Identifier(lua_tonumber(L, 1)));
		return 1;
	}
	return 0;
}

/* TT 4.8 Update 1 */
int Lua_Is_Connection_Bad(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		lua_pushboolean(L, Is_Connection_Bad(lua_tonumber(L, 1)));
		return 1;
	}
	return 0;
}

int Lua_Get_Time_Of_Day(lua_State* L)
{
	unsigned int hours, mins = 0;
	Get_Time_Of_Day(&hours, &mins);
	lua_pushnumber(L, hours);
	lua_pushnumber(L, mins);
	return 2;
}

int Lua_Set_Time_Of_Day(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		Set_Time_Of_Day(lua_tonumber(L, 1), lua_tonumber(L,2));
	}
	return 0;
}

int Lua_Get_Screen_Resolution(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Vector2 res;
		Get_Screen_Resolution(lua_tonumber(L, 1), &res);
		lua_pushnumber(L, res.X);
		lua_pushnumber(L, res.Y);
		return 2;
	}
	return 0;
}

int Lua_Enable_Global_Targeting(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Enable_Global_Targeting(lua_tobooleanCPP(L, 1));
	}
	return 0;
}

int Lua_Enable_Global_Targeting_Player(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Enable_Global_Targeting_Player(obj, lua_tobooleanCPP(L, 2));
		}
	}
	return 0;
}

int Lua_Get_Moon_Halo_Color(lua_State* L)
{
	auto color = Get_Moon_Halo_Color();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Get_Warm_Sky_Color(lua_State* L)
{
	auto color = Get_Warm_Sky_Color();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Get_Cold_Sky_Color(lua_State* L)
{
	auto color = Get_Cold_Sky_Color();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Get_Sun_Color(lua_State* L)
{
	auto color = Get_Sun_Color();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Get_Sun_Halo_Color(lua_State* L)
{
	auto color = Get_Sun_Halo_Color();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Get_Starfield_Alpha(lua_State* L)
{
	auto color = Get_Starfield_Alpha();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Get_Sky_Color(lua_State* L)
{
	auto color = Get_Sky_Color();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Get_Gloominess_Color(lua_State* L)
{
	auto color = Get_Gloominess_Color();
	lua_pushnumber(L, color);
	return 1;
}

int Lua_Set_Warm_Sky_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Warm_Sky_Color(val);
	}
	return 0;
}

int Lua_Set_Cold_Sky_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Cold_Sky_Color(val);
	}
	return 0;
}

int Lua_Set_Sun_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Sun_Color(val);
	}
	return 0;
}

int Lua_Set_Sun_Halo_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Sun_Halo_Color(val);
	}
	return 0;
}

int Lua_Set_Moon_Halo_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Moon_Halo_Color(val);
	}
	return 0;
}

int Lua_Set_Starfield_Alpha(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Starfield_Alpha(val);
	}
	return 0;
}

int Lua_Set_Sky_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Sky_Color(val);
	}
	return 0;
}

int Lua_Set_Gloominess_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Gloominess_Color(val);
	}
	return 0;
}

int Lua_Is_Overriding_Sky_Colors(lua_State* L)
{
	lua_pushboolean(L, Is_Overriding_Sky_Colors());
	return 0;
}

int Lua_Override_Sky_Colors(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Override_Sky_Colors(lua_tobooleanCPP(L, 1));
	}
	return 0;
}

int Lua_Change_Enemy_HUD_Color(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		float r = lua_tonumber(L, 1);
		float g = lua_tonumber(L, 2);
		float b = lua_tonumber(L, 3);
		Change_Enemy_HUD_Color(Vector3(r, g, b));
	}
	return 0;
}

int Lua_Change_Enemy_HUD_Color_Player(lua_State* L)
{
	if (lua_gettop(L) == 4)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			float r = lua_tonumber(L, 2);
			float g = lua_tonumber(L, 3);
			float b = lua_tonumber(L, 4);
			Change_Enemy_HUD_Color_Player(obj, Vector3(r, g, b));
		}
	}
	return 0;
}

int Lua_Change_Friendly_HUD_Color(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		float r = lua_tonumber(L, 1);
		float g = lua_tonumber(L, 2);
		float b = lua_tonumber(L, 3);
		Change_Friendly_HUD_Color(Vector3(r, g, b));
	}
	return 0;
}

int Lua_Change_Neutral_HUD_Color(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		float r = lua_tonumber(L, 1);
		float g = lua_tonumber(L, 2);
		float b = lua_tonumber(L, 3);
		Change_Neutral_HUD_Color(Vector3(r, g, b));
	}
	return 0;
}

int Lua_Change_Friendly_HUD_Color_Player(lua_State* L)
{
	if (lua_gettop(L) == 4)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			float r = lua_tonumber(L, 2);
			float g = lua_tonumber(L, 3);
			float b = lua_tonumber(L, 4);
			Change_Friendly_HUD_Color_Player(obj, Vector3(r, g, b));
		}
	}
	return 0;
}

int Lua_Change_Neutral_HUD_Color_Player(lua_State* L)
{
	if (lua_gettop(L) == 4)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			float r = lua_tonumber(L, 2);
			float g = lua_tonumber(L, 3);
			float b = lua_tonumber(L, 4);
			Change_Neutral_HUD_Color_Player(obj, Vector3(r, g, b));
		}
	}
	return 0;
}

int Lua_Create_HUD_Surface(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		int id = lua_tonumber(L, 1);
		HUDSurfaceClass *hud = Create_HUD_Surface(id);
		LuaHUDSurfaceClass::Wrap(L, hud);
		return 1;
	}
	return 0;
}

int Lua_Find_HUD_Surface(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		int id = lua_tonumber(L, 1);
		HUDSurfaceClass *hud = Find_HUD_Surface(id);
		LuaHUDSurfaceClass::Wrap(L, hud);
		return 1;
	}
	return 0;
}

int Lua_Show_HUD_Surface(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		HUDSurfaceClass *hud = LuaHUDSurfaceClass::GetInstance(L, 1);
		if (hud)
		{
			Show_HUD_Surface(hud);
		}
	}
	return 0;
}

int Lua_Hide_HUD_Surface(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		HUDSurfaceClass *hud = LuaHUDSurfaceClass::GetInstance(L, 1);
		if (hud)
		{
			Hide_HUD_Surface(hud);
		}
	}
	return 0;
}

int Lua_Delete_HUD_Surface(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		HUDSurfaceClass *hud = LuaHUDSurfaceClass::GetInstance(L, 1);
		if (hud)
		{
			Delete_HUD_Surface(hud);
		}
	}
	return 0;
}

int Lua_Enable_Spawners_By_Name(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		const char* name = lua_tostring(L, 1);
		bool enable = lua_tobooleanCPP(L, 2);
		Enable_Spawners_By_Name(name, enable);
	}
	return 0;
}

int Lua_Is_Extras_Enabled(lua_State* L)
{
	bool enable = Is_Extras_Enabled();
	lua_pushboolean(L, enable);
	return 1;
}

int Lua_Is_Sidebar_Enabled(lua_State* L)
{
	bool enable = Is_Sidebar_Enabled();
	lua_pushboolean(L, enable);
	return 1;
}

int Lua_Add_Shadow_Message(lua_State* L)
{
	if (lua_gettop(L) == 5)
	{
		float r = lua_tonumber(L, 2);
		float g = lua_tonumber(L, 3);
		float b = lua_tonumber(L, 4);
		WideStringClass str(luaL_checkstring(L, 5));
		Add_Shadow_Message(luaL_checkinteger(L, 1), Vector3(r, g, b), str);
	}
	return 0;
}

int Lua_Forward_Server(lua_State* L)
{
	if (lua_gettop(L) == 4)
	{
		Forward_Server(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3), luaL_checkstring(L, 4));
	}
	else if (lua_gettop(L) == 3)
	{
		Forward_Server(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3), nullptr);
	}
	return 0;
}

int Lua_Retrieve_Waypaths(lua_State* L)
{
	lua_newtable(L);
	int Table = lua_gettop(L);
	int index = 1;
	DynamicVectorClass<int> items;
	Retrieve_Waypaths(items);
	for (int i = 0; i < items.Count(); ++i)
	{
		lua_pushnumber(L, items[i]);
		lua_rawseti(L, Table, index++);
	}
	return 1;
}

int Lua_Get_Waypoint_Position(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		Vector3 pos;
		Get_Waypoint_Position(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), pos);
		LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
		lua_boxpointer(L, a);
		luaL_getmetatable(L, "Vector3");
		lua_setmetatable(L, -2);
		return 1;
	}
	return 0;
}

int Lua_Retrieve_Waypoints(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		lua_newtable(L);
		int Table = lua_gettop(L);
		int index = 1;
		DynamicVectorClass<int> items;
		Retrieve_Waypoints(luaL_checkinteger(L, 1), items);
		for (int i = 0; i < items.Count(); ++i)
		{
			lua_pushnumber(L, items[i]);
			lua_rawseti(L, Table, index++);
		}
		return 1;
	}
	return 0;
}

int Lua_Get_Human_State(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			HumanStateClass* State = obj->As_SoldierGameObj()->Get_Human_State();
			if (State)
			{
				lua_pushinteger(L, State->Get_State());
				return 1;
			}
		}
	}
	return 0;
}

int Lua_Get_Human_State_Flag(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			HumanStateClass* State = obj->As_SoldierGameObj()->Get_Human_State();
			if (State)
			{
				lua_pushboolean(L, State->Get_State_Flag(luaL_checkinteger(L, 2)));
				return 1;
			}
		}
	}
	return 0;
}

int Lua_Set_Skeleton_Height(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			float v = lua_tonumber(L, 2);
			obj->As_SoldierGameObj()->Set_Skeleton_Height(v);
		}
	}
	return 0;
}

int Lua_Set_Skeleton_Width(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			float v = lua_tonumber(L, 2);
			obj->As_SoldierGameObj()->Set_Skeleton_Width(v);
		}
	}
	return 0;
}

int Lua_Set_Stealth_Active(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			obj->As_SmartGameObj()->Set_Stealth_Active(lua_tobooleanCPP(L, 2));
		}
	}
	return 0;
}

int Lua_Add_Radar_Marker(lua_State* L)
{
	if (lua_gettop(L) == 4)
	{
		LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
		Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
		Commands->Add_Radar_Marker(luaL_checkinteger(L, 1),pos,luaL_checkinteger(L, 3),luaL_checkinteger(L, 4));
	}
	return 0;
}

int Lua_Clear_Radar_Marker(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Commands->Clear_Radar_Marker(luaL_checkinteger(L, 1));
	}
	return 0;
}

int Lua_Clear_Radar_Markers(lua_State* L)
{
	Commands->Clear_Radar_Markers();
	return 0;
}

int Lua_Set_Objective_Radar_Blip(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
		Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
		Commands->Set_Objective_Radar_Blip(luaL_checkinteger(L, 1), pos);
	}
	return 0;
}

int Lua_Set_Obj_Radar_Blip_Color(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Commands->Set_Obj_Radar_Blip_Color(obj, luaL_checkinteger(L, 2));
		}
	}
	return 0;
}

int Lua_Set_Obj_Radar_Blip_Shape(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Commands->Set_Obj_Radar_Blip_Shape(obj, luaL_checkinteger(L, 2));
		}
	}
	return 0;
}

int Lua_Get_Skeleton_Width(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushnumber(L, obj->As_SoldierGameObj()->Get_Skeleton_Width());
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Skeleton_Heigth(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			lua_pushnumber(L, obj->As_SoldierGameObj()->Get_Skeleton_Heigth());
			return 1;
		}
	}
	return 0;
}


int Lua_Is_Destroyed(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			bool n = nullptr;
			if (obj->As_SoldierGameObj())
			{
				n = obj->As_SoldierGameObj()->Is_Destroyed();
			}
			else if (obj->As_BuildingGameObj())
			{
				n = obj->As_BuildingGameObj()->Is_Destroyed();
			}
			lua_pushboolean(L, n);
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Camera(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			const char* cameraProfile = nullptr;
			if (obj->As_SoldierGameObj())
			{
				cameraProfile = "Normal";
			}
			else if (obj->As_VehicleGameObj())
			{
				cameraProfile = obj->As_VehicleGameObj()->Get_Profile();
			}
			lua_pushstring(L, cameraProfile);
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Object_Dirty_Bit(lua_State* L)
{
	if (lua_gettop(L) == 4 && lua_islightuserdata(L, 1) == true)
	{
		cScTextObj* obj = nullptr;
		obj = (cScTextObj*)(lua_topointer(L, 1));
		obj->Set_Object_Dirty_Bit(luaL_checkinteger(L, 2), static_cast<NetworkObjectClass::DIRTY_BIT>(luaL_checkinteger(L, 3)), lua_tobooleanCPP(L, 4));
	}
	else if (lua_gettop(L) == 3) {
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			obj->Set_Object_Dirty_Bit(static_cast<NetworkObjectClass::DIRTY_BIT>(luaL_checkinteger(L, 2)), lua_tobooleanCPP(L, 3));
		}
		else
		{
			cPlayer* p = Find_Player(luaL_checkinteger(L, 1));
			if (p)
			{
				p->Set_Object_Dirty_Bit(static_cast<NetworkObjectClass::DIRTY_BIT>(luaL_checkinteger(L, 2)), lua_tobooleanCPP(L, 3));
			}
		}
	}
	return 0;
}

int Lua_Send_Client_Text(lua_State* L)
{
	if (lua_gettop(L) == 7)
	{
		cScTextObj* Text = Send_Client_Text(WideStringClass(luaL_checkstring(L, 1)), static_cast<TextMessageEnum>(luaL_checkinteger(L, 2)), lua_tobooleanCPP(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), lua_tobooleanCPP(L, 6), lua_tobooleanCPP(L, 7));
		if (Text)
		{
			lua_pushlightuserdata(L, (void*)Text);
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Object_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			unsigned int Red = 0, Green = 0, Blue = 0;
			Get_Object_Color(obj, &Red, &Green, &Blue);
			lua_pushnumber(L, Red);
			lua_pushnumber(L, Green);
			lua_pushnumber(L, Blue);
			return 3;
		}
	}
	return 0;
}

int Lua_Get_Level_Extents(lua_State* L)
{
	Vector3 min, max;
	PhysicsSceneClass::Get_Instance()->Get_Level_Extents(min, max);
	LUA_Vector3* a = new LUA_Vector3(min.X, min.Y, min.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);
	LUA_Vector3* b = new LUA_Vector3(max.X, max.Y, max.Z);
	lua_boxpointer(L, b);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);
	return 2;
}

int Lua_Is_Skin_Impervious(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		lua_pushboolean(L,ArmorWarheadManager::Is_Skin_Impervious(static_cast<ArmorWarheadManager::SpecialDamageType>(luaL_checkinteger(L, 1)),luaL_checkinteger(L, 2)));
		return 1;
	}
	return 0;
}

int Lua_Get_Damage_Multiplier(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		lua_pushnumber(L, ArmorWarheadManager::Get_Damage_Multiplier(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
		return 1;
	}
	return 0;
}

int Lua_Get_Shield_Absorbsion(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		lua_pushnumber(L, ArmorWarheadManager::Get_Shield_Absorbsion(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
		return 1;
	}
	return 0;
}

int Lua_Get_Sight_Range(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			lua_pushnumber(L, obj->As_SmartGameObj()->Get_Definition().Get_Sight_Range());
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Sight_Range(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			const_cast<SmartGameObjDef&>(obj->As_SmartGameObj()->Get_Definition()).Set_Sight_Range(luaL_checknumber(L,2));
		}
	}
	return 0;
}

int Lua_Get_Sight_Arc(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			lua_pushnumber(L, obj->As_SmartGameObj()->Get_Definition().Get_Sight_Arc());
			return 1;
		}
	}
	return 0;
}

int Lua_Set_Sight_Arc(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			const_cast<SmartGameObjDef&>(obj->As_SmartGameObj()->Get_Definition()).Set_Sight_Arc(luaL_checknumber(L, 2));
		}
	}
	return 0;
}



int Lua_Get_Hide_Team_Battlefield_Information(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		BuildingGameObjDef* def = (BuildingGameObjDef*)Find_Definition(luaL_checknumber(L, 1));
		if (def)
		{
			lua_pushboolean(L, def->HideTeamBattlefieldInformation);
			return 1;
		}
	}
	return 0;
}


int Lua_HideTeamBattlefieldInformation(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		BuildingGameObjDef* def = (BuildingGameObjDef*)Find_Definition(luaL_checknumber(L, 1));
		if (def)
		{
			def->HideTeamBattlefieldInformation = lua_tobooleanCPP(L, 2);
		}
	}
	return 0;
}

int Lua_Trigger_Smooth_Skeleton_Width_Resize(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			float v = lua_tonumber(L, 2);
			float b = lua_tonumber(L, 3);
			obj->As_SoldierGameObj()->Trigger_Smooth_Skeleton_Width_Resize(v, b);
		}
	}
	return 0;
}

int Lua_Trigger_Smooth_Skeleton_Height_Resize(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			float v = lua_tonumber(L, 2);
			float b = lua_tonumber(L, 3);
			obj->As_SoldierGameObj()->Trigger_Smooth_Skeleton_Height_Resize(v, b);
		}
	}
	return 0;
}

int Lua_Set_Targeting(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
			Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
			obj->As_SmartGameObj()->Set_Targeting(pos, lua_tobooleanCPP(L, 3));
		}
	}
	return 0;
}

int Lua_Get_Human_Sub_State(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			HumanStateClass* State = obj->As_SoldierGameObj()->Get_Human_State();
			if (State)
			{
				lua_pushinteger(L, State->Get_Sub_State());
				return 1;
			}
		}
	}
	return 0;
}

int Lua_Get_Muzzle_Position(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_SmartGameObj())
		{
			Matrix3D mat = obj->As_SmartGameObj()->Get_Muzzle(luaL_checkinteger(L, 2));
			Vector3 pos = mat.Get_Translation();
			LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
			lua_boxpointer(L, a);
			luaL_getmetatable(L, "Vector3");
			lua_setmetatable(L, -2);
			return 1;
		}
	}
	return 0;
}

int Lua_Intersects_Object(lua_State* L)
{
	if (lua_gettop(L) == 4)
	{
		LUA_Vector3* a = LuaVector3::GetInstance(L, 1);
		LUA_Vector3* b = LuaVector3::GetInstance(L, 2);
		if (a == nullptr || b == nullptr) return 0;
		Vector3 origin_point = Vector3(a->GetX(), a->GetY(), a->GetZ());
		Vector3 direction = Vector3(b->GetX(), b->GetY(), b->GetZ());

		float length = lua_tonumber(L, 3);

		LineSegClass line(origin_point, direction, length);
		CastResultStruct result;
		result.ComputeContactPoint = true;
		PhysRayCollisionTestClass raycast(line, &result, static_cast<Collision_Group_Type>(luaL_checkinteger(L, 4)) );
		if (PhysicsSceneClass::Get_Instance()->Cast_Ray(raycast))
		{
			CombatPhysObserverClass* old_observer = (CombatPhysObserverClass*)raycast.CollidedPhysObj->Get_Observer();
			if (old_observer && old_observer->As_DamageableGameObj())
			{
				lua_pushnumber(L, old_observer->As_DamageableGameObj()->Get_ID());
				return 1;
			}
		}
	}
	return 0;
}

int Lua_Is_Targetable(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_DamageableGameObj())
		{
			lua_pushboolean(L, obj->As_DamageableGameObj()->Is_Targetable());
			return 1;
		}
	}
	return 0;
}

int Lua_Is_Health_Bar_Displayed(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_DamageableGameObj())
		{
			lua_pushboolean(L, obj->As_DamageableGameObj()->Is_Health_Bar_Displayed());
			return 1;
		}
	}
	return 0;
}

int Lua_Is_Flooding(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cPlayer* p = Find_Player(luaL_checknumber(L, 1));
		if (p)
		{
			lua_pushboolean(L, p->Is_Flooding());
			return 1;
		}
	}
	return 0;
}

int Lua_Increment_Flood_Counter(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cPlayer* p = Find_Player(luaL_checknumber(L, 1));
		if (p)
		{
			p->Increment_Flood_Counter();
		}
	}
	return 0;
}

int Lua_Decrement_Flood_Counter(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cPlayer* p = Find_Player(luaL_checknumber(L, 1));
		if (p)
		{
			p->Decrement_Flood_Counter();
		}
	}
	return 0;
}

int Lua_Get_All_Simple_Objects(lua_State* L)
{
	lua_newtable(L);
	int buildingTable = lua_gettop(L);
	int index = 1;

	for (SLNode<BaseGameObj>* node = GameObjManager::GameObjList.Head(); node; node = node->Next())
	{
		GameObject* o = (GameObject*)node->Data();
		if (o->As_PhysicalGameObj() && o->As_PhysicalGameObj()->As_SimpleGameObj())
		{
			lua_pushnumber(L, Commands->Get_ID(o));
			lua_rawseti(L, buildingTable, index++);
		}
	}
	return 1;
}

int Lua_Set_Can_Object_Die(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_DamageableGameObj() && obj->As_DamageableGameObj()->Get_Defense_Object())
		{
			obj->As_DamageableGameObj()->Get_Defense_Object()->Set_Can_Object_Die(lua_tobooleanCPP(L, 2));
		}
	}
	return 0;
}


int Lua_Get_Parent_Object(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_PhysicalGameObj())
		{
			GameObject* host = obj->As_PhysicalGameObj()->Get_Parent_Object();
			if (host)
			{
				lua_pushnumber(L, host->Get_ID());
				return 1;
			}
		}
	}
	return 0;
}

int Lua_Is_Rendered(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj && obj->As_PhysicalGameObj())
		{
			lua_pushboolean(L, obj->As_PhysicalGameObj()->Peek_Model()->Is_Hidden() == 0);
			return 1;
		}
	}
	return 0;
}

int Lua_Get_Interior_Building_Meshes(lua_State* L)
{
	lua_newtable(L);
	int meshTable = lua_gettop(L);
	int index = 1;

	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	if (obj && obj->As_BuildingGameObj())
	{
		BuildingGameObj* building = obj->As_BuildingGameObj();
		auto& meshes = building->*get(GetPrivate_IntBuildingGameObj{});

		RefMultiListIterator<StaticPhysClass> iter(&meshes);
		for (iter.First(); !iter.Is_Done(); iter.Next())
		{
			StaticPhysClass* mesh = iter.Peek_Obj();
			if (!mesh) continue;
			lua_pushnumber(L, mesh->Get_ID());
			lua_rawseti(L, meshTable, index++);
		}
	}
	return 1;
}

int Lua_Get_Exterior_Building_Meshes(lua_State* L)
{
	lua_newtable(L);
	int meshTable = lua_gettop(L);
	int index = 1;

	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	if (obj && obj->As_BuildingGameObj())
	{
		BuildingGameObj* building = obj->As_BuildingGameObj();
		auto& meshes = building->*get(GetPrivate_ExtBuildingGameObj{});

		RefMultiListIterator<StaticPhysClass> iter(&meshes);
		for (iter.First(); !iter.Is_Done(); iter.Next())
		{
			StaticPhysClass* mesh = iter.Peek_Obj();
			if (!mesh) continue;
			lua_pushnumber(L, mesh->Get_ID());
			lua_rawseti(L, meshTable, index++);
		}
	}
	return 1;
}

int Lua_Has_Interior_Building_Meshes(lua_State* L)
{
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	if (obj && obj->As_BuildingGameObj())
	{
		BuildingGameObj* building = obj->As_BuildingGameObj();
		auto& meshes = building->*get(GetPrivate_IntBuildingGameObj{});
		lua_pushboolean(L, !meshes.Is_Empty());
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}

int Lua_Has_Exterior_Building_Meshes(lua_State* L)
{
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	if (obj && obj->As_BuildingGameObj())
	{
		BuildingGameObj* building = obj->As_BuildingGameObj();
		auto& meshes = building->*get(GetPrivate_ExtBuildingGameObj{});
		lua_pushboolean(L, !meshes.Is_Empty());
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define REGFUNC(func) lua_register(L, #func, Lua_##func);

void AddFunctions(lua_State *L)
{

/* TODO
	Display_HUD_Ammo_Grant_Player
	Display_HUD_Weapon_Grant_Player
	Write_File_Async
	Set_Subobject_Animation_Player
	
	Set_Subobject_Animation
	Say_Dynamic_Dialogue
	Say_Dynamic_Dialogue_Player
	Say_Dynamic_Dialogue_Team
	Find_Closest_Poly_Position
	
	Kill_Messages_Disabled
	Set_Emot_Icon
	Set_Emote_Icon?
	Get_Radio_Command_String
	Get_Closest_Pathfind_Spot_Size
	Get_Closest_Pathfind_Spot
	
	Get_Multiplayer_Spawn_Location
	Get_Pathfind_Distance_Blocking
	Get_Pathfind_Distance_Async
	Cancel_Get_Pathfind_Distance
	Get_Pathfind_Distance

	void	(* Add_Objective)( int id, int type, int status, int short_description_id, char * description_sound_filename, int long_description_id);
	void	(* Remove_Objective)( int id );
	void	(* Set_Objective_Status)( int id, int status );
	void	(* Change_Objective_Type)( int id, int type );
	void	(* Set_Objective_Radar_Blip)( int id, const Vector3 & position );
	void	(* Set_Objective_Radar_Blip_Object)( int id, ScriptableGameObj * unit );
	void	(* Set_Objective_HUD_Info)( int id, float priority, const char * texture_name, int message_id );
	void	(* Set_Objective_HUD_Info_Position)( int id, float priority, const char * texture_name, int message_id, const Vector3 & position );

*/	
	REGFUNC(Is_Soldier_Busy)
	REGFUNC(Has_Interior_Building_Meshes)
	REGFUNC(Has_Exterior_Building_Meshes)
	REGFUNC(Get_Interior_Building_Meshes)
	REGFUNC(Get_Exterior_Building_Meshes)
	REGFUNC(Is_Rendered)
	REGFUNC(Get_Parent_Object)
	REGFUNC(Set_Can_Object_Die)
	REGFUNC(Get_All_Simple_Objects)
	REGFUNC(Increment_Flood_Counter)
	REGFUNC(Decrement_Flood_Counter)
	REGFUNC(Is_Flooding)
	REGFUNC(Is_Health_Bar_Displayed)
	REGFUNC(Is_Targetable)
	REGFUNC(Get_Human_Sub_State)
	REGFUNC(Set_Targeting)
	REGFUNC(Get_Muzzle_Position)
	REGFUNC(Intersects_Object)
	REGFUNC(Trigger_Smooth_Skeleton_Height_Resize)
	REGFUNC(Trigger_Smooth_Skeleton_Width_Resize)
	REGFUNC(HideTeamBattlefieldInformation)
	REGFUNC(Get_Hide_Team_Battlefield_Information)
	REGFUNC(Is_Attached_To_An_Object)
	REGFUNC(Set_Sight_Arc)
	REGFUNC(Get_Sight_Arc)
	REGFUNC(Set_Sight_Range)
	REGFUNC(Get_Sight_Range)
	REGFUNC(Get_Shield_Absorbsion)
	REGFUNC(Get_Damage_Multiplier)
	REGFUNC(Is_Skin_Impervious)
	REGFUNC(Get_Level_Extents)
	REGFUNC(Is_In_Elevator)
	REGFUNC(Get_Object_Color)
	REGFUNC(Send_Client_Text)
	REGFUNC(Set_Obj_Radar_Blip_Shape)
	REGFUNC(Set_Obj_Radar_Blip_Color)
	REGFUNC(Set_Objective_Radar_Blip)
	REGFUNC(Clear_Radar_Markers)
	REGFUNC(Clear_Radar_Marker)
	REGFUNC(Add_Radar_Marker)
	REGFUNC(Get_Stealth_Active)
	REGFUNC(Set_Stealth_Active)
	REGFUNC(Get_Skeleton_Heigth)
	REGFUNC(Get_Skeleton_Width)
	REGFUNC(Set_Skeleton_Height)
	REGFUNC(Set_Skeleton_Width)
	REGFUNC(Is_Destroyed)
	REGFUNC(Get_Human_State_Flag)
	REGFUNC(Get_Human_State)
	REGFUNC(Get_Camera)
	REGFUNC(Get_Waypoint_Position)
	REGFUNC(Retrieve_Waypoints)
	REGFUNC(Retrieve_Waypaths)
	REGFUNC(Forward_Server)
	REGFUNC(Add_Shadow_Message)
	REGFUNC(Is_Sidebar_Enabled)
	REGFUNC(Is_Extras_Enabled)
	REGFUNC(Enable_Spawners_By_Name)
	REGFUNC(Create_HUD_Surface)
	REGFUNC(Find_HUD_Surface)
	REGFUNC(Show_HUD_Surface)
	REGFUNC(Hide_HUD_Surface)
	REGFUNC(Delete_HUD_Surface)

	REGFUNC(Change_Neutral_HUD_Color_Player)
	REGFUNC(Change_Friendly_HUD_Color_Player)
	REGFUNC(Change_Neutral_HUD_Color)
	REGFUNC(Change_Friendly_HUD_Color)
	REGFUNC(Change_Enemy_HUD_Color_Player)
	REGFUNC(Change_Enemy_HUD_Color)
	REGFUNC(Override_Sky_Colors)
	REGFUNC(Is_Overriding_Sky_Colors)
	REGFUNC(Set_Gloominess_Color)
	REGFUNC(Set_Sky_Color)
	REGFUNC(Set_Gloominess_Color)
	REGFUNC(Set_Starfield_Alpha)
	REGFUNC(Set_Moon_Halo_Color)
	REGFUNC(Set_Sun_Halo_Color)
	REGFUNC(Set_Sun_Color)
	REGFUNC(Set_Cold_Sky_Color)
	REGFUNC(Set_Warm_Sky_Color)
	REGFUNC(Get_Gloominess_Color)
	REGFUNC(Get_Sky_Color)
	REGFUNC(Get_Starfield_Alpha)
	REGFUNC(Get_Moon_Halo_Color)
	REGFUNC(Get_Warm_Sky_Color)
	REGFUNC(Get_Cold_Sky_Color)
	REGFUNC(Get_Sun_Color)
	REGFUNC(Get_Sun_Halo_Color)
	REGFUNC(Is_Connection_Bad)
	REGFUNC(Get_Time_Of_Day)
	REGFUNC(Set_Time_Of_Day)
	REGFUNC(Get_Screen_Resolution)
	REGFUNC(Enable_Global_Targeting)
	REGFUNC(Enable_Global_Targeting_Player)

	REGFUNC(Set_Time_Scale)
	REGFUNC(Get_Client_Hardware_Identifier)
	REGFUNC(Set_Object_Visibility_For_Player)
	REGFUNC(Set_Intermission_Time_Seconds)
	REGFUNC(Set_Game_Title)
	REGFUNC(Set_Password)
	REGFUNC(Set_Remix_Teams)
	REGFUNC(Set_Is_Team_Changing_Allowed)
	REGFUNC(Set_Radar_Mode)
	REGFUNC(Set_Map_Name)
	REGFUNC(Set_Motd)
	REGFUNC(Set_Time_Remaining_Seconds)
	REGFUNC(Reset_Time_Remaining_Seconds)
	REGFUNC(Set_Starting_Credits)
	REGFUNC(Set_Beacon_Placement_Ends_Game)
	REGFUNC(Set_Base_Destruction_Ends_Game)
	REGFUNC(Set_Object_Visibility)
	REGFUNC(The_Cnc_Game)
	REGFUNC(Get_Targeting_Pos)
	REGFUNC(Force_Client_Reload)
	REGFUNC(Reload_Level)
	REGFUNC(Get_Player_ID_By_Name)
	REGFUNC(Stop_Sound_Team)
	REGFUNC(Stop_Sound_Player)
	REGFUNC(Is_Pathfind_Generated)
	REGFUNC(Get_All_PowerUps)
	REGFUNC(Is_Frozen)
	REGFUNC(Set_Freeze)
	REGFUNC(Get_All_Soldiers)
	REGFUNC(Innate_Soldier_Enable_Footsteps_Heard)
	REGFUNC(Innate_Soldier_Enable_Gunshot_Heard)
	REGFUNC(Innate_Soldier_Enable_Bullet_Heard)
	REGFUNC(Create_Logical_Sound)
	REGFUNC(Start_Sound)
	REGFUNC(Get_Team_Color)
	REGFUNC(Set_Gameplay_Allowed)
	REGFUNC(Is_Gameplay_Allowed)
	REGFUNC(Is_In_Pathfind_Sector)
	REGFUNC(Take_Screenshot)
	REGFUNC(Set_Screenshot_URL)
	REGFUNC(Get_Screenshot_URL)
	REGFUNC(Set_Repository_URL)
	REGFUNC(Get_Repository_URL)
	REGFUNC(Print_Client_Console_Player)
	REGFUNC(Print_Client_Console)
	REGFUNC(AddRefillHook)
	REGFUNC(Get_Override_Weapon_Hold_Style)
	REGFUNC(Set_Override_Weapon_Hold_Style)
	REGFUNC(Set_Player_Name)
	REGFUNC(Get_Bounding_Box_Volume)
	REGFUNC(Get_Bounding_Box_Center)
	REGFUNC(Get_Bounding_Box_Extent)
	REGFUNC(Get_Bounding_Sphere_Volume)
	REGFUNC(Get_Bounding_Sphere_Center)
	REGFUNC(Set_GameObj)
	REGFUNC(Set_Control_Owner)
	REGFUNC(Get_Control_Owner)
	/* Dialog stuff */
	REGFUNC(Create_Menu_Dialog)
	REGFUNC(Create_Popup_Dialog)
	REGFUNC(Find_Dialog)
	REGFUNC(Hide_Dialog)
	REGFUNC(Show_Dialog)
	REGFUNC(Delete_Dialog)
	REGFUNC(Create_Yes_No_Dialog)
	REGFUNC(Create_Yes_No_Cancel_Dialog)
	REGFUNC(Create_User_Prompt)
	REGFUNC(Create_Centered_Popup)
	REGFUNC(Set_Control_Bounds)
	REGFUNC(Create_ProgressBar_Control)
	REGFUNC(Create_Slider_Control)
	REGFUNC(Create_ComboBox_Control)
	REGFUNC(Create_TextArea_Control)
	REGFUNC(Create_Title_Button_Control)
	REGFUNC(Create_Image_Button_Control)
	REGFUNC(Create_Label_Control)
	REGFUNC(Create_Image_Control)
	REGFUNC(Create_Bordered_Button_Control)
	REGFUNC(Create_CheckBox_Control)

	REGFUNC(Set_Can_Steal_Vehicles)
	REGFUNC(Can_Steal_Vehicles)
	REGFUNC(Can_Drive_Vehicles)
	REGFUNC(Set_Can_Play_Damage_Animations)
	REGFUNC(Can_Play_Damage_Animations)
	REGFUNC(Get_Scale_Across_Network)
	REGFUNC(Set_Scale_Across_Network)
	REGFUNC(Block_Action_Key)
	REGFUNC(Set_Block_Action_Key)
	REGFUNC(Get_Bot_Tag)
	REGFUNC(Set_Bot_Tag)
	REGFUNC(Is_On_Enemy_Pedestal)
	REGFUNC(Enable_Letterbox_Player)
	REGFUNC(Is_Friendly_Fire_Permitted)
	REGFUNC(Set_Friendly_Fire_Permitted)
	REGFUNC(Get_Bounding_Sphere_Radius)
	REGFUNC(Get_All_Static_Objects)
	REGFUNC(Is_Door)
	REGFUNC(Set_Team_Score)
	REGFUNC(Get_All_Spawners_Data)
	REGFUNC(Get_MCT_Position)
	REGFUNC(Get_MCT_Facing)
	REGFUNC(Are_Transitions_Enabled)
	REGFUNC(Get_Warhead_Type)
	REGFUNC(Set_Team_Visibility)
	REGFUNC(Clear_Object_Dirty_Bits)
	REGFUNC(Set_Is_Delete_Pending)
	REGFUNC(Is_Delete_Pending)
	REGFUNC(Update_Network_Object_Player)
	REGFUNC(Disarm_All_C4_Beacons)
	REGFUNC(Set_Delete_Pending)
	REGFUNC(Is_Enemy)
	REGFUNC(Get_Bullseye_Position)
	REGFUNC(Innate_Disable)
	REGFUNC(Is_Object_Visible)
	REGFUNC(Disable_All_Collisions)
	REGFUNC(Is_Performing_Pathfind_Action)
	REGFUNC(Get_Action_Params)
	REGFUNC(Modify_Action)
	REGFUNC(Action_Follow_Input)
	REGFUNC(Action_Dock)
	REGFUNC(Action_Face_Location)
	REGFUNC(Action_Enter_Exit)
	REGFUNC(Action_Play_Animation)
	REGFUNC(Action_Attack)
	REGFUNC(Action_Goto)
	REGFUNC(Get_A_Star)
	REGFUNC(ActionParamsStruct)
	REGFUNC(Is_Obj_Visible)
	REGFUNC(Get_Translated_Preset_Name_Ex)
	REGFUNC(Get_Preset_ID)
	REGFUNC(Trigger_Weapon)
	REGFUNC(Get_Action_ID)
	REGFUNC(Lock_Vehicle)
	REGFUNC(Give_Money)
	REGFUNC(Get_Bone_Position)
	REGFUNC(Set_Alt_Definition)
	REGFUNC(Set_Definition)
	REGFUNC(Action_Reset)

	// 'unknown' functions
	REGFUNC(Get_Spawn_Definition_ID_List)
	REGFUNC(Is_On_Ladder)
	REGFUNC(Find_Closest_Object_With_Script)
	REGFUNC(Get_All_Spawners)
	REGFUNC(Get_All_Spawners_currentObject)
	REGFUNC(Set_Lightning)
	REGFUNC(Get_Warhead_Name)
	REGFUNC(Set_MCTSkin)
	REGFUNC(Get_MCTSkin)
	REGFUNC(Get_Velocity)
	REGFUNC(Is_Valid_Model)
	REGFUNC(Get_Weapon_Bag_Count)
	REGFUNC(Get_Harvester_Vehicle)
	REGFUNC(Get_Harvester_Def_ID)
	REGFUNC(Get_Weapon_Definition_ID_By_Definition_ID)
	REGFUNC(Get_PT_Soldiers)
	REGFUNC(Get_PT_Vehicles)
	REGFUNC(Set_Is_Scripts_Visible)
	REGFUNC(IsStealthUnit)
	REGFUNC(Is_Valid_Skin)
	REGFUNC(Is_Stealth_Enabled)
	REGFUNC(Has_Turret)
	REGFUNC(Get_All_C4)
	REGFUNC(Shake_Camera_Player)
	REGFUNC(Get_Player_Data)
	REGFUNC(Lock_Soldier_Collision_Group)
	REGFUNC(Set_Allow_Stealth_While_Empty)
	REGFUNC(Get_All_Bone_Names)
	REGFUNC(Set_Can_Drive_Vehicles)
	REGFUNC(Stop_Sound)
	REGFUNC(Monitor_Sound)
	REGFUNC(Create_Sound)
	REGFUNC(Get_Defense_Object_Health)
	REGFUNC(Set_Gravity_Multiplier)
	REGFUNC(Get_Gravity_Multiplier)
	REGFUNC(Set_Player_Type)
	REGFUNC(Give_PowerUp)
	REGFUNC(Expire_Powerup)
	REGFUNC(Is_Immovable)
	REGFUNC(Set_Scripts_Can_Fire)
	REGFUNC(Set_Collision_Group)
	REGFUNC(Get_Collision_Group)
	REGFUNC(Set_Object_Dirty_Bit)
	REGFUNC(Set_Immovable)
	REGFUNC(Get_All_Stars)
	REGFUNC(Get_All_Beacons)
	REGFUNC(Create_2D_WAV_Sound)
	REGFUNC(Set_Building_Power)
	REGFUNC(Has_Key)
	REGFUNC(Get_Building_Power)
	REGFUNC(Send_Custom_Event)
	REGFUNC(Give_Points)
	REGFUNC(Enable_Spawner)
	REGFUNC(Find_Object_By_Preset)
	REGFUNC(Enable_Enemy_Seen)
	REGFUNC(Enable_Hibernation)
	REGFUNC(Innate_Enable)
	REGFUNC(Get_Translated_Name_ID)
	REGFUNC(Send_Custom_Event_To_Objects_With_Script)
	REGFUNC(Set_Animation_Frame)
	REGFUNC(Is_Engine_Enabled)
	REGFUNC(Create_2D_Wave_Sound_Dialog_Player)
	REGFUNC(Force_Position_Update_Player)
	REGFUNC(Set_Background_Music_Player_Offset)
	REGFUNC(Set_Definition_TranslationID_Player)
	REGFUNC(Force_Orientation_Update)
	REGFUNC(Force_Orientation_Update_Player)
	REGFUNC(Create_2D_Wave_Sound_Cinematic_Player)
	REGFUNC(Force_Turret_Update)
	REGFUNC(Force_Turret_Update_Player)
	REGFUNC(Force_Velocity_Update)
	REGFUNC(Force_Velocity_Update_Player)
	REGFUNC(Set_Camera_Host_Network)
	REGFUNC(Get_Fog_Range)
	REGFUNC(Get_GDI_Soldier_Name)
	REGFUNC(Get_Nod_Soldier_Name)
	REGFUNC(Set_Fog_Override_Player)
	REGFUNC(Clear_Fog_Override_Player)
	REGFUNC(Display_Game_Hint)
	REGFUNC(Display_Game_Hint_Image)
	REGFUNC(Set_Global_Stealth_Disable)
	REGFUNC(Get_Tech_Level)
	REGFUNC(Set_Tech_Level)
	REGFUNC(Create_Explosion_Extended)
	REGFUNC(Get_Clouds)
	REGFUNC(Get_Lightning)
	REGFUNC(Get_Wind)
	REGFUNC(Get_Rain)
	REGFUNC(Get_Snow)
	REGFUNC(Get_Ash)
	REGFUNC(Update_Game_Options)
	REGFUNC(GetTTVersion)
	REGFUNC(GetTTRevision)
	REGFUNC(Force_Position_Update)
	REGFUNC(Set_Fog_Mode_Player)
	REGFUNC(Set_Fog_Mode)
	REGFUNC(Send_HUD_Number)
	REGFUNC(Set_Fog_Density_Player)
	REGFUNC(Set_Fog_Density)
	REGFUNC(GetExplosionObj)
	REGFUNC(SetExplosionObj)
	REGFUNC(Get_Game_Definitions)
	REGFUNC(Get_Player_List)
	REGFUNC(Create_2D_WAV_Sound_Player)
	REGFUNC(Get_Map_Count)
	REGFUNC(Set_Fog_Enable_Player)
	REGFUNC(Set_Fog_Range_Player)
	REGFUNC(Set_Background_Music_Player)
	REGFUNC(Fade_Background_Music_Player)
	REGFUNC(Enable_HUD_Player)
	REGFUNC(Create_Sound_Player)
	REGFUNC(Create_3D_WAV_Sound_At_Bone_Player)
	REGFUNC(Create_3D_Sound_At_Bone_Player)
	REGFUNC(Set_Fog_Color_Player)
	REGFUNC(Send_Message_With_Team_Color)
	REGFUNC(Send_Message_With_Obj_Color)
	REGFUNC(Send_Message_Team)
	REGFUNC(Send_Translated_Message_Team)
	REGFUNC(Set_Occupants_Fade)
	REGFUNC(Enable_Team_Radar)
	REGFUNC(Create_Sound_Team)
	REGFUNC(Create_2D_Sound_Team)
	REGFUNC(Create_2D_WAV_Sound_Team)
	REGFUNC(Create_2D_WAV_Sound_Team_Dialog)
	REGFUNC(Create_2D_WAV_Sound_Team_Cinematic)
	REGFUNC(Create_3D_WAV_Sound_At_Bone_Team)
	REGFUNC(Create_3D_Sound_At_Bone_Team)
	REGFUNC(Kill_All_Buildings_By_Team)
	REGFUNC(Get_All_ScriptZones)
	REGFUNC(Find_Closest_Zone)
	REGFUNC(Disable_Innate)
	REGFUNC(Set_Ammo_IsTracking)
	REGFUNC(Set_Velocity)
	REGFUNC(Is_Stealthed)
	REGFUNC(Get_Script_Parameters)
	REGFUNC(Get_All_Scripts_Object)
	REGFUNC(Get_All_Observers_Object)
	REGFUNC(Enable_Engine)
	REGFUNC(Get_Ammo_Definition)
	REGFUNC(Action_AttackLocation)
	REGFUNC(Action_AttackMoveLocation)
	REGFUNC(Action_MoveLocation)
	REGFUNC(Action_FollowObject)
	REGFUNC(Smart_EnableBot)
	REGFUNC(Set_Camera_Player)
	REGFUNC(Set_Net_Update_Rate_Player)
	REGFUNC(Change_String_Player)
	REGFUNC(Get_String_ID_By_Desc)
	REGFUNC(Get_Closest_Poly)
	REGFUNC(Get_Closest_Fake_Building)
	REGFUNC(Get_Closest_Building)
	REGFUNC(Grant_Weapon)
	REGFUNC(Enable_HUD_Pokable_Indicator)
	REGFUNC(Set_Camera_Player)
	REGFUNC(Force_Vehicle_Entry)
	REGFUNC(Can_Teleport)
	REGFUNC(Set_Deaths)
	REGFUNC(Get_Animation_Name)
	REGFUNC(Set_HUD_Help_Text_Player_Text)
	REGFUNC(Set_HUD_Help_Text_Player)
	REGFUNC(Get_Vehicle_Lock_Owner)
	REGFUNC(Set_Is_Powerup_Persistant)
	REGFUNC(Set_Is_Rendered)
	REGFUNC(Clear_Weapons)
	REGFUNC(Set_Is_Visible)
	REGFUNC(Set_Map)
	REGFUNC(Get_Current_Map_Index)
	REGFUNC(Get_Client_Revision)
	REGFUNC(Set_Damage_Points)
	REGFUNC(Set_Death_Points)
	REGFUNC(Damage_Objects_Half)
	REGFUNC(Get_Death_Points)
	REGFUNC(Get_Damage_Points)
	REGFUNC(Kill_Occupants)
	REGFUNC(Damage_All_Objects_Area)
	REGFUNC(Damage_All_Vehicles_Area)
	REGFUNC(Damage_All_Buildings_By_Team)
	REGFUNC(Set_Info_Texture)
	REGFUNC(Clear_Info_Texture)
	REGFUNC(Set_Naval_Vehicle_Limit)
	REGFUNC(Get_Naval_Vehicle_Limit)
	REGFUNC(Send_Message_Player)
	REGFUNC(Load_New_HUD_INI)
	REGFUNC(Change_Radar_Map)
	REGFUNC(Set_Currently_Building)
	REGFUNC(Is_Currently_Building)
	REGFUNC(Set_Fog_Color)
	REGFUNC(Display_Security_Dialog)
	REGFUNC(Do_Objectives_Dlg)
	REGFUNC(Set_Player_Limit)
	REGFUNC(Get_Player_Limit)
	REGFUNC(Set_GDI_Soldier_Name)
	REGFUNC(Set_Nod_Soldier_Name)
	REGFUNC(Set_Moon_Is_Earth)
	REGFUNC(Get_Revision)
	REGFUNC(Can_Team_Build_Vehicle)
	REGFUNC(Find_Naval_Factory)
	REGFUNC(Vehicle_Preset_Is_Air)
	REGFUNC(Vehicle_Preset_Is_Naval)
	REGFUNC(Busy_Preset_By_Name)
	REGFUNC(Hide_Preset_By_Name)
	REGFUNC(Attach_Script_Occupants)
#ifdef LUATT_EXPORTS
	REGFUNC(Load_Lua)
	REGFUNC(Get_Lua_By_Name)
#endif
	REGFUNC(Set_Kills)
	REGFUNC(Get_All_Buildings)
	REGFUNC(Get_All_Vehicles)
	REGFUNC(Get_All_Players)
	REGFUNC(HideTexture)
	REGFUNC(ShowTexture)
	REGFUNC(OColision)
	REGFUNC(Is_WOL_User)
	REGFUNC(Hide_Object_Player)
	REGFUNC(Warp_Soldier)
	REGFUNC(Set_Max_Speed)
	REGFUNC(Get_Max_Speed)
	REGFUNC(Is_Vehicle_Owner)
	REGFUNC(Revive_Building)
	REGFUNC(Memory_Read)
	REGFUNC(Memory_Write)
	REGFUNC(Set_cPlayer)
	REGFUNC(Create_Building)
	REGFUNC(Restore_Building)
	REGFUNC(Set_Game_Title)
	REGFUNC(Damage_Occupants)
	REGFUNC(Add_TTDamageHook)
	REGFUNC(Set_Animation)
	REGFUNC(Enable_Radar_Player)
	REGFUNC(Get_Distance)
	REGFUNC(Get_File_List)
	REGFUNC(Reload_Lua)
	REGFUNC(Resolve_IP)
	REGFUNC(Create_2D_Sound)
	REGFUNC(Create_2D_Sound_Player)
	REGFUNC(Select_Weapon)
	REGFUNC(Send_Message)
	REGFUNC(Create_3D_WAV_Sound_At_Bone)
	REGFUNC(Change_Time_Limit)
	REGFUNC(Change_Time_Remaining)
	REGFUNC(Stop_Background_Music_Player)
	REGFUNC(Get_Build_Time_Multiplier)
	REGFUNC(Set_Screen_Fade_Color_Player)
	REGFUNC(Set_Screen_Fade_Opacity_Player)
	REGFUNC(Force_Camera_Look_Player)
	REGFUNC(Get_Vehicle_Limit)
	REGFUNC(Set_Vehicle_Limit)
	REGFUNC(Get_Air_Vehicle_Limit)
	REGFUNC(Set_Air_Vehicle_Limit)
	REGFUNC(Is_Crate)
	REGFUNC(Add_RadioHook)
	REGFUNC(Shake_Camera)
	REGFUNC(Play_Building_Announcement)
	REGFUNC(Set_War_Blitz)
	REGFUNC(Set_Fog_Range)
	REGFUNC(Set_Fog_Enable)
	REGFUNC(Create_Explosion_At_Bone)
	REGFUNC(Create_Explosion)
	REGFUNC(Get_Client_Serial_Hash)
	REGFUNC(Get_Weapon_Style)
	REGFUNC(Remove_Weapon)
	REGFUNC(Get_Preset_Name_By_Preset_ID)
	REGFUNC(Remove_All_Weapons)
	REGFUNC(Add_Console_Hook)
	REGFUNC(Set_Background_Music)
	REGFUNC(Get_Map_By_Number)
	REGFUNC(Spectate)
	REGFUNC(Get_Background_Music)
	REGFUNC(Get_C4_Count)
	REGFUNC(Get_Mine_Limit)
	REGFUNC(Kill_Player)
	REGFUNC(Control_Enable)
	REGFUNC(Get_BW_Player)
	REGFUNC(Get_Vehicles_Limit)
	REGFUNC(Get_Player_Version)
	REGFUNC(Kick_Player)
	REGFUNC(Set_Health)
	REGFUNC(Set_Shield_Strength)
	REGFUNC(Set_Shield_Type)
	REGFUNC(FDSMessage)
	REGFUNC(Get_All_Objects)
	REGFUNC(Enable_Stealth)
	REGFUNC(Get_Rotation)
	REGFUNC(Is_A_Star)
	REGFUNC(The_Game)
	REGFUNC(cPlayer)
	REGFUNC(LongToIP)
	REGFUNC(IPToLong)
	REGFUNC(GetTimeRemaining)
	REGFUNC(Get_Health)
	REGFUNC(Get_Max_Health)
	REGFUNC(Get_Shield_Strength)
	REGFUNC(Get_Max_Shield_Strength)
	REGFUNC(Display_GDI_Player_Terminal_Player)
	REGFUNC(Display_Nod_Player_Terminal_Player)
	REGFUNC(Console_Input)
	REGFUNC(Console_Output)
	REGFUNC(Get_Definition_Name)
	REGFUNC(Get_Definition_ID)
	REGFUNC(Get_Definition_Class_ID)
	REGFUNC(Is_Valid_Preset_ID)
	REGFUNC(Is_Valid_Preset)
	REGFUNC(Set_Max_Health)
	REGFUNC(Set_Max_Shield_Strength)
	REGFUNC(Get_Shield_Type)
	REGFUNC(Get_Skin)
	REGFUNC(Set_Skin)
	REGFUNC(Power_Base)
	REGFUNC(Set_Can_Generate_Soldiers)
	REGFUNC(Set_Can_Generate_Vehicles)
	REGFUNC(Destroy_Base)
	REGFUNC(Beacon_Destroyed_Base)
	REGFUNC(Enable_Base_Radar)
	REGFUNC(Is_Harvester)
	REGFUNC(Is_Radar_Enabled)
	REGFUNC(Building_Type)
	REGFUNC(Is_Building_Dead)
	REGFUNC(Find_Building)
	REGFUNC(Find_Base_Defense)
	REGFUNC(Is_Map_Flying)
	REGFUNC(Find_Harvester)
	REGFUNC(Is_Base_Powered)
	REGFUNC(Can_Generate_Vehicles)
	REGFUNC(Can_Generate_Soliders)
	REGFUNC(Get_Building_Count_Team)
	REGFUNC(Find_Building_By_Team)
	REGFUNC(Find_Building_By_Name)
	REGFUNC(Find_Power_Plant)
	REGFUNC(Find_Refinery)
	REGFUNC(Find_Repair_Bay)
	REGFUNC(Find_Soldier_Factory)
	REGFUNC(Find_Airstrip)
	REGFUNC(Find_War_Factory)
	REGFUNC(Find_Vehicle_Factory)
	REGFUNC(Find_Com_Center)
	REGFUNC(Is_Gameplay_Permitted)
	REGFUNC(Is_Dedicated)
	REGFUNC(Get_Current_Game_Mode)
	REGFUNC(Get_Harvester_Preset_ID)
	REGFUNC(Is_Harvester_Preset)
	REGFUNC(Get_IP_Address)
	REGFUNC(Get_IP_Port)
	REGFUNC(Get_Bandwidth)
	REGFUNC(Get_Ping)
	REGFUNC(Get_Kbits)
	REGFUNC(Get_Object_Type)
	REGFUNC(Set_Object_Type)
	REGFUNC(Is_Building)
	REGFUNC(Is_Soldier)
	REGFUNC(Is_Vehicle)
	REGFUNC(Is_Cinematic)
	REGFUNC(Is_ScriptZone)
	REGFUNC(Is_Powerup)
	REGFUNC(Is_C4)
	REGFUNC(Is_Beacon)
	REGFUNC(Is_Armed)
	REGFUNC(Is_Simple)
	REGFUNC(Is_PowerPlant)
	REGFUNC(Is_SoldierFactory)
	REGFUNC(Is_VehicleFactory)
	REGFUNC(Is_Airstrip)
	REGFUNC(Is_WarFactory)
	REGFUNC(Is_Refinery)
	REGFUNC(Is_ComCenter)
	REGFUNC(Is_RepairBay)
	REGFUNC(Is_Scriptable)
	REGFUNC(Get_Object_Count)
	REGFUNC(Find_Random_Preset)
	REGFUNC(Send_Custom_To_Team_Buildings)
	REGFUNC(Send_Custom_To_Team_Preset)
	REGFUNC(Send_Custom_All_Objects)
	REGFUNC(Send_Custom_Event_To_Object)
	REGFUNC(Get_Is_Powerup_Persistant)
	REGFUNC(Get_Powerup_Always_Allow_Grant)
	REGFUNC(Set_Powerup_Always_Allow_Grant)
	REGFUNC(Get_Powerup_Grant_Sound)
	REGFUNC(Grant_Powerup)
	REGFUNC(Get_Vehicle)
	REGFUNC(Grant_Refill)
	REGFUNC(Change_Character)
	REGFUNC(Create_Vehicle)
	REGFUNC(Toggle_Fly_Mode)
	REGFUNC(Get_Vehicle_Occupant_Count)
	REGFUNC(Get_Vehicle_Occupant)
	REGFUNC(Get_Vehicle_Driver)
	REGFUNC(Get_Vehicle_Gunner)
	REGFUNC(Force_Occupant_Exit)
	REGFUNC(Force_Occupants_Exit)
	REGFUNC(Get_Vehicle_Return)
	REGFUNC(Get_Fly_Mode)
	REGFUNC(Get_Vehicle_Seat_Count)
	REGFUNC(Soldier_Transition_Vehicle)
	REGFUNC(Get_Vehicle_Mode)
	REGFUNC(Get_Vehicle_Owner)
	REGFUNC(Force_Occupants_Exit_Team)
	REGFUNC(Get_Vehicle_Definition_Mode)
	REGFUNC(IsInsideZone)
	REGFUNC(Get_Vehicle_Definition_Mode_By_ID)
	REGFUNC(Get_Zone_Type)
	REGFUNC(Is_Available_For_Purchase)
	REGFUNC(Get_Vehicle_Gunner_Pos)
	REGFUNC(Set_Vehicle_Gunner)
	REGFUNC(Get_Model)
	REGFUNC(Get_Animation_Frame)
	REGFUNC(Is_TrackedVehicle)
	REGFUNC(Is_VTOLVehicle)
	REGFUNC(Is_WheeledVehicle)
	REGFUNC(Is_Motorcycle)

	REGFUNC(Is_Elevator)
	REGFUNC(Is_DamageableStaticPhys)
	REGFUNC(Is_AccessablePhys)
	REGFUNC(Is_DecorationPhys)
	REGFUNC(Is_HumanPhys)
	REGFUNC(Is_MotorVehicle)
	REGFUNC(Is_Phys3)
	REGFUNC(Is_RigidBody)
	REGFUNC(Is_StaticAnimPhys)
	REGFUNC(Is_StaticPhys)
	REGFUNC(Is_TimedDecorationPhys)
	REGFUNC(Is_VehiclePhys)
	REGFUNC(Is_DynamicAnimPhys)
	REGFUNC(Is_MoveablePhys)
	REGFUNC(Is_VehiclePhys)
	REGFUNC(Is_WheeledVehiclePhys)
	REGFUNC(Is_MotorcyclePhys)
	REGFUNC(Is_TrackedVehiclePhys)
	REGFUNC(Is_VTOLVehiclePhys)
	REGFUNC(Is_DamageableStaticPhys)
	REGFUNC(Is_DecorationPhys)
	REGFUNC(Is_DynamicAnimPhys)
	REGFUNC(Is_LightPhys)
	REGFUNC(Is_RenderObjPhys)
	REGFUNC(Is_ProjectilePhys)

	REGFUNC(Is_Projectile);
	REGFUNC(Copy_Transform)
	REGFUNC(Get_Mass)
	REGFUNC(Get_Htree_Name)
	REGFUNC(Get_Sex)
	REGFUNC(Create_Effect_All_Of_Preset)
	REGFUNC(Get_GameObj)
	REGFUNC(Get_Player_ID)
	REGFUNC(Get_Player_Name)
	REGFUNC(Get_Player_Name_By_ID)
	REGFUNC(Change_Team)
	REGFUNC(Change_Team_By_ID)
	REGFUNC(Get_Player_Count)
	REGFUNC(Get_Team_Player_Count)
	REGFUNC(Get_Team)
	REGFUNC(Get_Rank)
	REGFUNC(Get_Kills)
	REGFUNC(Get_Deaths)
	REGFUNC(Get_Score)
	REGFUNC(Get_Money)
	REGFUNC(Get_Kill_To_Death_Ratio)
	REGFUNC(Get_Part_Name)
	REGFUNC(Get_Part_Names)
	REGFUNC(Get_GameObj_By_Player_Name)
	REGFUNC(Purchase_Item)
	REGFUNC(Set_Ladder_Points)
	REGFUNC(Set_Rung)
	REGFUNC(Set_Money)
	REGFUNC(Set_Score)
	REGFUNC(Find_First_Player)
	REGFUNC(Change_Player_Team)
	REGFUNC(Tally_Team_Size)
	REGFUNC(Get_Team_Score)
	REGFUNC(Send_Custom_All_Players)
	REGFUNC(Steal_Team_Credits)
	REGFUNC(Get_Team_Credits)
	REGFUNC(Change_Team_2)
	REGFUNC(Get_Player_Type)
	REGFUNC(Get_Team_Cost)
	REGFUNC(Get_Cost)
	REGFUNC(Get_Team_Icon)
	REGFUNC(Get_Icon)
	REGFUNC(Remove_Script)
	REGFUNC(Remove_All_Scripts)
	REGFUNC(Attach_Script_Preset)
	REGFUNC(Attach_Script_Type)
	REGFUNC(Remove_Script_Preset)
	REGFUNC(Remove_Script_Type)
	REGFUNC(Is_Script_Attached)
	REGFUNC(Attach_Script_Once)
	REGFUNC(Attach_Script_Preset_Once)
	REGFUNC(Attach_Script_Type_Once)
	REGFUNC(Attach_Script_Building)
	REGFUNC(Attach_Script_Is_Preset)
	REGFUNC(Attach_Script_Is_Type)
	REGFUNC(Attach_Script_Player_Once)
	REGFUNC(Remove_Duplicate_Script)
	REGFUNC(Attach_Script_All_Buildings_Team)
	REGFUNC(Attach_Script_All_Turrets_Team)
	REGFUNC(Find_Building_With_Script)
	REGFUNC(Find_Object_With_Script)
	REGFUNC(Get_Translated_String)
	REGFUNC(Get_Translated_Preset_Name)
	REGFUNC(Get_Translated_Weapon)
	REGFUNC(Get_Current_Translated_Weapon)
	REGFUNC(Get_Team_Name)
	REGFUNC(Get_Vehicle_Name)
	REGFUNC(Get_Translated_Definition_Name)
	REGFUNC(Get_Current_Bullets)
	REGFUNC(Get_Current_Clip_Bullets)
	REGFUNC(Get_Current_Total_Bullets)
	REGFUNC(Get_Total_Bullets)
	REGFUNC(Get_Clip_Bullets)
	REGFUNC(Get_Bullets)
	REGFUNC(Get_Current_Max_Bullets)
	REGFUNC(Get_Current_Clip_Max_Bullets)
	REGFUNC(Get_Current_Total_Max_Bullets)
	REGFUNC(Get_Max_Total_Bullets)
	REGFUNC(Get_Max_Clip_Bullets)
	REGFUNC(Get_Max_Bullets)
	REGFUNC(Get_Position_Total_Bullets)
	REGFUNC(Get_Position_Bullets)
	REGFUNC(Get_Position_Clip_Bullets)
	REGFUNC(Get_Position_Total_Max_Bullets)
	REGFUNC(Get_Position_Max_Bullets)
	REGFUNC(Get_Position_Clip_Max_Bullets)
	REGFUNC(Set_Current_Bullets)
	REGFUNC(Set_Current_Clip_Bullets)
	REGFUNC(Set_Position_Bullets)
	REGFUNC(Set_Position_Clip_Bullets)
	REGFUNC(Set_Bullets)
	REGFUNC(Set_Clip_Bullets)
	REGFUNC(Get_Powerup_Weapon)
	REGFUNC(Get_Powerup_Weapon_By_Obj)
	REGFUNC(Get_Current_Weapon_Style)
	REGFUNC(Disarm_Beacon)
	REGFUNC(Disarm_Beacons)
	REGFUNC(Disarm_C4)
	REGFUNC(Get_Current_Weapon)
	REGFUNC(Get_Weapon_Count)
	REGFUNC(Get_Weapon)
	REGFUNC(Has_Weapon)
	REGFUNC(Find_Beacon)
	REGFUNC(Get_C4_Count)
	REGFUNC(Get_Beacon_Count)
	REGFUNC(Get_C4_Mode)
	REGFUNC(Get_C4_Planter)
	REGFUNC(Get_C4_Attached)
	REGFUNC(Get_Beacon_Planter)
	REGFUNC(Create_Object)
	REGFUNC(Destroy_Object)
	REGFUNC(Get_Preset_Name)
	REGFUNC(Get_Position)
	REGFUNC(Enable_Vehicle_Transitions)
	REGFUNC(Set_Model)
	REGFUNC(Goto_Location)
	REGFUNC(Goto_Object)
	REGFUNC(Disable_Physical_Collisions)
	REGFUNC(Enable_Collisions)
	REGFUNC(Random_Building)
	REGFUNC(Get_Current_Map)
	REGFUNC(Get_Next_Map)
	REGFUNC(Set_Position)
	REGFUNC(Apply_Damage)
	REGFUNC(Get_Facing)
	REGFUNC(Set_Facing)
	REGFUNC(Set_Clouds)
	REGFUNC(Set_Ash)
	REGFUNC(Set_Rain)
	REGFUNC(Set_Snow)
	REGFUNC(Set_Wind)
	REGFUNC(Attach_Script)
	REGFUNC(Create_Object_At_Bone)
	REGFUNC(Attach_To_Object_Bone)
	REGFUNC(Display_Health_Bar)
	REGFUNC(Create_Script_Zone)
	REGFUNC(tClock)
	REGFUNC(Invoke)
	REGFUNC(Get_ID)

	LuaVector3::Register(L);
	LuaBox::Register(L);
	LuaActionParams::Register(L);
	LuaMatrix3::Register(L);
	LuaScriptManager::RegisterScriptFunctions(L);
	LuaScriptedControlClass::Register(L);
	LuaScriptedDialogClass::Register(L);
	LuaThread::Register_Lua_Thread(L);
	LuaHUDElementClass::Register(L);
	LuaHUDSurfaceClass::Register(L);
}
#endif
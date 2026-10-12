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
*  \param message The string content to send to the console.
*/
int Lua_Console_Input(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Console_Input(lua_tostring(L, 1));
	return 1;
}

/*!\details Writes text to the FDS Console. This function acts like a formatted print statement.
*  \par Lua Command
*  \code{.lua}
*			Console_Output(message)
*  \endcode
*  \par		Example
*  \code{.lua}
*			Console_Output("Hello World\n")
*  \endcode
*  \param message The string content to print to the console.
*/
int Lua_Console_Output(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Console_Output(lua_tostring(L, 1));
	return 1;
}

/*!\details Retrieves the string name corresponding to a Definition ID.
*  \par Lua Command
*  \code{.lua}
*			name = Get_Definition_Name(id)
*  \endcode
*  \par		Example
*  \code{.lua}
*			name = Get_Definition_Name(1000) -- returns: none
*  \endcode
*  \param def_id The numeric ID of the definition to query.
*  \return name The translated string name associated with the definition.
*/
int Lua_Get_Definition_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Definition_Name(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Retrieves the Definition ID associated with a given definition name.
*  \par Lua Command
*  \code{.lua}
*			id = Get_Definition_ID(name)
*  \endcode
*  \par		Example
*  \code{.lua}
*			id = Get_Definition_ID("Weapon_StealthTank_Player") -- returns: 409610054
*  \endcode
*  \param def_name The string name of the definition.
*  \return def_id The numeric ID associated with the name.
*/
int Lua_Get_Definition_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Definition_ID(lua_tostring(L, 1)));
	return 1;
}

/*!\details Retrieves the specific Definition Class ID based on a definition name.
*  \par Lua Command
*  \code{.lua}
*			id = Get_Definition_Class_ID(name)
*  \endcode
*  \par		Example
*  \code{.lua}
*			id = Get_Definition_Class_ID("Weapon_StealthTank_Player") -- returns: 45057 (The specific class ID)
*  \endcode
*  \param def_name The string name of the definition.
*  \return def_id The numeric class ID associated with the definition.
*/
int Lua_Get_Definition_Class_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Definition_Class_ID(lua_tostring(L, 1)));
	return 1;
}

/*!\details Checks if a specific preset ID is considered valid within the game system.
*  \par Lua Command
*  \code{.lua}
*			valid = Is_Valid_Preset_ID(preset_id)
*  \endcode
*  \par		Example
*  \code{.lua}
*			valid = Is_Valid_Preset_ID(1) -- returns: false
*  \endcode
*  \param preset_id The numeric ID of the preset to validate.
*  \return valid Boolean (`true` if the ID is active and valid, `false` otherwise).
*/
int Lua_Is_Valid_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Valid_Preset_ID(lua_tointeger(L, 1)));
	return 1;
}

/*!\details Checks if a preset name is valid and recognized by the system.
*  \par Lua Command
*  \code{.lua}
*			valid = Is_Valid_Preset(preset_name)
*  \endcode
*  \par		Example
*  \code{.lua}
*			valid = Is_Valid_Preset("Weapon_StealthTank_Player")  -- returns: false
*  \endcode
*  \param preset_name The string name of the preset to validate.
*  \return valid Boolean (`true` if the name corresponds to an existing definition, `false` otherwise).
*/
int Lua_Is_Valid_Preset(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Valid_Preset(lua_tostring(L, 1)));
	return 1;
}

/*!\details Sets the maximum health of a specified game object.
*  \par Lua Command
*  \code{.lua}
*			Set_Max_Health(Object, Amount)
*  \endcode
*  \par		Example
*  \code{.lua}
*			Set_Max_Health(Get_GameObj(1), 9000)
*  \endcode
*  \param Object The ID of the game object whose max health is to be changed.
*  \param Amount The new maximum health value (float).
*/
int Lua_Set_Max_Health(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Max_Health(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 1;
}

/*!\details Sets the maximum shield strength of a specified game object.
*  \par Lua Command
*  \code{.lua}
*			Set_Max_Shield_Strength(Object, Amount)
*  \endcode
*  \par		Example
*  \code{.lua}
*			Set_Max_Shield_Strength(Get_GameObj(1), 9000)
*  \endcode
*  \param Object The ID of the game object whose max shield strength is to be changed.
*  \param Amount The new maximum shield value (float).
*/
int Lua_Set_Max_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Max_Shield_Strength(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 1;
}

/*!\details Retrieves the type of shield attached to a game object.
 *  \par Lua Command
 *  \code{.lua}
 *			ShieldType = Get_Shield_Type(Object)
 *  \endcode
 *  \par	Example
 *  \code{.lua}
 *			ShieldType = Get_Shield_Type(Get_GameObj(1))
*  \endcode
*  \param Object The ID of the object to inspect.
*  \return shield_type The name string of the shield type, or empty string if none exists.
 */
int Lua_Get_Shield_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Shield_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Retrieves the skin name of a game object.
 *  \par Lua Command
 *  \code{.lua}
 *			SkinName = Get_Skin(Object)
 *  \endcode
 *  \par	Example
 *  \code{.lua}
 *			SkinName = Get_Skin(Get_GameObj(1))
*  \endcode
*  \param Object The ID of the object to inspect.
*  \return skin_name The name string of the current skin, or empty string if none exists.
 */
int Lua_Get_Skin(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Skin(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Changes the skin of a specified game object.
 *  \par Lua Command
 *  \code{.lua}
 *			Set_Skin(Object, Skin)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			Set_Skin(Get_GameObj(1), "PlayerSkin")
*  \endcode
*  \param Object The ID of the object whose skin is to be changed.
*  \param Skin The desired new skin name string.
*/
int Lua_Set_Skin(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	if (!obj) return 0;
	Set_Skin(obj, luaL_checkstring(L, 2));
	return 1;
}

/*!\details Powers or unpowers a base belonging to a specified team (using the object's implicit team).
 *  \par Lua Command
 *  \code{.lua}
 *			Power_Base(Object, Enable)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			Power_Base(Get_GameObj(1), true)   -- power on
*			Power_Base(Get_GameObj(1), false)  -- power off
*  \endcode
*  \param Object The ID of the building/base object.
*  \param Enable Boolean (`true` to power on, `false` to power off).
*/
int Lua_Power_Base(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Power_Base(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Sets whether a specified base can generate soldiers for that team.
 *  \par Lua Command
 *  \code{.lua}
 *		  Set_Can_Generate_Soldiers(TeamID, true)
 *  \endcode
*  \param TeamID Number   - The ID of the team whose generation limit is being modified.
*  \param allow Boolean  - `true` to enable soldier generation; `false` to disable it.
*/
int Lua_Set_Can_Generate_Soldiers(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Can_Generate_Soldiers(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Sets whether a specified base can generate vehicles for that team.
 *  \par Lua Command
 *  \code{.lua}
 *		  Set_Can_Generate_Vehicles(TeamID, false)
 *  \endcode
*  \param TeamID Number   - The ID of the team whose generation limit is being modified.
*  \param allow Boolean  - `true` to enable vehicle generation; `false` to disable it.
*/
int Lua_Set_Can_Generate_Vehicles(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Can_Generate_Vehicles(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Forces the destruction of a base using its ID.
 *  \par Lua Command
 *  \code{.lua}
 *		  Destroy_Base(BaseID)
 *  \endcode
*  \param BaseID The ID of the base to destroy.
*/
int Lua_Destroy_Base(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Destroy_Base(lua_tonumber(L, 1));
	return 1;
}

/*!\details Marks a base as destroyed due to beacon activity. This is used when an enemy beacon kills the base.
 *  \par Lua Command
 *  \code{.lua}
 *		  Beacon_Destroyed_Base(BaseID, true)
 *  \endcode
*  \param BaseID The ID of the affected base.
*  \param destroyed Boolean (`true` if the destruction occurred; `false` otherwise).
*/
int Lua_Beacon_Destroyed_Base(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Beacon_Destroyed_Base(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Enables or disables the radar function for a given base.
 *  \par Lua Command
 *  \code{.lua}
 *		  Enable_Base_Radar(BaseID, false)
 *  \endcode
*  \param BaseID The ID of the base to modify.
*  \param enable Boolean (`true` to activate radar; `false` to deactivate it).
*/
int Lua_Enable_Base_Radar(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Enable_Base_Radar(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Checks if the radar is currently enabled for a given base.
 *  \par Lua Command
 *  \code{.lua}
 *		  enabled = Is_Radar_Enabled(BaseID)
 *  \endcode
*  \param BaseID The ID of the base to query.
*  \return Boolean (`true` if radar is active; `false` otherwise).
*/
int Lua_Is_Radar_Enabled(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Radar_Enabled(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Retrieves the building type ID of a specific object.
 *  \par Lua Command
 *  \code{.lua}
 *		  type = Building_Type(ObjID)
*  \endcode
*  \param Object The ID of the game object to query.
*  \return Number - The numeric building type code (e.g., 1 for buildings, 2 for vehicles).
*/
int Lua_Building_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Building_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds the first building of a given type that belongs to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Building(TypeID, Team)
 *  \endcode
*  \param TypeID The desired building type ID.
*  \param Team The numeric team ID (0=Nod, 1=GDI, 2=Neutral).
*  \return Object ID The object ID of the found building, or `0` if none exists for that combination.
*/
int Lua_Find_Building(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_By_Type(lua_tonumber(L, 1), lua_tonumber(L, 2))));
	return 1;
}

/*!\details Finds a base-defence building by using the current team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Base_Defense(TeamId)
 *  \endcode
*  \param Number The ID of the Team.
*  \return Number - Object ID of the found base-defence, or `0` if none exists.
*/
int Lua_Find_Base_Defense(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Base_Defense(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Checks whether the current game map is in flight mode (i.e., a flying time map).
 *  \par Lua Command
 *  \code{.lua}
 *		  flying = Is_Map_Flying()
*  \endcode
*  \return Boolean - `true` if the map geometry supports flight; otherwise `false`.
*/
int Lua_Is_Map_Flying(lua_State *L)
{
	lua_pushboolean(L, Is_Map_Flying());
	return 1;
}

/*!\details Finds a harvester belonging to the specified team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Harvester(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose harvester you want to locate.
*  \return Number - Object ID of the harvester, or `0` if none exists for that team.
*/
int Lua_Find_Harvester(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Harvester(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Checks if a base is currently powered on.
 *  \par Lua Command
 *  \code{.lua}
 *		  powered = Is_Base_Powered(BaseID)
*  \endcode
*  \param BaseID The ID of the base to check.
*  \return Boolean - `true` if power is active; otherwise `false`.
*/
int Lua_Is_Base_Powered(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Is_Base_Powered(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Checks if a base can generate vehicles for its team.
 *  \par Lua Command
 *  \code{.lua}
 *		  canGen = Can_Generate_Vehicles(BaseID)
*  \endcode
*  \param BaseID The ID of the base to query.
*  \return Boolean - `true` if vehicle generation is permitted; otherwise `false`.
*/
int Lua_Can_Generate_Vehicles(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Can_Generate_Vehicles(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Checks if a base can generate soldiers for its team.
 *  \par Lua Command
 *  \code{.lua}
 *		  canGen = Can_Generate_Soldiers(BaseID)
*  \endcode
*  \param BaseID The ID of the base to query.
*  \return Boolean - `true` if soldier generation is permitted; otherwise `false`.
*/
int Lua_Can_Generate_Soliders(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Can_Generate_Soldiers(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the total count of buildings owned by a specified team.
 *  \par Lua Command
 *  \code{.lua}
 *		  count = Get_Building_Count_Team(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team to count for (0-based).
*  \return Number - Count of buildings owned by the specified team.
*/
int Lua_Get_Building_Count_Team(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Building_Count_Team(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Finds the first building belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Building_By_Team(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose building should be found.
*  \return Number - Object ID of the found building, or `0` if none exists for that team.
*/
int Lua_Find_Building_By_Team(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_By_Team(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds a specific building by matching both its preset name and the team that owns it.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Building_By_Name("Weapon_StealthTank_Player", "SomePreset")
 *  \endcode
*  \param TypeID The building type ID to search for (e.g., generic buildings).
*  \param PresetName The preset name required for the match.
*  \return Number - Object ID of the found building, or `0` if none exists.
*/
int Lua_Find_Building_By_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_By_Preset(lua_tonumber(L, 1), lua_tostring(L, 2))));
	return 1;
}

/*!\details Finds the power-plant building belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Power_Plant(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose plant you want to locate.
*  \return Number - Object ID of the power-plant, or `0` if none exists.
*/
int Lua_Find_Power_Plant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Power_Plant(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds the refinery building belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Refinery(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose refinery you want to locate.
*  \return Number - Object ID of the refinery, or `0` if none exists.
*/
int Lua_Find_Refinery(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Refinery(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds the repair bay belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Repair_Bay(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose repair bay you want to locate.
*  \return Number - Object ID of the repair bay, or `0` if none exists.
*/
int Lua_Find_Repair_Bay(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Repair_Bay(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds the soldier factory building belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Soldier_Factory(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose factory you want to locate.
*  \return Number - Object ID of the soldier factory, or `0` if none exists.
*/
int Lua_Find_Soldier_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Soldier_Factory(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds an airstrip (vehicle factory) belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Airstrip(TeamID)
 *  \endcode
 *  \param TeamID Number - The team whose airstrip you want to locate.
 *  \return Number - Object ID of the airstrip or `nil`.
 */
int Lua_Find_Airstrip(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Vehicle_Factory(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds a war factory (vehicle factory) belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_War_Factory(TeamID)
 *  \endcode
 *  \param TeamID Number - The team whose war factory you want to locate.
 *  \return Number - Object ID of the factory or `nil`.
 */
int Lua_Find_War_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Vehicle_Factory(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds a vehicle factory belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Vehicle_Factory(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose factory you want to locate.
*  \return Number - Object ID of the vehicle factory, or `0` if none exists.
*/
int Lua_Find_Vehicle_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Vehicle_Factory(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Finds the command centre building belonging to a specific team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Com_Center(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose center you want to locate.
*  \return Number - Object ID of the command centre, or `0` if none exists.
*/
int Lua_Find_Com_Center(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Com_Center(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Checks whether general gameplay is currently permitted across the map.
 *  \par Lua Command
 *  \code{.lua}
 *		  allowed = Is_Gameplay_Permitted()
*  \endcode
*  \return Boolean - `true` if game activity is allowed; otherwise `false`.
*/
int Lua_Is_Gameplay_Permitted(lua_State *L)
{
	lua_pushboolean(L, Is_Gameplay_Permitted());
	return 1;
}

/*!\details Checks whether the current server environment is dedicated.
 *  \par Lua Command
 *  \code{.lua}
 *		  isDedicated = Is_Dedicated()
 *  \endcode
 *  \return Boolean - `true` if this is a dedicated server; otherwise `false`.
 */
int Lua_Is_Dedicated(lua_State *L)
{
	lua_pushboolean(L, Is_Dedicated());
	return 1;
}

/*!\details Retrieves the numeric ID of the current game mode.
 *  \par Lua Command
 *  \code{.lua}
 *		  mode = Get_Current_Game_Mode()
*  \endcode
*  \return Number - The integer ID representing the active game mode (e.g., single player, skirmish).
*/
int Lua_Get_Current_Game_Mode(lua_State *L)
{
	lua_pushnumber(L, Get_Current_Game_Mode());
	return 1;
}

/*!\details Retrieves the preset ID assigned to a harvester belonging to a team.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Get_Harvester_Preset_ID(TeamID)
*  \endcode
*  \param TeamID The numeric ID of the team whose harvester you are querying.
*  \return Number - The preset ID used by the AI harvester, or `0` if no harvester is set for that team.
*/
int Lua_Get_Harvester_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Harvester_Preset_ID(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Retrieves the IP address string for a given client slot.
 *  \par Lua Command
 *       ip = Get_IP_Address(1)
 *  \code{.lua}
*  \endcode
*  \param ClientIndex The index of the client (typically 0-based).
*  \return String - The associated IP address, or `nil` if unavailable.
*/
int Lua_Get_IP_Address(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *IP = Get_IP_Address(lua_tonumber(L, 1));
	if (!IP) return 0;
	lua_pushstring(L, IP);
	return 1;
}

/*!\details Retrieves the port number string for a given client slot.
 *  \par Lua Command
 *  \code{.lua}
 *		  port = Get_IP_Port(1)
 *  \endcode
 *  \param ClientIndex Number - Index of the client (typically 0-based).
 *  \return String - Port number or `nil` if unavailable.
 */
int Lua_Get_IP_Port(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *IP = Get_IP_Port(lua_tonumber(L, 1));
	if (!IP) return 0;
	lua_pushstring(L, IP);
	return 1;
}

/*!\details Returns the current bandwidth for a client slot.
 *  \par Lua Command
 *  \code{.lua}
 *		  bw = Get_Bandwidth(1)
 *  \endcode
 *  \param ClientIndex Number - Index of the client.
 *  \return Number - Bandwidth value or `0` if not available.
 */
int Lua_Get_Bandwidth(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Bandwidth(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the ping time in milliseconds for a client slot.
 *  \par Lua Command
 *  \code{.lua}
 *		  ping = Get_Ping(1)
 *  \endcode
 *  \param ClientIndex Number - Index of the client.
 *  \return Number - Ping value or `0` if not available.
 */
int Lua_Get_Ping(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Ping(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the kilobits per second for a client slot.
 *  \par Lua Command
 *  \code{.lua}
 *		  kbits = Get_Kbits(1)
 *  \endcode
 *  \param ClientIndex Number - Index of the client.
 *  \return Number - Kbit rate or `0` if not available.
 */
int Lua_Get_Kbits(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Kbits(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Retrieves the object type of a game object instance.
 *  \par Lua Command
 *  \code{.lua}
 *		  type = Get_Object_Type(ObjID)
*  \endcode
*  \param ObjID The ID of the object to inspect.
*  \return Number - The object type code (e.g., 0 for Nod, 1 for GDI).
*/
int Lua_Get_Object_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Get_Object_Type(obj));
	return 1;
}

/*!\details Sets the object type of a game object.
 *  \par Lua Command
 *  \code{.lua}
 *		  Set_Object_Type(ObjID, NewType)
 *  \endcode
*  \param ObjID The ID of the object to modify.
*  \param NewType The desired new team/object type code.
*/
int Lua_Set_Object_Type(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Set_Object_Type(obj, lua_tonumber(L, 2));
	return 1;
}

/*!\details Counts the number of objects matching a specific type, with an optional preset name filter.
 *  \par Lua Command
 *  \code{.lua}
 *		  count = Get_Object_Count(TypeID, "SomePreset")
 *  \endcode
*  \param TypeID The object type ID to count.
*  \param PresetName Optional string; if provided, only matches this preset. If `nil`, counts all objects of the specified type.
*  \return Number - Total count of matching objects.
*/
int Lua_Get_Object_Count(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Object_Count(lua_tonumber(L, 1), lua_tostring(L, 2)));
	return 1;
}

/*!\details Finds a random object that satisfies given constraints.
 *  \par Lua Command
 *  \code{.lua}
 *		  id = Find_Random_Preset("Vehicle", 3, 7)
 *  \endcode
*  \param PresetPattern Pattern used to match the preset names of the desired objects (e.g., "Vehicle").
*  \param MinLevel Minimum level or tier of the object allowed.
*  \param MaxLevel Maximum level or tier allowed.
*  \return Number - Object ID of a matching random object, or `0` if none found.
*/
int Lua_Find_Random_Preset(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	lua_pushnumber(L, Get_ID(Find_Random_Preset(lua_tostring(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3))));
	return 1;
}

/*!\details Sends a custom message event to all buildings of a specified team.
 *  \par Lua Command
 *  \code{.lua}
 *		  Send_Custom_To_Team_Buildings(TeamID, Get_GameObj(5), CustomType, TargetID, Data)
 *  \endcode
 *  \param TeamID Number - The target team (0-based).
*  \param SenderObj GameObject - Object sending the custom event.
*  \param CustomType Number - Type identifier for the custom message payload.
*  \param TargetID Number - Optional specific target ID; use `0` if no single target is defined.
*  \param Data Number - Payload data attached to the event.
*/
int Lua_Send_Custom_To_Team_Buildings(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Custom_To_Team_Buildings(lua_tonumber(L, 1), Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}

/*!\details Sends a custom message event to all buildings of a team that match a given preset name.
 *  \par Lua Command
 *  \code{.lua}
 *		  Send_Custom_To_Team_Preset(TeamID, "Vehicle", Get_GameObj(5), CustomType, TargetID, Data)
 *  \endcode
 *  \param TeamID The target team (0-based).
*  \param PresetName Filter string; only matching building presets will be targeted.
*  \param SenderObj Object sending the custom event.
*  \param CustomType Number - Type identifier for the custom message payload.
*  \param TargetID Number - Optional target ID; use `0` if no single target is defined.
*  \param Data Number - Payload data attached to the event.
*/
int Lua_Send_Custom_To_Team_Preset(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	Send_Custom_To_Team_Preset(lua_tonumber(L, 1), lua_tostring(L, 2), Commands->Find_Object(lua_tonumber(L, 3)), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));
	return 1;
}

/*!\details Sends a custom message event to all objects across the map.
 *  \par Lua Command
 *  \code{.lua}
 *		  Send_Custom_All_Objects(CustomType, Get_GameObj(5), Data)
 *  \endcode
 *  \param CustomType Number - Type identifier for the custom event payload.
*  \param SenderObj Object sending the message.
*  \param Data Number - Payload data attached to the event.
*/
int Lua_Send_Custom_All_Objects(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Send_Custom_All_Objects(lua_tonumber(L, 1), Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3));
	return 1;
}

/*!\details Sends a custom event to a specific target object.
 *  \par Lua Command
 *  \code{.lua}
 *		  Send_Custom_Event_To_Object(TargetObj, "EVENT", CustomType, TargetID, Data)
 *  \endcode
 *  \param TargetObj The destination of the custom event.
*  \param EventName String - Name of the event to trigger on the target.
*  \param CustomType Number - Type identifier for the custom data payload.
*  \param TargetID Number - Optional specific target ID; use `0` if unused.
*  \param Data Number - Payload data attached to the event.
*/
int Lua_Send_Custom_Event_To_Object(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Custom_Event_To_Object(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}

/*!\details Checks if a power-up on an object is persistent (doesn't de-spawn on collect).
 *  \par Lua Command
 *  \code{.lua}
 *		  pers = Get_Is_Powerup_Persistant(15332)
 *  \endcode
 *  \param Obj The ID of the object containing the power-up.
*  \return Boolean - `true` if the power-up will persist; otherwise `false`.
*/
int Lua_Get_Is_Powerup_Persistant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Get_Is_Powerup_Persistant(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Checks if a power-up on an object is always allowed to grant regardless of game rules.
 *  \par Lua Command
 *  \code{.lua}
 *		  allow = Get_Powerup_Always_Allow_Grant(Get_GameObj(10))
 *  \endcode
 *  \param Obj GameObject - Object whose power-up policy to query.
 *  \return Boolean - `true` if the power-up will always grant; otherwise `false`.
 */
int Lua_Get_Powerup_Always_Allow_Grant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Get_Powerup_Always_Allow_Grant(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Sets whether a power-up on an object is configured to always allow granting.
 *  \par Lua Command
 *  \code{.lua}
 *		  Set_Powerup_Always_Allow_Grant(15332, true)
 *  \endcode
 *  \param Obj GameObject - Object whose power-up policy to modify.
 *  \param allow Boolean - `true` to force grant; `false` to revert to default.
 */
int Lua_Set_Powerup_Always_Allow_Grant(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Powerup_Always_Allow_Grant(Commands->Find_Object(lua_tonumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Returns the sound ID played when a power-up is granted on an object.
 *  \par Lua Command
 *  \code{.lua}
 *		  snd = Get_Powerup_Grant_Sound(Get_GameObj(10))
 *  \endcode
 *  \param Obj GameObject - Object whose grant-sound to query.
 *  \return Number - Sound ID or `0` if none is set.
 */
int Lua_Get_Powerup_Grant_Sound(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Powerup_Grant_Sound(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Grants a named power-up to an object.
 *  \par Lua Command
 *  \code{.lua}
 *		  Grant_Powerup(Get_GameObj(10), "POW_RepairGun_Player")
 *  \endcode
 *  \param Obj GameObject - Target of the grant.
 *  \param PowerupName String - Identifier of the power-up to apply.
 */
int Lua_Grant_Powerup(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Grant_Powerup(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2));
	return 1;
}

/*!\details Returns the ID of a vehicle that the object is inside (player inside).
 *  \par Lua Command
 *  \code{.lua}
 *		  vid = Get_Vehicle(Get_GameObj(1))
 *  \endcode
 *  \param Obj GameObject - Object whose vehicle to query.
 *  \return Number - Vehicle object ID or `0` if none.
 */
int Lua_Get_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Grants a refill to a object/player object
 *  \par Lua Command
 *  \code{.lua}
 *		  Grant_Refill(Get_GameObj(1))
 *  \endcode
 *  \param Obj GameObject - Object whose vehicle will be refilled.
 */
int Lua_Grant_Refill(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Grant_Refill(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

/*!\details Forces a character change for a game object.
 *  \par Lua Command
 *  \code{.lua}
 *		  changed = Change_Character(Get_GameObj(10), "CnC_Nod_Minigunner_0")
 *  \endcode
 *  \param Obj GameObject - Object whose character will be switched.
 *  \param NewPreset String - Name of the new preset to apply.
 *  \return Boolean - `true` if the change succeeded; otherwise `false`.
 */
int Lua_Change_Character(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushboolean(L, Change_Character(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2)));
	return 1;
}

/*!\details Creates a new vehicle instance.
 *  \par Lua Command
 *  \code{.lua}
 *      Create_Vehicle("CnC_Nod_Apache", 5, Get_GameObj(1), Get_Team(1))
 *  \endcode
 *  \param PresetName String - The name of the vehicle preset to create.
 *  \param Delay Number - Time in seconds before the vehicle is spawned (use 0 for instant spawn).
 *  \param Owner GameObject - Object that will own the newly created vehicle.  Pass nil if the vehicle has no owner.
 *  \param Team Number - ID of the team on which the vehicle should be constructed (0 = Nod, 1 = GDI, 2 = Neutral).  
 *      The preset is automatically matched to the correct factory for this team;  passing an invalid
 *      team results in a failure but no crash.
 */
int Lua_Create_Vehicle(lua_State *L)
{
    if (lua_gettop(L) < 4) return 0;
    Create_Vehicle(lua_tostring(L, 1), lua_tonumber(L, 2),
                   Commands->Find_Object(lua_tonumber(L, 3)),
                   lua_tonumber(L, 4));
    return 1;
}

/*!\details Toggles fly mode on the specified object.
 *  \par Lua Command
 *  \code{.lua}
 *      Toggle_Fly_Mode(Get_GameObj(1))
 *  \endcode
 *  \param Obj Number - The ID of the vehicle or infantry whose flying state should be toggled.  
 *      For vehicles this will switch between ground and VTOL mode; for infantry it will enable/disable
 *      airborne movement.
 */
int Lua_Toggle_Fly_Mode(lua_State *L)
{
    if (lua_gettop(L) < 1) return 0;
    Toggle_Fly_Mode(Commands->Find_Object(lua_tonumber(L, 1)));
    return 1;
}

/*!\details Returns the number of occupants currently in a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *      count = Get_Vehicle_Occupant_Count(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle to query.
 *  \return Number - Count of occupants (including driver).  Zero if the object is not a vehicle or has no occupants.
 */
int Lua_Get_Vehicle_Occupant_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Occupant_Count(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the ID of an occupant in a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *		  occID = Get_Vehicle_Occupant(51234, 0)   -- first seat
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \param SeatIndex Number - Zero-based index of the seat to query.
 *  \return Number - Object ID of the occupant or `0` if none.
 */
int Lua_Get_Vehicle_Occupant(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Occupant(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2))));
	return 1;
}

/*!\details Returns the ID of the driver in a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *		  driverID = Get_Vehicle_Driver(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Number - Object ID of the driver or `0` if no driver.
 */
int Lua_Get_Vehicle_Driver(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Driver(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Returns the ID of the gunner in a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *		  gunnerID = Get_Vehicle_Gunner(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Number - Object ID of the gunner or `0` if no gunner.
 */
int Lua_Get_Vehicle_Gunner(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Gunner(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Forces a specific occupant to exit a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *		  Force_Occupant_Exit(Get_GameObj(10), 0)   -- seat 0 exits
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \param SeatIndex Number - Zero-based index of the seat to force exit.
 */
int Lua_Force_Occupant_Exit(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Force_Occupant_Exit(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}

/*!\details Forces all occupants of a vehicle to exit.
 *  \par Lua Command
 *  \code{.lua}
 *		  Force_Occupants_Exit(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 */
int Lua_Force_Occupants_Exit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Force_Occupants_Exit(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

/*!\details Returns the ID of the vehicle that returns a unit to its owner.
 *  \par Lua Command
 *  \code{.lua}
 *		  returnID = Get_Vehicle_Return(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Number - Object ID that will be returned, or `0` if none.
 */
int Lua_Get_Vehicle_Return(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Return(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Returns whether a vehicle is currently in fly-mode.
 *  \par Lua Command
 *  \code{.lua}
 *		  flying = Get_Fly_Mode(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Boolean - `true` if fly mode is active; otherwise `false`.
 */
int Lua_Get_Fly_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Get_Fly_Mode(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the number of seats a vehicle contains.
 *  \par Lua Command
 *  \code{.lua}
 *		  seatCount = Get_Vehicle_Seat_Count(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Number - Count of seats (including driver).
 */
int Lua_Get_Vehicle_Seat_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Seat_Count(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Transitions the soldier currently inside a vehicle to walk-mode.
 *  \par Lua Command
 *  \code{.lua}
 *		  Soldier_Transition_Vehicle(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle to transition from.
 */
int Lua_Soldier_Transition_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Soldier_Transition_Vehicle(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;		  // function performs an action only
}

/*!\details Returns the current operating mode of a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *		  mode = Get_Vehicle_Mode(Get_Vehicle(Get_GameObj(1)))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Number - Mode code
 */
int Lua_Get_Vehicle_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Mode(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the owner of a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *		  ownerID = Get_Vehicle_Owner(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Number - Object ID of the owner or `0` if none.
 */
int Lua_Get_Vehicle_Owner(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Owner(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Forces all occupants of a vehicle to exit and sends them to the
 *		  specified team.
 *  \par Lua Command
 *  \code{.lua}
 *		  Force_Occupants_Exit_Team(Get_GameObj(10), 2)
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \param TeamNumber Number - Target team to re-assign occupants.
 */
int Lua_Force_Occupants_Exit_Team(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Force_Occupants_Exit_Team(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;		  // no Lua value returned
}

/*!\details Retrieves the definition mode string for a vehicle preset.
 *  \par Lua Command
 *  \code{.lua}
 *		  mode = Get_Vehicle_Definition_Mode("Vehicle_StealthTank")
 *  \endcode
 *  \param PresetName String - Name of the vehicle preset.
 *  \return Number - Integer representation of the definition mode.
 */
int Lua_Get_Vehicle_Definition_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Definition_Mode(lua_tostring(L, 1)));
	return 1;
}

/*!\details Checks whether a unit is inside a zone object.
 *  \par Lua Command
 *  \code{.lua}
 *		  inside = IsInsideZone(Get_GameObj(10), Get_GameObj(20))
 *  \endcode
 *  \param UnitID Number - ID of the unit to test.
 *  \param ZoneID Number - ID of the zone object.
 *  \return Boolean - `true` if the unit is inside the zone; otherwise `false`.
 */
int Lua_IsInsideZone(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushboolean(L, IsInsideZone(Commands->Find_Object(lua_tonumber(L, 1)), Commands->Find_Object(lua_tonumber(L, 2))));
	return 1;
}

/*!\details Returns the definition mode for a vehicle preset by its ID.
 *  \par Lua Command
 *  \code{.lua}
 *		  mode = Get_Vehicle_Definition_Mode_By_ID(1234)
 *  \endcode
 *  \param PresetID Number - Numeric ID of the vehicle preset.
 *  \return Number - Definition mode integer.
 */
int Lua_Get_Vehicle_Definition_Mode_By_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Vehicle_Definition_Mode_By_ID(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the type identifier of a zone object.
 *  \par Lua Command
 *		  ztype = Get_Zone_Type(Get_GameObj(20))
 *  \code{.lua}
 *  \endcode
 *  \param ZoneID Number - ID of the zone object.
 *  \return Number - Integer code for the zone type.
 */
int Lua_Get_Zone_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Zone_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the ID of the gunner position for a vehicle.
 *  \par Lua Command
 *		  posID = Get_Vehicle_Gunner_Pos(Get_GameObj(10))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \return Number - Object ID of the gunner seat or `0` if none.
 */
int Lua_Get_Vehicle_Gunner_Pos(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Vehicle_Gunner_Pos(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Sets the gunner for a vehicle.
 *  \par Lua Command
 *		  Set_Vehicle_Gunner(Get_GameObj(10), Get_GameObj(5))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle.
 *  \param GunterID Number - Object ID to assign as gunner.
 */
int Lua_Set_Vehicle_Gunner(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Vehicle_Gunner(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;		  // no value pushed
}

/*!\details Returns the model name of a game object.
 *  \par Lua Command
 *		  model = Get_Model(Get_GameObj(10))
 *
 *  The optional second argument requests the *full* static object name.  
 *  \code{.lua}
 *		  fullName = Get_Model(Get_GameObj(10), true)
 *  \endcode
 *
 *  \param ObjectID Number - ID of the object.
 *  \param full Boolean (optional) - `true` to return the complete static-object name.
 *  \return String - The model or static object name, depending on `full`.
 */
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

/*!\details Returns the current animation frame index of a game object.
 *  \par Lua Command
 *		  frame = Get_Animation_Frame(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the object whose animation is queried.
 *  \return Number - Zero-based frame number or `-1` if no animation.
 */
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

/*!\details Returns true if the object is attached to another physical object.
 *  \par Lua Command
 *          attached = Is_Attached_To_An_Object(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is attached; otherwise `false`.
 */
LUA_IS_OBJ(Is_Attached_To_An_Object,
           obj->As_PhysicalGameObj() &&
           obj->As_PhysicalGameObj()->Is_Attached_To_An_Object())

/*!\details Returns true if the specified object is a harvester.
 *  \par Lua Command
 *          isHarv = Is_Harvester(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a harvester; otherwise `false`.
 */
LUA_IS_OBJ(Is_Harvester, Is_Harvester(obj))

/*!\details Returns true if the specified object is an active projectile.
 *  \par Lua Command
 *          isProj = Is_Projectile(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a projectile; otherwise `false`.
 */
LUA_IS_OBJ(Is_Projectile,
           obj->As_PhysicalGameObj() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_ProjectileClass())

/*!\details Returns true if the specified object is a tracked vehicle.
 *  \par Lua Command
 *          isTracked = Is_TrackedVehicle(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a tracked vehicle; otherwise `false`.
 */
LUA_IS_OBJ(Is_TrackedVehicle,
           obj->As_PhysicalGameObj() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_TrackedVehicleClass())

/*!\details Returns true if the specified object is a VTOL (vertical-take-off) vehicle.
 *  \par Lua Command
 *          isVTOL = Is_VTOLVehicle(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a VTOL vehicle; otherwise `false`.
 */
LUA_IS_OBJ(Is_VTOLVehicle,
           obj->As_PhysicalGameObj() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_VTOLVehicleClass())

/*!\details Returns true if the specified object is a wheeled vehicle.
 *  \par Lua Command
 *          isWheeled = Is_WheeledVehicle(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a wheeled vehicle; otherwise `false`.
 */
LUA_IS_OBJ(Is_WheeledVehicle,
           obj->As_PhysicalGameObj() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_WheeledVehicleClass())

/*!\details Returns true if the specified object is a motorcycle.
 *  \par Lua Command
 *          isMotorcycle = Is_Motorcycle(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a motorcycle; otherwise `false`.
 */
LUA_IS_OBJ(Is_Motorcycle,
           obj->As_PhysicalGameObj() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object() &&
           obj->As_PhysicalGameObj()->Peek_Physical_Object()->As_MotorcycleClass())

/*!\details Returns true if the specified object is a building (any type).
 *  \par Lua Command
 *          isBuilding = Is_Building(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a building; otherwise `false`.
 */
LUA_IS_OBJ(Is_Building, obj->As_BuildingGameObj())

/*!\details Returns true if the specified object is a soldier (human) unit.
 *  \par Lua Command
 *          isSoldier = Is_Soldier(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a soldier; otherwise `false`.
 */
LUA_IS_OBJ(Is_Soldier, obj->As_SoldierGameObj())

/*!\details Returns true if the specified object is a vehicle (any type).
 *  \par Lua Command
 *          isVehicle = Is_Vehicle(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a vehicle; otherwise `false`.
 */
LUA_IS_OBJ(Is_Vehicle, obj->As_VehicleGameObj())

/*!\details Returns true if the specified object is a scripted zone.
 *  \par Lua Command
 *          isScriptZone = Is_ScriptZone(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a script-zone; otherwise `false`.
 */
LUA_IS_OBJ(Is_ScriptZone, obj->As_ScriptZoneGameObj())

/*!\details Returns true if the specified object is a cinematic game object.
 *  \par Lua Command
 *          isCinematic = Is_Cinematic(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a cinematic; otherwise `false`.
 */
LUA_IS_OBJ(Is_Cinematic,
           obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_CinematicGameObj())

/*!\details Returns true if the specified object is a power-up (pickup).
 *  \par Lua Command
 *          isPowerUp = Is_Powerup(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a power-up; otherwise `false`.
 */
LUA_IS_OBJ(Is_Powerup,
           obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_PowerUpGameObj())

/*!\details Returns true if the specified object is a C4 explosive.
 *  \par Lua Command
 *          isC4 = Is_C4(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is C4; otherwise `false`.
 */
LUA_IS_OBJ(Is_C4,
           obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_C4GameObj())

/*!\details Returns true if the specified object is a beacon.
 *  \par Lua Command
 *          isBeacon = Is_Beacon(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is a beacon; otherwise `false`.
 */
LUA_IS_OBJ(Is_Beacon,
           obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_BeaconGameObj())

/*!\details Returns true if the specified object is armed (has weapons).
 *  \par Lua Command
 *          isArmed = Is_Armed(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is armed; otherwise `false`.
 */
LUA_IS_OBJ(Is_Armed,
           obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_ArmedGameObj())

/*!\details Returns true if the specified object is a simple (no complex behaviour) game object.
 *  \par Lua Command
 *          isSimple = Is_Simple(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object is simple; otherwise `false`.
 */
LUA_IS_OBJ(Is_Simple,
           obj->As_PhysicalGameObj() && obj->As_PhysicalGameObj()->As_SimpleGameObj())

/*!\details Returns true if the specified object is a power-plant building.
 *  \par Lua Command
 *          isPowerPlant = Is_PowerPlant(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is a power plant; otherwise `false`.
 */
LUA_IS_OBJ(Is_PowerPlant,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_PowerPlantGameObj())

/*!\details Returns true if the specified object is a soldier factory building.
 *  \par Lua Command
 *          isSoldierFactory = Is_SoldierFactory(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is a soldier factory; otherwise `false`.
 */
LUA_IS_OBJ(Is_SoldierFactory,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_SoldierFactoryGameObj())

/*!\details Returns true if the specified object is a vehicle factory building.
 *  \par Lua Command
 *          isVehicleFactory = Is_VehicleFactory(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is a vehicle factory; otherwise `false`.
 */
LUA_IS_OBJ(Is_VehicleFactory,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_VehicleFactoryGameObj())

/*!\details Returns true if the specified object is an airstrip (vehicle launch pad).
 *  \par Lua Command
 *          isAirstrip = Is_Airstrip(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is an airstrip; otherwise `false`.
 */
LUA_IS_OBJ(Is_Airstrip,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_AirStripGameObj())

/*!\details Returns true if the specified object is a war factory building.
 *  \par Lua Command
 *          isWarFactory = Is_WarFactory(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is a war factory; otherwise `false`.
 */
LUA_IS_OBJ(Is_WarFactory,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_WarFactoryGameObj())

/*!\details Returns true if the specified object is a refinery building.
 *  \par Lua Command
 *          isRefinery = Is_Refinery(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is a refinery; otherwise `false`.
 */
LUA_IS_OBJ(Is_Refinery,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_RefineryGameObj())

/*!\details Returns true if the specified object is a command centre building.
 *  \par Lua Command
 *          isComCenter = Is_ComCenter(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is a command centre; otherwise `false`.
 */
LUA_IS_OBJ(Is_ComCenter,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_ComCenterGameObj())

/*!\details Returns true if the specified object is a repair bay building.
 *  \par Lua Command
 *          isRepairBay = Is_RepairBay(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is a repair bay; otherwise `false`.
 */
LUA_IS_OBJ(Is_RepairBay,
           obj->As_BuildingGameObj() && obj->As_BuildingGameObj()->As_RepairBayGameObj())

/*!\details Returns true if the specified object is script-able (can be controlled by scripts).
 *  \par Lua Command
 *          isScriptable = Is_Scriptable(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if scriptable; otherwise `false`.
 */
LUA_IS_OBJ(Is_Scriptable, obj->As_ScriptableGameObj())

/*!\details Returns true if the specified building has already been destroyed.
 *  \par Lua Command
 *          isDead = Is_Building_Dead(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if the building is dead; otherwise `false`.
 */
LUA_IS_OBJ(Is_Building_Dead, Is_Building_Dead(obj))

/*!\details Returns true if the specified object matches a harvester preset (by name).
 *  \par Lua Command
 *          isHarvPreset = Is_Harvester_Preset(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if the object matches a harvester preset; otherwise `false`.
 */
LUA_IS_OBJ(Is_Harvester_Preset, Is_Harvester_Preset(obj))

/*!\details Returns true if the specified building is available for purchase.
 *  \par Lua Command
 *          avail = Is_Available_For_Purchase(ObjID)
 *  \param ObjID Number - ID of the building to test.
 *  \return Boolean - `true` if available; otherwise `false`.
 */
LUA_IS_OBJ(Is_Available_For_Purchase, Is_Available_For_Purchase(obj))

/*!\details Returns true if the engine of a vehicle is enabled.
 *  \par Lua Command
 *          engEnabled = Is_Engine_Enabled(ObjID)
 *  \param ObjID Number - ID of the vehicle to test.
 *  \return Boolean - `true` if the engine is enabled; otherwise `false`.
 */
LUA_IS_OBJ(Is_Engine_Enabled, Is_Engine_Enabled(obj))

/*!\details Returns true if the object belongs to an A-Star node (pathfinder).
 *  \par Lua Command
 *          isAStar = Is_A_Star(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if it’s an A-Star node; otherwise `false`.
 */
LUA_IS_OBJ(Is_A_Star, Commands->Is_A_Star(obj))

/*!\details Returns true if a smart object is currently stealthed.
 *  \par Lua Command
 *          stealth = Is_Stealthed(ObjID)
 *  \param ObjID Number - ID of the smart object to test.
 *  \return Boolean - `true` if stealthed; otherwise `false`.
 */
LUA_IS_OBJ(Is_Stealthed,
           obj->As_SmartGameObj() && obj->As_SmartGameObj()->Is_Stealthed())

/*!\details Returns true if the object is performing a path-finding action.
 *  \par Lua Command
 *          doingPath = Is_Performing_Pathfind_Action(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if it’s executing pathfind logic; otherwise `false`.
 */
LUA_IS_OBJ(Is_Performing_Pathfind_Action, Commands->Is_Performing_Pathfind_Action(obj))

/*!\details Returns true if an object is queued for deletion.
 *  \par Lua Command
 *          delPending = Is_Delete_Pending(ObjID)
 *  \param ObjID Number - ID of the object to test.
 *  \return Boolean - `true` if marked for delete; otherwise `false`.
 */
LUA_IS_OBJ(Is_Delete_Pending, obj->Is_Delete_Pending())

/*!\details Returns true if a vehicle is immovable (locked in place).
 *  \par Lua Command
 *          immovable = Is_Immovable(ObjID)
 *  \param ObjID Number - ID of the vehicle to test.
 *  \return Boolean - `true` if immovable; otherwise `false`.
 */
LUA_IS_OBJ(Is_Immovable,
           obj->As_VehicleGameObj() && obj->As_VehicleGameObj()->Is_Immovable())

/*!\details Returns the stealth activation status of a smart object.
 *  \par Lua Command
 *          active = Get_Stealth_Active(ObjID)
 *  \param ObjID Number - ID of the smart object to query.
 *  \return Boolean - `true` if stealth is currently active; otherwise `false`.
 */
LUA_IS_OBJ(Get_Stealth_Active,
           obj->As_SmartGameObj() && obj->As_SmartGameObj()->Get_Stealth_Active())

/*!\details Returns true if a soldier is inside an elevator.
 *  \par Lua Command
 *          inElev = Is_In_Elevator(ObjID)
 *  \param ObjID Number - ID of the soldier to test.
 *  \return Boolean - `true` if the soldier is in an elevator; otherwise `false`.
 */
LUA_IS_OBJ(Is_In_Elevator,
           obj->As_SoldierGameObj() && obj->As_SoldierGameObj()->Is_In_Elevator())

/*!\details Returns true if a soldier is currently busy (attacking, moving, …).
 *  \par Lua Command
 *          busy = Is_Soldier_Busy(ObjID)
 *  \param ObjID Number - ID of the soldier to test.
 *  \return Boolean - `true` if busy; otherwise `false`.
 */
LUA_IS_OBJ(Is_Soldier_Busy,
           obj->As_SoldierGameObj() && Is_Soldier_Busy(obj->As_SoldierGameObj()))

#define LUA_IS_PHYS(name, check) int Lua_##name(lua_State *L)\
{\
	if (lua_gettop(L) < 1) return 0;\
	StaticPhysClass* phys = PhysicsSceneClassLua::Get_Instance()->Get_Static_Object_By_ID(lua_tonumber(L, 1));\
	lua_pushboolean(L, (phys && check));\
	return 1;\
}

/*!\details Returns true if a static physics object is a door.
 *  \par Lua Command
 *          isDoor = Is_Door(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if it’s a door; otherwise `false`.
 */
LUA_IS_PHYS(Is_Door, phys->As_DoorPhysClass())

/*!\details Returns true if a static physics object is an elevator platform.
 *  \par Lua Command
 *          isElevator = Is_Elevator(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if it’s an elevator; otherwise `false`.
 */
LUA_IS_PHYS(Is_Elevator, phys->As_ElevatorPhysClass())

/*!\details Returns true if a static physics object can be damaged.
 *  \par Lua Command
 *          isDamPhys = Is_DamageableStaticPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if damageable; otherwise `false`.
 */
LUA_IS_PHYS(Is_DamageableStaticPhys, phys->As_DamageableStaticPhysClass())

/*!\details Returns true if a static physics object is accessible (e.g. can be opened).
 *  \par Lua Command
 *          isAccPhys = Is_AccessablePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if accessible; otherwise `false`.
 */
LUA_IS_PHYS(Is_AccessablePhys, phys->As_AccessiblePhysClass())

/*!\details Returns true if a static physics object is decorative.
 *  \par Lua Command
 *          isDecorPhys = Is_DecorationPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if decorative; otherwise `false`.
 */
LUA_IS_PHYS(Is_DecorationPhys, phys->As_DecorationPhysClass())

/*!\details Returns true if a static physics object represents a human character.
 *  \par Lua Command
 *          isHumanPhys = Is_HumanPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if it’s a human; otherwise `false`.
 */
LUA_IS_PHYS(Is_HumanPhys, phys->As_HumanPhysClass())

/*!\details Returns true if a static physics object is a motor vehicle.
 *  \par Lua Command
 *          isMotorPhys = Is_MotorVehicle(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if motor vehicle; otherwise `false`.
 */
LUA_IS_PHYS(Is_MotorVehicle, phys->As_MotorVehicleClass())

/*!\details Returns true if a static physics object has a Phys3 interface.
 *  \par Lua Command
 *          isPhys3 = Is_Phys3(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if Phys3; otherwise `false`.
 */
LUA_IS_PHYS(Is_Phys3, phys->As_Phys3Class())

/*!\details Returns true if a static physics object is a rigid body.
 *  \par Lua Command
 *          isRigid = Is_RigidBody(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if rigid; otherwise `false`.
 */
LUA_IS_PHYS(Is_RigidBody, phys->As_RigidBodyClass())

/*!\details Returns true if a static physics object has a static animation interface.
 *  \par Lua Command
 *          isStaticAnim = Is_StaticAnimPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if it has a static animation; otherwise `false`.
 */
LUA_IS_PHYS(Is_StaticAnimPhys, phys->As_StaticAnimPhysClass())

/*!\details Returns true if a static physics object is a plain static object.
 *  \par Lua Command
 *          isStatic = Is_StaticPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if it’s a static object; otherwise `false`.
 */
LUA_IS_PHYS(Is_StaticPhys, phys->As_StaticPhysClass())

/*!\details Returns true if a static physics object is a timed decoration.
 *  \par Lua Command
 *          isTimedDec = Is_TimedDecorationPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if timed decoration; otherwise `false`.
 */
LUA_IS_PHYS(Is_TimedDecorationPhys, phys->As_TimedDecorationPhysClass())

/*!\details Returns true if a static physics object represents a vehicle (timed).
 *  \par Lua Command
 *          isVehiclePhys = Is_VehiclePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if it’s a vehicle; otherwise `false`.
 */
LUA_IS_PHYS(Is_VehiclePhys, phys->As_TimedDecorationPhysClass())

/*!\details Returns true if a static physics object has a dynamic animation interface.
 *  \par Lua Command
 *          isDynAnim = Is_DynamicAnimPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if it has a dynamic animation; otherwise `false`.
 */
LUA_IS_PHYS(Is_DynamicAnimPhys, phys->As_DynamicPhysClass())

/*!\details Returns true if a static physics object can be moved.
 *  \par Lua Command
 *          isMoveable = Is_MoveablePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if movable; otherwise `false`.
 */
LUA_IS_PHYS(Is_MoveablePhys, phys->As_MoveablePhysClass())

/*!\details Returns true if a static physics object is a wheeled vehicle.
 *  \par Lua Command
 *          isWheeledPhys = Is_WheeledVehiclePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if wheeled; otherwise `false`.
 */
LUA_IS_PHYS(Is_WheeledVehiclePhys, phys->As_WheeledVehicleClass())

/*!\details Returns true if a static physics object is a motorcycle.
 *  \par Lua Command
 *          isMotorcyclePhys = Is_MotorcyclePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if motorcycle; otherwise `false`.
 */
LUA_IS_PHYS(Is_MotorcyclePhys, phys->As_MotorcycleClass())

/*!\details Returns true if a static physics object is a tracked vehicle.
 *  \par Lua Command
 *          isTrackedPhys = Is_TrackedVehiclePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if tracked; otherwise `false`.
 */
LUA_IS_PHYS(Is_TrackedVehiclePhys, phys->As_TrackedVehicleClass())

/*!\details Returns true if a static physics object is a VTOL vehicle.
 *  \par Lua Command
 *          isVTOLPhys = Is_VTOLVehiclePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if VTOL; otherwise `false`.
 */
LUA_IS_PHYS(Is_VTOLVehiclePhys, phys->As_VTOLVehicleClass())

/*!\details Returns true if a static physics object is a light source.
 *  \par Lua Command
 *          isLight = Is_LightPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if light; otherwise `false`.
 */
LUA_IS_PHYS(Is_LightPhys, phys->As_LightPhysClass())

/*!\details Returns true if a static physics object has a renderable component.
 *  \par Lua Command
 *          isRenderObj = Is_RenderObjPhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if renderable; otherwise `false`.
 */
LUA_IS_PHYS(Is_RenderObjPhys, phys->As_RenderObjPhysClass())

/*!\details Returns true if a static physics object represents a projectile.
 *  \par Lua Command
 *          isProjectilePhys = Is_ProjectilePhys(PhysID)
 *  \param PhysID Number - ID of the physical object to test.
 *  \return Boolean - `true` if projectile; otherwise `false`.
 */
LUA_IS_PHYS(Is_ProjectilePhys, phys->As_ProjectileClass())
/* End of Is functions */

/*!\details Copies the world transform (position, rotation and scale) from one game object to another.
 *  \par Lua Command
 *		  Copy_Transform(SourceObj, TargetObj)
 *  \par Example
 *		  Copy_Transform(Get_GameObj(5), Get_GameObj(12))
 *  \param SourceObj Number - Object ID of the source.
 *  \param TargetObj Number - Object ID of the target that will receive the transform.
 */
int Lua_Copy_Transform(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Copy_Transform(Commands->Find_Object(lua_tonumber(L, 1)), Commands->Find_Object(lua_tonumber(L, 2)));
	return 0;		  // no Lua value pushed
}

/*!\details Returns the mass of a game object (in kilograms).
 *  \par Lua Command
 *		  mass = Get_Mass(Object)
 *  \par Example
 *		  mass = Get_Mass(Get_GameObj(10))
 *  \param Object Number - ID of the object whose mass to query.
 *  \return Number - Mass value; `0` if the object is invalid or mass cannot be retrieved.
 */
int Lua_Get_Mass(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Mass(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the name of the hierarchical tree object that owns the supplied game object.
 *  \par Lua Command
 *		  htree = Get_Htree_Name(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object whose owning HTree is requested.
 *  \return String - Name of the HTree or `nil` if none exists.
 */
int Lua_Get_Htree_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Htree_Name(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the gender/sex of a game object (e.g. 0 = male, 1 = female).
 *  \par Lua Command
 *		  sex = Get_Sex(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object whose sex to query.
 *  \return Number - Integer value representing the sex (0/1) or `-1` if unknown.
 */
int Lua_Get_Sex(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Sex(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Creates an effect for every instance of a preset in the map.
 *  \par Lua Command
 *		  Create_Effect_All_Of_Preset("FX_Hail", "Weapon_Missile_Nuke", 3, true)
 *  \param PresetName String - Name of the effect preset to spawn.
 *  \param TargetPreset String - Preset that triggers the effect (e.g. weapon or building).
 *  \param Count Number - Number of instances to create.
 *  \param random Boolean - If `true`, positions are randomized around each instance.
 */
int Lua_Create_Effect_All_Of_Preset(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Create_Effect_All_Of_Preset(lua_tostring(L, 1), lua_tostring(L, 2),
								lua_tonumber(L, 3), lua_tobooleanCPP(L, 4));
	return 0;		  // no Lua value returned
}

/*!\details Returns the numeric ID of a game object.
 *  \par Lua Command
 *		  id = Get_GameObj(10)
 *  \param ObjectNumber Number - ID supplied to Find_Object().
 *  \return Number - The same numeric ID (or `0` if the object does not exist).
 */
int Lua_Get_GameObj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_GameObj(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Retrieves the player index that owns a given object.
 *  \par Lua Command
 *		  playerIdx = Get_Player_ID(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object to inspect.
 *  \return Number - Player number (0-based) or `-1` if not owned by a player.
 */
int Lua_Get_Player_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Player_ID(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the name of the player who owns a given object.
 *  \par Lua Command
 *		  name = Get_Player_Name(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object to inspect.
 *  \return String - Player’s display name or `nil` if not found.
 */
int Lua_Get_Player_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Player_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Retrieves the name of a player by their numeric ID.
 *  \par Lua Command
 *		  name = Get_Player_Name_By_ID(3)
 *  \param PlayerID Number - Numeric player index (0-based).
 *  \return String - Player’s display name or `nil` if not found.
 */
int Lua_Get_Player_Name_By_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Player_Name_By_ID(lua_tonumber(L, 1));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Changes the team of a game object.
 *  \par Lua Command
 *		  Change_Team(Get_GameObj(10), 2)
 *  \param ObjectID Number - ID of the object to move.
 *  \param NewTeam Number - Target team index (0-based).
 */
int Lua_Change_Team(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Change_Team(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;		  // no value pushed
}

/*!\details Changes the team of a player by numeric ID.
 *  \par Lua Command
 *		  Change_Team_By_ID(3, 1)
 *  \param PlayerID Number - Index of the player to move.
 *  \param NewTeam Number - Target team index (0-based).
 */
int Lua_Change_Team_By_ID(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Change_Team_By_ID(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}

/*!\details Returns the total number of active players in the match.
 *  \par Lua Command
 *		  count = Get_Player_Count()
 *  \return Number - Player count (>= 0).
 */
int Lua_Get_Player_Count(lua_State *L)
{
	lua_pushnumber(L, Get_Player_Count());
	return 1;
}

/*!\details Returns how many players belong to a specific team.
 *  \par Lua Command
 *		  count = Get_Team_Player_Count(1)
 *  \param Team Number - Team index (0-based).
 *  \return Number - Number of players on that team.
 */
int Lua_Get_Team_Player_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team_Player_Count(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the team index that owns a given object.
 *  \par Lua Command
 *		  team = Get_Team(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object to inspect.
 *  \return Number - Team number (0-based) or `-1` if unknown.
 */
int Lua_Get_Team(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Retrieves a player’s rank in the current match.
 *  \par Lua Command
 *		  rank = Get_Rank(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \return Number - Rank (1-based) or `0` if not ranked.
 */
int Lua_Get_Rank(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Rank(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the number of kills a player has made.
 *  \par Lua Command
 *		  kills = Get_Kills(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \return Number - Kill count (>= 0).
 */
int Lua_Get_Kills(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Kills(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the number of deaths a player has suffered.
 *  \par Lua Command
 *		  deaths = Get_Deaths(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \return Number - Death count (>= 0).
 */
int Lua_Get_Deaths(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Deaths(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Retrieves a player’s current score.
 *  \par Lua Command
 *		  score = Get_Score(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \return Number - Score value (>= 0).
 */
int Lua_Get_Score(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Score(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the amount of money a player currently owns.
 *  \par Lua Command
 *		  money = Get_Money(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \return Number - Money amount (>= 0).
 */
int Lua_Get_Money(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Money(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Computes the kill-to-death ratio for a player.
 *  \par Lua Command
 *		  kd = Get_Kill_To_Death_Ratio(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \return Number - Ratio (kills / deaths) or `0` if deaths are zero.
 */
int Lua_Get_Kill_To_Death_Ratio(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Kill_To_Death_Ratio(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the part of a nickname
 *  \par Lua Command
 *		  gameobj = Get_Part_Name("sla") -- If sla.ro ingame, will return gameobj of sla.ro
 *  \param String nickname
 *  \return ObjectID Number - ID of the object to query.
 */
int Lua_Get_Part_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Commands->Get_ID(Get_Part_Name(lua_tostring(L, 1))));
	return 1;
}

/*!\details Retrieves the names of all parts that belong to a given preset.
 *  \par Lua Command
 *		  names = Get_Part_Names("Weapon_StealthTank")
 *  \param PresetName String - Name of the preset whose part list is desired.
 *  \return Number - A Lua table containing part names (or `nil` if none).
 */
int Lua_Get_Part_Names(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Part_Names(lua_tostring(L, 1)));
	return 1;
}

/*!\details Finds a game object that belongs to a player with the supplied name.
 *  \par Lua Command
 *		  id = Get_GameObj_By_Player_Name("Alex")
 *  \param PlayerName String - Display name of the player to locate.
 *  \return Number - Object ID belonging to that player or `0` if not found.
 */
int Lua_Get_GameObj_By_Player_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_GameObj_By_Player_Name(lua_tostring(L, 1))));
	return 1;
}

/*!\details Attempts to purchase an item for the object’s owner.
 *  \par Lua Command
 *		  cost = Purchase_Item(Get_GameObj(10), 5)
 *  \param ObjectID Number - ID of the object that owns the player.
 *  \param ItemIndex Number - Index of the item to buy (see shop table).
 *  \return Number - Cost charged (or `0` if purchase failed).
 */
int Lua_Purchase_Item(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Purchase_Item(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2)));
	return 1;
}

/*!\details Sets the total number of points a ladder holds.
 *  \par Lua Command
 *		  Set_Ladder_Points(10, 5)
 *  \param LadderID Number - Identifier of the ladder to modify.
 *  \param Points Number - New point value.
 */
int Lua_Set_Ladder_Points(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Ladder_Points(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}

/*!\details Sets the rung index of a ladder.
 *  \par Lua Command
 *		  Set_Rung(10, 3)
 *  \param LadderID Number - Identifier of the ladder to modify.
 *  \param RungIndex Number - Desired rung (1-based) or `0` for default.
 */
int Lua_Set_Rung(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Rung(lua_tonumber(L, 1), lua_tonumber(L, 3));
	return 0;
}

/*!\details Sets the amount of money a player owns.
 *  \par Lua Command
 *		  Set_Money(Get_GameObj(10), 15000)
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \param Amount Number - New money value.
 */
int Lua_Set_Money(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Money(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}

/*!\details Sets the score of a player.
 *  \par Lua Command
 *		  Set_Score(Get_GameObj(10), 120)
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \param Score Number - New score value.
 */
int Lua_Set_Score(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Score(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}

/*!\details Returns the ID of the first player belonging to a given team.
 *  \par Lua Command
 *		  id = Find_First_Player(2)
 *  \param Team Number - Team index (0-based).
 *  \return Number - Object ID or `0` if no player found.
 */
int Lua_Find_First_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_First_Player(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Moves a player to a different team and optionally forces a respawn.
 *  \par Lua Command
 *		  newTeam = Change_Player_Team(Get_GameObj(10), true, false, true)
 *  \param ObjectID Number - ID of the object belonging to the player.
 *  \param forceRespawn Boolean - If `true`, the player is forced to respawn immediately.
 *  \param forceTeleport Boolean - If `true`, teleport the player to the new team’s spawn point.
 *  \param keepCamera Boolean - If `false`, reset camera focus after team change.
 *  \return Number - New team index (or `-1` on failure).
 */
int Lua_Change_Player_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	lua_pushnumber(L, Change_Player_Team(Commands->Find_Object(lua_tonumber(L, 1)),
										lua_tobooleanCPP(L, 2),
										lua_tobooleanCPP(L, 3),
										lua_tobooleanCPP(L, 4)));
	return 1;
}

/*!\details Counts how many players are currently alive on a given team.
 *  \par Lua Command
 *		  size = Tally_Team_Size(1)
 *  \param Team Number - Team index (0-based).
 *  \return Number - Count of living players on that team.
 */
int Lua_Tally_Team_Size(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Tally_Team_Size(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Retrieves the total score of a team.
 *  \par Lua Command
 *		  score = Get_Team_Score(1)
 *  \param Team Number - Team index (0-based).
 *  \return Number - Accumulated score for that team.
 */
int Lua_Get_Team_Score(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team_Score(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Sends a custom event to every player in the match.
 *  \par Lua Command
 *		  Send_Custom_All_Players(2, Get_GameObj(5), 100)
 *  \param CustomType Number - Type ID of the custom message.
 *  \param SenderObj GameObject - Object that is sending the message.
 *  \param Data Number - Payload data for the event.
 */
int Lua_Send_Custom_All_Players(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Send_Custom_All_Players(lua_tonumber(L, 1),
							Commands->Find_Object(lua_tonumber(L, 2)),
							lua_tonumber(L, 3));
	return 0;
}

/*!\details Steals a specified amount of credits from one team and transfers them to another.
 *  \par Lua Command
 *		  stolen = Steal_Team_Credits(1, 500)
 *  \param SourceTeam Number - Team losing the credits.
 *  \param Amount Number - Amount of money to transfer.
 *  \return Number - Actual amount transferred (may be less than requested).
 */
int Lua_Steal_Team_Credits(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Steal_Team_Credits(lua_tonumber(L, 1), lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the current credit balance of a team.
 *  \par Lua Command
 *		  credits = Get_Team_Credits(2)
 *  \param Team Number - Team index (0-based).
 *  \return Number - Money held by that team.
 */
int Lua_Get_Team_Credits(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Team_Credits(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Changes the team of a game object without affecting its position.
 *  \par Lua Command
 *		  Change_Team_2(Get_GameObj(10), 0)
 *  \param ObjectID Number - ID of the object to move.
 *  \param NewTeam Number - Target team index (0-based).
 */
int Lua_Change_Team_2(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Change_Team_2(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 0;
}

/*!\details Returns the player type code of an object.
 *  \par Lua Command
 *		  type = Get_Player_Type(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object belonging to a player.
 *  \return Number - Player type enumeration value.
 */
int Lua_Get_Player_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Player_Type(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Calculates the total cost for a team based on its current build queue.
 *  \par Lua Command
 *		  cost = Get_Team_Cost("Vehicle_StealthTank", 2)
 *  \param PresetName String - Name of the preset to evaluate.
 *  \param Team Number - Team index (0-based).
 *  \return Number - Total cost in credits.
 */
int Lua_Get_Team_Cost(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Team_Cost(lua_tostring(L, 1), lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the base price of a given preset.
 *  \par Lua Command
 *		  cost = Get_Cost("Weapon_StealthTank")
 *  \param PresetName String - Name of the preset to query.
 *  \return Number - Cost in credits (or `0` if unknown).
 */
int Lua_Get_Cost(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Cost(lua_tostring(L, 1)));
	return 1;
}

/*!\details Retrieves the file path to a team’s icon image.
 *  \par Lua Command
 *		  icon = Get_Team_Icon("Team_Red", 2)
 *  \param TeamName String - Identifier of the team (e.g., "Team_Red").
 *  \param Team Number - Optional numeric index (used by some engines).
 *  \return String - File path to the icon image.
 */
int Lua_Get_Team_Icon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushstring(L, Get_Team_Icon(lua_tostring(L, 1), lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the file path to an icon image for a given preset.
 *  \par Lua Command
 *		  icon = Get_Icon("Vehicle_StealthTank")
 *  \param PresetName String - Name of the preset whose icon you want.
 *  \return String - File path to the icon image or `nil` if none found.
 */
int Lua_Get_Icon(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Icon(lua_tostring(L, 1)));
	return 1;
}

/*!\details Removes a specific script from an object’s active script list.
 *  \par Lua Command
 *		  Remove_Script(Get_GameObj(10), "AI_Defense")
 *  \param ObjectID Number - ID of the object to modify.
 *  \param ScriptName String - Name of the script to remove.
 */
int Lua_Remove_Script(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Remove_Script(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2));
	return 1;
}

/*!\details Removes all scripts from an object’s active script list.
 *  \par Lua Command
 *		  Remove_All_Scripts(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object to clear.
 */
int Lua_Remove_All_Scripts(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Remove_All_Scripts(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;		  // no value pushed
}

/*!\details Attaches a script to all objects that match a particular preset.
 *  \par Lua Command
 *		  Attach_Script_Preset("PresetName", "M03_Goto_Star",
 *							   "AdditionalParam", 2)
 *  \param PresetName String   - Name of the object preset to match.
 *  \param ScriptFile String   - Name of the script to attach.
 *  \param ExtraParam String   - Optional extra parameter for the script.
 *  \param MaxCount Number	- Maximum number of objects to attach the script to.
 */
int Lua_Attach_Script_Preset(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Preset(lua_tostring(L, 1), lua_tostring(L, 2),
						 lua_tostring(L, 3), lua_tonumber(L, 4));
	return 0;		  // no value pushed
}

/*!\details Attaches a script to all objects that match a particular type.
 *  \par Lua Command
 *		  Attach_Script_Type("Weapon", "M03_Goto_Star",
 *							 2, 1)
 *  \param TypeName String   - Name of the object type to match (e.g. “Vehicle”).
 *  \param ScriptFile String - Name of the script to attach.
 *  \param MaxCount Number  - Maximum number of objects to receive the script.
 *  \param Team Number	 - Team index; only objects belonging to this team are affected.
 */
int Lua_Attach_Script_Type(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Type(lua_tostring(L, 1), lua_tostring(L, 2),
					   lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}

/*!\details Removes a script from all objects that match the supplied preset.
 *  \par Lua Command
 *		  Remove_Script_Preset("PresetName", "M03_Goto_Star", 2)
 *  \param PresetName String   - Name of the object preset to match.
 *  \param ScriptFile String   - Name of the script to remove.
 *  \param MaxCount Number	- Maximum number of objects from which the script will be removed.
 */
int Lua_Remove_Script_Preset(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Remove_Script_Preset(lua_tostring(L, 1), lua_tostring(L, 2),
						 lua_tonumber(L, 3));
	return 0;
}

/*!\details Removes a script from all objects that match the supplied type.
 *  \par Lua Command
 *		  Remove_Script_Type("Vehicle", 2, 1)
 *  \param TypeName String   - Name of the object type to match.
 *  \param MaxCount Number  - Maximum number of objects to affect.
 *  \param Team Number	 - Team index; only objects belonging to this team are considered.
 */
int Lua_Remove_Script_Type(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Remove_Script_Type(lua_tostring(L, 1), lua_tonumber(L, 2),
					   lua_tonumber(L, 3));
	return 0;
}

/*!\details Checks whether a particular script is attached to an object.
 *  \par Lua Command
 *		  attached = Is_Script_Attached(Get_GameObj(10), "M03_GotoStar")
 *  \param ObjectID Number - ID of the object to query.
 *  \param ScriptName String - Name of the script to look for.
 *  \return Boolean - `true` if the script is attached; otherwise `false`.
 */
int Lua_Is_Script_Attached(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushboolean(L, Is_Script_Attached(Commands->Find_Object(lua_tonumber(L, 1)),
										  lua_tostring(L, 2)));
	return 1;
}

/*!\details Attaches a script to a single object and guarantees that it runs only once.
 *  \par Lua Command
 *		  Attach_Script_Once(Get_GameObj(10), "M03_Goto_Star",
 *							 "Param")
 *  \param ObjectID Number - ID of the target object.
 *  \param ScriptFile String - Name of the script to attach.
 *  \param ExtraParam String - Optional parameter for the script.
 */
int Lua_Attach_Script_Once(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Attach_Script_Once(Commands->Find_Object(lua_tonumber(L, 1)),
					   lua_tostring(L, 2), lua_tostring(L, 3));
	return 0;
}

/*!\details Attaches a script to all objects matching a preset - the script is guaranteed
 *		  to run only once per object.
 *  \par Lua Command
 *		  Attach_Script_Preset_Once("PresetName", "M03_GotoStar",
 *									"Param", 2)
 *  \param PresetName String   - Name of the object preset.
 *  \param ScriptFile String   - Name of the script to attach.
 *  \param ExtraParam String   - Optional parameter for the script.
 *  \param MaxCount Number	- Maximum number of objects that receive the script.
 */
int Lua_Attach_Script_Preset_Once(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Preset_Once(lua_tostring(L, 1), lua_tostring(L, 2),
							  lua_tostring(L, 3), lua_tonumber(L, 4));
	return 0;
}

/*!\details Attaches a script to all objects matching a type - the script runs only once per object.
 *  \par Lua Command
 *		  Attach_Script_Type_Once("M03_Goto_Star", "Params",
 *								   2, 1)
 *  \param Script String   - Name of the object type.
 *  \param ScriptParams String   - Name of the script to attach.
 *  \param MaxCo Number	- Maximum number of objects affected.
 *  \param Team Number	  - Team index; only objects belonging to this team receive the script.
 */
int Lua_Attach_Script_Type_Once(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_Type_Once(lua_tostring(L, 1), lua_tostring(L, 2),
							lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}

/*!\details Attaches a script to all building objects of a particular type.
 *  \par Lua Command
 *		  Attach_Script_Building("BuildingPreset", "M03_Goto_Star",
 *								 2)
 *  \param PresetName String   - Name of the building preset to match.
 *  \param ScriptFile String   - Name of the script to attach.
 *  \param MaxCount Number	- Maximum number of buildings that receive the script.
 */
int Lua_Attach_Script_Building(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Attach_Script_Building(lua_tostring(L, 1), lua_tostring(L, 2),
						   lua_tonumber(L, 3));
	return 0;
}

/*!\details Attaches a script to an object only if it matches the specified preset.
 *		  The script is executed once per match.
 *  \par Lua Command
 *		  Attach_Script_Is_Preset(Get_GameObj(10), "PresetName",
 *								 "M03_Goto_Star", "Param", 2)
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String   - Preset name to match.
 *  \param ScriptFile String   - Name of the script to attach.
 *  \param ExtraParam String   - Optional parameter for the script.
 *  \param MaxCount Number	- Maximum number of objects that can receive this script.
 */
int Lua_Attach_Script_Is_Preset(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Attach_Script_Is_Preset(Commands->Find_Object(lua_tonumber(L, 1)),
							lua_tostring(L, 2), lua_tostring(L, 3),
							lua_tostring(L, 4), lua_tonumber(L, 5));
	return 0;
}

/*!\details Attaches a script to an object only if it matches the specified type.
 *		  The script runs once per match.
 *  \par Lua Command
 *		  Attach_Script_Is_Type(Get_GameObj(10), 1,
 *								"M03_Goto_Star", "Param", 2)
 *  \param ObjectID Number - ID of the target object.
 *  \param TypeIndex Number - Numeric type index to match.
 *  \param ScriptFile String   - Name of the script to attach.
 *  \param ExtraParam String   - Optional parameter for the script.
 *  \param MaxCount Number	- Maximum number of objects that can receive this script.
 */
int Lua_Attach_Script_Is_Type(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Attach_Script_Is_Type(Commands->Find_Object(lua_tonumber(L, 1)),
						  lua_tonumber(L, 2), lua_tostring(L, 3),
						  lua_tostring(L, 4), lua_tonumber(L, 5));
	return 0;
}

/*!\details Attaches a script to a player once - used for per-player initialisation.
 *  \par Lua Command
 *		  Attach_Script_Player_Once("PlayerName", "M03_Goto_Star",
 *								   2)
 *  \param PlayerName String   - Name of the target player.
 *  \param ScriptFile String   - Name of the script to attach.
 *  \param MaxCount Number	- Maximum number of players that can receive this script.
 */
int Lua_Attach_Script_Player_Once(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Attach_Script_Player_Once(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}

/*!\details Removes duplicate instances of a particular script from an object.
 *  \par Lua Command
 *		  Remove_Duplicate_Script(Get_GameObj(10), "M03_Goto_Star")
 *  \param ObjectID Number - ID of the target object.
 *  \param ScriptFile String - Name of the script to dedupe.
 */
int Lua_Remove_Duplicate_Script(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Remove_Duplicate_Script(Commands->Find_Object(lua_tonumber(L, 1)),
							lua_tostring(L, 2));
	return 0;
}

/*!\details Attaches a script to every building of a given team.
 *  \par Lua Command
 *		  Attach_Script_All_Buildings_Team(1, "BuildingPreset",
 *										   "M03_Goto_Star", true)
 *  \param Team Number - Index of the target team.
 *  \param PresetName String - Building preset to match.
 *  \param ScriptFile String - Name of the script to attach.
 *  \param repeat Boolean - If `true`, the script will be re-attached each round.
 */
int Lua_Attach_Script_All_Buildings_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_All_Buildings_Team(lua_tonumber(L, 1), lua_tostring(L, 2),
									 lua_tostring(L, 3), lua_tobooleanCPP(L, 4));
	return 0;
}

/*!\details Attaches a script to every turret of a given team.
 *  \par Lua Command
 *		  Attach_Script_All_Turrets_Team(1, "TurretPreset",
 *										 "M03_Goto_Star", false)
 *  \param Team Number - Index of the target team.
 *  \param PresetName String - Turret preset to match.
 *  \param ScriptFile String - Name of the script to attach.
 *  \param repeat Boolean - If `true`, the script will be re-attached each round.
 */
int Lua_Attach_Script_All_Turrets_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Attach_Script_All_Turrets_Team(lua_tonumber(L, 1), lua_tostring(L, 2),
								   lua_tostring(L, 3), lua_tobooleanCPP(L, 4));
	return 0;
}

/*!\details Finds a building that matches the supplied preset and already has
 *		  the specified script attached.
 *  \par Lua Command
 *		  id = Find_Building_With_Script(1, 2,
 *										 "BuildingPreset", Get_GameObj(5))
 *  \param Team Number - Target team index.
 *  \param MaxCount Number - Maximum number of buildings to search.
 *  \param PresetName String - Building preset to match.
 *  \param ScriptObject GameObject - Object that represents the script (usually a trigger).
 *  \return Number - ID of the first matching building or `0` if none found.
 */
int Lua_Find_Building_With_Script(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	lua_pushnumber(L, Get_ID(Find_Building_With_Script(lua_tonumber(L, 1),
													  lua_tonumber(L, 2),
													  lua_tostring(L, 3),
													  Commands->Find_Object(lua_tonumber(L, 4)))));
	return 1;
}

/*!\details Returns the ID of an object that has a particular script attached.
 *  \par Lua Command
 *		  id = Find_Object_With_Script("M03_Goto_Star")
 *  \param ScriptName String - Name of the script to look for.
 *  \return Number - ID of the first matching object or `0` if none found.
 */
int Lua_Find_Object_With_Script(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Find_Object_With_Script(lua_tostring(L, 1))));
	return 1;
}

/*!\details Returns a translated string for the given translation key.
 *  \par Lua Command
 *		  txt = Get_Translated_String(1234)
 *  \param Key Number - Translation key identifier.
 *  \return String - Translated text (or `nil` if not found).
 */
int Lua_Get_Translated_String(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Translated_String(lua_tonumber(L, 1));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Returns the translated name of a preset belonging to an object.
 *  \par Lua Command
 *		  name = Get_Translated_Preset_Name(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return String - Translated preset name (or `nil` if not found).
 */
int Lua_Get_Translated_Preset_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Translated_Preset_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Returns the translated name of a weapon belonging to an object.
 *  \par Lua Command
 *		  wname = Get_Translated_Weapon(Get_GameObj(10), 2)
 *  \param ObjectID Number - ID of the target object.
 *  \param Slot Number	- Weapon slot index to translate.
 *  \return String - Translated weapon name (or `nil` if not found).
 */
int Lua_Get_Translated_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	const char *ptr = Get_Translated_Weapon(Commands->Find_Object(lua_tonumber(L, 1)),
											lua_tonumber(L, 2));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Returns the name of the currently active weapon for an object,
 *		  translated into the current language.
 *  \par Lua Command
 *		  curWpn = Get_Current_Translated_Weapon(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return String - Translated name of the active weapon (or `nil` if none).
 */
int Lua_Get_Current_Translated_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Current_Translated_Weapon(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Retrieves the display name of a team.
 *  \par Lua Command
 *		  tname = Get_Team_Name(2)
 *  \param Team Number - Index of the target team.
 *  \return String - Human-readable team name (or `nil` if not found).
 */
int Lua_Get_Team_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Team_Name(lua_tonumber(L, 1));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Retrieves the display name of a vehicle belonging to an object.
 *  \par Lua Command
 *		  vname = Get_Vehicle_Name(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return String - Vehicle name (or `nil` if none).
 */
int Lua_Get_Vehicle_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Vehicle_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Returns the translated name of a definition (e.g. preset or weapon).
 *  \par Lua Command
 *		  dname = Get_Translated_Definition_Name("Weapon_StealthTank")
 *  \param DefinitionName String - Name of the definition to translate.
 *  \return String - Translated definition name (or `nil` if not found).
 */
int Lua_Get_Translated_Definition_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Get_Translated_Definition_Name(lua_tostring(L, 1));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	delete[] ptr;
	return 1;
}

/*!\details Returns the number of bullets currently loaded in the active weapon of an object.
 *  \par Lua Command
 *		  bullets = Get_Current_Bullets(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return Number - Bullets remaining in the current clip (or `-1` if unknown).
 */
int Lua_Get_Current_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the number of bullets in the active clip for an object.
 *  \par Lua Command
 *		  clip = Get_Current_Clip_Bullets(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return Number - Clip bullets remaining (or `-1` if unknown).
 */
int Lua_Get_Current_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the total number of bullets remaining for the current weapon.
 *		  This includes both the active clip and any reserve ammo.
 *  \par Lua Command
 *		  total = Get_Current_Total_Bullets(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return Number - Total bullets remaining (or `-1` if unknown).
 */
int Lua_Get_Current_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the maximum amount of reserve bullets for a weapon preset.
 *  \par Lua Command
 *		  maxReserve = Get_Total_Bullets(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to query.
 *  \return Number - Maximum reserve bullets (or `-1` if unknown).
 */
int Lua_Get_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
										lua_tostring(L, 2)));
	return 1;
}

/*!\details Returns the number of bullets currently loaded in a specific weapon slot.
 *  \par Lua Command
 *		  clip = Get_Clip_Bullets(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to query.
 *  \return Number - Clip bullets remaining (or `-1` if unknown).
 */
int Lua_Get_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
									   lua_tostring(L, 2)));
	return 1;
}

/*!\details Returns the current number of bullets in a weapon slot.
 *		  Similar to `Get_Current_Bullets` but for a specific preset.
 *  \par Lua Command
 *		  cur = Get_Bullets(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to query.
 *  \return Number - Bullets currently loaded (or `-1` if unknown).
 */
int Lua_Get_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
								  lua_tostring(L, 2)));
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

/*!\details Returns the maximum amount of bullets allowed in a weapon preset.
 *  \par Lua Command
 *		  max = Get_Max_Total_Bullets(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to query.
 *  \return Number - Maximum total bullets (or `-1` if unknown).
 */
int Lua_Get_Max_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Max_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
											lua_tostring(L, 2)));
	return 1;
}

/*!\details Returns the maximum number of bullets that can be loaded into a clip.
 *  \par Lua Command
 *		  maxClip = Get_Max_Clip_Bullets(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to query.
 *  \return Number - Maximum clip bullets (or `-1` if unknown).
 */
int Lua_Get_Max_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Max_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
										   lua_tostring(L, 2)));
	return 1;
}

/*!\details Returns the maximum number of bullets allowed in a weapon preset.
 *		  Same as `Get_Max_Total_Bullets` but exposed under a different name for
 *		  scripting convenience.
 *  \par Lua Command
 *		  max = Get_Max_Bullets(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to query.
 *  \return Number - Maximum total bullets (or `-1` if unknown).
 */
int Lua_Get_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
									  lua_tostring(L, 2)));
	return 1;
}

/*!\details Returns the total number of bullets remaining for a specific
 *		  weapon slot at a given position index (e.g. inventory slot).
 *  \par Lua Command
 *		  posTotal = Get_Position_Total_Bullets(Get_GameObj(10), 3)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \return Number - Total bullets for that slot (or `-1` if unknown).
 */
int Lua_Get_Position_Total_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Total_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
												 lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the number of bullets remaining in a specific slot.
 *  \par Lua Command
 *		  posBullets = Get_Position_Bullets(Get_GameObj(10), 3)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \return Number - Bullets remaining for that slot (or `-1` if unknown).
 */
int Lua_Get_Position_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
										   lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the clip bullets for a specific slot index.
 *  \par Lua Command
 *		  posClip = Get_Position_Clip_Bullets(Get_GameObj(10), 3)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \return Number - Clip bullets for that slot (or `-1` if unknown).
 */
int Lua_Get_Position_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
												lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the maximum total bullets for a specific slot index.
 *  \par Lua Command
 *		  posTotalMax = Get_Position_Total_Max_Bullets(Get_GameObj(10), 3)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \return Number - Maximum total bullets for that slot (or `-1` if unknown).
 */
int Lua_Get_Position_Total_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Total_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
													 lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the maximum clip bullets for a specific slot index.
 *  \par Lua Command
 *		  posClipMax = Get_Position_Clip_Max_Bullets(Get_GameObj(10), 3)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \return Number - Maximum clip bullets for that slot (or `-1` if unknown).
 */
int Lua_Get_Position_Clip_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Clip_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
													lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the maximum number of bullets allowed in a slot.
 *		  Alias for `Get_Position_Total_Max_Bullets`.
 *  \par Lua Command
 *		  posMax = Get_Position_Max_Bullets(Get_GameObj(10), 3)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \return Number - Maximum total bullets for that slot (or `-1` if unknown).
 */
int Lua_Get_Position_Max_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Max_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
											   lua_tonumber(L, 2)));
	return 1;
}

/*!\details Sets the number of bullets currently loaded in a weapon slot.
 *		  This is typically used to synchronise ammo counts after an event.
 *  \par Lua Command
 *		  Set_Current_Bullets(Get_GameObj(10), 45)
 *  \param ObjectID Number - ID of the target object.
 *  \param Count Number	- New bullet count for the active clip.
 */
int Lua_Set_Current_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Current_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
						lua_tonumber(L, 2));
	return 0;
}

/*!\details Sets the number of bullets in the active clip for an object.
 *  \par Lua Command
 *		  Set_Current_Clip_Bullets(Get_GameObj(10), 30)
 *  \param ObjectID Number - ID of the target object.
 *  \param Count Number	- New bullet count for the active clip.
 */
int Lua_Set_Current_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Current_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
							 lua_tonumber(L, 2));
	return 0;
}

/*!\details Sets the number of bullets in a specific slot’s clip.
 *  \par Lua Command
 *		  Set_Position_Clip_Bullets(Get_GameObj(10), 3, 25)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \param Count Number	- New bullet count for that slot’s clip.
 */
int Lua_Set_Position_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Position_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
							  lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}

/*!\details Sets the number of bullets for a particular weapon preset.
 *		  This is equivalent to `Set_Current_Bullets` but operates on an arbitrary
 *		  slot rather than the active one.
 *  \par Lua Command
 *		  Set_Bullets(Get_GameObj(10), "Weapon_StealthTank", 100)
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to modify.
 *  \param Count Number	- New bullet count for that preset.
 */
int Lua_Set_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
				lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}

/*!\details Sets the clip bullet count for a particular weapon preset.
 *  \par Lua Command
 *		  Set_Clip_Bullets(Get_GameObj(10), "Weapon_StealthTank", 30)
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to modify.
 *  \param Count Number	- New clip bullet count for that preset.
 */
int Lua_Set_Clip_Bullets(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Clip_Bullets(Commands->Find_Object(lua_tonumber(L, 1)),
					 lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}

/*!\details Returns the name of the weapon that powers a given power-up.
 *  \par Lua Command
 *		  wpn = Get_Powerup_Weapon("PowerUp_Stealth")
 *  \param PowerupName String - Name of the power-up preset.
 *  \return String - Weapon name (or `nil` if not found).
 */
int Lua_Get_Powerup_Weapon(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1) return 0;
	lua_pushstring(L, Get_Powerup_Weapon(lua_tostring(L, 1)));
	return 1;
}

/*!\details Returns the weapon associated with a power-up on a specific object.
 *  \par Lua Command
 *		  wpn = Get_Powerup_Weapon_By_Obj(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return String - Weapon name (or `nil` if not found).
 */
int Lua_Get_Powerup_Weapon_By_Obj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Powerup_Weapon_By_Obj(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the current weapon style index for an object.
 *		  A “style” is a variant of a weapon (e.g. scoped vs. unscoped).
 *  \par Lua Command
 *		  style = Get_Current_Weapon_Style(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return Number - Weapon style index (0-based) or `-1` if not applicable.
 */
int Lua_Get_Current_Weapon_Style(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Current_Weapon_Style(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the weapon style index for a specific slot position.
 *  \par Lua Command
 *		  style = Get_Position_Weapon_Style(Get_GameObj(10), 3)
 *  \param ObjectID Number - ID of the target object.
 *  \param PositionIndex Number - Index into the weapon/slot list.
 *  \return Number - Style index or `-1` if not applicable.
 */
int Get_Position_Weapon_Style(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_Position_Weapon_Style(Commands->Find_Object(lua_tonumber(L, 1)),
											   lua_tonumber(L, 2)));
	return 1;
}

/*!\details Returns the weapon style for a specific slot and preset name.
 *  \par Lua Command
 *		  style = Get_Weapon_Style(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to query.
 *  \return Number - Style index (or `-1` if not found).
 */
int Lua_Get_Weapon_Style(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	if (!lua_tostring(L, 1)) { return 0; }
	if (!Commands->Find_Object(lua_tonumber(L, 1))) { return 0; }
	if (!lua_tostring(L, 2)) { return 0; }

	lua_pushnumber(L, Get_Weapon_Style(Commands->Find_Object(lua_tonumber(L, 1)),
									   lua_tostring(L, 2)));
	return 1;
}

/*!\details Disarms a beacon object so it can no longer trigger attacks.
 *  \par Lua Command
 *		  Disarm_Beacon(Get_GameObj(10))
 *  \param ObjectID Number - ID of the beacon to disarm.
 */
int Lua_Disarm_Beacon(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_Beacon(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

/*!\details Disarms all beacons on a given team.
 *  \par Lua Command
 *		  Disarm_Beacons(2)
 *  \param Team Number - Index of the target team.
 */
int Lua_Disarm_Beacons(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_Beacons(lua_tonumber(L, 1));
	return 0;
}

/*!\details Disarms a C4 explosive so it will no longer detonate.
 *  \par Lua Command
 *		  Disarm_C4(Get_GameObj(10))
 *  \param ObjectID Number - ID of the C4 object to disarm.
 */
int Lua_Disarm_C4(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_C4(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

/*!\details Returns the name of the currently active weapon on an object.
 *  \par Lua Command
 *		  wpn = Get_Current_Weapon(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return String - Weapon name (or `nil` if no weapon).
 */
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

/*!\details Returns the total number of weapon slots a vehicle or unit possesses.
 *  \par Lua Command
 *		  count = Get_Weapon_Count(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return Number - Number of weapon slots (0 if none).
 */
int Lua_Get_Weapon_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Weapon_Count(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the name of a specific weapon slot on an object.
 *  \par Lua Command
 *		  wpn = Get_Weapon(Get_GameObj(10), 2)
 *  \param ObjectID Number - ID of the target object.
 *  \param SlotIndex Number - Zero-based index of the desired weapon slot.
 *  \return String - Weapon name (or `nil` if not present).
 */
int Lua_Get_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushstring(L, Get_Weapon(Commands->Find_Object(lua_tonumber(L, 1)),
								 lua_tonumber(L, 2)));
	return 1;
}

/*!\details Checks whether a particular weapon preset is present on an object.
 *  \par Lua Command
 *		  has = Has_Weapon(Get_GameObj(10), "Weapon_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param PresetName String - Weapon preset to search for.
 *  \return Boolean - `true` if the weapon is present; otherwise `false`.
 */
int Lua_Has_Weapon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Has_Weapon(Commands->Find_Object(lua_tonumber(L, 1)),
								 lua_tostring(L, 2)));
	return 1;
}

/*!\details Finds a beacon that matches the supplied parameters.
 *  \par Lua Command
 *		  id = Find_Beacon(Get_GameObj(10), 2)
 *  \param ObjectID Number - ID of the target object (used as a reference point).
 *  \param TeamNumber Number - Team index to restrict the search.
 *  \return Number - ID of the matching beacon or `0` if none found.
 */
int Lua_Find_Beacon(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Find_Beacon(lua_tonumber(L, 1), lua_tonumber(L, 2))));
	return 1;
}

/*!\details Returns the number of C4 devices owned by a player.
 *  \par Lua Command
 *		  count = Get_C4_Count(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object (player).
 *  \return Number - C4 count. If an extra argument is supplied, it will be used as
 *		  a filter index; otherwise only the first value is returned.
 */
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

/*!\details Returns the number of active beacons on a team.
 *  \par Lua Command
 *		  count = Get_Beacon_Count(TeamIndex)
 *  \param TeamNumber Number - Index of the target team.
 *  \return Number - Beacon count (0 if none).
 */
int Lua_Get_Beacon_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Beacon_Count(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Returns the current mine deployment limit for a player.
 *  \par Lua Command
 *		  limit = Get_Mine_Limit()
 *  \return Number - Mine limit value (typically 10 or 0 if mines are disabled).
 */
int Lua_Get_Mine_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Mine_Limit());
	return 1;
}

/*!\details Returns the current C4 mode setting for an object.
 *  \par Lua Command
 *		  mode = Get_C4_Mode(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target C4 or player object.
 *  \return Number - Mode code (0-based).
 */
int Lua_Get_C4_Mode(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_C4_Mode(Commands->Find_Object(lua_tonumber(L, 1))));
	return 1;
}

/*!\details Returns the ID of the player who planted a particular C4.
 *  \par Lua Command
 *		  planter = Get_C4_Planter(Get_GameObj(10))
 *  \param ObjectID Number - ID of the C4 object.
 *  \return Number - Player ID (or `0` if unknown).
 */
int Lua_Get_C4_Planter(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_C4_Planter(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Returns the ID of the object a C4 is currently attached to.
 *  \par Lua Command
 *		  attached = Get_C4_Attached(Get_GameObj(10))
 *  \param ObjectID Number - ID of the C4 object.
 *  \return Number - Attached object ID (or `0` if none).
 */
int Lua_Get_C4_Attached(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_C4_Attached(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Returns the ID of the player who planted a particular beacon.
 *  \par Lua Command
 *		  planter = Get_Beacon_Planter(Get_GameObj(10))
 *  \param ObjectID Number - ID of the beacon object.
 *  \return Number - Player ID (or `0` if unknown).
 */
int Lua_Get_Beacon_Planter(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_ID(Get_Beacon_Planter(Commands->Find_Object(lua_tonumber(L, 1)))));
	return 1;
}

/*!\details Creates a new object of the specified preset at a given position.
 *  \par Lua Command
 *		  id = Create_Object("Vehicle_StealthTank",
 *							{x=100,y=200,z=0})
 *  \param PresetName String - Name of the object preset to spawn.
 *  \param Position Table - Lua table containing `x`, `y` and `z` coordinates.
 *  \return Number - ID of the created object or `nil` if creation failed.
 */
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

/*!\details Destroys an existing game object.
 *  \par Lua Command
 *		  Destroy_Object(Get_GameObj(10))
 *  \param ObjectID Number - ID of the object to remove from the world.
 */
int Lua_Destroy_Object(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Destroy_Object(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

/*!\details Retrieves the preset name of a game object.
 *  \par Lua Command
 *		  name = Get_Preset_Name(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \return String - Preset name (or `nil` if not found).
 */
int Lua_Get_Preset_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	const char *ptr = Commands->Get_Preset_Name(Commands->Find_Object(lua_tonumber(L, 1)));
	if (!ptr) return 0;
	lua_pushstring(L, ptr);
	return 1;
}

/*!\details Returns the world position of an object.
 *		  The second optional argument forces a query against the physics scene
 *		  rather than the command system (useful for static objects).
 *  \par Lua Command
 *		  pos = Get_Position(Get_GameObj(10))
 *  \param ObjectID Number - ID of the target object.
 *  \param usePhysics Boolean (optional) - `true` to query physics, otherwise `false`.
 *  \return Table - Lua table with fields `x`, `y`, and `z`.
 */
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

/*!\details Enables or disables automatic vehicle transition for a specific object.
 *		  This is typically used to control whether a vehicle is accessible by players.
 *  \par Lua Command
 *  \code{.lua}
 *      Enable_Vehicle_Transitions(Get_GameObj(10), true)
 *  \endcode
 *  \param ObjectID Number - ID of the target vehicle or unit.
 *  \param enable Boolean - `true` to allow transitions; `false` to disable.
 */
int Lua_Enable_Vehicle_Transitions(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Enable_Vehicle_Transitions(Commands->Find_Object(lua_tonumber(L, 1)),
										 lua_tobooleanCPP(L, 2));
	return 0;
}

/*!\details Sets the visual model of a game object.
 *  \par Lua Command
 *		  Set_Model(Get_GameObj(10), "Vehicle_StealthTank")
 *  \param ObjectID Number - ID of the target object.
 *  \param ModelName String - Name of the new model preset.
 */
int Lua_Set_Model(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Set_Model(Commands->Find_Object(lua_tonumber(L, 1)),
						lua_tostring(L, 2));
	return 0;
}

/*!\details Moves a unit to the specified coordinates.
 *  \par Lua Command
 *  \code{.lua}
 *      Goto_Location(Get_GameObj(10), 12.5, -4.0, 18.2, 0.8, 3, false)
 *  \endcode
 *  \param ObjectID Number - ID of the object that should move.
 *  \param X float - Target X coordinate.
 *  \param Y float - Target Y coordinate.
 *  \param Z float - Target Z coordinate (height).
 *  \param Speed float - Movement speed. Values > 0 will use the specified speed; a value of 0 uses default speed.
 *  \param RotationSpeed float - Speed at which the object rotates to face the target direction.
 *  \param Immediate boolean - If `true`, the move command is issued immediately, bypassing any current actions.
 *
 *  The function sets up an `ActionParamsStruct` with movement data and queues it via
 *  `Commands->Action_Goto`.  The parameters correspond directly to those used by
 *  the in-game "Goto" action (see `ActionParamsStruct::Set_Movement`).  
 */
int Lua_Goto_Location(lua_State *L)
{
	if (lua_gettop(L) < 7) return 0;
	ActionParamsStruct aps;
	aps.Set_Movement(Vector3(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4)), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tobooleanCPP(L, 7));
	Commands->Action_Goto(Commands->Find_Object(lua_tonumber(L, 1)), aps);
	return 0;
}

/*!\details Makes an object move towards a target object.
 *  \par Lua Command
 *      Goto_Object(Get_GameObj(10), Get_GameObj(20), 0.8, 3)
 *  \endcode
 *  \param ObjectID Number - ID of the object that should move.
 *  \param TargetObjectID Number - ID of the target object to follow or approach.
 *  \param Speed float - Movement speed. Values > 0 will use the specified speed; a value of 0 uses default speed.
 *  \param RotationSpeed float - Speed at which the object rotates to face the movement direction.
 *
 *  This function constructs an `ActionParamsStruct` with movement data that targets another
 *  object instead of a static location.  It is equivalent to scripting a "Goto" action
 *  toward a moving target.  
 */
int Lua_Goto_Object(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	ActionParamsStruct aps;
	aps.Set_Movement(Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3), lua_tonumber(L, 4));
	Commands->Action_Goto(Commands->Find_Object(lua_tonumber(L, 1)), aps);
	return 0;
}

/*!\details Disables all physical collisions for a specific object.
 *  \par Lua Command
 *      Disable_Physical_Collisions(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the object to freeze.
 */
int Lua_Disable_Physical_Collisions(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Disable_Physical_Collisions(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

/*!\details Enables physical collisions for a specific object.
 *  \par Lua Command
 *      Enable_Collisions(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the object to enable collisions for.
 *
 *  The function forwards this argument to `Commands->Enable_Collisions`.  
 */
int Lua_Enable_Collisions(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Enable_Collisions(Commands->Find_Object(lua_tonumber(L, 1)));
	return 0;
}

/*!\details Returns a randomly chosen building of the specified team and type.
 *  \par Lua Command
 *      id = Random_Building()
 *  \endcode
 *
 *  The function selects a random building from one of two team types (Nod or GDI) until a valid
 *  object is found, then pushes its numeric ID onto the Lua stack.  
 */
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

/*!\details Returns the name of the current map.
 *  \par Lua Command
 *      mapName = Get_Current_Map()
 *  \endcode
 *
 *  The function forwards this call to `The_Game()->Get_Map_Name()`.  
 */
int Lua_Get_Current_Map(lua_State *L)
{
	lua_pushstring(L, The_Game()->Get_Map_Name());
	return 1;
}

/*!\details Returns the name of the next map in the cycle.
 *  \par Lua Command
 *	  nextMap = Get_Next_Map()
 *  \endcode
 *
 *  If the requested index is out of bounds the function falls back to the first map.  
 */
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

/*!\details Sets the world position of an object.
 *  \par Lua Command
 *	  Set_Position(Get_GameObj(10), Vector3(12.0, 0.0, 18.5))
 *  \endcode
 *  \param ObjectID Number - ID of the object whose position should be set.
 *  \param Position Table - Lua vector table containing `x`, `y`, and `z`.
 *
 *  The function converts a Lua vector into a `Vector3` and calls `Commands->Set_Position`.  
 */
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

/*!\details Applies damage to an object.
 *  \par Lua Command
 *	  Apply_Damage(Get_GameObj(10), 50.0, "Death", Get_GameObj(20))
 *  \endcode
 *  \param ObjectID Number - ID of the object to be damaged.
 *  \param Amount Number - Damage amount (float).
 *  \param Type String - Warhead type (e.g., `"Death"`).
 *  \param SourceID Number (optional) - ID of the damaging object. If omitted, damage is un-originated.
 *
 *  The function forwards these arguments to `Commands->Apply_Damage`.  
 */
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

/*!\details Returns the facing direction of an object.
 *  \par Lua Command
 *	  f = Get_Facing(Get_GameObj(10))
 *  \endcode
 *
 *  If a second boolean argument `true` is supplied the function returns the world-space facing
 *  from the underlying physics object (`StaticObject`).  
 */
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

/*!\details Sets the facing direction of an object.
 *  \par Lua Command
 *	  Set_Facing(Get_GameObj(10), 90.0)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Set_Facing`.  
 */
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

/*!\details Sets the global cloud coverage.
 *  \par Lua Command
 *	  Set_Clouds(0.3, 10.0, 20.0)
 *  \endcode
 *
 *  \param CloudCover Number - Overall cloud density (0 - 1).
 *  \param StartDistance Number - Distance at which clouds start to appear.
 *  \param EndDistance Number - Distance at which clouds become fully opaque.
 *
 *  These values affect the atmospheric rendering.  
 */
int Lua_Set_Clouds(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Set_Clouds(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}

/*!\details Sets the global ash effect.
 *  \par Lua Command
 *	  Set_Ash(5.0, 10.0, true)
 *  \endcode
 *
 *  \param Size Number - Scale of the ash particles.
 *  \param Distance Number - Distance at which ash becomes visible.
 *  \param Enable Boolean - `true` to enable ash; `false` to disable.
 */
int Lua_Set_Ash(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	bool enable = (lua_tonumber(L, 3) > 0 ? 1 : 0);
	Commands->Set_Ash(lua_tonumber(L, 1), lua_tonumber(L, 2), enable);
	return 0;
}

/*!\details Sets the global rain effect.
 *  \par Lua Command
 *	  Set_Rain(5.0, 10.0, true)
 *  \endcode
 *
 *  \param Size Number - Scale of the rain particles.
 *  \param Distance Number - Distance at which rain becomes visible.
 *  \param Enable Boolean - `true` to enable rain; `false` to disable.
 */
int Lua_Set_Rain(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	bool enable = (lua_tonumber(L, 3) > 0 ? 1 : 0);
	Commands->Set_Rain(lua_tonumber(L, 1), lua_tonumber(L, 2), enable);
	return 0;
}

/*!\details Sets the global snow effect.
 *  \par Lua Command
 *	  Set_Snow(5.0, 10.0, true)
 *  \endcode
 *
 *  \param Size Number - Scale of the snow particles.
 *  \param Distance Number - Distance at which snow becomes visible.
 *  \param Enable Boolean - `true` to enable snow; `false` to disable.
 */
int Lua_Set_Snow(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	bool enable = (lua_tonumber(L, 3) > 0 ? 1 : 0);
	Commands->Set_Snow(lua_tonumber(L, 1), lua_tonumber(L, 2), enable);
	return 0;
}

/*!\details Sets the global wind direction and speed.
 *  \par Lua Command
 *	  Set_Wind(0.5, 0.0, 0.2, 100.0)
 *  \endcode
 *
 *  \param X Number - Wind vector X component.
 *  \param Y Number - Wind vector Y component.
 *  \param Z Number - Wind vector Z component.
 *  \param Speed Number - Overall wind speed.
 */
int Lua_Set_Wind(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Commands->Set_Wind(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}

/*!\details Attaches a Lua script to an object.
 *  \par Lua Command
 *	  Attach_Script(Get_GameObj(10), "MyScript", "param1=foo,param2=bar")
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Attach_Script`.  
 */
int Lua_Attach_Script(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Attach_Script(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3));
	return 0;
}

/*!\details Creates a new instance of the named preset and attaches it to a specific bone.
 *  \par Lua Command
 *	  id = Create_Object_At_Bone(Get_GameObj(10), "Particle", "Arm")
 *  \endcode
 *
 *  The function returns the numeric ID of the newly spawned object.  
 */
int Lua_Create_Object_At_Bone(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	lua_pushnumber(L, Commands->Get_ID(Commands->Create_Object_At_Bone(
					 Commands->Find_Object(lua_tonumber(L, 1)),
					 lua_tostring(L, 2),
					 lua_tostring(L, 3))));
	return 1;
}

/*!\details Attaches an object to a bone on another object.
 *  \par Lua Command
 *	  Attach_To_Object_Bone(Get_GameObj(10), Get_GameObj(20), "Arm")
 *  \endcode
 *
 *  The function forwards parameters to `Commands->Attach_To_Object_Bone`.  
 */
int Lua_Attach_To_Object_Bone(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Attach_To_Object_Bone(Commands->Find_Object(lua_tonumber(L, 1)),
								   Commands->Find_Object(lua_tonumber(L, 2)),
								   lua_tostring(L, 3));
	return 0;
}

/*!\details Displays or hides a health bar for an object.
 *  \par Lua Command
 *	  Display_Health_Bar(Get_GameObj(10), true)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Display_Health_Bar`.  
 */
int Lua_Display_Health_Bar(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Display_Health_Bar(Commands->Find_Object(lua_tonumber(L, 1)),
								 lua_tobooleanCPP(L, 2));
	return 0;
}

/*!\details Creates a script zone with the specified preset and bounding box.
 *  \par Lua Command
 *	  id = Create_Script_Zone("AlertZone", {x=10, y=0, z=20}, {x=5, y=0, z=5})
 *  \endcode
 *
 *  If only a single table argument is supplied the function interprets it as an existing
 *  `OBBoxClass` instance (via a Lua vector wrapper).  
 */
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

/*!\details Returns the current system clock tick count.
 *  \par Lua Command
 *	  t = tClock()
 *  \endcode
 *
 *  Useful for timing events in scripts.  
 */
int Lua_tClock(lua_State *L)
{
	lua_pushnumber(L, clock());
	return 1;
}

/*!\details Calls a registered Lua invoke hook.
 *  \par Lua Command
 *	  Invoke("HookName", "parameter")
 *  \endcode
 *
 *  The function forwards these arguments to `LuaManager::CallInvoke`.  
 */
int Lua_Invoke(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;

	LuaManager::CallInvoke(lua_tostring(L, 1), lua_tostring(L, 2));
	return 0;
}

/*!\details Enables or disables stealth for a specific object.
 *  \par Lua Command
 *	  Enable_Stealth(Get_GameObj(10), true)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Enable_Stealth`.  
 */
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

/*!\details Returns all currently existing objects as a Lua table.
 *  \par Lua Command
 *	  objs = Get_All_Objects()
 *  \endcode
 *
 *  The returned table contains the numeric IDs of every object in the game.  
 */
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

/*!\details Sets the shield type of an object.
 *  \par Lua Command
 *	  Set_Shield_Type(Get_GameObj(10), "Plasma")
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Set_Shield_Type`.  
 */
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

/*!\details Displays a translated message on the screen.
 *  \par Lua Command
 *	  FDSMessage("GameStarted", "GDI")
 *  \endcode
 *
 *  The function forwards these arguments to `SSGMGameLog::Log_Message`.  
 */
int Lua_FDSMessage(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	const char *m = luaL_checkstring(L, 1);
	const char *h = luaL_checkstring(L, 2);
	SSGMGameLog::Log_Message(m, h);
	return 0;
}

/*!\details Sets an object's health.
 *  \par Lua Command
 *	  Set_Health(Get_GameObj(10), 75)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Set_Health`.  
 */
int Lua_Set_Health(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Commands->Set_Health(obj, luaL_checknumber(L, 2));
	return 0;
}

/*!\details Sets an object's shield strength.
 *  \par Lua Command
 *	  Set_Shield_Strength(Get_GameObj(10), 50)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Set_Shield_Strength`.  
 */
int Lua_Set_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Commands->Set_Shield_Strength(obj, luaL_checknumber(L, 2));
	return 0;
}

/*!\details Returns C&C game specific data as a Lua table.
 *  \par Lua Command
 *	  cnc = The_Cnc_Game()
 *  \endcode
 *
 *  The returned table contains booleans for base-destruction rules and the starting credit amount.  
 */
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

/*!\details Returns general game data as a Lua table.
 *  \par Lua Command
 *	  game = The_Game()
 *  \endcode
 *
 *  The returned table contains flags for repair permissions, map looping, timers, player counts,
 *  and many other properties.  
 */
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

/*!\details Returns a table of player statistics for a given ID.
 *  \par Lua Command
 *	  p = cPlayer(12)
 *  \endcode
 *
 *  The returned table contains many fields (health, kills, score, etc.) that can be accessed from Lua.  
 */
int Lua_cPlayer(lua_State *L)
{
	int ID = luaL_checknumber(L, 1);
	if (ID == 0)
		return 0;

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
		lua_pushboolean(L, p->Get_Is_Active());
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
#ifdef DAPLUGIN
		lua_pushnumber(L, p->Fps);
		lua_setfield(L, -2, "Fps");
#endif
		return 1;
	}
	return 0;
}

/*!\details Converts a numeric IP address to four octets.
 *  \par Lua Command
 *	  ip = LongToIP(3232235777)   -- returns 192, 168, 1, 1
 *  \endcode
 *
 *  The function splits the 32-bit integer into four bytes and pushes them onto the stack.  
 */
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

/*!\details Converts four octets into a numeric IP address.
 *  \par Lua Command
 *	  long = IPToLong(192,168,1,1)   -- returns 3232235777
 *  \endcode
 *
 *  The function combines the four bytes into a single 32-bit integer and pushes it as a string.  
 */
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

/*!\details Returns the current game time in seconds.
 *  \par Lua Command
 *	  t = GetTimeRemaining()
 *  \endcode
 *
 *  If the game is not time-limited this function returns `-1`.  
 */
int Lua_GetTimeRemaining(lua_State *L)
{
	lua_pushnumber(L, The_Game()->Get_Time_Remaining_Seconds());
	return 1;
}

/*!\details Returns the current health of an object.
 *  \par Lua Command
 *	  h = Get_Health(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Get_Health`.  
 */
int Lua_Get_Health(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Health(x));
	return 1;
}

/*!\details Returns the maximum health of an object.
 *  \par Lua Command
 *	  hmax = Get_Max_Health(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Get_Max_Health`.  
 */
int Lua_Get_Max_Health(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Max_Health(x));
	return 1;
}

/*!\details Returns the current shield strength of an object.
 *  \par Lua Command
 *	  s = Get_Shield_Strength(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Get_Shield_Strength`.  
 */
int Lua_Get_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Shield_Strength(x));
	return 1;
}

/*!\details Returns the maximum shield strength of an object.
 *  \par Lua Command
 *	  smax = Get_Max_Shield_Strength(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Get_Max_Shield_Strength`.  
 */
int Lua_Get_Max_Shield_Strength(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;

	lua_pushnumber(L, Commands->Get_Max_Shield_Strength(x));
	return 1;
}

/*!\details Displays the GDI terminal UI for a specified player.
 *  \par Lua Command
 *	  Display_GDI_Player_Terminal_Player(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Display_GDI_Player_Terminal_Player`.  
 */
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

/*!\details Displays the NOD terminal UI for a specified player.
 *  \par Lua Command
 *	  Display_Nod_Player_Terminal_Player(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Display_NOD_Player_Terminal_Player`.  
 */
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

/*!\details Returns the Lua ID of a game object.
 *  \par Lua Command
 *	  id = Get_IDobj(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Get_ID`.  
 */
int Lua_Get_IDobj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *x = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!x) return 0;
	int n = Commands->Get_ID(x);
	lua_pushnumber(L, n);
	return 1;
}

/*!\details Retrieves the map name by its numeric index.
 *  \par Lua Command
 *	  map = Get_Map_By_Number(3)
 *  \endcode
 *
 *  If an invalid index is supplied the function returns `"NULL"`.  
 */
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

/*!\details Places the player into spectator mode.
 *  \par Lua Command
 *	  Spectate(Get_GameObj(10))
 *  \endcode
 *
 *  The function moves the specified unit out of combat, removes its weapons,
 *  and places it in a hidden invisible state.  
 */
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

/*!\details Sets the background music track for all players.
 *  \par Lua Command
 *	  Set_Background_Music("Ambience")
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Set_Background_Music`.  
 */
int Lua_Set_Background_Music(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Commands->Set_Background_Music(lua_tostring(L, 1));
	return 1;
}

/*!\details Returns the current background music track name.
 *  \par Lua Command
 *	  current = Get_Background_Music()
 *  \endcode
 *
 *  The function forwards this call to `GetCurrentMusicTrack`.  
 */
int Lua_Get_Background_Music(lua_State *L)
{
	lua_pushstring(L, GetCurrentMusicTrack());
	return 1;
}

/*!\details Kills a player object and any vehicle they may be riding.
 *  \par Lua Command
 *	  Kill_Player(Get_GameObj(10))
 *  \endcode
 *
 *  The function first checks if the object is inside a vehicle, kills both the vehicle (if owned)
 *  and the player, then applies lethal damage.  
 */
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

/*!\details Enables or disables player control for a specific object.
 *  \par Lua Command
 *	  Control_Enable(Get_GameObj(10), true)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Control_Enable`.  
 */
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

/*!\details Returns the bandwidth usage of a client.
 *  \par Lua Command
 *	  bw = Get_BW_Player(12)
 *  \endcode
 *
 *  The function forwards this argument to `Get_Bandwidth`.  
 */
int Lua_Get_BW_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Get_Bandwidth(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Retrieves the global vehicle limit.
 *  \par Lua Command
 *	  max = Get_Vehicles_Limit()
 *  \endcode
 *
 *  The function forwards this call to `Get_Vehicle_Limit`.  
 */
int Lua_Get_Vehicles_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Vehicle_Limit());
	return 1;
}

/*!\details Returns the client version for a specific player.
 *  \par Lua Command
 *	  ver = Get_Player_Version(12)
 *  \endcode
 *
 *  The function forwards this argument to `Get_Client_Version`.  
 */
int Lua_Get_Player_Version(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Client_Version(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Kicks a client from the server.
 *  \par Lua Command
 *	  Kick_Player(12, "Violation")
 *  \endcode
 *
 *  The function forwards these arguments to `Evict_Client`.  
 */
int Lua_Kick_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Evict_Client(lua_tonumber(L, 1), lua_tostring(L, 2));
	return 1;
}

/*!\details Adds a console output hook for custom logging.
 *  \par Lua Command
 *	  Add_Console_Hook()
 *  \endcode
 *
 *  The function registers `LuaManager::Call_Console_Output_Hook` and `LuaManager::Call_Host_Hook`.  
 */
int Lua_Add_Console_Hook(lua_State *L)
{
	AddConsoleOutputHook(LuaManager::Call_Console_Output_Hook);
	AddHostHook(LuaManager::Call_Host_Hook);
	return 1;
}

/*!\details Removes a weapon from an object.
 *  \par Lua Command
 *	  Remove_Weapon(Get_GameObj(10), "C4")
 *  \endcode
 *
 *  The function forwards these arguments to `Remove_Weapon`.  
 */
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

/*!\details Removes all weapons from an object.
 *  \par Lua Command
 *	  Remove_All_Weapons(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Clear_Weapons`.  
 */
int Lua_Remove_All_Weapons(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Clear_Weapons(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Retrieves the translated name of a preset by its definition ID.
 *  \par Lua Command
 *	  name = Get_Preset_Name_By_Preset_ID(Get_Definition_ID("Vehicle_StealthTank"))
 *  \endcode
 *
 *  The function forwards this call to `Get_Translated_Definition_Name_Ini`.  
 */
int Lua_Get_Preset_Name_By_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, Get_Translated_Definition_Name_Ini(Get_Definition_Name(luaL_checknumber(L, 1))));
	return 1;
}

/*!\details Returns the client serial hash string for a player.
 *  \par Lua Command
 *	  hash = Get_Client_Serial_Hash(12)
 *  \endcode
 *
 *  The function forwards this argument to `Get_Client_Serial_Hash`.  
 */
int Lua_Get_Client_Serial_Hash(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	int pID = luaL_checknumber(L, 1);
	if (!pID) { return 0; }
	lua_pushstring(L, Get_Client_Serial_Hash(pID));
	return 1;
}

/*!\details Creates an explosion at a location attached to a bone.
 *  \par Lua Command
 *	  Create_Explosion("Fire", Vector3(10,0,20), Get_GameObj(30))
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Create_Explosion`.  
 */
int Lua_Create_Explosion(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;

	LUA_Vector3 *a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 3));

	Commands->Create_Explosion(lua_tostring(L, 1), pos, obj);
	return 1;
}

/*!\details Creates an explosion at a location attached to a bone.
 *  \par Lua Command
 *	  Create_Explosion_At_Bone("Fire", Get_GameObj(10), "Arm")
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Create_Explosion_At_Bone`.  
 */
int Lua_Create_Explosion_At_Bone(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 2));
	GameObject *creator = Commands->Find_Object(lua_tonumber(L, 4));
	Commands->Create_Explosion_At_Bone(lua_tostring(L, 1), obj, lua_tostring(L, 3), creator);
	return 1;
}

/*!\details Enables or disables fog effect for the current player.
 *  \par Lua Command
 *	  Set_Fog_Enable(true)
 *  \endcode
 *
 *  The function forwards this argument to `Set_Fog_Enable`.  
 */
int Lua_Set_Fog_Enable(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	int stealth = luaL_checknumber(L, 1);
	Commands->Set_Fog_Enable(stealth == 0 ? false : true);
	return 1;
}

/*!\details Sets the fog range and transition for the current player.
 *  \par Lua Command
 *	  Set_Fog_Range(0.3, 10.0, 2.0)
 *  \endcode
 *
 *  The function forwards these arguments to `Set_Fog_Range`.  
 */
int Lua_Set_Fog_Range(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Set_Fog_Range(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 1;
}

/*!\details Sets the global war blitz parameters.
 *  \par Lua Command
 *	  Set_War_Blitz(0.5, 1.0, 10, 100, 200, true)
 *  \endcode
 *
 *  The function forwards these arguments to `Set_War_Blitz`.  
 */
int Lua_Set_War_Blitz(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	Commands->Set_War_Blitz(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3),
							lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6));
	return 1;
}

/*!\details Plays a building announcement at the specified index.
 *  \par Lua Command
 *	  Play_Building_Announcement(Get_GameObj(10), 2)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Play_Building_Announcement`.  
 */
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

/*!\details Shakes the camera at a given position with specified parameters.
 *  \par Lua Command
 *	  Shake_Camera(Vector3(10,0,20), 5.0, 1.5, 2)
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Shake_Camera`.  
 */
int Lua_Shake_Camera(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());

	Commands->Shake_Camera(pos, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 1;
}

/*!\details Attaches a radio hook for in-game communications.
 *  \par Lua Command
 *	  Add_RadioHook()
 *  \endcode
 *
 *  The function registers `LuaManager::RadioHook` as the global radio callback.  
 */
int Lua_Add_RadioHook(lua_State *L)
{
	AddRadioHook(LuaManager::RadioHook);
	return 1;
}

/*!\details Checks whether a unit is a crate (used by certain weapons).
 *  \par Lua Command
 *	  isCrate = Is_Crate(Get_GameObj(10))
 *  \endcode
 *
 *  The function returns `true` if the object is a power-up preset containing the string `"crate"`.  
 */
int Lua_Is_Crate(lua_State *L) 
{
	if (lua_gettop(L) < 1) return 0;
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

/*!\details Sets the maximum number of air vehicles for a team.
 *  \par Lua Command
 *	  Set_Air_Vehicle_Limit(5)
 *  \endcode
 *
 *  The function forwards this argument to `Set_Air_Vehicle_Limit`.  
 */
int Lua_Set_Air_Vehicle_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Set_Air_Vehicle_Limit(lua_tonumber(L, 1));
	return 1;
}

/*!\details Retrieves the maximum number of air vehicles for a team.
 *  \par Lua Command
 *	  limit = Get_Air_Vehicle_Limit()
 *  \endcode
 *
 *  The function forwards this call to `Get_Air_Vehicle_Limit`.  
 */
int Lua_Get_Air_Vehicle_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Air_Vehicle_Limit());
	return 1;
}

/*!\details Sets the maximum number of vehicles for a team.
 *  \par Lua Command
 *	  Set_Vehicle_Limit(10)
 *  \endcode
 *
 *  The function forwards this argument to `Set_Vehicle_Limit`.  
 */
int Lua_Set_Vehicle_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Set_Vehicle_Limit(lua_tonumber(L, 1));
	return 1;
}

/*!\details Retrieves the maximum number of vehicles for a team.
 *  \par Lua Command
 *	  limit = Get_Vehicle_Limit()
 *  \endcode
 *
 *  The function forwards this call to `Get_Vehicle_Limit`.  
 */
int Lua_Get_Vehicle_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Vehicle_Limit());
	return 1;
}

/*!\details Makes the camera look at a specified position for a player.
 *  \par Lua Command
 *	  Force_Camera_Look_Player(Get_GameObj(10), Vector3(15,0,30))
 *  \endcode
 *
 *  The function forwards these arguments to `Force_Camera_Look_Player`.  
 */
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

/*!\details Sets the screen fade opacity for a player.
 *  \par Lua Command
 *	  Set_Screen_Fade_Opacity_Player(Get_GameObj(10), 0.5, 2)
 *  \endcode
 *
 *  The function forwards these arguments to `Set_Screen_Fade_Opacity_Player`.  
 */
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

/*!\details Sets the screen fade color for a player.
 *  \par Lua Command
 *	  Set_Screen_Fade_Color_Player(Get_GameObj(10), 255, 0, 0, 1)
 *  \endcode
 *
 *  The function forwards these arguments to `Set_Screen_Fade_Color_Player`.  
 */
int Lua_Set_Screen_Fade_Color_Player(lua_State *L)
{
	if (lua_gettop(L) == 5)
	{
		GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
		if (obj)
		{
			Set_Screen_Fade_Color_Player(obj,
										 luaL_checknumber(L, 2),
										 luaL_checknumber(L, 3),
										 luaL_checknumber(L, 4),
										 luaL_checknumber(L, 5));
		}
	}
	return 0;
}

/*!\details Enables or disables radar for a player.
 *  \par Lua Command
 *	  Enable_Radar_Player(Get_GameObj(10), true)
 *  \endcode
 *
 *  The function forwards these arguments to `Enable_Radar_Player`.  
 */
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

/*!\details Returns the build-time multiplier for a given team.
 *  \par Lua Command
 *	  mult = Get_Build_Time_Multiplier(1)
 *  \endcode
 *
 *  The function forwards this argument to `Get_Build_Time_Multiplier`.  
 */
int Lua_Get_Build_Time_Multiplier(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Get_Build_Time_Multiplier(lua_tonumber(L, 1)));
	return 1;
}

/*!\details Stops background music for a specific player.
 *  \par Lua Command
 *	  Stop_Background_Music_Player(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Stop_Background_Music_Player`.  
 */
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

/*!\details Changes the remaining time in the current map.
 *  \par Lua Command
 *	  Change_Time_Remaining(120)
 *  \endcode
 *
 *  The function forwards this argument to `Change_Time_Remaining`.  
 */
int Lua_Change_Time_Remaining(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Change_Time_Remaining(lua_tonumber(L, 1));
	return 1;
}

/*!\details Changes the time limit for the current map.
 *  \par Lua Command
 *	  Change_Time_Limit(300)
 *  \endcode
 *
 *  The function forwards this argument to `Change_Time_Limit`.  
 */
int Lua_Change_Time_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	Change_Time_Limit(lua_tonumber(L, 1));
	return 1;
}

/*!\details Creates a 3-D WAV sound at a specific bone.
 *  \par Lua Command
 *	  Create_3D_WAV_Sound_At_Bone("Explosion", Get_GameObj(10), "Arm")
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Create_3D_WAV_Sound_At_Bone`.  
 */
int Lua_Create_3D_WAV_Sound_At_Bone(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	lua_pushnumber(L, Commands->Create_3D_WAV_Sound_At_Bone(lua_tostring(L, 1), Commands->Find_Object(lua_tonumber(L, 2)), lua_tostring(L, 3)));
	return 1;
}

/*!\details Sends a message to all players or the current client.
 *  \par Lua Command
 *	  Send_Message(12, 255, 0, 0, "Hello")
 *  \endcode
 *
 *  The function forwards these arguments to `Send_Message`.  
 */
int Lua_Send_Message(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;

	Send_Message(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tostring(L, 4));
	return 1;
}

/*!\details Selects a weapon on an object.
 *  \par Lua Command
 *	  Select_Weapon(Get_GameObj(10), "Rifle")
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Select_Weapon`.  
 */
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

/*!\details Creates a 2-D sound attached to a player.
 *  \par Lua Command
 *	  id = Create_2D_Sound_Player(Get_GameObj(10), "Explosion")
 *  \endcode
 *
 *  The function forwards these arguments to `Create_2D_Sound_Player`.  
 */
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

/*!\details Creates a global 2-D WAV sound.
 *  \par Lua Command
 *	  id = Create_2D_Sound("Explosion")
 *  \endcode
 *
 *  The function forwards this argument to `Commands->Create_2D_WAV_Sound`.  
 */
int Lua_Create_2D_Sound(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Commands->Create_2D_WAV_Sound(lua_tostring(L, 1)));
	return 1;
}

/*!\details Resolves a host name to an IP address.
 *  \par Lua Command
 *	  ip = Resolve_IP("127.0.0.1")
 *  \endcode
 *
 *  The function forwards this argument to `gethostbyaddr` or `gethostbyname`.  
 */
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

/*!\details Returns the current build version.
 *  \par Lua Command
 *	  ver = Get_Build_Version()
 *  \endcode
 *
 *  The function currently returns `1`.  
 */
int Lua_Get_Build_Version(lua_State *L)
{
	lua_pushnumber(L, 1);
	return 1;
}

/*!\details Computes the distance between two positions.
 *  \par Lua Command
 *	  dist = Get_Distance(Vector3(0,0,0), Vector3(10,5,2))
 *  \endcode
 *
 *  The function forwards these arguments to `Commands->Get_Distance`.  
 */
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

/*!\details Returns the list of files in a directory as a Lua table.
 *  \par Lua Command
 *	  files = Get_File_List("Data\\")
 *  \endcode
 *
 *  The function uses `GetCurrentDirectory` and `GetFiles`.  
 */
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

/*!\details Reloads all Lua scripts.
 *  \par Lua Command
 *	  Reload_Lua()
 *  \endcode
 *
 *  The function iterates over every loaded state, calls `UnloadingLua` if defined,
 *  and sets a global reload flag.  
 */
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

/*!\details Sets an animation on an object.
 *  \par Lua Command
 *	  Set_Animation(Get_GameObj(10), "Walk", true, "", 0.0, 2.0, false)
 *  \endcode
 *
 *  \param ObjectID Number - ID of the object to animate.
 *  \param AnimationName String - Name of the animation preset.
 *  \param Loop Boolean - `true` if the animation should loop.
 *  \param Subobject String - Optional sub-object name (e.g., a weapon). Empty string disables it.
 *  \param StartFrame Number - Frame to start at.
 *  \param EndFrame Number - Frame to end at.
 *  \param Blend Boolean - `true` to blend the animation in/out smoothly.
 *
 *  The function forwards these arguments to `Commands->Set_Animation`.  
 */
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

/*!\details Adds a TTDamage hook.
 *  \par Lua Command
 *	  Add_TTDamageHook()
 *  \endcode
 *
 *  The function registers `LuaManager::TT_Damage_Hook` with the engine.  
 */
int Lua_Add_TTDamageHook(lua_State *L)
{
	AddTtDamageHook(LuaManager::TT_Damage_Hook);
	return 1;
}

/*!\details Restores a building to life.
 *  \par Lua Command
 *	  Restore_Building(Get_GameObj(10))
 *  \endcode
 *
 *  The function forwards this argument to `Restore_Building`.  
 */
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

/*!\details Revives a building that was destroyed.
 *  \par Lua Command
 *	  Revive_Building(Get_GameObj(10))
 *  \endcode
 *
 *  The function restores the building, refills its health to maximum, and sets power status.  
 */
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

/*!\details Creates a building at a given position.
 *  \par Lua Command
 *	  id = Create_Building("Barracks", Vector3(10,0,20))
 *  \endcode
 *
 *  The function forwards these arguments to `Create_Building` and returns the new object's ID.  
 */
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

/*!\details Damages all occupants of a vehicle.
 *  \par Lua Command
 *	  Damage_Occupants(Get_GameObj(10), 50, "Fire")
 *  \endcode
 *
 *  The function forwards these arguments to `Damage_Occupants`.  
 */
int Lua_Damage_Occupants(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	int ob = lua_tonumber(L, 1);
	Damage_Occupants(Commands->Find_Object(ob), lua_tonumber(L, 2), lua_tostring(L, 3));
	return 1;
}

/*!\details Sets the kill count for a player.
 *  \par Lua Command
 *	  Set_Kills(12, 5)
 *  \endcode
 *
 *  The function forwards these arguments to `Set_Kills`.  
 */
int Lua_Set_Kills(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Kills(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 1;
}

/*!\details Sets a player's detailed statistics and cosmetic information using data from a Lua table.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_cPlayer(12, {Name="Admin", PlayerId=12, Kills=99, Deaths=0, PlayerType=1})
 *  \endcode
 *  \param PlayerID Number - ID of the target player object.
 *  \param Data Table - A Lua table containing key-value pairs for stat updates:
 *      Name (String) - New name to set for the player.
 *      PlayerId (Number) - The numerical unique ID for the player.
 *      Kills/Deaths (Number) - Current kill count or death count.
 *      PlayerType (Number) - Defines the type of player model/stats used.
 *      AlliesKilled, ArmHit, etc. (Number) - Specific combat metrics (DA Plugin only).
 *
 *  The function reads relevant fields from the table and updates corresponding
 *  player data members in the game object. After setting all values, a dirty-bit
 *  flag is set to ensure synchronization across networked clients.
 *  \return Number - Always returns 1 on success if the player ID was valid.
 */
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
    /* DA Plugin Stats */
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

/*!\details Writes a Lua table of bytes to an arbitrary memory location.
 *  \par Lua Command
 *  \code{.lua}
 *      MemoryWrite(Address, {10, 20, 30})
 *  \endcode
 *  \param Address Number - The starting memory address where the writing should begin.
 *  \param Data Table - A Lua table where each element (Number) represents a byte value (0-255) to be written sequentially at increasing offsets from `Address`.
 *
 *  This is used for low-level cheating or debugging tools and requires elevated privileges.
 *  \return Number - Always returns 0 on success.
 */
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

/*!\details Checks if a specified unit (Player/Vehicle) is the true owner of an object.
 *  \par Lua Command
 *      isOwner = Is_Vehicle_Owner(Get_GameObj(10), Get_GameObj(20))
 *  \endcode
 *  \param VehicleID Number - ID of the vehicle/object being checked for ownership.
 *  \param OwnerID Number - ID of the potential owner unit.
 *
 *  This checks both plugin and default engine mechanisms to determine if the owner
 *  is indeed the specified unit. Returns `true` or `false`.
 *  \return Boolean - True if the object's current registered owner matches the provided OwnerID.
 */
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


/*!\details Warps a soldier to a specified location.
 *  \par Lua Command
 *  \code{.lua}
 *      Warp_Soldier(Get_GameObj(10), Vector3(25, 0, 40))
 *  \endcode
 *  \param SoldierID Number - ID of the soldier unit that will be warped.
 *  \param Position Table - Lua vector table containing `x`, `y`, and `z` for the target world coordinates.
 *
 *  Attempts to use physics checks (`Find_Teleport_Location`) to ensure the destination is valid (not stuck) before warping the unit's position.
 *  \return Boolean - True if the warp was successful, false otherwise.
 */
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

/*!\details Sets the maximum movement speed for a soldier unit.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Max_Speed(Get_GameObj(10), 5.5)
 *  \endcode
 *  \param ObjectID Number - ID of the soldier object whose max speed should be adjusted.
 *  \param Speed Number - New maximum movement speed in units per second (float).
 *
 *  Updates the soldier's internal speed limit parameters.
 *  \return Number - Always returns 0 on success.
 */
int Lua_Set_Max_Speed(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	obj->As_SoldierGameObj()->Set_Max_Speed((float)lua_tonumber(L, 2));
	return 0;
}

/*!\details Retrieves the current maximum movement speed of a soldier unit.
 *  \par Lua Command
 *  \code{.lua}
 *      speed = Get_Max_Speed(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the soldier object to query.
 *
 *  Reads and returns the maximum allowable movement speed.
 *  \return Number - The current max speed (float).
 */
int Lua_Get_Max_Speed(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, obj->As_SoldierGameObj()->Get_Max_Speed());
	return 1;
}

/*!\details Hides a specific object from a single player's view.
 *  \par Lua Command
 *  \code{.lua}
 *      Hide_Object_Player(Get_GameObj(20), false)
 *  \endcode
 *  \param PlayerID Number - ID of the player to whom the object should be invisible.
 *  \param Hidden Boolean - `false` to hide, `true` to restore visibility (currently only shows hiding).
 *
 *  This is a client-side visibility override controlled by scripting logic.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Hide_Object_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Get_GameObj(luaL_checknumber(L, 2));
	if (!obj) return 0;
	Set_Object_Visibility_For_Player(obj, luaL_checknumber(L, 1), false);
	return 1;
}

/*!\details Sets the persistent status for a specific power-up object.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Is_Powerup_Persistant(Get_GameObj(10), true)
 *  \endcode
 *  \param ObjectID Number - ID of the power-up to modify.
 *  \param Persistent Boolean - `true` if the object should respawn or remain in the world after being collected.
 *
 *  This function accesses the underlying definition to change persistence rules.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Set_Is_Powerup_Persistant(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	((PowerUpGameObjDef &)(obj->As_PhysicalGameObj()->As_PowerUpGameObj()->Get_Definition())).Persistent = lua_tobooleanCPP(L, 2);
	return 1;
}

/*!\details Checks if a given unit name belongs to an anti-warhole (WOL) user.
 *  \par Lua Command
 *  \code{.lua}
 *      isWOL = Is_WOL_User("PlayerName")
 *  \endcode
 *  \param Username String - The nickname of the player/unit to check.
 *
 *  This function queries the network services for WOL membership status.
 *  \return Boolean - True if the user is confirmed as a WOL member, false otherwise.
 */
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

/*!\details Performs a collision check (raycast) from an object's position to determine available space.
 *  \par Lua Command
 *  \code{.lua}
 *      result = OColision(Vector3(10, 5, 20), Vector3(1, 1, 1), Get_GameObj(10), 45.0)
 *  \endcode
 *  \param Pos Table - Lua vector table containing `x`, `y`, and `z` of the starting position.
 *  \param Size Table - Lua vector table containing `x`, `y`, and `z` dimensions (bounding box size).
 *  \param PlayerID Number - ID of the physical object performing the check.
 *  \param Facing Float - The current facing angle (degrees) for directional alignment checks.
 *
 *  This function performs multiple physics ray/box casts to determine if there is sufficient, non-colliding space at the specified location and facing direction.
 *  The return value is an enumeration constant indicating failure reasons: 0=No Ground, 1=Building Collision, 2=No Space.
 *  \return Number - An integer error code (0 to 2) representing collision outcome.
 */
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

/*!\details Displays a decorative information texture on an object's HUD model.
 *  \par Lua Command
 *  \code{.lua}
 *      ShowTexture(Get_GameObj(10), "WarningSign")
 *  \endcode
 *  \param ObjectID Number - ID of the object to apply the texture to.
 *  \param TextureName String - Name of the texture preset file (e.g., `"Danger"`, `"TargetingCross"`).
 *
 *  The function forwards this argument to `Commands->Set_Info_Texture`.  
 */
int Lua_ShowTexture(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Info_Texture(obj, lua_tostring(L, 2));
	return 1;
}

/*!\details Clears any applied info texture from an object's HUD model.
 *  \par Lua Command
 *  \code{.lua}
 *      HideTexture(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the object to clear the texture from.
 *
 *  The function forwards this argument to `Commands->Clear_Info_Texture`.  
 */
int Lua_HideTexture(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Clear_Info_Texture(obj);
	return 1;
}


// LuaTT.lua ported here..

/*!\details Retrieves a list of all active player IDs in the game session.
 *  \par Lua Command
 *  \code{.lua}
 *      players = Get_All_Players()
 *  \endcode
 *
 *  Returns a Lua table containing the numeric IDs (1-999) of every player currently connected and active in the game world.
 *  \return Number - The number of players found, pushed onto the stack.
 */
int Lua_Get_All_Players(lua_State *L)
{
	lua_newtable(L);
	int rotationTable = lua_gettop(L);
	int totalofplayersfound = 0;
	if (Get_Player_Count() == 0) return 1;

	// TODO improve this code

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

/*!\details Retrieves a list of all map names in the game cycle.
 *  \par Lua Command
 *  \code{.lua}
 *      maps = Get_Rotation()
 *  \endcode
 *
 *  Returns a Lua table where each element is a string containing the name of an available map, indexed starting from 1.
 *  \return Number - The total number of maps found in the game cycle.
 */
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


/*!\details Retrieves the IDs of all active vehicles on the map.
 *  \par Lua Command
 *  \code{.lua}
 *      vehicles = Get_All_Vehicles()
 *  \endcode
 *
 *  Returns a Lua table containing the numeric IDs of every vehicle currently in the game world.
 *  \return Number - The total count of vehicles found on the map.
 */
int Lua_Get_All_Vehicles(lua_State *L)
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

/*!\details Retrieves the IDs of all active buildings on the map.
 *  \par Lua Command
 *  \code{.lua}
 *      buildings = Get_All_Buildings()
 *  \endcode
 *
 *  Returns a Lua table containing the numeric IDs of every building currently in the game world.
 *  \return Number - The total count of buildings found on the map.
 */
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

/*!\details Sets the damage points granted upon a unit's death for a specific object.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Damage_Points(Get_GameObj(10), 5)
 *  \endcode
 *  \param ObjectID Number - ID of the object whose damage points are set.
 *  \param Points Number - The number of points awarded upon death.
 * \return Number - Returns 1 on success.
 */
int Lua_Set_Damage_Points(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Damage_Points(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checknumber(L, 2));
	return 1;
}

/*!\details Sets the death points granted upon a unit's defeat for a specific object.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Death_Points(Get_GameObj(10), 3)
 *  \endcode
 *  \param ObjectID Number - ID of the object whose death points are set.
 *  \param Points Number - The number of points awarded upon death.
 * \return Number - Returns 1 on success.
 */
int Lua_Set_Death_Points(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Death_Points(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checknumber(L, 2));
	return 1;
}

/*!\details Retrieves the damage points awarded for a specific object.
 *  \par Lua Command
 *  \code{.lua}
 *      points = Get_Damage_Points(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the object to query.
 * \return Number - The current damage points value (integer).
 */
int Lua_Get_Damage_Points(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Damage_Points(Commands->Find_Object(luaL_checknumber(L, 1))));
	return 1;
}

/*!\details Retrieves the death points awarded for a specific object.
 *  \par Lua Command
 *  \code{.lua}
 *      points = Get_Death_Points(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the object to query.
 * \return Number - The current death points value (integer).
 */
int Lua_Get_Death_Points(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Death_Points(Commands->Find_Object(luaL_checknumber(L, 1))));
	return 1;
}

/*!\details Deals damage to all occupants within a specified area.
 *  \par Lua Command
 *  \code{.lua}
 *      Damage_Objects_Half(Get_GameObj(10), "Fire")
 *  \endcode
 *  \param ObjectType Number - Target object type (e.g., 1=Soldier).
 *  \param DamageType String - The type of damage being dealt (e.g., "Explosion").
 *
 *  This action hits all specified unit types within the object's range/area.
 *  \return Number - Returns 1 on success.
 */
int Lua_Damage_Objects_Half(lua_State *L)
{
	Damage_Objects_Half();
	return 1;
}

/*!\details Kills all occupants (soldiers, drivers) of a specific object.
 *  \par Lua Command
 *  \code{.lua}
 *      Kill_Occupants(Get_GameObj(10))
 *  \endcode
 *  \param ObjectID Number - ID of the vehicle or unit whose occupants should be killed.
 *
 *  Kills all units contained within the given object. Returns 1 on success.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Kill_Occupants(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Kill_Occupants(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Deals damage to all objects within a defined area.
 *  \par Lua Command
 *  \code{.lua}
 *      Damage_All_Objects_Area(1, "Fire", Vector3(10, 5, 2), "Mine", Get_GameObj(10), Get_GameObj(20))
 *  \endcode}
 *  \param TeamID Number - The team ID of the objects targeted.
 *  \param DamageType String - The type of damage being dealt.
 *  \param Pos Table - Lua vector table containing `x`, `y`, and `z` of the center position.
 *  \param Range Number - The spherical range (radius) of the area effect.
 *  \param SourceID Number - ID of the primary source object causing damage.
 *  \param TargetSourceID Number - Optional secondary source object influencing damage.
 *
 *  Deals damage to all objects matching criteria within a specified radius from `Pos`.
 *  \return Number - Returns 1 on success.
 */
int Lua_Damage_All_Objects_Area(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 3);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	Damage_All_Objects_Area(luaL_checknumber(L, 1), luaL_checkstring(L, 2), pos, luaL_checknumber(L, 4), Commands->Find_Object(luaL_checknumber(L, 5)), Commands->Find_Object(luaL_checknumber(L, 6)));
	return 1;
}

/*!\details Deals damage to all buildings belonging to a specified team.
 *  \par Lua Command
 *  \code{.lua}
 *      Damage_All_Buildings_By_Team(0, "Fire", "Nod", Get_GameObj(10))
 *  \endcode}
 *  \param TeamID Number - The team ID of the buildings to damage.
 *  \param DamageType String - The type of damage being dealt (e.g., `"Explosion"`).
 *  \param TargetTeamName String - The string name of the team whose structures are affected.
 *  \param SourceID Number - ID of the primary source object causing damage.
 *
 *  Damage is applied to all buildings owned by the target team found within range of `SourceID`.
 *  \return Number - Returns 1 on success.
 */
int Lua_Damage_All_Buildings_By_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Damage_All_Buildings_By_Team(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checkstring(L, 3), Commands->Find_Object(luaL_checknumber(L, 4)));
	return 1;
}

/*!\details Deals damage to all vehicles within a defined area.
 *  \par Lua Command
 *  \code{.lua}
 *      Damage_All_Vehicles_Area(0, "Fire", Vector3(10, 5, 2), 10, Get_GameObj(10), Get_GameObj(20))
 *  \endcode}
 *  \param TeamID Number - The team ID of the vehicles targeted.
 *  \param DamageType String - The type of damage being dealt.
 *  \param Pos Table - Lua vector table containing `x`, `y`, and `z` of the center position.
 *  \param Range Number - The spherical range (radius) of the area effect.
 *  \param SourceID Number - ID of the primary source object causing damage.
 *  \param TargetSourceID Number - Optional secondary source object influencing damage.
 *
 *  Deals damage to all vehicles matching criteria within a specified radius from `Pos`.
 *  \return Number - Returns 1 on success.
 */
int Lua_Damage_All_Vehicles_Area(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 3);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	Damage_All_Vehicles_Area(luaL_checknumber(L, 1), luaL_checkstring(L, 2), pos, luaL_checknumber(L, 4), Commands->Find_Object(luaL_checknumber(L, 5)), Commands->Find_Object(luaL_checknumber(L, 6)));
	return 1;
}

/*!\details Sets an information texture on a specified object's HUD model.
 *  \par Lua Command
 *  \code{.lua}
 *      ShowTexture(Get_GameObj(10), "WarningSign")
 *  \endcode}
 *  \param ObjectID Number - ID of the object to apply the texture to.
 *  \param TextureName String - Name of the texture preset file (e.g., `"Danger"`, `"TargetingCross"`).
 *
 *  The function forwards this argument to `Commands->Set_Info_Texture`.  
 */
int Lua_Set_Info_Texture(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Info_Texture(obj, lua_tostring(L, 2));
	return 0;
}

/*!\details Clears any applied info texture from an object's HUD model.
 *  \par Lua Command
 *  \code{.lua}
 *      HideTexture(Get_GameObj(10))
 *  \endcode}
 *  \param ObjectID Number - ID of the object to clear the texture from.
 *
 *  The function forwards this argument to `Commands->Clear_Info_Texture`.  
 */
int Lua_Clear_Info_Texture(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Clear_Info_Texture(obj);
	return 0;
}

/*!\details Sets the limit on naval vehicles for a team.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Naval_Vehicle_Limit(5)
 *  \endcode}
 *  \param Limit Number - The maximum number of naval units allowed per team.
 *
 *  The function forwards this argument to `Set_Naval_Vehicle_Limit`.  
 */
int Lua_Set_Naval_Vehicle_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Naval_Vehicle_Limit(luaL_checknumber(L, 1));
	return 1;
}

/*!\details Retrieves the current limit on naval vehicles for a team.
 *  \par Lua Command
 *  \code{.lua}
 *      limit = Get_Naval_Vehicle_Limit()
 *  \endcode}
 *
 *  The function forwards this call to `Get_Naval_Vehicle_Limit`.  
 *  \return Number - The current vehicle limit.
 */
int Lua_Get_Naval_Vehicle_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Naval_Vehicle_Limit());
	return 1;
}

/*!\details Sends a message to a specific player.
 *  \par Lua Command
 *  \code{.lua}
 *      Send_Message_Player(Get_GameObj(20), 12, 1, 4, "Enemy spotted!")
 *  \endcode}
 *  \param TargetID Number - ID of the player who should receive the message.
 *  \param PlayerA Number - Player A's ID (sender).
 *  \param ColorNumber Number - The color component for display (e.g., 255 for red).
 *  \param Brightness Number - The brightness level of the displayed text.
 *  \param Message String - The message content to be displayed in the chat/HUD.
 *
 *  The function forwards these arguments to `Send_Message_Player`.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Send_Message_Player(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Message_Player(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checkstring(L, 5));
	return 1;
}


/*!\details Loads a custom HUD configuration from an INI file for an object.
 *  \par Lua Command
 *  \code{.lua}
 *      Load_New_HUD_INI(Get_GameObj(10), "custom_hud.ini")
 *  \endcode}
 *  \param ObjectID Number - ID of the object to configure.
 *  \param INIFileName String - The path and name of the configuration file (`.ini`).
 *
 *  The function forwards these arguments to `Commands->Load_New_HUD_INI`.  
 */
int Lua_Load_New_HUD_INI(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Load_New_HUD_INI(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2));
	return 1;
}

/*!\details Changes the radar map visible to players.
 *  \par Lua Command
 *  \code{.lua}
 *      Change_Radar_Map(Get_GameObj(10), 1, 2, "Air")
 *  \endcode}
 *  \param ObjectID Number - ID of the object (usually player) whose radar map is changed.
 *  \param MapIndex Number - The index of the desired radar map to load (e.g., 1 for Air).
 *  \param PlayerID Number - ID of the specific player viewing the radar.
 *  \param RadarMapName String - Optional name or identifier for the advanced map configuration.
 *
 *  The function forwards these arguments to `Commands->Change_Radar_Map`.  
 */
int Lua_Change_Radar_Map(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	Change_Radar_Map(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checkstring(L, 4));
	return 1;
}

/*!\details Sets whether the object is currently designated as a building under construction.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Currently_Building(Get_GameObj(10), true)
 *  \endcode}
 *  \param IsBuilding Boolean - `true` if the object is currently under build status, `false` otherwise.
 *
 *  The function forwards this argument to `Commands->Set_Currently_Building`.  
 */
int Lua_Set_Currently_Building(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Currently_Building(lua_tobooleanCPP(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Checks if an object is marked as currently under construction.
 *  \par Lua Command
 *  \code{.lua}
 *      isBuilding = Is_Currently_Building(Get_GameObj(10))
 *  \endcode}
 *  \param ObjectID Number - ID of the object to check status for.
 *
 *  The function forwards this argument to `Is_Currently_Building`.  
 *  \return Boolean - True if the object is currently under construction, false otherwise.
 */
int Lua_Is_Currently_Building(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, (Is_Currently_Building(luaL_checknumber(L, 1))));
	return 1;
}

/*!\details Sets the fog color for all players in the area.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Fog_Color(255, 100, 0) -- Orange fog
 *  \endcode}
 *  \param R Number - The Red component of the desired fog color (0-255).
 *  \param G Number - The Green component of the desired fog color (0-255).
 *  \param B Number - The Blue component of the desired fog color (0-255).
 *
 *  The function forwards these arguments to `Commands->Set_Fog_Color`.  
 */
int Lua_Set_Fog_Color(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Fog_Color(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	return 1;
}

/*!\details Displays a security alert dialog to the object's owner or nearby players.
 *  \par Lua Command
 *  \code{.lua}
 *      Display_Security_Dialog(Get_GameObj(10))
 *  \endcode}
 *  \param ObjectID Number - ID of the object associated with the security warning.
 *
 *  The function forwards this argument to `Commands->Display_Security_Dialog`.  
 */
int Lua_Display_Security_Dialog(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Display_Security_Dialog(obj);
	return 1;
}

/*!\details Executes a series of objectives or dialog prompts for an object.
 *  \par Lua Command
 *  \code{.lua}
 *      Do_Objectives_Dlg(Get_GameObj(10), "PreWar")
 *  \endcode}
 *  \param ObjectID Number - ID of the unit associated with the objectives.
 *  \param ObjectiveName String - Name or identifier of the objective set to display.
 *
 *  The function forwards these arguments to `Commands->Do_Objectives_Dlg`.  
 */
int Lua_Do_Objectives_Dlg(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Do_Objectives_Dlg(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2));
	return 1;
}

/*!\details Sets the maximum number of players allowed in the game.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Player_Limit(24)
 *  \endcode}
 *  \param Limit Number - The maximum player count allowed for the current match.
 *
 *  The function forwards this argument to `Commands->Set_Player_Limit`.  
 */
int Lua_Set_Player_Limit(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Player_Limit(luaL_checknumber(L, 1));
	return 1;
}

/*!\details Retrieves the current maximum player limit for the game session.
 *  \par Lua Command
 *  \code{.lua}
 *      limit = Get_Player_Limit()
 *  \endcode}
 *
 *  The function forwards this call to `Get_Player_Limit`.  
 *  \return Number - The current maximum player limit.
 */
int Lua_Get_Player_Limit(lua_State *L)
{
	lua_pushnumber(L, Get_Player_Limit());
	return 1;
}

/*!\details Sets the custom name for all GDI soldiers/units in the game world.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_GDI_Soldier_Name("AlphaUnit")
 *  \endcode}
 *  \param NewName String - The uniform name to assign to all GDI units.
 *
 *  The function forwards this argument to `Commands->Set_GDI_Soldier_Name`.  
 */
int Lua_Set_GDI_Soldier_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_GDI_Soldier_Name(luaL_checkstring(L, 1));
	return 1;
}

/*!\details Sets the custom name for all NOD soldiers/units in the game world.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Nod_Soldier_Name("OmegaUnit")
 *  \endcode}
 *  \param NewName String - The uniform name to assign to all NOD units.
 *
 *  The function forwards this argument to `Commands->Set_Nod_Soldier_Name`.  
 */
int Lua_Set_Nod_Soldier_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Nod_Soldier_Name(luaL_checkstring(L, 1));
	return 1;
}

/*!\details Sets the Moon's celestial body status to Earth or simulate a different planet.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Moon_Is_Earth(true)
 *  \endcode}
 *  \param IsEarth Boolean - `true` if simulating Earth (default), `false` for alien worlds.
 *
 *  Controls global rendering parameters related to the celestial body model.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Set_Moon_Is_Earth(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Moon_Is_Earth(lua_tobooleanCPP(L, 1));
	return 1;
}

/*!\details Retrieves the current game revision number.
 *  \par Lua Command
 *  \code{.lua}
 *      rev = Get_Revision()
 *  \endcode}
 *  \param Ignored Number - Dummy argument, not used by the function.
 *
 *  This is useful for checking which build version of the game client is running.
 *  \return Number - The current revision number (integer).
 */
int Lua_Get_Revision(lua_State *L)
{
	lua_pushnumber(L, Get_Revision(0));
	return 1;
}

/*!\details Checks if a team is allowed to build vehicles in the current game state.
 *  \par Lua Command
 *  \code{.lua}
 *      canBuild = Can_Team_Build_Vehicle(Get_GameObj(10))
 *  \endcode}
 *  \param ObjectID Number - ID of the object associated with the team/owner to check.
 *
 *  The function forwards this argument to `Commands->Can_Team_Build_Vehicle`.  
 */
int Lua_Can_Team_Build_Vehicle(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Can_Team_Build_Vehicle(luaL_checknumber(L, 1));
	return 1;
}

/*!\details Finds the nearest valid naval factory building for a specified team.
 *  \par Lua Command
 *  \code{.lua}
 *      factory = Find_Naval_Factory(0) -- Search for Nod (team 0)
 *  \endcode}
 *  \param TeamID Number - ID of the team whose factory is being searched for.
 *
 *  The function forwards this argument to `Commands->Find_Naval_Factory`.  
 */
int Lua_Find_Naval_Factory(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Find_Naval_Factory(luaL_checknumber(L, 1));
	return 1;
}

/*!\details Checks if a vehicle preset is designed for air combat units.
 *  \par Lua Command
 *  \code{.lua}
 *      isAir = Vehicle_Preset_Is_Air(123)
 *  \endcode}
 *  \param PresetID Number - The numeric ID of the vehicle preset to check.
 *
 *  Returns `true` if the preset is categorized as an air vehicle (e.g., VTOL, helicopter).
 *  \return Boolean - True if the unit is designed for flight.
 */
int Lua_Vehicle_Preset_Is_Air(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Vehicle_Preset_Is_Air(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Checks if a vehicle preset is designed for naval/aquatic units.
 *  \par Lua Command
 *  \code{.lua}
 *      isNaval = Vehicle_Preset_Is_Naval(123)
 *  \endcode}
 *  \param PresetID Number - The numeric ID of the vehicle preset to check.
 *
 *  Returns `true` if the unit is categorized as a naval vessel (e.g., submarine, boat).
 *  \return Boolean - True if the unit is designed for water transport/combat.
 */
int Lua_Vehicle_Preset_Is_Naval(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushboolean(L, Vehicle_Preset_Is_Naval(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Hides a vehicle preset if its ID matches the specified name.
 *  \par Lua Command
 *  \code{.lua}
 *      Hide_Preset_By_Name(123, "MySecretTank", true)
 *  \endcode}
 *  \param PresetID Number - The numeric ID of the preset to modify.
 *  \param Name String - The name used by the preset file (e.g., `"HiddenUnit"`).
 *  \param ShouldHide Boolean - `true` forces hiding, `false` attempts to keep it visible.
 *
 *  Allows scripting manipulation of vehicle availability on the map.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Hide_Preset_By_Name(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Hide_Preset_By_Name(luaL_checknumber(L, 1), luaL_checkstring(L, 2), lua_tobooleanCPP(L, 3));
	return 1;
}

/*!\details Sets the active status of a vehicle preset if its ID matches the specified name.
 *  \par Lua Command
 *  \code{.lua}
 *      Busy_Preset_By_Name(123, "MySecretTank", true)
 *  \endcode}
 *  \param PresetID Number - The numeric ID of the preset to modify.
 *  \param Name String - The name used by the preset file (e.g., `"HiddenUnit"`).
 *  \param IsBusy Boolean - `true` if the unit is considered currently in use, `false` otherwise.
 *
 *  Changes whether the factory should restrict spawning of this unit type.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Busy_Preset_By_Name(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Busy_Preset_By_Name(luaL_checknumber(L, 1), luaL_checkstring(L, 2), lua_tobooleanCPP(L, 3));
	return 1;
}

/*!\details Attaches a Lua script to all occupants of an object.
 *  \par Lua Command
 *  \code{.lua}
 *      Attach_Script_Occupants(Get_GameObj(10), "CrewScript", "param=value")
 *  \endcode}
 *  \param ObjectID Number - ID of the vehicle/object whose occupants will receive the script.
 *  \param ScriptName String - Name of the Lua script file (without extension).
 *  \param Parameters String - Optional parameters string passed to the script.
 *
 *  The function forwards these arguments to `Commands->Attach_Script_Occupants`.  
 */
int Lua_Attach_Script_Occupants(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Attach_Script_Occupants(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2), luaL_checkstring(L, 3));
	return 1;
}

/*!\details Destroys a specific Lua scripting thread ID.
 *  \par Lua Command
 *  \code{.lua}
 *      Destroy_Lua_Thread(3)
 *  \endcode}
 *  \param ThreadID Number - The integer ID assigned to the script's running thread (0-999).
 *
 *  Safely closes and removes a specific Lua execution context from the manager.
 *  \return Boolean - True if the thread was successfully closed, false otherwise.
 */
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
/*!\details Loads a Lua scripting module by name and registers it with the engine's Lua manager.
 *  \par Lua Command
 *  \code{.lua}
 *      Load_Lua("MyAwesomeScript")
 *  \endcode}
 *  \param FileName String - The file name (without extension) of the script to load.
 *
 *  The function forwards this argument to `LuaManager::LoadLua`.  
 */
int Lua_Load_Lua(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	std::string whatfind = std::string(luaL_checkstring(L, 1));
	LuaManager::LoadLua(whatfind.c_str());
	return 1;
}

/*!\details Retrieves the numeric ID assigned to a loaded Lua script module.
 *  \par Lua Command
 *  \code{.lua}
 *      id = Get_Lua_By_Name("MyAwesomeScript")
 *  \endcode}
 *  \param ModuleName String - The registered name of the script/module.
 *
 *  This function returns the internal ID used by `LuaManager` to manage the module.
 *  \return Number - The assigned Lua manager ID, or 0 if not found.
 */
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

/*!\details Retrieves the index of the current map in the game cycle sequence.
 *  \par Lua Command
 *  \code{.lua}
 *      currentIndex = Get_Current_Map_Index()
 *  \endcode}
 *  \param Ignored Number - Dummy argument, not used by the function (required for signature consistency).
 *
 *  Returns the zero-based index of the currently loaded map.
 *  \return Number - The current map cycle index.
 */
int Lua_Get_Current_Map_Index(lua_State *L)
{
	lua_pushnumber(L, Get_Current_Map_Index());
	return 1;
}

/*!\details Forces the game state to switch maps.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Map("DesertWar", 2) -- Map name and index (2 = secondary check/fallback)
 *  \endcode}
 *  \param MapName String - Name of the target map to load. If `NULL`, only the index is used.
 *  \param Index Number - The preferred index or fallback index for the map selection process.
 *
 *  Immediately attempts to change the active game environment.
 *  \return Number - Returns 1 on success.
 */
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

/*!\details Retrieves the client revision number for a specified player.
 *  \par Lua Command
 *  \code{.lua}
 *      revision = Get_Client_Revision(1)
 *  \endcode}
 *  \param PlayerID Number - ID of the player to query.
 *
 *  Used for compatibility checking between clients and server scripts.
 *  \return Number - The client's revision number (integer).
 */
int Lua_Get_Client_Revision(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Get_Client_Revision(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Sets the visible status of an object for all players.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Is_Visible(Get_GameObj(10), false)
 *  \endcode}
 *  \param ObjectID Number - ID of the object whose visibility should be toggled.
 *  \param Visible Boolean - `true` to show, `false` to hide.
 *
 *  Manually controls if the physical and visual representation of an object is rendered in the game world.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Set_Is_Visible(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Set_Is_Visible(Commands->Find_Object(luaL_checknumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Removes all weapons from an object.
 *  \par Lua Command
 *  \code{.lua}
 *      Clear_Weapons(Get_GameObj(10))
 *  \endcode}
 *  \param ObjectID Number - ID of the object to clear weapons from.
 *
 *  Clears the entire weapon inventory, making the unit unarmed.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Clear_Weapons(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Clear_Weapons(Commands->Find_Object(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Sets whether an object is visible in the world geometry/render passes.
 *  \par Lua Command
 *  \code{.lua}
 *      Set_Is_Rendered(Get_GameObj(10), false)
 *  \endcode}
 *  \param ObjectID Number - ID of the object whose rendering status should be changed.
 *  \param Visible Boolean - `true` to render, `false` to prevent rendering (ghosting).
 *
 *  Controls basic visibility separate from collision/script invisibility.
 *  \return Number - Always returns 1 on success.
 */
int Lua_Set_Is_Rendered(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Set_Is_Rendered(Commands->Find_Object(luaL_checknumber(L, 1)), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Retrieves the ID of the object currently locking down ownership rights over a vehicle.
 *  \par Lua Command
 *  \code{.lua}
 *      ownerID = Get_Vehicle_Lock_Owner(Get_GameObj(10))
 *  \endcode}
 *  \param VehicleID Number - ID of the vehicle object to check for locks.
 *
 *  Returns the numeric ID of the player or unit currently possessing lock ownership.
 *  \return Number - The owner's ID, or 0 if no owner is present.
 */
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

// Function to check if an object can be teleported to a given position.
/*!\details Checks if a moveable physical object can safely transition (teleport) to a specified world location. This function accounts for movement restrictions inherent to the game environment, such as solid objects or barriers.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Can_Teleport(Object, Vector3)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			local obj = Get_Object(obj_id); -- Assume helper function exists to get object by ID
*			if (Can_Teleport(obj, Vector3("100", 5.0, -50")))
*			    print("Teleport destination is safe.");
*  \endcode
*  \param Object The ID of the physical object intended to move.
*  \param Location The LuaVector3 containing the X, Y, and Z coordinates of the target position.
*  \return Boolean (`true` if teleportation is possible; `false` otherwise).
*/
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

// Function to set the death count (score variable).
/*!\details Sets the displayed death counter for a specified player ID. This is typically used by scripts that track player scores or kill/death stats.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Deaths(PlayerID, Amount)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			-- Set player 5's death count to 10
*			Set_Deaths(5, 10);
*  \endcode
*  \param PlayerID The unique ID of the player whose stats are being modified.
*  \param Amount The new total number of deaths (integer).
*/
int Lua_Set_Deaths(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Deaths(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 1;
}

// Function to retrieve the current animation name of an object.
/*!\details Retrieves the name of the currently playing animation on a given game object. This is useful for scripting synchronized events (e.g., detecting if a character is reloading).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Animation_Name(Object)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			local anim = Get_Animation_Name(obj); -- The function call name remains, but the underlying API is cleaner.
*			if (anim == "Reload") then print("Player is reloading!"); end
*  \endcode
*  \param Object The ID of the object to check for its current animation.
*  \return String Returns the name of the animation, or `nil` if no animation is playing or supported.
*/
int Lua_Get_Animation_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj)
		lua_pushstring(L, Get_Animation_Name(obj));

	return 1;
}

// Function to set the custom help text for a player's HUD.
/*!\details Sets a customized informational message displayed on a player's Heads-Up Display (HUD). This allows modders to display contextual information (e.g., "Press E to Open Shield").
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_HUD_Help_Text_Player(Object, MessageString, Vector3)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			local obj = Get_Object(player_id);
*			-- Set message: "Next waypoint visible" with a yellow color.
*			Set_HUD_Help_Text_Player(obj, "Next waypoint visible", 1.0, 1.0, 0.0);
*  \endcode
*  \param Object The ID of the player/object receiving the HUD message.
*  \param MessageString The text string to display (must be Lua string type).
*  \param Vector3 The RGB color components for the text (floats 0.0-1.0).
*/
int Lua_Set_HUD_Help_Text_Player_Text(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_HUD_Help_Text_Player_Text(obj, lua_tonumber(L, 2), lua_tostring(L, 3), Vector3(lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6)));
	return 1;
}

// Function to set the custom help text for a player's HUD (Simplified version).
/*!\details Sets a customized informational message displayed on a player's Heads-Up Display (HUD). This simplified version uses RGB floats for color.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_HUD_Help_Text_Player(Object, MessageString, Vector3)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			local obj = Get_Object(player_id);
*			-- Set message: "Warning!", using default RGB colors.
*			Set_HUD_Help_Text_Player(obj, "Warning!", 1.0, 0.0, 0.0);
*  \endcode
*  \param Object The ID of the player/object receiving the HUD message.
*  \param MessageString The text string to display (must be Lua string type).
*  \param Vector3 The RGB color components for the text (floats 0.0-1.0).
*/
int Lua_Set_HUD_Help_Text_Player(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_HUD_Help_Text_Player(obj, lua_tonumber(L, 2), Vector3(lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5)));
	return 1;
}

// Function to force a character into a vehicle.
/*!\details Attempts to force an object (soldier/character) to enter another game object (the vehicle). This performs checks to prevent errors like trying to enter a full vehicle or if the source object isn't even in a vehicle.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Force_Vehicle_Entry(Object, Vehicle)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			local player = Get_Object(player_id);
*			local vehicle = Get_Object(vehicle_id);
*			if (Force_Vehicle_Entry(player, vehicle))
*				print("Player entered the vehicle successfully.");
*  \endcode
*  \param Object The ID of the object that should enter the vehicle.
*  \param Vehicle The ID of the target vehicle game object.
*  \return Boolean (`true` if entry was forced; `false` otherwise, usually due to invalid arguments or state checks).
*/
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

// Function to set the player's camera view.
/*!\details Forces an object to adopt a specified camera perspective or character model (e.g., simulating a cinematic setup).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Camera_Player(Object, CameraName)
 *  \endcode
*  \param Object The ID of the object whose camera view should be set.
*  \param CameraName The name of the camera definition or desired view state string.
*/
int Lua_Set_Camera_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_Camera_Player(obj, lua_tostring(L, 2));

	return 0;
}

// Function to enable/disable HUD indicators for objects in the area.
/*!\details Toggles a specific type of visual indicator on the object's HUD, often used for gameplay cues like showing enemies or objectives.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Enable_HUD_Pokable_Indicator(Object, Enable)
 *  \endcode
*  \param Object The ID of the object being monitored.
*  \param Enable Boolean (`true` to enable visibility; `false` to disable).
*  \return Integer (Always 1 if parameters were valid, 0 otherwise).
*/
int Lua_Enable_HUD_Pokable_Indicator(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Commands->Enable_HUD_Pokable_Indicator(obj, lua_tobooleanCPP(L, 2));
	return 0;
}

// Function to grant a weapon model and ammo type to an object.
/*!\details Commands the system to manually grant a specific weapon preset to an object, along with optionally setting its visible state (e.g., if it should appear in inventory).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Grant_Weapon(Object, WeaponName, VisibleFlag, AmmoCount, IsClips)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			local obj = Get_Object(player_id);
*			Grant_Weapon(obj, "CnC_Weapon_RepairGun_Player_Special", true, 30, 12);
*  \endcode
*  \param Object The ID of the object receiving the weapon.
*  \param WeaponName The preset name of the weapon to grant.
*  \param VisibleFlag Boolean (`true` if visible in HUD).
*  \param AmmoCount Integer number of initial rounds loaded.
*  \param IsClips Boolean (`true` if grants separate clip count).
*/
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

/*!\details Finds the closest physical building object at a given position. If no dedicated building is found, it falls back to finding a similarly defined structure type (Fake Building).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Closest_Building(Vector3)
 *  \endcode
*  \param Position The world coordinates used for proximity search.
*  \return Number Returns the Game Object ID of the nearest building, or 0 if none is found.
*/
int Lua_Get_Closest_Building(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	GameObject *obj = Get_Closest_Building(location);
	lua_pushnumber(L, Get_ID(obj));
	return 1;
}

/*!\details Finds the closest physics-based object that resembles a building. Useful for structures not categorized as standard `BuildingGameObj`.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Closest_Fake_Building(Vector3)
 *  \endcode
*  \param Position The world coordinates used for proximity search.
*  \return Number Returns the Game Object ID of the nearest fake building, or 0 if none is found.
*/
int Lua_Get_Closest_Fake_Building(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	LUA_Vector3 *a = LuaVector3::GetInstance(L, 1);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	GameObject *obj = Get_Closest_Fake_Building(location);
	lua_pushnumber(L, Get_ID(obj));
	return 1;
}

/*!\details Uses the internal BuildingGameObj::Find_Closest_Poly method to calculate the distance from a given object to its nearest polygon within a specified range, providing an estimate of proximity to structural geometry.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Closest_Poly(Object, Vector3)
 *  \endcode
*  \param Object The ID of the building whose poly structure is checked.
*  \param Location The target world coordinates for distance calculation.
*  \return Number Returns the calculated floating-point distance to the nearest polygon point.
*/
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


/*!\details Retrieves an integer ID from the global Translation Database by using its descriptive name (ID\_Desc). This is essential for linking dynamic content to predefined localized strings.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_String_ID_By_Desc("Example Message Description")
 *  \endcode
*  \param Description The descriptive name of the string in the translation database (string).
*  \return Number Returns the corresponding unique Translation Database ID, or 0 if not found.
*/
int Lua_Get_String_ID_By_Desc(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;

	lua_pushnumber(L, Get_String_ID_By_Desc(lua_tostring(L, 1)));

	return 1;
}

// Function to change a player's name within the game world.
/*!\details Changes the displayed in-game name of an object (assumed to be a character/player). This modifies how the name appears across all UI elements and chat logs.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Change_String_Player(Object, PlayerID, OldName)
 *  \endcode
*  \param Object The ID of the player object whose name is being changed.
*  \param NewName The new desired in-game display name.
*  \param OldName (Optional) The original name to be replaced. Null/nil can be passed if only changing the name.
*/
int Lua_Change_String_Player(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Change_String_Player(obj, lua_tonumber(L, 2), lua_tostring(L, 3));

	return 1;
}

// Function to retrieve and structure detailed information about an Ammo Definition.
/*!\details Gets the definition object for a weapon's ammo type. The function attempts to return a serialized Lua table containing key stats (AliasedSpeed, Damage, Range, etc.) rather than the complex C++ class pointer itself, making it usable by scripting logic.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Ammo_Definition(Obj, IsPrimaryFire, OptionalReferenceOnly)
 *  \endcode
*  \param Object Pointer/ID representing the source of the weapon definition (can be null if only using preset name).
*  \param IsPrimaryFire Boolean (`true`) indicating if primary fire ammunition stats are required. If `false`, falls back to string argument check.
*  \return Number Returns 1 on success, or 0/error code otherwise. The returned value is a complex Lua table containing ammo statistics.
*/
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
		return 0; // Invalid ammo?
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

// Function to execute a complex attack sequence on an object.
/*!\details Initiates an attack maneuver from the given object at specified coordinates. This involves setting up detailed movement and targeting parameters for the engine's AI system. Requires five key components: the attacker, target position, range, deviation, and primary fire status.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Action_AttackLocation(Object, Vector3, Range, Deviation)
 *  \endcode
*  \par	Example
*  \code{.lua}
*			local attacker = Get_Object(self.id);
*			-- Attack at a point 100 units away with 50 unit deviation.
*			Action_AttackLocation(attacker, Vector3("100", 0, -100"), 150, 50);
*  \endcode
*  \param Object The ID of the object performing the attack.
*  \param TargetPosition A LuaVector3 containing the X, Y, and Z coordinates where the attack should be directed.
*  \param Range Float The maximum effective range for the attack.
*  \param Deviation Float How far off the target trajectory the object is allowed to deviate (spread).
*  \return Number Returns 1 on success.
*/
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

// Function to perform an attack sequence while simultaneously moving the object.
/*!\details Combines movement and attacking into a single action. The object moves towards a target point while maintaining line-of-sight with its current position (defined by Vector3 in parameter 2) for continuous fire.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Action_AttackMoveLocation(Object, Vector3, Range, Deviation, Vector3, Speed, Distance)
 *  \endcode
*  \param Object The ID of the object performing both movement and attack.
*  \param TargetPosition A LuaVector3 containing the X, Y, and Z coordinates of the destination for the move action (Position 2).
*  \param Range Float The maximum effective range for the continuous attack fire.
*  \param Deviation Float How far off the target trajectory the object is allowed to deviate.
*  \param CurrentPos A LuaVector3 containing the current position used for targeting.
*  \param Speed Float Target movement speed (m/s).
*  \param Distance Float The distance parameter for the move action.
*  \return Number Returns 1 on success, or 0 if required parameters are missing.
*/
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

// Function to move an object to a specified location using the standard game movement system.
/*!\details Forces a GameObject to begin moving toward a target position across the map. This is the primary method for programmatic pathfinding and relocation of units.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Action_MoveLocation(Object, Vector3, Speed, Distance)
 *  \endcode
*  \param Object The ID of the object to move.
*  \param TargetPosition A LuaVector3 containing the destination coordinates.
*  \param Speed Float Desired movement speed (m/s).
*  \param Distance Float Movement distance parameter.
*  \return Number Returns 1 on success, or 0 if parameters are missing.
*/
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

// Function to make one object follow another specified object's location.
/*!\details Forces the primary object (Object) to move and maintain a specific distance or bearing relative to a secondary target object (Target). Used for controlled escort missions or convoy movement.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Action_FollowObject(Object, TargetObject)
 *  \endcode
*  \param Object The ID of the follower object.
*  \param TargetObject The ID of the object that should be followed (the lead).
*  \return Number Returns 1 on success, or 0 if parameters are missing.
*/
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


// Function to upgrade an object's AI capabilities (making it a smart bot).
/*!\details Initializes several components of the object's script-based intelligence: turning off stationary status, enabling sensory input (enemy sight, sound hearing), and enabling general machine operation. This is often used during respawn or scripted enemy spawns.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Smart_EnableBot(Object)
 *  \endcode
*  \param Object The ID of the object to initialize as a smart bot.
*  \return Number Returns 1 if successfully initialized, or 0 otherwise.
*/
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

// Function to set the network update frequency for a player object.
/*!\details Adjusts how often an object's state is synced across the network (tick rate). Higher rates improve responsiveness but consume more resources.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Net_Update_Rate_Player(Object, Rate)
 *  \endcode
*  \param Object The ID of the player object whose update rate is being set.
*  \param Rate Float The desired network update rate (e.g., 10.0 for a high frequency).
*  \return Number Returns 1 on success, or 0 if parameters are missing.
*/
int Lua_Set_Net_Update_Rate_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_Net_Update_Rate_Player(obj, lua_tonumber(L, 2));
	return 1;
}

// Function to retrieve a list of all attached script observers on an object.
/*!\details Enumerates and returns the names and indices of all currently active script observer components attached to a target object. Useful for scripting management systems that need to know what logic is running on an instance.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_All_Scripts_Object(Object)
 *  \endcode
*  \param Object The ID of the object being inspected for attached scripts.
*  \return Number Returns 1 and sets the Lua stack with a table containing pairs of `{index, scriptName}` for every active script observer.
*/
int Lua_Get_All_Scripts_Object(lua_State *L)
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


// Function to retrieve the name of all attached observers (redundant in purpose but exposed for completeness).
/*!\details Enumerates and returns the names of all registered scripts or components acting as event observers on an object. This is primarily useful for inspecting the object's scripting capabilities without relying on type casting.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_All_Observers_Object(Object)
 *  \endcode
*  \param Object The ID of the object being inspected for attached observers/scripts.
*  \return Number Returns 1 and sets the Lua stack with a table containing pairs of `{index, observerName}`.
*/
int Lua_Get_All_Observers_Object(lua_State *L)
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

/*!\details Scans an object for all instances of a named script and then returns a Lua table listing all unique parameter names defined across all found scripts. This is critical for automated script documentation tools.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Script_Parameters(Object, ScriptName)
 *  \endcode
*  \param Object The ID of the object being analyzed.
*  \param ScriptName The name of the script to inspect parameters for.
*  \return Number Returns 1 and sets the Lua stack with a table where each entry is a comma-separated string of parameter names.
*/
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

/*!\details Manually sets the physical velocity vector for a GameObject. This allows scripts to instantly propel or halt objects using scripted forces. Requires a valid world position (X, Y, Z).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Velocity(Object, Vector3)
 *  \endcode
*  \param Object The ID of the object whose velocity is to be changed.
*  \param NewVector A LuaVector3 defining the desired velocity vector (X, Y, Z).
*  \return Number Returns 0 on success, or non-zero on failure (e.g., missing parameters).
*/
int Lua_Set_Velocity(lua_State* L) // replace existing one with this sla
{
	if (lua_gettop(L) < 2) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	Set_Velocity(obj, pos);
	return 0;
}

/*!\details Modifies whether or not an ammunition type definition (`AmmoDefinitionClass`) should track in-game stats (like remaining bullets). This is primarily used for scripting special ammo types that bypass standard ballistics rules.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Ammo_IsTracking(ObjPointer, IsPrimaryFire, NewStatus)
 *  \endcode
*  \param Object Pointer/ID to the source object (ignored if using direct pointer).
*  \param IsPrimaryFire Boolean (`true`) indicating if primary fire is being checked.
*  \param NewStatus Boolean (`true` to enable tracking, `false` otherwise).
*  \return Number Returns 1 on success, or a failure code if the ammunition definition is invalid.
*/
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

/*!\details Forces an object to revert to a non-AI state: stationary, deaf, and visually invisible to threats. Useful for scripting takedowns or disabling enemy bots.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Disable_Innate(Object)
 *  \endcode
*  \param Object The ID of the object whose AI should be disabled.
*  \return Number Returns 0 on success, or non-zero on failure.
*/
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

/*!\details Executes a proximity check using defined game zones. The function currently uses `Beacon` as the default zone type but allows checking for other types of scripted areas by modifying the underlying C++ call.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Find_Closest_Zone(Get_Position(Get_GameObj(1)))
 *  \endcode
*  \param Position A LuaVector3 containing the target location.
*  \return Number Returns 1 and sets the Lua stack with the Game Object ID of the closest zone found (defaulting to Beacon).
*/
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


/*!\details Populates a Lua table listing every Script Zone found
 *  \par Command Usage Example
 *  \code{.lua}
 *			scriptszones = Get_All_ScriptZones()
 *  \endcode
*  \param None No arguments required.
*  \return Table
*/
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

/*!\details Populates a Lua table listing every defined gameplay definition (preset).
 *  \par Command Usage Example
 *  \code{.lua}
 *			defs = Get_Game_Definitions()
 *  \endcode
*  \param None No arguments required.
*  \return Table
*/
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

/*!\details Retrieves a list of all currently active player objects on the map, returning their unique IDs for immediate access in scripts.
 *  \par Command Usage Example
 *  \code{.lua}
 *			player_list = Get_Player_List()
 *  \endcode
*  \param None No arguments required.
*  \return Table Returns a table of active player IDs indexed by ID number.
*/
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

/*!\details Spawns and controls a 2D WAV audio emitter attached to a given game object for background music or persistent effects. This function returns the unique ID of the newly created sound instance, which can be used by the script to later control it.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_2D_WAV_Sound_Player(Object, SoundName)
 *  \endcode
*  \param Object The ID of the object that will act as the sound source.
*  \param SoundName The file name (string) of the WAV file to be played.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
int Lua_Create_2D_WAV_Sound_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Player(obj, luaL_checkstring(L, 2)));
	return 1;
}

/*!\details Retrieves the total count of maps available in the game environment. This can be useful for boundary checks or scripting content that needs to cycle through map indices.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Map_Count()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns the total number of map indices supported by the system.
*/
int Lua_Get_Map_Count(lua_State *L)
{
	lua_pushnumber(L, Get_Map_Count());
	return 1;
}

/*!\details Controls the visibility of atmospheric fog around a specific player object. Used for scripted visual cues (e.g., marking an area as hostile or camouflaged).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Fog_Enable_Player(Object, Enable)
 *  \endcode
*  \param Object The ID of the player/object affected by fog settings.
*  \param Enable Boolean (`true` to enable; `false` to disable).
*  \return Number Returns 0 on success.
*/
int Lua_Set_Fog_Enable_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	bool enable = lua_tobooleanCPP(L, 2);
	Set_Fog_Enable_Player(obj, enable);
	return 0;
}

/*!\details Controls how far fog starts and ends relative to an object, simulating environmental density changes around a point of interest (e.g., base perimeter).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Fog_Range_Player(Object, StartDistance, EndDistance, Transition)
 *  \endcode
*  \param Object The ID of the object affected by fog range settings.
*  \param StartDistance Float The distance at which the fog begins (start).
*  \param EndDistance Float The distance at which the fog reaches maximum density (end).
*  \param Transition Float A transition factor controlling how quickly the effect changes between start and end distances.
*/
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

/*!\details Forces an object to begin playing a specified piece of background music (e.g., cinematic score or atmospheric loop). The music continues until overridden by other scripts.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Background_Music_Player(Object, MusicName)
 *  \endcode
*  \param Object The ID of the object associated with the background audio source.
*  \param MusicName The file name (string) of the music track to play.
*  \return Number Returns 0 on success.
*/
int Lua_Set_Background_Music_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	const char* music = luaL_checkstring(L, 2);
	Set_Background_Music_Player(obj, music);
	return 0;
}

/*!\details Gradually changes the volume of background music over time, useful for dramatic scene transitions or focusing player attention.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Fade_Background_Music_Player(Object, MusicName, DurationSeconds, FadeOutTime)
 *  \endcode
*  \param Object The ID of the object associated with the background audio source.
*  \param MusicName The name of the music track being faded.
*  \param x Integer The duration (time) for the fade effect start/transition time.
*  \param y Integer The completion time for the fade effect.
*/
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

/*!\details Controls whether the object's Heads-Up Display elements are visible or hidden. This is critical for scripted invisibility effects.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Enable_HUD_Player(Object, Enable)
 *  \endcode
*  \param Object The ID of the object whose HUD visibility is controlled.
*  \param Enable Boolean (`true` to enable; `false` to disable).
*  \return Number Returns 0 on success.
*/
int Lua_Enable_HUD_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	bool enable = lua_tobooleanCPP(L, 2);
	Enable_HUD_Player(obj, enable);

	return 0;
}

/*!\details Spawns and manages a temporary, localized 3D sound source for cinematic or environmental audio events. The instance ID is returned for script control (e.g., stopping the sound later).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_Sound_Player(Object, SoundName, Vector3, Object2)
 *  \endcode
*  \param Object The ID of the object acting as the primary source point.
*  \param SoundName The file name (string) of the sound to play.
*  \param Location A LuaVector3 defining the precise sound position.
*  \param OtherObject The optional secondary object used for spatial calculations or ownership.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
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

/*!\details Used for attaching sound effects (like machinery noise or voice lines) exactly to a skeletal joint of a target object. Requires both the parent object and the target bone name.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_3D_WAV_Sound_At_Bone_Player(Object, SoundName, TargetObject, BoneName)
 *  \endcode
*  \param Object The ID of the object acting as the sound emitter source.
*  \param SoundName The file name (string) of the WAV sound to play.
*  \param TargetObject The ID of the character model defining the bone structure.
*  \param BoneName The precise name of the skeletal joint on the target object where the sound should originate.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
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

/*!\details A general purpose function for spawning 3D sounds pinned directly to an object's specific skeletal bone.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_3D_Sound_At_Bone_Player(Object, SoundName, TargetObject, BoneName)
 *  \endcode
*  \param Object The ID of the object acting as the sound emitter source.
*  \param SoundName The file name (string) of the WAV sound to play.
*  \param TargetObject The ID of the character model defining the bone structure.
*  \param BoneName The precise name of the skeletal joint on the target object where the sound should originate.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
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

/*!\details Overwrites the atmospheric fog color around a specific player object. Used for scripted visual cues (e.g., marking an area as hostile or camouflaged).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Fog_Color_Player(Object, R, G, B)
 *  \endcode
*  \param Object The ID of the player whose fog color is being set.
*  \param R Red component (float 0-1).
*  \param G Green component (float 0-1).
*  \param B Blue component (float 0-1).
*  \return Number Returns 0 on success.
*/
int Lua_Set_Fog_Color_Player(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Fog_Color_Player(obj, luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
	return 0;
}

/*!\details Controls the global atmospheric fog mode around a player object. Requires checking predefined constants (1-3).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Fog_Mode_Player(Object, Mode)
 *  \endcode
*  \param Object The ID of the player affected by fog mode settings.
*  \param Mode Integer A constant value (1=EXP, 2=EXP2, 3=LINEAR).
*  \return Number Returns 1 on success.
*/
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

/*!\details Displays a numbered marker or icon overlay on an object's HUD. Useful for objective markers or indicating target priority.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Send_HUD_Number(Object, Mode)
 *  \endcode
*  \param Object The ID of the object that will display the number.
*  \param Mode Integer The style or mode of the indicator to show (e.g., 1 for flashing).
*  \return Number Returns 1 on success.
*/
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

/*!\details Retrieves the unique ID of an explosion object currently active in the game.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_ExplosionObj()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns the Game Object ID, or 0 if no explosion object is active.
*/
int Lua_GetExplosionObj(lua_State *L)
{
	lua_pushnumber(L, Get_ID(GetExplosionObj()));
	return 1;
}

/*!\details Sets a specific game object to be treated as the primary explosion source in the map. This controls scripted area-of-effect effects.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_ExplosionObj(Object)
 *  \endcode
*  \param Object The ID of the game object that should be flagged as the explosion origin.
*  \return Number Returns 1 on success, or 0 if the provided object is invalid.
*/
int Lua_SetExplosionObj(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	SetExplosionObj(obj);
	return 1;
}


/*!\details Retrieves the current fog start and end range limits set for the entire game.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local start, end = Get_Fog_Range()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns 2 (two floats) representing {startDistance, endDistance}.
*/
int Lua_Get_Fog_Range(lua_State *L)
{
	float start = 0.f;
	float end = 0.f;
	Get_Fog_Range(start, end); // is this correct way ?
	lua_pushnumber(L, start);
	lua_pushnumber(L, end);
	return 2;
}

/*!\details Retrieves the name of the current GDI soldier model.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_GDI_Soldier_Name()
 *  \endcode
*  \param None No arguments required.
*  \return String Returns the localized display name for the GDI model type.
*/
int Lua_Get_GDI_Soldier_Name(lua_State *L)
{
	lua_pushstring(L, Get_GDI_Soldier_Name());
	return 1;
}

/*!\details Retrieves the name of the current NOD soldier model.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Nod_Soldier_Name()
 *  \endcode
*  \param None No arguments required.
*  \return String Returns the localized display name for the NOD model type.
*/
int Lua_Get_Nod_Soldier_Name(lua_State *L)
{
	lua_pushstring(L, Get_Nod_Soldier_Name());
	return 1;
}

/*!\details Sets a customizable fog override range for a specific player. Used to force visual environmental effects despite global settings.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Fog_Override_Player(Object, StartDistance, EndDistance)
 *  \endcode
*  \param Object The ID of the object affected by fog override settings.
*  \param StartDistance Float The start distance for the fog override effect.
*  \param EndDistance Float The end distance for the fog override effect.
*  \return Number Returns 1 on success (though reliability is questionable).
*/
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

/*!\details Forces the entire map's atmosphere to instantly clear all scripted fog overlays.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Clear_Fog_Override_Player(Object)
 *  \endcode
*  \param Object The ID of the object whose localized fog should be cleared.
*  \return Number Returns 1 on success.
*/
int Lua_Clear_Fog_Override_Player(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Clear_Fog_Override_Player(obj);
	return 1;
}

/*!\details Displays a complex visual hint or prompt on the player's HUD. Supports multiple parameter types for rich scripting display. This function is highly advanced and intended for modding game tutorials/mechanics.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Display_Game_Hint(Object, CooldownSeconds, TitleString, XPos, YPos, ZPos, Duration)
 *  \endcode
*  \param Object The ID of the object receiving the hint.
*  \param Coordinates/Parameters Various numerical and string arguments defining the visual properties of the prompt.
*  \return Number Returns 1 on success.
*/
int Lua_Display_Game_Hint(lua_State *L)
{
	if (lua_gettop(L) < 7) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Display_Game_Hint(obj, luaL_checknumber(L, 2), luaL_checkstring(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6), luaL_checknumber(L, 7)); // untested
	return 1;
}

/*!\details Displays a complex visual hint using an image texture on the player's HUD. Used for advanced graphical tutorials and instructions.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Display_Game_Hint_Image(Object, CooldownSeconds, TitleString, XPos, YPos, ZPos, Duration, ImagePath)
 *  \endcode
*  \param Object The ID of the object receiving the hint.
*  \param Coordinates/Parameters Various numerical and string arguments defining the visual properties of the prompt.
*  \return Number Returns 1 on success.
*/
int Lua_Display_Game_Hint_Image(lua_State *L)
{
	if (lua_gettop(L) < 8) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Display_Game_Hint_Image(obj, luaL_checknumber(L, 2), luaL_checkstring(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6), luaL_checknumber(L, 7), luaL_checkstring(L, 8)); // untested
	return 1;
}

/*!\details Sets a global flag to disable all stealth and camouflage features across the entire game environment. Useful for triggering anti-stealth mechanics.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Global_Stealth_Disable(Enable)
 *  \endcode
*  \param Enable Boolean (`true` to enable global visibility; `false` otherwise).
*  \return Number Returns 1 on success.
*/
int Lua_Set_Global_Stealth_Disable(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Global_Stealth_Disable(lua_tobooleanCPP(L, 1));
	return 1;
}

/*!\details Retrieves the current technical level of the game/server instance. Useful for modifying scripted difficulty or content based on the running version.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Tech_Level()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns the current technical level integer.
*/
int Lua_Get_Tech_Level(lua_State *L)
{
	lua_pushnumber(L, Get_Tech_Level()); // unable to confirm if works. maybe not ren supported?
	return 1;
}

/*!\details Sets the technical level for the game instance. This can be used in advanced scripting scenarios to simulate different epochs or tech branches.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Tech_Level(NewLevel)
 *  \endcode
*  \param NewLevel Number The desired technical level integer.
*  \return Number Returns 0 on success.
*/
int Lua_Set_Tech_Level(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Set_Tech_Level(luaL_checknumber(L, 1)); // unable to confirm if works. maybe not ren supported?
	return 0;
}

/*!\details Creates a large-scale area of effect explosion at a defined position and targets a specific object. Used for triggering controlled demolition or tactical bombing runs.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_Explosion_Extended(SoundName, Vector3, Object)
 *  \endcode
*  \param SoundName The file name (string) of the explosion sound to play.
*  \param Location A LuaVector3 defining the blast center coordinates.
*  \param Object The ID of the target object near the blast zone.
*  \return Number Returns 0 on success.
*/
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

/*!\details Retrieves current global environmental metrics, including cloud cover and gloom intensity.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local cloud, gloom = Get_Clouds()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns 2 (two floats) representing {cloudCoverage, gloomLevel}.
*/
int Lua_Get_Clouds(lua_State *L)
{
	float cloud = 0.f;
	float gloom = 0.f;
	Get_Clouds(cloud, gloom);
	lua_pushnumber(L, cloud);
	lua_pushnumber(L, gloom);
	return 2;
}

/*!\details Retrieves current global weather conditions: lightning intensity and the coordinates defining its spread/heading across the map.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local intensity, start, end, heading, distribution = Get_Lightning()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns 5 (five floats) representing {intensity, start, end, heading, distribution}.
*/
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

/*!\details Retrieves current global wind conditions: direction, speed, and variability.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local heading, speed, variability = Get_Wind()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns 3 (three floats) representing {heading, speed, variability}.
*/
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

/*!\details Retrieves the current global rain density factor. Used for environmental scripting and visibility checks.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local density = Get_Rain()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns a single float representing the rain density (-99.0 means no visible rain).
*/
int Lua_Get_Rain(lua_State *L)
{
	float density = -99.f;
	Get_Rain(density);
	lua_pushnumber(L, (double)density);
	return 1;
}

/*!\details Retrieves the current global snow density factor. Used for environmental scripting and visibility checks.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local density = Get_Snow()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns a single float representing the snow density (-99.0 means no visible snow).
*/
int Lua_Get_Snow(lua_State *L)
{
	float density = -99.f;
	Get_Snow(density);
	lua_pushnumber(L, (double)density);
	return 1;
}

/*!\details Retrieves the current global ash density factor. Used for environmental scripting and visibility checks related to fire/dust.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local density = Get_Ash()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns a single float representing the ash density (-99.0 means no visible ash).
*/
int Lua_Get_Ash(lua_State *L)
{
	float density = -99.f;
	Get_Ash(density);
	lua_pushnumber(L, (double)density);
	return 1;
}

/*!\details Re-synchronizes all game options and internal states after significant scripted changes to prevent desync issues for local clients.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Update_Game_Options()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns 1 on success.
*/
int Lua_Update_Game_Options(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Update_Game_Options(luaL_checknumber(L, 1)); // wth does this even do?
	return 1;
}

/*!\details Retrieves the current version number of the scripting engine. Used for compatibility checks in modded environments.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local version = Get_TTVersion()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns the current major version number of the script engine.
*/
int Lua_GetTTVersion(lua_State *L)
{
	lua_pushnumber(L, GetTTVersion());
	return 1;
}

/*!\details Retrieves the current revision number of the scripting engine. Used for pinpointing specific patch versions.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_TTRevision()
 *  \endcode
*  \param None No arguments required.
*  \return Number Returns the current revision integer number.
*/
int Lua_GetTTRevision(lua_State *L)
{
	lua_pushnumber(L, GetTTRevision());
	return 1;
}

/*!\details Forces an immediate update of a game object's position in the server/client synchronization cycle. Used when physics or movement tracking fails to register intended movement.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Force_Position_Update(Object)
 *  \endcode
*  \param Object The ID of the object whose position needs forcing update.
*  \return Number Returns 1 on success.
*/
int Lua_Force_Position_Update(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Force_Position_Update(obj);
	return 1;
}

/*!\details Creates a specialized 2D WAV audio emitter for cinematic or mission-specific dialogues/narration.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_2D_Wave_Sound_Dialog_Player(Object, SoundName)
 *  \endcode
*  \param Object The ID of the object associated with the sound source.
*  \param SoundName The file name (string) of the dialogue track to play.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
int Lua_Create_2D_Wave_Sound_Dialog_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Create_2D_Wave_Sound_Dialog_Player(obj, luaL_checkstring(L, 2)));
	return 1;
}

/*!\details Forces an object to update its position relative to a secondary object. Useful for synchronizing two objects that should move together (e.g., parent/child models).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Force_Position_Update_Player(Object, SecondaryObject)
 *  \endcode
*  \param Object The ID of the primary object being updated.
*  \param SecondaryObject The ID of the secondary object defining the relative position anchor.
*  \return Number Returns 0 on success.
*/
int Lua_Force_Position_Update_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj || !obj2) return 0;
	Force_Position_Update_Player(obj, obj2);
	return 0;
}

/*!\details Sets an offset or secondary piece of background music to play relative to the main soundtrack. Used for layer effects (e.g., ambient crackling).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Background_Music_Player_Offset(Object, SoundName, Volume)
 *  \endcode
*  \param Object The ID of the object associated with the sound source.
*  \param SoundName The file name (string) of the secondary music track to play.
*  \param Volume Float The relative volume level for this offset track.
*  \return Number Returns 0 on success.
*/
int Lua_Set_Background_Music_Player_Offset(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Background_Music_Player_Offset(obj, luaL_checkstring(L, 2), luaL_checknumber(L, 3));
	return 0;
}

/*!\details Sets the translation ID that a game object should use for display purposes. Useful in localized modding when overriding standard name tags.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Definition_TranslationID_Player(Object, TranslationID, Enable)
 *  \endcode
*  \param Object The ID of the object whose translation ID is being set.
*  \param TranslationID The unique numerical identifier for the desired localized text.
*  \param Enable Boolean (`true` to activate custom localization).
*  \return Number Returns 0 on success.
*/
int Lua_Set_Definition_TranslationID_Player(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Definition_TranslationID_Player(obj, luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	return 0;
}

/*!\details Forces an object's orientation (rotation and pitch) to update immediately. Used when networking or physics updates lag behind scripting intent.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Force_Orientation_Update(Object)
 *  \endcode
*  \param Object The ID of the object whose orientation needs forcing update.
*  \return Number Returns 0 on success.
*/
int Lua_Force_Orientation_Update(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Force_Orientation_Update(obj);
	return 0;
}

/*!\details Forces two objects to synchronize their rotational state. Useful for scripting tethered or coupled machines that must maintain a consistent alignment relative to each other.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Force_Orientation_Update_Player(Object, SecondaryObject)
 *  \endcode
*  \param Object The ID of the primary object whose orientation is being forced.
*  \param SecondaryObject The ID of the secondary object dictating the target orientation.
*  \return Number Returns 0 on success.
*/
int Lua_Force_Orientation_Update_Player(lua_State *L)
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

/*!\details Forces two objects to synchronize their velocity state. Used for advanced physics scripting where the momentum of one object must immediately affect another.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Force_Velocity_Update_Player(Object, SecondaryObject)
 *  \endcode
*  \param Object The ID of the primary object whose velocity is being forced.
*  \param SecondaryObject The ID of the secondary object dictating the target momentum/velocity.
*  \return Number Returns 0 on success.
*/
int Lua_Force_Velocity_Update_Player(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj || !obj2) return 0;
	Force_Velocity_Update_Player(obj, obj2);
	return 0;
}

/*!\details Sets the object as the network host for camera viewing. This dictates which client's view is used when running multi-client networked scripting scenarios.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Camera_Host_Network(Object)
 *  \endcode
*  \param Object The ID of the object that should act as the camera host.
*  \return Number Returns 0 on success.
*/
int Lua_Set_Camera_Host_Network(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Camera_Host_Network(obj);
	return 0;
}

/*!\details Sends a chat message that highlights the team affiliation with a specified color. Used for critical tactical communications in multiplayer environments.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Send_Message_With_Team_Color(TeamID, MessageString)
 *  \endcode
*  \param TeamID The ID of the team whose color should be used for highlighting.
*  \param MessageString The text to send via chat.
*  \return Number Returns 0 on success.
*/
int Lua_Send_Message_With_Team_Color(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Send_Message_With_Team_Color(luaL_checknumber(L, 1), luaL_checkstring(L, 2));
	return 0;
}

/*!\details Sends a chat message that highlights the game object's color. Used for marking specific objects or team assets in multiplayer environments.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Send_Message_With_Obj_Color(Object, MessageString)
 *  \endcode
*  \param Object The ID of the object whose color should be used for highlighting.
*  \param MessageString The text to send via chat.
*  \return Number Returns 0 on success.
*/
int Lua_Send_Message_With_Obj_Color(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Send_Message_With_Obj_Color(obj, luaL_checkstring(L, 2));
	return 0;
}

/*!\details Sends a general team message using specific IDs and controls. Useful for coordinating large-scale scripted events or team announcements.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Send_Message_Team(TargetTeamID, SenderTeamID, SignalType, MessageString)
 *  \endcode
*  \param TargetTeamID The ID of the team receiving the message.
*  \param SenderTeamID The ID of the team sending the message.
*  \param SignalType An integer defining the channel or signal type (e.g., 1 for public).
*  \param MessageString The text to send via chat.
*  \return Number Returns 0 on success.
*/
int Lua_Send_Message_Team(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Message_Team(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checkstring(L, 5));
	return 0;
}

/*!\details Sends a translated team message using a specific translation ID. Used for ensuring scripted communications respect the game's localization files.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Send_Translated_Message_Team(TargetTeamID, SenderTeamID, TranslationID, MessageString, Force)
 *  \endcode
*  \param TargetTeamID The ID of the team receiving the message.
*  \param SenderTeamID The ID of the team sending the message.
*  \param TranslationID The unique translated string identifier.
*  \param MessageString A fallback or primary message string.
*  \param Force Boolean (`true`) to bypass default language checks.
*  \return Number Returns 0 on success.
*/
int Lua_Send_Translated_Message_Team(lua_State *L)
{
	if (lua_gettop(L) < 6) return 0;
	Send_Translated_Message_Team(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), lua_tobooleanCPP(L, 6));
	return 0;
}

/*!\details Sets the visual state (fade in/out) of all occupants within an object's physical boundaries. Useful for simulating smoke or cloaking fields around vehicles or bases.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Occupants_Fade(Object, StartAlpha, EndAlpha, Duration, TargetID)
 *  \endcode
*  \param Object The ID of the object whose occupants are being visually modified.
*  \param StartAlpha Float The starting visibility alpha (0-1).
*  \param EndAlpha Float The ending visibility alpha (0-1).
*  \param Duration Float Time duration for the fade effect in seconds.
*  \param TargetID Optional ID of a specific occupant to focus the fade on.
*  \return Number Returns 0 on success.
*/
int Lua_Set_Occupants_Fade(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) return 0;
	Set_Occupants_Fade(obj, luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5));
	return 0;
}

/*!\details Enables or disables the team radar display for a player object. Critical for tactical gameplay scripting and resource management.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Enable_Team_Radar(Object, Enable)
 *  \endcode
*  \param Object The ID of the player whose radar is being controlled.
*  \param Enable Boolean (`true` to enable; `false` to disable).
*  \return Number Returns 0 on success.
*/
int Lua_Enable_Team_Radar(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	Enable_Team_Radar(luaL_checknumber(L, 1), lua_tobooleanCPP(L, 2));
	return 0;
}

/*!\details Spawns and controls a localized team-based sound source. Used for communicating specific information to all members of the object's current team.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_Sound_Team(SoundName, Vector3, Object, Range)
 *  \endcode
*  \param SoundName The file name (string) of the sound to play.
*  \param Location A LuaVector3 defining the sound origin position.
*  \param Object The ID of a reference object for ownership/placement.
*  \param Range Float The effective transmission range for team members.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
int Lua_Create_Sound_Team(lua_State *L)
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

/*!\details Spawns and controls a general 2D WAV audio emitter for the entire team. Used for global cinematic effects or ambient background loops shared by all members of the current squad.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_2D_Sound_Team(SoundName, Volume)
 *  \endcode
*  \param SoundName The file name (string) of the sound to play.
*  \param Volume Float The global volume level for this team-wide audio effect.
*  \return Number Returns 1 on success.
*/
int Lua_Create_2D_Sound_Team(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_Sound_Team(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}

/*!\details Spawns and controls a specialized 2D WAV audio emitter for the entire team. Used when a specific sound needs to be linked to a general team broadcast (e.g., all vehicles playing a custom alert).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_2D_WAV_Sound_Team(SoundName, Volume)
 *  \endcode
*  \param SoundName The file name (string) of the sound to play.
*  \param Volume Float The global volume level for this team-wide audio effect.
*  \return Number Returns 1 on success.
*/
int Lua_Create_2D_WAV_Sound_Team(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Team(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}

/*!\details Spawns and controls a specialized cinematic sound emitter for the entire team. Used for highly noticeable, dramatic announcements or mission triggers.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_2D_WAV_Sound_Team_Dialog(SoundName, Volume)
 *  \endcode
*  \param SoundName The file name (string) of the dialogue track to play.
*  \param Volume Float The global volume level for this team-wide audio effect.
*  \return Number Returns 1 on success.
*/
int Lua_Create_2D_WAV_Sound_Team_Dialog(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Team_Dialog(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}

/*!\details Spawns and controls a specialized cinematic sound emitter for the entire team. Used for mission objective triggers or major structural announcements.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_2D_WAV_Sound_Team_Cinematic(SoundName, Volume)
 *  \endcode
*  \param SoundName The file name (string) of the cinematic track to play.
*  \param Volume Float The global volume level for this team-wide audio effect.
*  \return Number Returns 1 on success.
*/
int Lua_Create_2D_WAV_Sound_Team_Cinematic(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Create_2D_WAV_Sound_Team_Cinematic(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
	return 1;
}

/*!\details Spawns and controls a localized team-based sound emitter attached to the bone of another object.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_3D_WAV_Sound_At_Bone_Team(SoundName, Object, BoneName, Range)
 *  \endcode
*  \param SoundName The file name (string) of the WAV sound to play.
*  \param Object The ID of the object acting as the sound emitter source.
*  \param BoneName The precise name of the skeletal joint on the target object where the sound should originate.
*  \param Range Float The effective transmission range for team members.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
int Lua_Create_3D_WAV_Sound_At_Bone_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj) return 0;
	lua_pushnumber(L, Create_3D_WAV_Sound_At_Bone_Team(luaL_checkstring(L, 1), obj, luaL_checkstring(L, 3), luaL_checknumber(L, 4)));
	return 1;
}


/*!\details Spawns and controls a general team-based 3D sound emitter attached to a bone.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Create_3D_Sound_At_Bone_Team(SoundName, Object, BoneName)
 *  \endcode
*  \param SoundName The file name (string) of the sound to play.
*  \param Object The ID of the object acting as the sound emitter source.
*  \param BoneName The precise name of the skeletal joint on the target object where the sound should originate.
*  \return Number Returns the unique sound instance ID (or 0 if creation fails).
*/
int Lua_Create_3D_Sound_At_Bone_Team(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 2));
	if (!obj) return 0;
	lua_pushnumber(L, Create_3D_Sound_At_Bone_Team(luaL_checkstring(L, 1), obj, luaL_checkstring(L, 3), luaL_checknumber(L, 4)));
	return 1;
}

/*!\details Forces the destruction of all buildings belonging to a specified team. Used for scripted scenarios like enemy base collapse or cleanup operations.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Kill_All_Buildings_By_Team(TeamID)
 *  \endcode
*  \param TeamID The ID of the team whose buildings should be destroyed.
*  \return Number Returns 0 on success.
*/
int Lua_Kill_All_Buildings_By_Team(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	Kill_All_Buildings_By_Team(luaL_checknumber(L, 1));
	return 0;
}

/*!\details Sends a scripted custom event to all objects on the map that are equipped with scripting capabilities. This is used for triggering coordinated actions across multiple spawned units.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Send_Custom_Event_To_Objects_With_Script(Object, EventName, Param1, Param2, Param3)
 *  \endcode
*  \param Object The primary object used as a reference point for the event.
*  \param EventName The string identifier of the custom event to trigger (e.g., "AmbushStart").
*  \param Param1 Float First parameter data associated with the event.
*  \param Param2 Float Second parameter data associated with the event.
*  \param Param3 Integer Third parameter data associated with the event.
*  \return Number Returns 1 on success.
*/
int Lua_Send_Custom_Event_To_Objects_With_Script(lua_State *L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Custom_Event_To_Objects_With_Script(Commands->Find_Object(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}

/*!\details Forces the object's animation state to a specific named frame or sequence within its current action cycle. Used for cinematic cutscenes or forced combat poses.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Animation_Frame(Object, FrameName, Force)
 *  \endcode
*  \param Object The ID of the object whose animation frame is being set.
*  \param FrameName The name of the specific animation sequence or frame.
*  \param Force Boolean (`true`) to forcibly override current animation state.
*  \return Number Returns 0 on success.
*/
int Lua_Set_Animation_Frame(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	Commands->Set_Animation_Frame(Commands->Find_Object(luaL_checknumber(L, 1)), luaL_checkstring(L, 2), luaL_checknumber(L, 3));
	return 0;
}

/*!\details Retrieves the unique translation ID for a given game object based on its class definition. This raw ID is useful for scripting systems that interact with localization databases directly.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Translated_Name_ID(Object)
 *  \endcode
*  \param Object The ID of the object to query.
*  \return Number Returns the unique translation database ID, or 0 if not found.
*/
int Lua_Get_Translated_Name_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 1));
	if (!obj) { return 0; }
	lua_pushnumber(L, obj->As_DamageableGameObj()->Get_Translated_Name_ID());
	return 1;
}

/*!\details Retrieves the current bone position (coordinates) for a specified skeletal joint on a given object. Essential for scripting attachment points for weapons or effects.
 *  \par Command Usage Example
 *  \code{.lua}
 *			local vec = Get_Bone_Position(Object, BoneName) -- Note: LuaVector3 needs manual reconstruction from return data
 *  \endcode
*  \param Object The ID of the object being analyzed.
*  \param BoneName The name of the skeletal joint (bone).
*  \return Number Returns 1 and pushes a new `LuaVector3` structure onto the stack containing the XYZ coordinates.
*/
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

/*!\details Resets the object's action state to its default standing/ready position and behavior. Used after scripted combat or positional manipulation.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Action_Reset(Object, ResetForce)
 *  \endcode
*  \param Object The ID of the object to reset.
*  \param ResetForce Boolean (`true`) if a force should be applied during the reset process.
*  \return Number Returns 1 on success.
*/
int Lua_Action_Reset(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Action_Reset(obj, lua_tonumber(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

/*!\details Grants currency (money) to a specified object. Used by vendors or scripted financial transactions.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Give_Money(Object, Amount, Force)
 *  \endcode
*  \param Object The ID of the recipient object.
*  \param Amount Number of money to grant.
*  \param Force Boolean (`true`) if funds must be granted regardless of current balance.
*  \return Number Returns 1 on success.
*/
int Lua_Give_Money(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Give_Money(obj, lua_tonumber(L, 2), lua_tobooleanCPP(L, 3));
	lua_pushboolean(L, 1);
	return 1;
}


/*!\details Modifies the available definition presets for a specific object's equipment slot. Used in advanced scripting to simulate dynamic armor changes or weapon customization.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Alt_Definition(Type, Team, Slot, AltPresetID, NewPresetID)
 *  \endcode
*  \param Type Integer The category type (e.g., Armor).
*  \param Team Integer The team associated with the equipment slot.
*  \param Slot Integer The specific item slot being modified (e.g., 1 for chest, 2 for gloves).
*  \param Alt Integer A secondary value or flag used in combination with Type/Team.
*  \param PresetID The unique ID of the new preset definition to enable.
*  \return Number Returns 1 on success.
*/
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

/*!\details Overwrites the primary definition preset for a specific object slot. Used by vendors or scripted equipment equipping systems.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Set_Definition(Type, Team, Slot, PresetID)
 *  \endcode
*  \param Type Integer The category type (e.g., Armor).
*  \param Team Integer The team associated with the equipment slot.
*  \param Slot Integer The specific item slot being modified.
*  \param PresetID The unique ID of the new preset definition to equip.
*  \return Number Returns 1 on success.
*/
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

/*!\details Forces a vehicle to be locked onto a specific target object for the duration of a timer. Used by scripted security protocols or convoy escorts.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Lock_Vehicle(Object, TargetObject, DurationSeconds)
 *  \endcode
*  \param Object The ID of the vehicle to be locked onto the target.
*  \param TargetObject The ID of the object that must remain in range for the duration.
*  \param Timer Integer The duration (in seconds) that the lock will persist.
*  \return Number Returns 1 on success.
*/
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

/*!\details Retrieves the unique game object ID associated with a given preset definition.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Preset_ID(Object)
 *  \endcode
*  \param Object The ID of the object containing the desired preset.
*  \return Number Returns the unique ID associated with the equipped preset, or 0 if none is found.
*/
int Lua_Get_Preset_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Commands->Get_Preset_ID(obj));
	return 1;
}

/*!\details Forces the object to fire a currently equipped weapon at a specified target world location. Useful for scripted automatic weapons or directed combat actions.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Trigger_Weapon(Object, AimingMode, TargetLocation, IsPrimary)
 *  \endcode
*  \param Object The ID of the object firing the weapon.
*  \param ReadyState Boolean (`true` to force full readiness).
*  \param Target A LuaVector3 defining the target coordinates for aiming.
*  \param Primary Fire Boolean (`true`) specifying if primary fire is intended.
*  \return Number Returns 1 on success.
*/
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

/*!\details Retrieves the action ID assigned to an object by its current game state definition. Used in scripting to categorize and identify the nature of a unit's active role (e.g., Combatant, Support).
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Action_ID(Object)
 *  \endcode
*  \param Object The ID of the object whose action state is being queried.
*  \return Number Returns the integer action ID, or 0 if not found.
*/
int Lua_Get_Action_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, Commands->Get_Action_ID(obj));
	return 1;
}

/*!\details Sends a custom event notification to all scripted objects on the map. Used to trigger complex, synchronized gameplay logic (e.g., "AlarmSound", "IncomingSignal").
 *  \par Command Usage Example
 *  \code{.lua}
 *			Send_Custom_Event(Object, EventName, Param1, Param2, Param3)
 *  \endcode
*  \param Object The primary object used as a reference point.
*  \param EventName The string identifier of the custom event to trigger (e.g., "AlarmSound").
*  \param Param1 Float First parameter data associated with the event.
*  \param Param2 Float Second parameter data associated with the event.
*  \param Param3 Integer Third parameter data associated with the event.
*  \return Number Returns 1 on success.
*/
int Lua_Send_Custom_Event(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	Commands->Send_Custom_Event(Commands->Find_Object(lua_tonumber(L, 1)), Commands->Find_Object(lua_tonumber(L, 2)), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 1;
}

/*!\details Adds points/score to a specified object. Used for point-based scoring or achieving objectives.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Give_Points(Object, Amount, Force)
 *  \endcode
*  \param Object The ID of the recipient object.
*  \param Amount Integer number of points to grant.
*  \param Force Boolean (`true`) if points must be granted regardless of current score rules.
*  \return Number Returns 1 on success.
*/
int Lua_Give_Points(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Give_Points(obj, lua_tonumber(L, 2), lua_tobooleanCPP(L, 3));
	lua_pushboolean(L, 1);
	return 1;
}

/*!\details Enables an object to act as a source for spawning units or resources. Used by automated defense systems or deployable bases.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Enable_Spawner(Object, Enable)
 *  \endcode
*  \param Object The ID of the object that can spawn content.
*  \param Enable Boolean (`true` to allow spawning; `false` otherwise).
*  \return Number Returns 0 on success.
*/
int Lua_Enable_Spawner(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Enable_Spawner(lua_tonumber(L, 1), lua_tobooleanCPP(L, 2));
	return 0;
}

/*!\details Searches the game world for an object matching a given preset and name definition. Returns the ID of the found object instance.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Find_Object_By_Preset(Type, Name)
 *  \endcode
*  \param Type Integer The general category type (e.g., "Building").
*  \param Name String The specific name of the preset to search for.
*  \return Number Returns the Game Object ID found, or 0 if no matching object exists.
*/
int Lua_Find_Object_By_Preset(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	lua_pushnumber(L, Get_ID(Find_Object_By_Preset(lua_tonumber(L, 1), lua_tostring(L, 2))));
	return 1;
}

/*!\details Toggles the object's visible detection status for enemy AI. Controls whether nearby enemies can sense the presence of this unit via visual means.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Enable_Enemy_Seen(Object, Enable)
 *  \endcode
*  \param Object The ID of the object whose detection status is being controlled.
*  \param Enable Boolean (`true` to enable enemy sight; `false` otherwise).
*  \return Number Returns 1 on success.
*/
int Lua_Enable_Enemy_Seen(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Enable_Enemy_Seen(obj, lua_tobooleanCPP(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

/*!\details Toggles the object's hibernation state. When enabled, the object effectively becomes non-functional until manually revived or triggered by a scripted event.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Enable_Hibernation(Object, Enable)
 *  \endcode
*  \param Object The ID of the object whose hibernation status is being controlled.
*  \param Enable Boolean (`true` to enter hibernation; `false` otherwise).
*  \return Number Returns 1 on success.
*/
int Lua_Enable_Hibernation(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Enable_Hibernation(obj, lua_tobooleanCPP(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

/*!\details Initiates the inherent AI capabilities for a given object. This effectively 'wakes up' passive units and allows them to process enemy sightings, combat status, etc.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Innate_Enable(Object)
 *  \endcode
*  \param Object The ID of the object whose innate functions are being enabled.
*  \return Number Returns 1 on success.
*/
int Lua_Innate_Enable(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Innate_Enable(obj);
	lua_pushboolean(L, 1);
	return 1;
}

/*!\details Retrieves the localized display name for a game object's preset definition. This is useful when scripting requires displaying readable names rather than internal IDs.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_Translated_Preset_Name_Ex(Object)
 *  \endcode
*  \param Object The ID of the object to get the name from.
*  \return String Returns the translated display name.
*/
int Lua_Get_Translated_Preset_Name_Ex(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushstring(L, Get_Translated_Preset_Name_Ex(obj));
	return 1;
}

/*!\details Checks if one smart object can visually detect another physical object from its current vantage point. Used for scripting line-of-sight checks based on AI physics models.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Is_Obj_Visible(Object, Object2)
 *  \endcode
*  \param Object The ID of the detecting object (must be smart).
*  \param Object2 The ID of the object potentially being seen.
*  \return Number Returns 1 and pushes a boolean indicating visibility.
*/
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

/*!\details Creates and returns a dedicated LuaActionParams structure. This complex object is necessary when calling specific gameplay functions that require structured input parameters (like targeting data).
 *  \par Command Usage Example
 *  \code{.lua}
 *			local params = ActionParamsStruct()
 *  \endcode
*  \param None No arguments required.
*  \return Table Returns a Lua table wrapper of the necessary action parameters struct.
*/
int Lua_ActionParamsStruct(lua_State *L)
{
	LuaActionParams *a = new LuaActionParams;
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "ActionParams");
	lua_setmetatable(L, -2);
	return 1;
}

/*!\details Executes an A* pathfinding query to find the nearest reachable location within the map relative to a starting point. Useful for scripting movement targets that respect game geometry.
 *  \par Command Usage Example
 *  \code{.lua}
 *			Get_A_Star(Vector3)
 *  \endcode
*  \param Location The initial LuaVector3 coordinates where pathfinding should begin.
*  \return Number Returns 1 and sets the Lua stack with the Game Object ID of the calculated target location node.
*/
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

/*!\details Commands an object to move toward a specified destination using the engine's navigation system.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Action_Goto(Object, ActionParams)
 *  \endcode
 *  \param Object ID of the object that should execute the GOTO action.
 *  \param ActionParams Table containing movement parameters (see LuaActionParams).
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Commands an object to attack a target using the parameters supplied in the ActionParams table.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Action_Attack(Object, ActionParams)
 *  \endcode
 *  \param Object ID of the attacking object.
 *  \param ActionParams Table containing attack parameters (range, deviation, etc.).
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Instructs an object to play a specified animation defined in the ActionParams table.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Action_Play_Animation(Object, ActionParams)
 *  \endcode
 *  \param Object ID of the object that should play the animation.
 *  \param ActionParams Table containing animation data (animation name, frame, etc.).
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Instructs an object to enter or exit a location (e.g., entering a vehicle or a building) based on the parameters in ActionParams.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Action_Enter_Exit(Object, ActionParams)
 *  \endcode
 *  \param Object ID of the object that will enter/exit.
 *  \param ActionParams Table containing enter/exit parameters (target position, speed, etc.).
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Commands an object to face a target location supplied in ActionParams.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Action_Face_Location(Object, ActionParams)
 *  \endcode
 *  \param Object ID of the object that should change orientation.
 *  \param ActionParams Table containing the target position and optional heading offsets.
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Commands an object to dock with a target using the parameters in ActionParams.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Action_Dock(Object, ActionParams)
 *  \endcode
 *  \param Object ID of the object that will perform the docking action.
 *  \param ActionParams Table containing dock target and timing information.
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Commands an object to follow user input as defined in ActionParams.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Action_Follow_Input(Object, ActionParams)
 *  \endcode
 *  \param Object ID of the object that will follow the input.
 *  \param ActionParams Table containing follow parameters (input source, thresholds, etc.).
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Modifies an existing action identified by `id` for a given object. Parameters are supplied in ActionParams and modification flags control movement and attack updates.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Modify_Action(Object, ActionID, ActionParams, ModifyMove, ModifyAttack)
 *  \endcode
 *  \param Object ID of the object whose action is being modified.
 *  \param ActionID Integer identifying the specific action to modify.
 *  \param ActionParams Table containing new movement or attack parameters.
 *  \param ModifyMove Boolean (`true` to modify movement component).
 *  \param ModifyAttack Boolean (`true` to modify attack component).
 *  \return Number Returns 1 and pushes `true` onto the stack to indicate success.
*/
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

/*!\details Retrieves the current ActionParams structure associated with an object’s active action. Returns a Lua table representation of the parameters.
 *  \par Command Usage Example
 *  \code{.lua}
 *          params = Get_Action_Params(Object)
 *  \endcode
 *  \param Object ID of the object whose action parameters are requested.
 *  \return Table Returns an `ActionParamsStruct` table containing movement, attack, and other action data.
*/
int Lua_Get_Action_Params(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject *obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;

	ActionParamsStruct *a = new ActionParamsStruct;
	Commands->Get_Action_Params(obj, *a); // TODO? must be tested!

	if (a == nullptr)
		return 0;

	lua_boxpointer(L, a);
	luaL_getmetatable(L, "ActionParamsStruct");
	lua_setmetatable(L, -2);

	return 1;
}

/*!\details Checks whether an object possesses a key of a specified type.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Has_Key(Object, KeyType)
 *  \endcode
 *  \param Object ID of the object to query.
 *  \param KeyType Integer representing the key category (e.g., door lock, vehicle).
 *  \return Number Returns `true` if the object has the key; otherwise returns `false`.
*/
int Lua_Has_Key(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;

	lua_pushboolean(L, Commands->Has_Key(obj, luaL_checknumber(L, 2)));
	return 1;
}

/*!\details Queries whether a building currently has power supplied to it.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Get_Building_Power(Building)
 *  \endcode
 *  \param Building ID of the building object to query.
 *  \return Number Returns `true` if the building is powered; otherwise returns `false`.
*/
int Lua_Get_Building_Power(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushboolean(L, Commands->Get_Building_Power(obj));
	return 1;
}

/*!\details Enables or disables power for a building object.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Set_Building_Power(Building, Enable)
 *  \endcode
 *  \param Building ID of the building to modify.
 *  \param Enable Boolean (`true` to supply power; `false` to cut power).
 *  \return Number Returns `true` on success (the value is always returned as boolean for consistency).
*/
int Lua_Set_Building_Power(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Set_Building_Power(obj, lua_tobooleanCPP(L, 2));
	lua_pushboolean(L, 1);
	return 1;
}

/*!\details Removes all collision relationships for a given object, effectively making it non-collidable with other objects.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Disable_All_Collisions(Object)
 *  \endcode
 *  \param Object ID of the object whose collisions are to be disabled.
*/
int Lua_Disable_All_Collisions(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Disable_All_Collisions(obj);
	return 0;
}

/*!\details Turns the internal engine of a vehicle on or off.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Enable_Engine(Vehicle, Enable)
 *  \endcode
 *  \param Vehicle ID of the vehicle object.
 *  \param Enable Boolean (`true` to start engine; `false` to stop).
*/
int Lua_Enable_Engine(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Enable_Engine(obj, lua_tobooleanCPP(L, 2));
	return 0;
}

/*!\details Tests whether one object can see another from its current perspective.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Is_Object_Visible(Looker, Target)
 *  \endcode
 *  \param Looker ID of the observing object.
 *  \param Target ID of the object to test for visibility.
 *  \return Number Returns `true` if visible; otherwise returns `false`.
*/
int Lua_Is_Object_Visible(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* looker = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 2));
	if (!obj || !looker) return 0;
	lua_pushboolean(L, Commands->Is_Object_Visible(looker, obj));
	return 1;
}

/*!\details Disables the innate AI processing for an object (e.g., stops it from reacting to enemies or environment).
 *  \par Command Usage Example
 *          Innate_Disable(Object)
 *
 *  \param Object ID of the object to disable.
*/
int Lua_Innate_Disable(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Innate_Disable(obj);
	return 1;
}

/*!\details Retrieves the bullseye position of a soldier object (the point that represents where the soldier is aiming).
 *  \par Command Usage Example
 *          Bullseye_Position(Soldier)
 *
 *  \param Soldier ID of the soldier object.
 *  \return Table Returns a `Vector3` table representing the bullseye world coordinates.
*/
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

/*!\details Determines if two damageable objects are considered enemies of one another.
 *  \par Command Usage Example
 *          Is_Enemy(Object1, Object2)
 *
 *  \param Object1 ID of the first object.
 *  \param Object2 ID of the second object.
 *  \return Number Returns `true` if the two objects are enemies; otherwise returns `false`.
*/
int Lua_Is_Enemy(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj1 = Commands->Find_Object(lua_tonumber(L, 1));
	GameObject* obj2 = Commands->Find_Object(lua_tonumber(L, 2));
	if (!obj1 || !obj2 || !obj1->As_DamageableGameObj() || !obj2->As_DamageableGameObj()) return 0;
	lua_pushboolean(L, obj1->As_DamageableGameObj()->Is_Enemy(obj2->As_DamageableGameObj()));
	return 1;
}

/*!\details Sets the player type for an object (e.g., GDI or NOD).
 *  \par Command Usage Example
 *          Set_Player_Type(Object, Type)
 *
 *  \param Object ID of the object to modify.
 *  \param Type Integer representing the new player type (1=GDI, 2=NOD, etc.).
*/
int Lua_Set_Player_Type(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Set_Player_Type(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2));
	return 1;
}

/*!\details Grants a power-up item to an object.
 *  \par Command Usage Example
 *          Give_PowerUp(Object, PowerName, Force)
 *
 *  \param Object ID of the recipient.
 *  \param PowerName String name of the power-up preset.
 *  \param Force Boolean (`true` to force application even if conditions would normally prevent it).
*/
int Lua_Give_PowerUp(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Give_PowerUp(obj, luaL_checkstring(L, 2), lua_tobooleanCPP(L, 3));
	return 1;
}

/*!\details Forces any active power-up on an object to expire immediately.
 *  \par Command Usage Example
 *          Expire_Powerup(Object)
 *
 *  \param Object ID of the object whose power-up should be removed.
*/
int Lua_Expire_Powerup(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Expire_Powerup(obj);
	return 1;
}

/*!\details Enables or disables scripted firing of weapons for a vehicle.
 *  \par Command Usage Example
 *          Set_Scripts_Can_Fire(Vehicle, Enable)
 *
 *  \param Vehicle ID of the vehicle object.
 *  \param Enable Boolean (`true` to allow script-initiated fire; `false` otherwise).
*/
int Lua_Set_Scripts_Can_Fire(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	obj->As_VehicleGameObj()->Set_Scripts_Can_Fire(lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Assigns a collision group ID to an object’s physical representation.
 *  \par Command Usage Example
 *          Set_Collision_Group(Object, GroupID)
 *
 *  \param Object ID of the object to modify.
 *  \param GroupID Integer representing the new collision group.
*/
int Lua_Set_Collision_Group(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	obj->As_PhysicalGameObj()->Set_Collision_Group(luaL_checknumber(L, 2));
	return 1;
}

/*!\details Retrieves the collision group ID of an object's physical representation.
 *  \par Command Usage Example
 *          Get_Collision_Group(Object)
 *
 *  \param Object ID of the object to query.
 *  \return Number Returns the current collision group identifier.
*/
int Lua_Get_Collision_Group(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	lua_pushnumber(L, obj->As_PhysicalGameObj()->Get_Collision_Group());
	return 1;
}

/*!\details Sets whether a vehicle object should be considered immovable by physics simulation.
 *  \par Command Usage Example
 *          Set_Immovable(Vehicle, Enable)
 *
 *  \param Vehicle ID of the vehicle to modify.
 *  \param Enable Boolean (`true` to lock position; `false` to allow movement).
*/
int Lua_Set_Immovable(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	obj->As_VehicleGameObj()->Set_Immovable(lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Returns a list of all soldier (Star) objects currently in the game.
 *  \par Command Usage Example
 *          stars = Get_All_Stars()
 *
 *  \param None No arguments required.
 *  \return Table A table containing the IDs of every star object found.
*/
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

/*!\details Returns a list of all beacon objects currently in the game.
 *  \par Command Usage Example
 *          beacons = Get_All_Beacons()
 *
 *  \param None No arguments required.
 *  \return Table A table containing the IDs of every beacon object found.
*/
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

/*!\details Creates a global 2-D WAV sound that is not tied to any object.
 *  \par Command Usage Example
 *          soundID = Create_2D_WAV_Sound(SoundName)
 *
 *  \param SoundName String name of the WAV file to play.
 *  \return Number Returns the ID of the created sound instance, or 0 if creation failed.
*/
int Lua_Create_2D_WAV_Sound(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, Commands->Create_2D_WAV_Sound(lua_tostring(L, 1)));
	return 1;
}

/*!\details Locks a soldier object’s collision group to prevent it from colliding with other objects of the same or different groups.
 *  \par Command Usage Example
 *          Lock_Soldier_Collision_Group(Soldier, GroupType)
 *
 *  \param Soldier ID of the soldier object.
 *  \param GroupType Integer representing the new collision group type.
*/
int Lua_Lock_Soldier_Collision_Group(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	Lock_Soldier_Collision_Group(obj, static_cast<Collision_Group_Type>(lua_tointeger(L, 2)) );
	return 1;
}

/*!\details Enables or disables stealth for a vehicle when it is empty.
 *  \par Command Usage Example
 *          Set_Allow_Stealth_While_Empty(Vehicle, Enable)
 *
 *  \param Vehicle ID of the vehicle object.
 *  \param Enable Boolean (`true` to allow stealth while empty; `false` otherwise).
*/
int Lua_Set_Allow_Stealth_While_Empty(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_VehicleGameObj()) return 0;
	obj->As_VehicleGameObj()->Set_Allow_Stealth_While_Empty(lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Returns a table of all bone names belonging to an object’s model.
 *  \par Command Usage Example
 *          bones = Get_All_Bone_Names(Object)
 *
 *  \param Object ID of the object whose model bones are queried.
 *  \return Table A list of bone name strings indexed by integer position. Returns `nil` if the object has no physical model.
*/
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

/*!\details Grants a soldier object permission to drive vehicles.
 *  \par Command Usage Example
 *          Set_Can_Drive_Vehicles(Soldier, Enable)
 *
 *  \param Soldier ID of the soldier object.
 *  \param Enable Boolean (`true` to allow driving; `false` otherwise).
*/
int Lua_Set_Can_Drive_Vehicles(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	obj->As_SoldierGameObj()->Set_Can_Drive_Vehicles(lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Stops a previously created sound instance.
 *  \par Command Usage Example
 *          Stop_Sound(SoundID, Interrupt)
 *
 *  \param SoundID Integer ID of the sound to stop.
 *  \param Interrupt Boolean (`true` to immediately interrupt playback; `false` to allow it to finish naturally).
*/
int Lua_Stop_Sound(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Commands->Stop_Sound(luaL_checknumber(L, 1), lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Starts playback of a previously created sound instance.
 *  \par Command Usage Example
 *          Start_Sound(SoundID)
 *
 *  \param SoundID Integer ID of the sound to play.
*/
int Lua_Start_Sound(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Commands->Start_Sound(luaL_checknumber(L, 1));
	return 1;
}

/*!\details Enables or disables monitoring of a particular sound instance for changes such as volume or position.
 *  \par Command Usage Example
 *          Monitor_Sound(SoundID, Enable)
 *
 *  \param SoundID Integer ID of the sound to monitor.
 *  \param Enable Boolean (`true` to enable monitoring; `false` to disable).
*/
int Lua_Monitor_Sound(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj) return 0;
	Commands->Monitor_Sound(obj, luaL_checknumber(L, 2));
	return 1;
}

/*!\details Creates a spatial sound at a specified position, optionally attached to an object.
 *  \par Command Usage Example
 *          soundID = Create_Sound(SoundName, Position, TargetObject)
 *
 *  \param SoundName String name of the WAV file to play.
 *  \param Position LuaVector3 defining the world coordinates for the sound source.
 *  \param TargetObject ID of an object to attach the sound to (can be `nil`).
 *  \return Number Returns the created sound instance ID, or 0 if creation failed.
*/
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

/*!\details Returns the current health value of a building’s defense object.
 *  \par Command Usage Example
 *          health = Get_Defense_Object_Health(Building)
 *
 *  \param Building ID of the building to query.
 *  \return Number Returns the integer health value, or `nil` if not applicable.
*/
int Lua_Get_Defense_Object_Health(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_BuildingGameObj()) { return 0; }
	lua_pushnumber(L, obj->As_BuildingGameObj()->Get_Defense_Object()->Get_Health());
	return 1;
}

/*!\details Sets the gravity multiplier for a physical object’s movable component.
 *  \par Command Usage Example
 *          Set_Gravity_Multiplier(Object, Multiplier)
 *
 *  \param Object ID of the physical object to modify.
 *  \param Multiplier Float value specifying how many times the default gravity should be applied (e.g., 0.5 = half gravity).
*/
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

/*!\details Retrieves the current gravity multiplier for a physical object's movable component.
 *  \par Command Usage Example
 *          multiplier = Get_Gravity_Multiplier(Object)
 *
 *  \param Object ID of the physical object to query.
 *  \return Number Returns the current gravity multiplier value, or `nil` if not applicable.
*/
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

/*!\details Retrieves a table of PlayerData values for the given soldier object.
 *  \par Command Usage Example
 *  \code{.lua}
 *          data = Get_Player_Data(Soldier)
 *  \endcode
 *  \param Object ID of the soldier to query.
 *  \return Table A dictionary containing score, money, timer, session time and various shot/kill statistics.
 */
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

/*!\details Causes a specified player to have the camera shake effect.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Shake_Camera_Player(PlayerID, Position, XRadius, YRadius, ZRadius)
 *  \endcode
 *  \param PlayerID ID of the player whose camera will shake.
 *  \param Position LuaVector3 representing the center of the shake area.
 *  \param XRadius Float value for horizontal shaking range.
 *  \param YRadius Float value for vertical shaking range.
 *  \param ZRadius Float value for depth shaking range.
 */
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

/*!\details Marks an object as scheduled for deletion by the engine.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Set_Delete_Pending(Object)
 *  \endcode
 *  \param Object ID of the object to be deleted on next cleanup cycle.
 */
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

/*!\details Forces an object to be flagged as delete-pending immediately.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Set_Is_Delete_Pending(Object, Flag)
 *  \endcode
 *  \param Object ID of the target object.
 *  \param Flag Boolean (`true` to mark pending deletion; `false` to cancel).
 */
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

/*!\details Removes all active C4 beacons from the map.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Disarm_All_C4_Beacons()
 *  \endcode
 *  \param None No arguments required.
 */
int Lua_Disarm_All_C4_Beacons(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Disarm_All_C4_Beacons(lua_tonumber(L, 1));
	return 1;
}

/*!\details Sends a network update packet for a specific object to the client that owns it.
 *  \par Command Usage Example
 *  \code{.lua}
 *          Update_Network_Object_Player(Object, PlayerID)
 *  \endcode
 *  \param Object ID of the object to sync.
 *  \param PlayerID Numeric identifier of the target player/client.
 */
int Lua_Update_Network_Object_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	Update_Network_Object_Player(obj, lua_tonumber(L, 2));
	return 1;
}

/*!\details Clears the dirty flag bits on a given object.
 *  \par Command Usage Example
 *          Clear_Object_Dirty_Bits(Object)
 *
 *  \param Object ID of the object to clean.
 */
int Lua_Clear_Object_Dirty_Bits(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	obj->Clear_Object_Dirty_Bits();
	return 1;
}

/*!\details Returns the armor warhead type associated with a weapon definition name.
 *  \par Command Usage Example
 *          warhead = Get_Warhead_Type(WeaponName)
 *
 *  \param WeaponName String name of the weapon whose warhead is queried.
 *  \return Number The integer identifier for the warhead, or `0` if not found.
 */
int Lua_Get_Warhead_Type(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, ArmorWarheadManager::Get_Warhead_Type(lua_tostring(L, 1)));
	return 1;
}

/*!\details Determines whether a vehicle’s transition mode is enabled.
 *  \par Command Usage Example
 *          enabled = Are_Transitions_Enabled(Vehicle)
 *
 *  \param Vehicle ID of the vehicle to query.
 *  \return Number `true` if transitions are allowed; otherwise `false`.
 */
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

/*!\details Returns a list of all C4 beacons present in the current map.
 *  \par Command Usage Example
 *          c4list = Get_All_C4()
 *
 *  \param None No arguments required.
 *  \return Table A table containing IDs of each C4 beacon object found.
 */
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

/*!\details Checks whether a vehicle object has a turret component.
 *  \par Command Usage Example
 *          hasTurret = Has_Turret(Vehicle)
 *
 *  \param Vehicle ID of the vehicle to test.
 *  \return Number `true` if a turret exists; otherwise `false`.
 */
int Lua_Has_Turret(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_VehicleGameObj()) { return 0; }
	lua_pushboolean(L, obj->As_VehicleGameObj()->Has_Turret());
	return 1;
}

/*!\details Returns the armor type identifier for a given model name.
 *  \par Command Usage Example
 *          armor = Is_Valid_Skin(ModelName)
 *
 *  \param ModelName String name of the model to check.
 *  \return Number Integer ID of the corresponding armor type, or `0` if not found.
 */
int Lua_Is_Valid_Skin(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushnumber(L, ArmorWarheadManager::Get_Armor_Type(luaL_checkstring(L, 1)));
	return 1;
}

/*!\details Determines whether a soldier object is considered stealth.
 *  \par Command Usage Example
 *          stealth = IsStealthUnit(Soldier)
 *
 *  \param Soldier ID of the soldier to query.
 *  \return Number `true` if the unit is stealthed; otherwise `false`.
 */
int Lua_IsStealthUnit(lua_State * L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject * obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SmartGameObj()) { return 0; }
	lua_pushboolean(L, obj->As_SmartGameObj()->Get_Definition().Is_Stealthed());
	return 1;
}

/*!\details Queries whether a soldier object’s stealth mode is currently enabled.
 *  \par Command Usage Example
 *          enabled = Is_Stealth_Enabled(Soldier)
 *
 *  \param Soldier ID of the target unit.
 *  \return Number `true` if stealth is active; otherwise `false`.
 */
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

/*!\details Sets a vehicle’s visibility flag for scripted elements (e.g., whether its script HUD is shown).
 *  \par Command Usage Example
 *          Set_Is_Scripts_Visible(Vehicle, Visible)
 *
 *  \param Vehicle ID of the vehicle object.
 *  \param Visible Boolean (`true` to show scripts; `false` to hide them).
 */
int Lua_Set_Is_Scripts_Visible(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_VehicleGameObj()) { return 0; }
	obj->As_VehicleGameObj()->Set_Is_Scripts_Visible(lua_tobooleanCPP(L, 2));
	return 1;
}

/*!\details Returns a table containing the IDs of all soldier types available in the current purchase settings.
 *  \par Command Usage Example
 *          soldiers = Get_PT_Soldiers()
 *
 *  \param None No arguments required.
 *  \return Table List of soldier definition IDs that can be purchased by either team.
 */
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

/*!\details Returns a table containing the IDs of all vehicle types available in the current purchase settings.
 *  \par Command Usage Example
 *          vehicles = Get_PT_Vehicles()
 *
 *  \param None No arguments required.
 *  \return Table List of vehicle definition IDs that can be purchased by either team.
 */
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

/*!\details Returns the weapon definition ID corresponding to a given object definition ID.
 *  \par Command Usage Example
 *          wid = Get_Weapon_Definition_ID_By_Definition_ID(ObjDefID)
 *
 *  \param ObjDefID Integer identifier of the object’s definition.
 *  \return Number The integer ID of the associated weapon definition, or `0` if not found.
 */
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

/*!\details Retrieves the count of weapons stored in a soldier’s weapon bag.
 *  \par Command Usage Example
 *          bagCount = Get_Weapon_Bag_Count(Soldier)
 *
 *  \param Soldier ID of the soldier to inspect.
 *  \return Number Number of weapon items currently carried by the soldier, or `0` if none.
 */
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

/*!\details Retrieves the vehicle object currently assigned to a refinery building as its harvester.
 *  \par Command Usage Example
 *          hvid = Get_Harvester_Vehicle(Refinery)
 *
 *  \param Refinery ID of the refinery building.
 *  \return Number The ID of the harvester vehicle, or `0` if none is assigned.
 */
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

/*!\details Retrieves the definition ID of the harvester vehicle used by a refinery building.
 *  \par Command Usage Example
 *          defID = Get_Harvester_Def_ID(Refinery)
 *
 *  \param Refinery ID of the target refinery.
 *  \return Number The integer ID of the harvester definition, or `0` if not set.
 */
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

/*!\details Checks whether a model file exists and is available for loading.
 *  \par Command Usage Example
 *          valid = Is_Valid_Model(ModelName)
 *
 *  \param ModelName String name of the model to verify (without extension).
 *  \return Number `true` if the model file can be found; otherwise `false`.
 */
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

/*!\details Returns the current velocity vector of a specified object.
 *  \par Command Usage Example
 *          vel = Get_Velocity(Object)
 *
 *  \param Object ID of the target object.
 *  \return Table A `Vector3` table containing X, Y, and Z components of velocity.
 */
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

/*!\details Retrieves the human-readable name of a warhead type.
 *  \par Command Usage Example
 *          name = Get_Warhead_Name(WarheadID)
 *
 *  \param WarheadID Integer identifier of the warhead.
 *  \return String The display name of the specified warhead.
 */
int Lua_Get_Warhead_Name(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	lua_pushstring(L, ArmorWarheadManager::Get_Warhead_Name(luaL_checknumber(L, 1)));
	return 1;
}

/*!\details Sets the multi-color texture (MCT) skin for a building definition.
 *  \par Command Usage Example
 *          Set_MCTSkin(DefID, SkinName)
 *
 *  \param DefID Integer ID of the building definition to modify.
 *  \param SkinName String name of the new MCT skin.
 */
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

/*!\details Returns the current MCT skin identifier for a building definition.
 *  \par Command Usage Example
 *          skinID = Get_MCTSkin(DefID)
 *
 *  \param DefID Integer ID of the building definition to query.
 *  \return Number The integer identifier of the assigned MCT skin, or `0` if none.
 */
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

/*!\details Configures global lightning parameters for the current map.
 *  \par Command Usage Example
 *          Set_Lightning(Intensity, StartDist, EndDist, Heading, Distribution)
 *
 *  \param Intensity Float value controlling overall brightness of lightning.
 *  \param StartDist Float start distance of effect.
 *  \param EndDist Float end distance where intensity drops to zero.
 *  \param Heading Float direction of the lightning beam in degrees.
 *  \param Distribution Float spread amount of the effect.
 */
int Lua_Set_Lightning(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;
	Commands->Set_Lightning(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6));
	return 1;
}

/*!\details Determines if a soldier is currently standing on a ladder.
 *  \par Command Usage Example
 *          onLadder = Is_On_Ladder(Soldier)
 *
 *  \param Soldier ID of the target soldier.
 *  \return Number `true` if the soldier is on a ladder; otherwise `false`.
 */
int Lua_Is_On_Ladder(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (!obj || !obj->As_SoldierGameObj()) { return 0; }
	lua_pushboolean(L, obj->As_SoldierGameObj()->Is_On_Ladder());
	return 1;
}

/*!\details Adjusts the firing position of a weapon’s bullets relative to an object.
 *  \par Command Usage Example
 *          Set_Position_Bullets(Object, OffsetX, OffsetY)
 *
 *  \param Object ID of the weapon-holding object.
 *  \param OffsetX Float horizontal offset from the default muzzle point.
 *  \param OffsetY Float vertical offset from the default muzzle point.
 */
int Lua_Set_Position_Bullets(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	Set_Position_Bullets(Commands->Find_Object(lua_tonumber(L, 1)), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}

/*!\details Finds the nearest object that has a script matching a given name near a specified position.
 *  \par Command Usage Example
 *          objID = Find_Closest_Object_With_Script(ScriptName, Position)
 *
 *  \param ScriptName String identifier of the target script.
 *  \param Position LuaVector3 defining the search origin.
 *  \return Number The ID of the closest matching object, or `0` if none is found.
 */
int Lua_Find_Closest_Object_With_Script(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 location = Vector3(a->X(), a->Y(), a->Z());
	GameObject* obj = Find_Closest_Object_With_Script(luaL_checkstring(L, 1), location);
	lua_pushnumber(L, Get_ID(obj));
	return 1;
}

/*!\details Retrieves all spawner definition objects currently loaded in memory.
 *  \par Command Usage Example
 *          spawners = Get_All_Spawners()
 *
 *  \param None No arguments required.
 *  \return Table List of IDs for every spawner definition present.
 */
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

/*!\details Returns the list of spawn definition IDs associated with a particular spawner.
 *  \par Command Usage Example
 *          list = Get_Spawn_Definition_ID_List(SpawnerID)
 *
 *  \param SpawnerID Integer ID of the spawner to query.
 *  \return Table List of integer IDs for each spawn item configured on the spawner, or `nil` if none exist.
 */
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

/*!\details Returns the IDs of all objects currently spawned by each spawner.
 *  \par Command Usage Example
 *          current = Get_All_Spawners_currentObject()
 *
 *  \param None No arguments required.
 *  \return Table A list of object IDs that are presently owned by each spawner, or `0` for empty entries.
 */
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

/*!\details Returns detailed data for every spawner in the current map.
 *  \par Command Usage Example
 *          data = Get_All_Spawners_Data()
 *
 *  \param None No arguments required.
 *  \return Table List of tables containing definition ID, position, name, and various flags for each spawner.
 */
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

/*!\details Retrieves the world position of a building’s MCT (multi-component tracker) aggregate.
 *  \par Command Usage Example
 *          pos = Get_MCT_Position(Building)
 *
 *  \param Building ID of the target building object.
 *  \return Table A `Vector3` table containing X, Y, and Z coordinates of the MCT.
 */
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

/*!\details Retrieves the current facing angle of a building’s MCT aggregate.
 *  \par Command Usage Example
 *          rot = Get_MCT_Facing(Building)
 *
 *  \param Building ID of the target building object.
 *  \return Number The rotation in degrees (0-360) representing the current facing direction.
 */
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

/*!\details Adds points to a team’s cumulative score.
 *  \par Command Usage Example
 *          Set_Team_Score(TeamID, Score)
 *
 *  \param TeamID Integer representing the team (e.g., 1 for GDI, 2 for NOD).
 *  \param Score Integer number of points to add to that team’s total.
 */
int Lua_Set_Team_Score(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Set_Team_Score(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
	return 1;
}

/*!\details Retrieves a table containing the IDs of all static objects in the current physics scene.
 *  \par Command Usage Example
 *          statics = Get_All_Static_Objects()
 *
 *  \param None No arguments required.
 *  \return Table List of integer IDs for every static physics object.
 */
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

/*!\details Returns the radius of an object’s bounding sphere.
 *  \par Command Usage Example
 *          radius = Get_Bounding_Sphere_Radius(Object)
 *
 *  \param Object ID of the target object.
 *  \return Number The radius value used for collision and visibility checks.
 */
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

/*!\details Enables or disables friendly fire for all combat units in the current session.
 *  \par Command Usage Example
 *          Set_Friendly_Fire_Permitted(Enable)
 *
 *  \param Enable Boolean (`true` to allow friendly fire; `false` to disallow).
 */
int Lua_Set_Friendly_Fire_Permitted(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
#ifdef DAPLUGIN
	CombatManager::Set_Friendly_Fire_Permitted(lua_tobooleanCPP(L, 1));
#endif
	return 1;
}

/*!\details Queries whether friendly fire is currently permitted.
 *  \par Command Usage Example
 *          allowed = Is_Friendly_Fire_Permitted()
 *
 *  \return Number `true` if friendly fire is enabled; otherwise `false`.
 */
int Lua_Is_Friendly_Fire_Permitted(lua_State* L)
{
#ifdef DAPLUGIN
	lua_pushboolean(L, CombatManager::Is_Friendly_Fire_Permitted());
#endif
	return 1;
}

/*!\details Activates or deactivates the letterbox visual effect for a soldier’s HUD.
 *  \par Command Usage Example
 *          Enable_Letterbox_Player(Soldier, Show, Duration)
 *
 *  \param Soldier ID of the target soldier.
 *  \param Show Boolean (`true` to show; `false` to hide).
 *  \param Duration Float duration in seconds that the letterbox will remain visible.
 */
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

/*!\details Sets the global simulation time-scale factor.
 *  \par Command Usage Example
 *          Set_Time_Scale(scale)
 *
 *  \param scale Float value that multiplies all in-game timers (e.g., 2.0 doubles speed).
 */
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

/*! \details Sets the emoticon displayed over a soldier's head.
 *  \par Lua Command
 *          result = Set_Emot_Icon(ObjID, EmotIconID)
 *  \param ObjID Number - ID of the soldier.
 *  \param EmotIconID Number - ID of the emoticon to display.
 *  \return Number - Result of the call; no return on an invalid object.
 */
int Lua_Set_Emot_Icon(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr || obj->As_SoldierGameObj() == nullptr) return 0;
	int emoticonId = luaL_checknumber(L, 2);
	int result = Set_Emot_Icon(obj->As_SoldierGameObj(), emoticonId);
	lua_pushnumber(L, result);
	return 1;
}

/*! \details Asynchronously writes text to a file.
 *  \par Lua Command
 *          result = Write_File_Async(FileName, Text)
 *  \param FileName String - Path to the file to write.
 *  \param Text String - Contents to write.
 *  \return Number - Result of the call.
 */
int Lua_Write_File_Async(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const char* fileName = luaL_checkstring(L, 1);
	const char* text = luaL_checkstring(L, 2);
	int result = Write_File_Async(fileName, text);
	lua_pushnumber(L, result);
	return 1;
}

/*!\details Determines if a vehicle is currently positioned on an enemy pedestal.
 *  \par Command Usage Example
 *          onPedestal = Is_On_Enemy_Pedestal(Vehicle)
 *
 *  \param Vehicle ID of the object to test.
 *  \return Number `true` if the vehicle sits on an enemy pedestal; otherwise `false`.
 */
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

/*!\details Assigns a custom tag string to a soldier unit.
 *  \par Command Usage Example
 *          Set_Bot_Tag(Soldier, TagString)
 *
 *  \param Soldier ID of the target soldier.
 *  \param TagString String label used for identification or AI targeting.
 */
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

/*!\details Retrieves the tag string previously assigned to a soldier.
 *  \par Command Usage Example
 *          tag = Get_Bot_Tag(Soldier)
 *
 *  \param Soldier ID of the target soldier.
 *  \return String The current bot tag, or an empty string if none has been set.
 */
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

/*!\details Enables or disables a soldier’s ability to block action input.
 *  \par Command Usage Example
 *          Set_Block_Action_Key(Soldier, Block)
 *
 *  \param Soldier ID of the target soldier.
 *  \param Block Boolean (`true` to ignore input; `false` to allow actions).
 */
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

/*!\details Queries whether a soldier is currently blocking action input.
 *  \par Command Usage Example
 *          blocked = Block_Action_Key(Soldier)
 *
 *  \param Soldier ID of the target soldier.
 *  \return Number `true` if action key input is blocked; otherwise `false`.
 */
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

/*!\details Sets a scaling factor that will be applied to the soldier across networked clients.
 *  \par Command Usage Example
 *          Set_Scale_Across_Network(Soldier, Scale)
 *
 *  \param Soldier ID of the target soldier.
 *  \param Scale Float value indicating how much to scale (e.g., 1.5 for 150 % size).
 */
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

/*!\details Retrieves the current scaling factor that is being applied to a soldier across networked clients.
 *  \par Command Usage Example
 *          scale = Get_Scale_Across_Network(Soldier)
 *
 *  \param Soldier ID of the target soldier.
 *  \return Number The active network scaling value for this unit.
 */
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

/*!\details Checks if a soldier can play damage animations for an incoming hit.
 *  \par Command Usage Example
 *          canPlay = Can_Play_Damage_Animations(Soldier)
 *
 *  \param Soldier ID of the target soldier.
 *  \return Number `true` if damage animations are enabled; otherwise `false`.
 */
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

/*!\details Enables or disables damage animations for a soldier.
 *  \par Command Usage Example
 *          Set_Can_Play_Damage_Animations(Soldier, Enable)
 *
 *  \param Soldier ID of the target soldier.
 *  \param Enable Boolean (`true` to allow damage visuals; `false` to suppress).
 */
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

/*!\details Queries whether a soldier currently has permission to drive vehicles.
 *  \par Command Usage Example
 *          canDrive = Can_Drive_Vehicles(Soldier)
 *
 *  \param Soldier ID of the target soldier.
 *  \return Number `true` if the unit may control a vehicle; otherwise `false`.
 */
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

/*!\details Queries whether a soldier can steal vehicles from enemies.
 *  \par Command Usage Example
 *          canSteal = Can_Steal_Vehicles(Soldier)
 *
 *  \param Soldier ID of the target soldier.
 *  \return Number `true` if vehicle stealing is allowed; otherwise `false`.
 */
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

/*!\details Sets whether a soldier can steal vehicles from enemies.
 *  \par Command Usage Example
 *          Set_Can_Steal_Vehicles(Soldier, Enable)
 *
 *  \param Soldier ID of the target soldier.
 *  \param Enable Boolean (`true` to allow stealing; `false` to block).
 */
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

/*!\details Creates a Yes/No dialog box attached to a soldier.
 *  \par Command Usage Example
 *          dlgID, yesID, noID = Create_Yes_No_Dialog(Soldier, Title, Label, YesColorR, YesColorG, YesColorB)
 *
 *  \param Soldier ID of the unit that will own the dialog.
 *  \param Title Wide string title displayed at top of the dialog.
 *  \param Label Wide string prompt shown to the player.
 *  \param YesColorR Float red component of the “Yes” button color (0-1).
 *  \param YesColorG Float green component of the “Yes” button color (0-1).
 *  \param YesColorB Float blue component of the “Yes” button color (0-1).
 *  \return Table containing dialog reference, IDs for “yes” and “no” buttons.
 */
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

/*!\details Displays a previously created dialog to the player.
 *  \par Command Usage Example
 *          Show_Dialog(DialogRef)
 *
 *  \param DialogRef Reference (table) to an existing scripted dialog.
 */
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

/*!\details Instantiates a menu dialog by its predefined ID.
 *  \par Command Usage Example
 *          dlgRef = Create_Menu_Dialog(DialogID)
 *
 *  \param DialogID Integer identifier of the pre-registered menu dialog.
 *  \return Table Reference to the created scripted dialog, or `nil` on failure.
 */
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

/*!\details Instantiates a popup dialog by its predefined ID.
 *  \par Command Usage Example
 *          dlgRef = Create_Popup_Dialog(DialogID)
 *
 *  \param DialogID Integer identifier of the pre-registered popup dialog.
 *  \return Table Reference to the created scripted dialog, or `nil` on failure.
 */
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

/*!\details Deletes an existing dialog and frees its resources.
 *  \par Command Usage Example
 *          Delete_Dialog(DialogRef)
 *
 *  \param DialogRef Reference (table) to the scripted dialog to remove.
 */
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

/*!\details Hides a displayed dialog without destroying it.
 *  \par Command Usage Example
 *          Hide_Dialog(DialogRef)
 *
 *  \param DialogRef Reference (table) to the scripted dialog to hide.
 */
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

/*!\details Looks up an existing dialog by its numeric ID.
 *  \par Command Usage Example
 *          dlgRef = Find_Dialog(DialogID)
 *
 *  \param DialogID Integer identifier of the desired scripted dialog.
 *  \return Table Reference to the found dialog, or `nil` if it does not exist.
 */
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

/*!\details Creates a Yes/No/Cancel dialog box attached to a soldier.
 *  \par Command Usage Example
 *          dlgID, yesID, noID, cancelID = Create_Yes_No_Cancel_Dialog(Soldier, Title, Label, YesColorR, YesColorG, YesColorB)
 *
 *  \param Soldier ID of the unit that will own the dialog.
 *  \param Title Wide string title displayed at top of the dialog.
 *  \param Label Wide string prompt shown to the player.
 *  \param YesColorR Float red component of the “Yes” button color (0-1).
 *  \param YesColorG Float green component of the “Yes” button color (0-1).
 *  \param YesColorB Float blue component of the “Yes” button color (0-1).
 *  \return Table containing dialog reference, IDs for “yes”, “no” and “cancel” buttons.
 */
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

/*!\details Creates a user prompt dialog box with an input field.
 *  \par Command Usage Example
 *          dlgID, textboxID, yesID, cancelID = Create_User_Prompt(Soldier, Title, Label, Limit, InitialText, YesColorR, YesColorG, YesColorB)
 *
 *  \param Soldier ID of the unit that will own the dialog.
 *  \param Title Wide string title displayed at top of the dialog.
 *  \param Label Wide string prompt shown to the player.
 *  \param Limit Integer maximum length of user input.
 *  \param InitialText Wide string initial value displayed in the text box.
 *  \param YesColorR Float red component of the “Yes” button color (0-1).
 *  \param YesColorG Float green component of the “Yes” button color (0-1).
 *  \param YesColorB Float blue component of the “Yes” button color (0-1).
 *  \return Table containing dialog reference and IDs for textbox, yes, and cancel buttons.
 */
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

/*!\details Creates a centered popup dialog for a specific client.
 *  \par Command Usage Example
 *          dlgRef = Create_Centered_Popup(ClientID, Width, Height, Title)
 *
 *  \param ClientID Integer identifier of the player/client who will see the popup.
 *  \param Width Integer pixel width of the popup window.
 *  \param Height Integer pixel height of the popup window.
 *  \param Title Wide string title displayed in the popup header.
 *  \return Table Reference to the created scripted dialog, or `nil` on failure.
 */
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

/*!\details Sets the screen coordinates and size of a scripted control element.
 *  \par Command Usage Example
 *          Set_Control_Bounds(ControlRef, X, Y, Width, Height)
 *
 *  \param ControlRef Reference (table) to the scripted control object.
 *  \param X Integer horizontal position in pixels.
 *  \param Y Integer vertical position in pixels.
 *  \param Width Integer width of the control in pixels.
 *  \param Height Integer height of the control in pixels.
 */
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

/*!\details Creates a progress bar control inside a dialog.
 *  \par Command Usage Example
 *          barRef = Create_ProgressBar_Control(DialogRef, X, Y, Width, Height, Value, MinValue, MaxValue)
 *
 *  \param DialogRef Reference (table) to the parent dialog.
 *  \param X Integer horizontal position of the bar.
 *  \param Y Integer vertical position of the bar.
 *  \param Width Integer width of the bar in pixels.
 *  \param Height Integer height of the bar in pixels.
 *  \param Value Integer current value displayed on the progress bar.
 *  \param MinValue Integer minimum possible value for the bar.
 *  \param MaxValue Integer maximum possible value for the bar.
 *  \return Table Reference to the created progress bar control, or `nil` if creation failed.
 */
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

/*!\details Creates a slider control inside a dialog.
 *  \par Command Usage Example
 *          sliderRef = Create_Slider_Control(DialogRef, X, Y, Width, Height, Value, MinValue, MaxValue)
 *
 *  \param DialogRef Reference (table) to the parent dialog.
 *  \param X Integer horizontal position of the slider.
 *  \param Y Integer vertical position of the slider.
 *  \param Width Integer width of the slider in pixels.
 *  \param Height Integer height of the slider in pixels.
 *  \param Value Integer current value displayed on the slider.
 *  \param MinValue Integer minimum possible value for the slider.
 *  \param MaxValue Integer maximum possible value for the slider.
 *  \return Table Reference to the created slider control, or `nil` if creation failed.
 */
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

/*!\details Creates a combo box control inside a dialog.
 *  \par Command Usage Example
 *          comboRef = Create_ComboBox_Control(DialogRef, X, Y, Width, Height, OptionsTable, SelectedIndex)
 *
 *  \param DialogRef Reference (table) to the parent dialog.
 *  \param X Integer horizontal position of the combo box.
 *  \param Y Integer vertical position of the combo box.
 *  \param Width Integer width of the combo box in pixels.
 *  \param Height Integer height of the combo box in pixels.
 *  \param OptionsTable Table containing string options to display.
 *  \param SelectedIndex Integer index (1-based) of the default selected item.
 *  \return Table Reference to the created combo box control, or `nil` if creation failed.
 */
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

/*!\details Creates a checkbox control inside a dialog.
 *  \par Command Usage Example
 *          checkRef = Create_CheckBox_Control(DialogRef, X, Y, Width, Height, Text, InitialState)
 *
 *  \param DialogRef Reference (table) to the parent dialog.
 *  \param X Integer horizontal position of the checkbox.
 *  \param Y Integer vertical position of the checkbox.
 *  \param Width Integer width of the checkbox in pixels.
 *  \param Height Integer height of the checkbox in pixels.
 *  \param Text Wide string label displayed next to the checkbox.
 *  \param InitialState Boolean initial checked state (`true` for checked).
 *  \return Table Reference to the created checkbox control, or `nil` if creation failed.
 */
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

/*!\details Creates a multi-line text area control inside a dialog.
 *  \par Command Usage Example
 *          textAreaRef = Create_TextArea_Control(DialogRef, X, Y, Width, Height, InitialText, MaxLines)
 *
 *  \param DialogRef Reference (table) to the parent dialog.
 *  \param X Integer horizontal position of the text area.
 *  \param Y Integer vertical position of the text area.
 *  \param Width Integer width of the text area in pixels.
 *  \param Height Integer height of the text area in pixels.
 *  \param Text Wide string initial content to display.
 *  \param Limit Integer maximum number of lines allowed in the area.
 *  \return Table Reference to the created text area control, or `nil` if creation failed.
 */
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

/*!\details Creates a title button control inside a dialog.
 *  \par Command Usage Example
 *          titleBtnRef = Create_Title_Button_Control(DialogRef, X, Y, Width, Height, Text)
 *
 *  \param DialogRef Reference (table) to the parent dialog.
 *  \param X Integer horizontal position of the button.
 *  \param Y Integer vertical position of the button.
 *  \param Width Integer width of the button in pixels.
 *  \param Height Integer height of the button in pixels.
 *  \param Text Wide string label displayed on the button.
 *  \return Table Reference to the created title button control, or `nil` if creation failed.
 */
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

/*!\details Creates an image button control inside a dialog.
 *  \par Command Usage Example
 *          imgBtnRef = Create_Image_Button_Control(DialogRef, X, Y, Width, Height, ImagePath, Text)
 *
 *  \param DialogRef Reference (table) to the parent dialog.
 *  \param X Integer horizontal position of the button.
 *  \param Y Integer vertical position of the button.
 *  \param Width Integer width of the button in pixels.
 *  \param Height Integer height of the button in pixels.
 *  \param ImagePath String path to the image resource used for the button’s icon.
 *  \param Text String label displayed next to or on top of the image.
 *  \return Table Reference to the created image button control, or `nil` if creation failed.
 */
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

/*!\details Creates a bordered button control inside a scripted dialog.
 *  \par Command Usage Example
 *          btn = Create_Bordered_Button_Control(Dialog, X, Y, Width, Height, Text)
 *
 *  \param Dialog LuaScriptedDialogClass instance to host the control.
 *  \param X Integer horizontal position of the button (pixels).
 *  \param Y Integer vertical position of the button (pixels).
 *  \param Width Integer width of the button (pixels).
 *  \param Height Integer height of the button (pixels).
 *  \param Text Wide string displayed on the button.
 *  \return Table Reference to the created LuaScriptedControl object, or nil if creation failed.
 */
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

/*!\details Creates an image control inside a scripted dialog.
 *  \par Command Usage Example
 *          img = Create_Image_Control(Dialog, X, Y, Width, Height, ImagePath)
 *
 *  \param Dialog LuaScriptedDialogClass instance to host the control.
 *  \param X Integer horizontal position of the image (pixels).
 *  \param Y Integer vertical position of the image (pixels).
 *  \param Width Integer width of the image (pixels).
 *  \param Height Integer height of the image (pixels).
 *  \param ImagePath String path to the texture or sprite file.
 *  \return Table Reference to the created LuaScriptedControl object, or nil if creation failed.
 */
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

/*!\details Creates a text label control inside a scripted dialog.
 *  \par Command Usage Example
 *          lbl = Create_Label_Control(Dialog, X, Y, Width, Height, Text, Style, R, G, B)
 *
 *  \param Dialog LuaScriptedDialogClass instance to host the control.
 *  \param X Integer horizontal position of the label (pixels).
 *  \param Y Integer vertical position of the label (pixels).
 *  \param Width Integer width of the label (pixels).
 *  \param Height Integer height of the label (pixels).
 *  \param Text Wide string displayed in the label.
 *  \param Style Integer representing the text style enumeration (e.g., Bold, Italic).
 *  \param R Float red component of the label color (0-1).
 *  \param G Float green component of the label color (0-1).
 *  \param B Float blue component of the label color (0-1).
 *  \return Table Reference to the created LuaScriptedControl object, or nil if creation failed.
 */
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

/*!\details Retrieves the control owner ID of a soldier object.
 *  \par Command Usage Example
 *          ownerID = Get_Control_Owner(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Number The current owner ID, or nil if not available.
 */
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

/*!\details Assigns a new control owner ID to a soldier object.
 *  \par Command Usage Example
 *          Set_Control_Owner(Soldier, NewOwnerID)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param NewOwnerID Integer identifier for the new controller.
 */
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

/*!\details Links a player to a specific soldier object.
 *  \par Command Usage Example
 *          Set_GameObj(PlayerID, SoldierObject)
 *
 *  \param PlayerID Integer ID of the client/player.
 *  \param SoldierObject Integer ID of the soldier to assign.
 */
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

/*!\details Retrieves the center of an object’s bounding sphere.
 *  \par Command Usage Example
 *          center = Get_Bounding_Sphere_Center(Object)
 *
 *  \param Object Integer ID of the target object.
 *  \return Table A `Vector3` table containing X, Y, Z coordinates of the sphere center.
 */
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

/*!\details Calculates the volume of an object’s bounding sphere.
 *  \par Command Usage Example
 *          vol = Get_Bounding_Sphere_Volume(Object)
 *
 *  \param Object Integer ID of the target object.
 *  \return Number The computed volume (cubic units).
 */
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

/*!\details Retrieves the extent vector of an object’s bounding box.
 *  \par Command Usage Example
 *          extent = Get_Bounding_Box_Extent(Object)
 *
 *  \param Object Integer ID of the target object.
 *  \return Table A `Vector3` table containing X, Y, Z extent values.
 */
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

/*!\details Retrieves the center of an object’s bounding box.
 *  \par Command Usage Example
 *          center = Get_Bounding_Box_Center(Object)
 *
 *  \param Object Integer ID of the target object.
 *  \return Table A `Vector3` table containing X, Y, Z coordinates of the box center.
 */
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

/*!\details Calculates the volume of an object’s bounding box.
 *  \par Command Usage Example
 *          vol = Get_Bounding_Box_Volume(Object)
 *
 *  \param Object Integer ID of the target object.
 *  \return Number The computed volume (cubic units).
 */
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

/*!\details Sets a player’s display name.
 *  \par Command Usage Example
 *          Set_Player_Name(PlayerID, NewName)
 *
 *  \param PlayerID Integer ID of the target player.
 *  \param NewName String new name to assign (must be non-empty).
 */
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

/*!\details Overrides the weapon hold style for a soldier.
 *  \par Command Usage Example
 *          Set_Override_Weapon_Hold_Style(Soldier, StyleID)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param StyleID Integer representing the new hold style index.
 */
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

/*!\details Retrieves the currently overridden weapon hold style for a soldier.
 *  \par Command Usage Example
 *          styleID = Get_Override_Weapon_Hold_Style(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Number The active hold style index, or nil if none is set.
 */
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

/*!\details Registers the default refill hook in Lua.
 *  \par Command Usage Example
 *          AddRefillHook()
 *
 *  \param None No arguments required.
 */
int Lua_AddRefillHook(lua_State* L)
{
	AddRefillHook(LuaManager::Refill_Hook);
	return 1;
}

/*!\details Prints a message to all clients’ consoles with a specified color.
 *  \par Command Usage Example
 *          Print_Client_Console(Message, R, G, B, A)
 *
 *  \param Message String text to display.
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 *  \param A Float alpha component (0-1).
 */
int Lua_Print_Client_Console(lua_State* L)
{
	if (lua_gettop(L) == 5)
	{
		Print_Client_Console(luaL_checkstring(L, 1), Vector4(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5)));
		return 1;
	}
	return 0;
}

/*!\details Prints a message to the console of a specific player with color.
 *  \par Command Usage Example
 *          Print_Client_Console_Player(Soldier, Message, R, G, B, A)
 *
 *  \param Soldier Integer ID of the target soldier (must be a star).
 *  \param Message String text to display.
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 *  \param A Float alpha component (0-1).
 */
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

/*!\details Retrieves the current repository URL used for plugin updates.
 *  \par Command Usage Example
 *          url = Get_Repository_URL()
 *
 *  \return String The current repository URL.
 */
int Lua_Get_Repository_URL(lua_State* L)
{
	lua_pushstring(L, Get_Repository_URL());
	return 1;
}

/*!\details Sets the repository URL used for plugin updates.
 *  \par Command Usage Example
 *          Set_Repository_URL(URL)
 *
 *  \param URL String new repository URL to set.
 */
int Lua_Set_Repository_URL(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Set_Repository_URL(luaL_checkstring(L, 1));
		return 1;
	}
	return 0;
}

/*!\details Retrieves the current screenshot URL used for online sharing.
 *  \par Command Usage Example
 *          url = Get_Screenshot_URL()
 *
 *  \return String The current screenshot URL.
 */
int Lua_Get_Screenshot_URL(lua_State* L)
{
	lua_pushstring(L, Get_Screenshot_URL());
	return 1;
}

/*!\details Sets the screenshot URL used for online sharing.
 *  \par Command Usage Example
 *          Set_Screenshot_URL(URL)
 *
 *  \param URL String new screenshot URL to set.
 */
int Lua_Set_Screenshot_URL(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Set_Screenshot_URL(luaL_checkstring(L, 1));
		return 1;
	}
	return 0;
}

/*!\details Forces a player’s client to capture a screenshot.
 *  \par Command Usage Example
 *          Take_Screenshot(PlayerID)
 *
 *  \param PlayerID Integer ID of the player whose screen should be captured.
 */
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

/*!\details Determines whether a world position lies within the active pathfind sector for a given sector ID.
 *  \par Command Usage Example
 *          inSector = Is_In_Pathfind_Sector(Position, SectorID)
 *
 *  \param Position LuaVector3 coordinate to test.
 *  \param SectorID Integer ID of the pathfind sector to query.
 *  \return Number `true` if inside; otherwise `false`.
 */
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

/*!\details Indicates whether gameplay is currently allowed (e.g., not during intermission).
 *  \par Command Usage Example
 *          allowed = Is_Gameplay_Allowed()
 *
 *  \return Number `true` if gameplay can proceed; otherwise `false`.
 */
int Lua_Is_Gameplay_Allowed(lua_State* L)
{
	lua_pushboolean(L, Is_Gameplay_Allowed());
	return 1;
}

/*!\details Enables or disables gameplay for all players.
 *  \par Command Usage Example
 *          Set_Gameplay_Allowed(Enable)
 *
 *  \param Enable Boolean (`true` to allow; `false` to pause).
 */
int Lua_Set_Gameplay_Allowed(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Set_Gameplay_Allowed(lua_tobooleanCPP(L, 1));
		return 1;
	}
	return 0;
}

/*!\details Retrieves the RGB color value associated with a specific team.
 *  \par Command Usage Example
 *          r, g, b = Get_Team_Color(TeamID)
 *
 *  \param TeamID Integer ID of the team (typically 1 or 2).
 *  \return Three numbers representing the red, green and blue components (0-255) of the team color.
 */
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

/*!\details Creates a logical sound attached to an object at a specified world location.
 *  \par Command Usage Example
 *          soundID = Create_Logical_Sound(Object, SoundIndex, Position, Volume)
 *
 *  \param Object Integer ID of the target object.
 *  \param SoundIndex Integer index representing the desired sound preset.
 *  \param Position LuaVector3 world coordinates where the sound should originate.
 *  \param Volume Float intensity value (0-1).
 *  \return Number The created logical sound instance ID, or `0` if creation failed.
 */
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

/*!\details Enables or disables the bullet-heard detection flag for a soldier.
 *  \par Command Usage Example
 *          Innate_Soldier_Enable_Bullet_Heard(Soldier, Enable)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
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

/*!\details Enables or disables the gunshot-heard detection flag for a soldier.
 *  \par Command Usage Example
 *          Innate_Soldier_Enable_Gunshot_Heard(Soldier, Enable)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
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

/*!\details Enables or disables the footsteps-heard detection flag for a soldier.
 *  \par Command Usage Example
 *          Innate_Soldier_Enable_Footsteps_Heard(Soldier, Enable)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
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

/*!\details Returns a table of all soldier objects present in the game world.
 *  \par Command Usage Example
 *          soldiers = Get_All_Soldiers()
 *
 *  \return Table List of integer IDs for every soldier object found.
 */
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

/*!\details Enables or disables a soldier’s freeze state.
 *  \par Command Usage Example
 *          Set_Freeze(Soldier, Freeze)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param Freeze Boolean (`true` to freeze; `false` to unfreeze).
 */
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

/*!\details Checks whether a soldier is currently frozen.
 *  \par Command Usage Example
 *          frozen = Is_Frozen(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Number `true` if the soldier is frozen; otherwise `false`.
 */
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

/*!\details Retrieves a table of all active power-up objects in the current map.
 *  \par Command Usage Example
 *          powers = Get_All_PowerUps()
 *
 *  \return Table List of integer IDs for each active power-up object.
 */
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

/*!\details Indicates whether the pathfinding graph for the current map has been generated.
 *  \par Command Usage Example
 *          generated = Is_Pathfind_Generated()
 *
 *  \return Number `true` if the pathfinder is ready; otherwise `false`.
 */
int Lua_Is_Pathfind_Generated(lua_State* L)
{
	lua_pushboolean(L, Is_Pathfind_Generated());
	return 1;
}

/*!\details Stops a specific sound playing for all players in a team.
 *  \par Command Usage Example
 *          Stop_Sound_Team(SoundID, Interrupt, TeamID)
 *
 *  \param SoundID Integer ID of the sound to stop.
 *  \param Interrupt Boolean (`true` to immediately interrupt playback).
 *  \param TeamID Integer ID of the team whose members should stop hearing the sound.
 */
int Lua_Stop_Sound_Team(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	Stop_Sound_Team(luaL_checknumber(L, 1), lua_tobooleanCPP(L, 2), luaL_checknumber(L, 3));
	return 1;
}

/*!\details Stops a specific sound playing for a particular player.
 *  \par Command Usage Example
 *          Stop_Sound_Player(Soldier, SoundID, Interrupt)
 *
 *  \param Soldier Integer ID of the target soldier who owns the sound.
 *  \param SoundID Integer ID of the sound to stop.
 *  \param Interrupt Boolean (`true` to immediately interrupt playback).
 */
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

/*!\details Finds the player ID associated with a given display name.
 *  \par Command Usage Example
 *          id = Get_Player_ID_By_Name(Name)
 *
 *  \return Number The numeric player ID, or nil if no match is found.
 */
int Lua_Get_Player_ID_By_Name(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		lua_pushnumber(L, Get_Player_ID_By_Name(luaL_checkstring(L, 1)));
		return 1;
	}
	return 0;
}

/*!\details Forces the current map to be reloaded from disk.
 *  \par Command Usage Example
 *          Reload_Level()
 */
int Lua_Reload_Level(lua_State* L)
{
	Reload_Level();
	return 1;
}

/*!\details Causes a specified player’s client to reload its state from the server.
 *  \par Command Usage Example
 *          Force_Client_Reload(PlayerID)
 *
 *  \param PlayerID Integer ID of the target player.
 */
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

/*!\details Retrieves the world coordinates of a soldier’s targeting position.
 *  \par Command Usage Example
 *          pos = Get_Targeting_Pos(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Table A `Vector3` table containing X, Y, Z components of the targeting point.
 */
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

/*!\details Sets whether the destruction of any base ends the game.
 *  \par Command Usage Example
 *          Set_Base_Destruction_Ends_Game(Enable)
 *
 *  \param Enable Boolean (`true` to end the game on base destruction; `false` otherwise).
 */
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

/*!\details Sets whether placing a beacon ends the game.
 *  \par Command Usage Example
 *          Set_Beacon_Placement_Ends_Game(Enable)
 *
 *  \param Enable Boolean (`true` to end the game on beacon placement; `false` otherwise).
 */
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

/*!\details Sets the starting credits for the game.
 *  \par Command Usage Example
 *          Set_Starting_Credits(Credits)
 *
 *  \param Credits Integer number of initial credits to grant each team.
 */
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

/*!\details Resets the remaining game time to its initial value.
 *  \par Command Usage Example
 *          Reset_Time_Remaining_Seconds()
 */
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

/*!\details Sets the remaining game time in seconds.
 *  \par Command Usage Example
 *          Set_Time_Remaining_Seconds(Seconds)
 *
 *  \param Seconds Integer number of seconds left on the timer.
 */
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

/*!\details Sets the message-of-the-day text for the game lobby.
 *  \par Command Usage Example
 *          Set_Motd(Message)
 *
 *  \param Message Wide string containing the MOTD to display.
 */
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

/*!\details Sets the name of the current map.
 *  \par Command Usage Example
 *          Set_Map_Name(Name)
 *
 *  \param Name Wide string containing the new map name.
 */
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

/*!\details Sets the radar rendering mode for all players.
 *  \par Command Usage Example
 *          Set_Radar_Mode(Mode)
 *
 *  \param Mode Integer value representing the desired RadarModeEnum.
 */
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

/*!\details Enables or disables team-changing during the game.
 *  \par Command Usage Example
 *          Set_Is_Team_Changing_Allowed(Enable)
 *
 *  \param Enable Boolean (`true` to allow; `false` to disallow).
 */
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

/*!\details Enables or disables remix teams (dynamic team balancing).
 *  \par Command Usage Example
 *          Set_Remix_Teams(Enable)
 *
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
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

/*!\details Sets the password required for a game session.
 *  \par Command Usage Example
 *          Set_Password(Password)
 *
 *  \param Password Wide string containing the new session password.
 */
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

/*!\details Sets the displayed title of the game lobby.
 *  \par Command Usage Example
 *          Set_Game_Title(Title)
 *
 *  \param Title Wide string containing the new game title.
 */
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

/*!\details Sets the duration of the intermission period in seconds.
 *  \par Command Usage Example
 *          Set_Intermission_Time_Seconds(Seconds)
 *
 *  \param Seconds Integer number of seconds for the intermission timer.
 */
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

/*!\details Sets the visibility state of a specific object.
 *  \par Command Usage Example
 *          Set_Object_Visibility(ObjectID, Visible)
 *
 *  \param ObjectID Integer ID of the target object.
 *  \param Visible Boolean (`true` to show; `false` to hide).
 */
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

/*!\details Sets the visibility of a specific object for an individual player.
 *  \par Command Usage Example
 *          Set_Object_Visibility_For_Player(ObjID, PlayerObjID, Visible)
 *
 *  \param ObjID Integer ID of the target object.
 *  \param PlayerObjID Integer ID of the player’s owning object (must be a star).
 *  \param Visible Boolean (`true` to show; `false` to hide) for that player only.
 */
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

/*!\details Sets the visibility setting for an object relative to team members.
 *  \par Command Usage Example
 *          Set_Team_Visibility(ObjectID, VisibilitySetting)
 *
 *  \param ObjectID Integer ID of the target object.
 *  \param VisibilitySetting Integer value from TeamVisibilitySetting enumeration.
 */
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

/*!\details Retrieves the hardware identifier string for a specific client.
 *  \par Command Usage Example
 *          id = Get_Client_Hardware_Identifier(PlayerID)
 *
 *  \param PlayerID Integer ID of the target player/client.
 *  \return String Hardware identifier associated with that client.
 */
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
/*!\details Determines whether the connection to a client is considered bad.
 *  \par Command Usage Example
 *          bad = Is_Connection_Bad(PlayerID)
 *
 *  \param PlayerID Integer ID of the target player.
 *  \return Number `true` if the connection is bad; otherwise `false`.
 */
int Lua_Is_Connection_Bad(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		lua_pushboolean(L, Is_Connection_Bad(lua_tonumber(L, 1)));
		return 1;
	}
	return 0;
}

/*!\details Retrieves the current in-game time of day.
 *  \par Command Usage Example
 *          hours, mins = Get_Time_Of_Day()
 *
 *  \return Two numbers: hours (0-23) and minutes (0-59).
 */
int Lua_Get_Time_Of_Day(lua_State* L)
{
	unsigned int hours, mins = 0;
	Get_Time_Of_Day(&hours, &mins);
	lua_pushnumber(L, hours);
	lua_pushnumber(L, mins);
	return 2;
}

/*!\details Sets the current in-game time of day.
 *  \par Command Usage Example
 *          Set_Time_Of_Day(Hours, Minutes)
 *
 *  \param Hours Integer hour component (0-23).
 *  \param Minutes Integer minute component (0-59).
 */
int Lua_Set_Time_Of_Day(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		Set_Time_Of_Day(lua_tonumber(L, 1), lua_tonumber(L,2));
	}
	return 0;
}

/*!\details Retrieves the screen resolution in pixels for a specific monitor index.
 *  \par Command Usage Example
 *          w, h = Get_Screen_Resolution(MonitorIndex)
 *
 *  \param MonitorIndex Integer index of the monitor to query (typically 1 or 2).
 *  \return Two numbers: width and height in pixels.
 */
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

/*!\details Enables or disables global targeting for all players.
 *  \par Command Usage Example
 *          Enable_Global_Targeting(Enable)
 *
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
int Lua_Enable_Global_Targeting(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Enable_Global_Targeting(lua_tobooleanCPP(L, 1));
	}
	return 0;
}

/*!\details Enables or disables global targeting for a specific player.
 *  \par Command Usage Example
 *          Enable_Global_Targeting_Player(Soldier, Enable)
 *
 *  \param Soldier Integer ID of the target soldier (must be a star).
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
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

/*!\details Returns the current hue of the moon halo effect.
 *  \par Command Usage Example
 *          color = Get_Moon_Halo_Color()
 *
 *  \return Number The integer value representing the moon halo color (RGB packed).
 */
int Lua_Get_Moon_Halo_Color(lua_State* L)
{
	auto color = Get_Moon_Halo_Color();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Returns the current hue of the warm sky effect.
 *  \par Command Usage Example
 *          color = Get_Warm_Sky_Color()
 *
 *  \return Number The integer value representing the warm sky color (RGB packed).
 */
int Lua_Get_Warm_Sky_Color(lua_State* L)
{
	auto color = Get_Warm_Sky_Color();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Returns the current hue of the cold sky effect.
 *  \par Command Usage Example
 *          color = Get_Cold_Sky_Color()
 *
 *  \return Number The integer value representing the cold sky color (RGB packed).
 */
int Lua_Get_Cold_Sky_Color(lua_State* L)
{
	auto color = Get_Cold_Sky_Color();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Returns the current hue of the sun.
 *  \par Command Usage Example
 *          color = Get_Sun_Color()
 *
 *  \return Number The integer value representing the sun color (RGB packed).
 */
int Lua_Get_Sun_Color(lua_State* L)
{
	auto color = Get_Sun_Color();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Returns the current hue of the sun halo effect.
 *  \par Command Usage Example
 *          color = Get_Sun_Halo_Color()
 *
 *  \return Number The integer value representing the sun halo color (RGB packed).
 */
int Lua_Get_Sun_Halo_Color(lua_State* L)
{
	auto color = Get_Sun_Halo_Color();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Returns the current alpha of the starfield.
 *  \par Command Usage Example
 *          alpha = Get_Starfield_Alpha()
 *
 *  \return Number The integer value representing the starfield transparency (0-255).
 */
int Lua_Get_Starfield_Alpha(lua_State* L)
{
	auto color = Get_Starfield_Alpha();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Returns the current hue of the overall sky.
 *  \par Command Usage Example
 *          color = Get_Sky_Color()
 *
 *  \return Number The integer value representing the sky color (RGB packed).
 */
int Lua_Get_Sky_Color(lua_State* L)
{
	auto color = Get_Sky_Color();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Returns the current hue of the gloominess effect.
 *  \par Command Usage Example
 *          color = Get_Gloominess_Color()
 *
 *  \return Number The integer value representing the gloominess color (RGB packed).
 */
int Lua_Get_Gloominess_Color(lua_State* L)
{
	auto color = Get_Gloominess_Color();
	lua_pushnumber(L, color);
	return 1;
}

/*!\details Sets the hue of the warm sky effect.
 *  \par Command Usage Example
 *          Set_Warm_Sky_Color(ColorValue)
 *
 *  \param ColorValue Integer value (RGB packed) for the new warm sky hue.
 */
int Lua_Set_Warm_Sky_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Warm_Sky_Color(val);
	}
	return 0;
}

/*!\details Sets the hue of the cold sky effect.
 *  \par Command Usage Example
 *          Set_Cold_Sky_Color(ColorValue)
 *
 *  \param ColorValue Integer value (RGB packed) for the new cold sky hue.
 */
int Lua_Set_Cold_Sky_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Cold_Sky_Color(val);
	}
	return 0;
}

/*!\details Sets the hue of the sun.
 *  \par Command Usage Example
 *          Set_Sun_Color(ColorValue)
 *
 *  \param ColorValue Integer value (RGB packed) for the new sun hue.
 */
int Lua_Set_Sun_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Sun_Color(val);
	}
	return 0;
}

/*!\details Sets the hue of the sun halo effect.
 *  \par Command Usage Example
 *          Set_Sun_Halo_Color(ColorValue)
 *
 *  \param ColorValue Integer value (RGB packed) for the new sun halo hue.
 */
int Lua_Set_Sun_Halo_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Sun_Halo_Color(val);
	}
	return 0;
}

/*!\details Sets the hue of the moon halo effect.
 *  \par Command Usage Example
 *          Set_Moon_Halo_Color(ColorValue)
 *
 *  \param ColorValue Integer value (RGB packed) for the new moon halo hue.
 */
int Lua_Set_Moon_Halo_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Moon_Halo_Color(val);
	}
	return 0;
}

/*!\details Sets the alpha of the starfield.
 *  \par Command Usage Example
 *          Set_Starfield_Alpha(AlphaValue)
 *
 *  \param AlphaValue Integer value (0-255) for the new starfield transparency.
 */
int Lua_Set_Starfield_Alpha(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Starfield_Alpha(val);
	}
	return 0;
}

/*!\details Sets the hue of the overall sky.
 *  \par Command Usage Example
 *          Set_Sky_Color(ColorValue)
 *
 *  \param ColorValue Integer value (RGB packed) for the new sky hue.
 */
int Lua_Set_Sky_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Sky_Color(val);
	}
	return 0;
}

/*!\details Sets the hue of the gloominess effect.
 *  \par Command Usage Example
 *          Set_Gloominess_Color(ColorValue)
 *
 *  \param ColorValue Integer value (RGB packed) for the new gloominess hue.
 */
int Lua_Set_Gloominess_Color(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		unsigned int val = static_cast<unsigned int>(lua_tonumber(L, 1));
		Set_Gloominess_Color(val);
	}
	return 0;
}

/*!\details Indicates whether the sky colors are currently being overridden.
 *  \par Command Usage Example
 *          overriding = Is_Overriding_Sky_Colors()
 *
 *  \return Number `true` if overrides are active; otherwise `false`.
 */
int Lua_Is_Overriding_Sky_Colors(lua_State* L)
{
	lua_pushboolean(L, Is_Overriding_Sky_Colors());
	return 0;
}

/*!\details Enables or disables overriding of the sky colors.
 *  \par Command Usage Example
 *          Override_Sky_Colors(Enable)
 *
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
int Lua_Override_Sky_Colors(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Override_Sky_Colors(lua_tobooleanCPP(L, 1));
	}
	return 0;
}

/*!\details Changes the enemy HUD color for all players.
 *  \par Command Usage Example
 *          Change_Enemy_HUD_Color(R, G, B)
 *
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 */
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

/*!\details Changes the enemy HUD color for a specific player.
 *  \par Command Usage Example
 *          Change_Enemy_HUD_Color_Player(Soldier, R, G, B)
 *
 *  \param Soldier Integer ID of the target soldier (must be a star).
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 */
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

/*!\details Changes the friendly HUD color for all players.
 *  \par Command Usage Example
 *          Change_Friendly_HUD_Color(R, G, B)
 *
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 */
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

/*!\details Changes the neutral HUD color for all players.
 *  \par Command Usage Example
 *          Change_Neutral_HUD_Color(R, G, B)
 *
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 */
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

/*!\details Changes the friendly HUD color for a specific player.
 *  \par Command Usage Example
 *          Change_Friendly_HUD_Color_Player(Soldier, R, G, B)
 *
 *  \param Soldier Integer ID of the target soldier (must be a star).
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 */
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

/*!\details Changes the neutral HUD color for a specific player.
 *  \par Command Usage Example
 *          Change_Neutral_HUD_Color_Player(Soldier, R, G, B)
 *
 *  \param Soldier Integer ID of the target soldier (must be a star).
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 */
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

/*!\details Creates a HUD surface with a specific identifier.
 *  \par Command Usage Example
 *          hud = Create_HUD_Surface(ID)
 *
 *  \param ID Integer identifier for the HUD surface.
 *  \return Table Reference to the created LuaHUDSurface object, or nil on failure.
 */
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

/*!\details Finds a HUD surface by its identifier.
 *  \par Command Usage Example
 *          hud = Find_HUD_Surface(ID)
 *
 *  \param ID Integer identifier of the HUD surface to locate.
 *  \return Table Reference to the found LuaHUDSurface object, or nil if not found.
 */
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

/*!\details Shows a HUD surface on the screen.
 *  \par Command Usage Example
 *          Show_HUD_Surface(Hud)
 *
 *  \param Hud LuaHUDSurface reference to display.
 */
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

/*!\details Hides a HUD surface from the screen.
 *  \par Command Usage Example
 *          Hide_HUD_Surface(Hud)
 *
 *  \param Hud LuaHUDSurface reference to hide.
 */
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

/*!\details Deletes a HUD surface and frees its resources.
 *  \par Command Usage Example
 *          Delete_HUD_Surface(Hud)
 *
 *  \param Hud LuaHUDSurface reference to delete.
 */
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

/*!\details Enables or disables a set of spawners identified by name.
 *  \par Command Usage Example
 *          Enable_Spawners_By_Name(Name, Enable)
 *
 *  \param Name String name of the spawner group to modify.
 *  \param Enable Boolean (`true` to enable; `false` to disable).
 */
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

/*!\details Returns whether extra UI elements (e.g., custom HUD) are enabled.
 *  \par Command Usage Example
 *          extras = Is_Extras_Enabled()
 *
 *  \return Number `true` if extras are active; otherwise `false`.
 */
int Lua_Is_Extras_Enabled(lua_State* L)
{
	bool enable = Is_Extras_Enabled();
	lua_pushboolean(L, enable);
	return 1;
}

/*!\details Returns whether the sidebar UI is currently enabled.
 *  \par Command Usage Example
 *          sidebar = Is_Sidebar_Enabled()
 *
 *  \return Number `true` if sidebar is shown; otherwise `false`.
 */
int Lua_Is_Sidebar_Enabled(lua_State* L)
{
	bool enable = Is_Sidebar_Enabled();
	lua_pushboolean(L, enable);
	return 1;
}

/*!\details Adds a stylized shadow message to the HUD for a specific player.
 *  \par Command Usage Example
 *          Add_Shadow_Message(PlayerID, R, G, B, Text)
 *
 *  \param PlayerID Integer ID of the target player.
 *  \param R Float red component (0-1).
 *  \param G Float green component (0-1).
 *  \param B Float blue component (0-1).
 *  \param Text Wide string message to display.
 */
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

/*!\details Sends a command to forward data to another server or client.
 *  \par Command Usage Example
 *          Forward_Server(SenderID, ReceiverID, Channel, Message)
 *
 *  \param SenderID Integer ID of the sending player.
 *  \param ReceiverID Integer ID of the receiving player.
 *  \param Channel Integer channel number for routing.
 *  \param Message String message to forward. Can be nil if omitted.
 */
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

/*!\details Retrieves a list of all available waypath identifiers in the current map.
 *  \par Command Usage Example
 *          paths = Retrieve_Waypaths()
 *
 *  \return Table List of integer IDs for each available waypath.
 */
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

/*!\details Retrieves the world coordinates of a waypoint given its path and index.
 *  \par Command Usage Example
 *          pos = Get_Waypoint_Position(PathID, WaypointIndex)
 *
 *  \param PathID Integer identifier of the path containing the waypoint.
 *  \param WaypointIndex Integer index (1-based) of the waypoint within that path.
 *  \return Table A `Vector3` table with X, Y and Z components of the waypoint position.
 */
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

/*!\details Returns a table containing the indices of all waypoints belonging to a given path.
 *  \par Command Usage Example
 *          ways = Retrieve_Waypoints(PathID)
 *
 *  \param PathID Integer identifier of the path whose waypoints are queried.
 *  \return Table List of integer waypoint indices indexed by 1-based position.
 */
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

/*!\details Retrieves the current human state of a soldier object.
 *  \par Command Usage Example
 *          state = Get_Human_State(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Number The numeric identifier of the soldier’s current state, or nil if not found.
 */
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

/*!\details Checks a specific flag within a soldier’s human state.
 *  \par Command Usage Example
 *          flag = Get_Human_State_Flag(Soldier, FlagIndex)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param FlagIndex Integer index of the state flag to query (1-based).
 *  \return Number `true` if the flag is set; otherwise `false`.
 */
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

/*!\details Sets the vertical skeleton height of a soldier.
 *  \par Command Usage Example
 *          Set_Skeleton_Height(Soldier, Height)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param Height Float new skeleton height value (in world units).
 */
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

/*!\details Sets the horizontal skeleton width of a soldier.
 *  \par Command Usage Example
 *          Set_Skeleton_Width(Soldier, Width)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param Width Float new skeleton width value (in world units).
 */
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

/*!\details Activates or deactivates a smart game object’s stealth mode.
 *  \par Command Usage Example
 *          Set_Stealth_Active(SmartObject, Active)
 *
 *  \param SmartObject Integer ID of the target smart game object (typically a soldier).
 *  \param Active Boolean (`true` to enable stealth; `false` to disable).
 */
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

/*!\details Adds a radar marker at the specified world position for a given team.
 *  \par Command Usage Example
 *          Add_Radar_Marker(TeamID, Position, Type, Radius)
 *
 *  \param TeamID Integer ID of the team that will see the marker (1 or 2).
 *  \param Position LuaVector3 world coordinates where the marker should be placed.
 *  \param Type Integer identifier for the marker type (icon index).
 *  \param Radius Integer distance radius for the marker visibility.
 */
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

/*!\details Removes a single radar marker identified by its unique ID.
 *  \par Command Usage Example
 *          Clear_Radar_Marker(MarkerID)
 *
 *  \param MarkerID Integer identifier of the radar marker to remove.
 */
int Lua_Clear_Radar_Marker(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		Commands->Clear_Radar_Marker(luaL_checkinteger(L, 1));
	}
	return 0;
}

/*!\details Clears all radar markers from the current view.
 *  \par Command Usage Example
 *          Clear_Radar_Markers()
 */
int Lua_Clear_Radar_Markers(lua_State* L)
{
	Commands->Clear_Radar_Markers();
	return 0;
}

/*!\details Sets a visual blip on the objective marker at a specific world position for a given team.
 *  \par Command Usage Example
 *          Set_Objective_Radar_Blip(TeamID, Position)
 *
 *  \param TeamID Integer ID of the team that will see the blip (1 or 2).
 *  \param Position LuaVector3 coordinates where the objective blip should appear.
 */
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

/*!\details Assigns a color index to the radar blip of a specific object.
 *  \par Command Usage Example
 *          Set_Obj_Radar_Blip_Color(ObjectID, ColorIndex)
 *
 *  \param ObjectID Integer ID of the target object whose blip color is set.
 *  \param ColorIndex Integer identifier for the desired color.
 */
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

/*!\details Assigns a shape index to the radar blip of a specific object.
 *  \par Command Usage Example
 *          Set_Obj_Radar_Blip_Shape(ObjectID, ShapeIndex)
 *
 *  \param ObjectID Integer ID of the target object whose blip shape is set.
 *  \param ShapeIndex Integer identifier for the desired shape.
 */
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

/*!\details Retrieves the horizontal skeleton width of a soldier.
 *  \par Command Usage Example
 *          width = Get_Skeleton_Width(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Number The current skeleton width in world units.
 */
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

/*!\details Retrieves the vertical skeleton height of a soldier.
 *  \par Command Usage Example
 *          height = Get_Skeleton_Heigth(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Number The current skeleton height in world units.
 */
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

/*!\details Determines whether a game object has been destroyed.
 *  \par Command Usage Example
 *          destroyed = Is_Destroyed(Object)
 *
 *  \param Object Integer ID of the target object.
 *  \return Number `true` if the object is in a destroyed state; otherwise `false`.
 */
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

/*!\details Retrieves the camera profile name associated with a game object.
 *  \par Command Usage Example
 *          cam = Get_Camera(Object)
 *
 *  \param Object Integer ID of the target object (soldier or vehicle).
 *  \return String The current camera profile identifier (“Normal” for soldiers, vehicle’s profile otherwise).
 */
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

/*!\details Sets a specific dirty bit flag for an object, player or text component.
 *  \par Command Usage Example
 *          Set_Object_Dirty_Bit(ObjectID/LightUserData, BitIndex, BitType, Flag)
 *
 *  \param ObjectIDOrPtr Either integer ID of the target object/player or light-userdata pointer to a cScTextObj instance.
 *  \param BitIndex Integer index of the dirty bit to modify.
 *  \param BitType Integer representing the network dirty bit enumeration.
 *  \param Flag Boolean (`true` to set; `false` to clear).
 */
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

/*!\details Sends a text message to all clients or a specific client with optional styling.
 *  \par Command Usage Example
 *          TextObj = Send_Client_Text(Message, Type, Bold, Flash, Duration, Shout, Show)
 *
 *  \param Message Wide string containing the text to display.
 *  \param Type Integer identifier for the message type (e.g., chat, system).
 *  \param Bold Boolean (`true` for bold font).
 *  \param Flash Boolean (`true` to flash the message on screen).
 *  \param Duration Integer time in seconds the message remains visible.
 *  \param Shout Boolean (`true` to play a shout sound with the message).
 *  \param Show Boolean (`true` to show the message immediately).
 *  \return Table Light-userdata reference to the created text object, or nil if creation failed.
 */
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

/*!\details Retrieves the RGB color components of a game object’s material.
 *  \par Command Usage Example
 *          r, g, b = Get_Object_Color(Object)
 *
 *  \param Object Integer ID of the target object.
 *  \return Three numbers representing red, green and blue components (0-255).
 */
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

/*!\details Returns the world-space minimum and maximum extents of the level.
 *  \par Command Usage Example
 *          min, max = Get_Level_Extents()
 *
 *  \return Two `Vector3` tables: first containing the minimum corner (X,Y,Z),
 *          second containing the maximum corner (X,Y,Z) of the level bounds.
 */
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

/*!\details Checks whether a skin has impervious (indestructible) status against a given damage type.
 *  \par Command Usage Example
 *          imp = Is_Skin_Impervious(SkinID, DamageType)
 *
 *  \param SkinID Integer identifier of the armor/skin.
 *  \param DamageType Integer value representing the special damage type to test.
 *  \return Number `true` if the skin is impervious to that damage; otherwise `false`.
 */
int Lua_Is_Skin_Impervious(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		lua_pushboolean(L,ArmorWarheadManager::Is_Skin_Impervious(static_cast<ArmorWarheadManager::SpecialDamageType>(luaL_checkinteger(L, 1)),luaL_checkinteger(L, 2)));
		return 1;
	}
	return 0;
}

/*!\details Retrieves the damage multiplier for a specific warhead against a target armor type.
 *  \par Command Usage Example
 *          mul = Get_Damage_Multiplier(WarheadID, ArmorType)
 *
 *  \param WarheadID Integer identifier of the warhead.
 *  \param ArmorType Integer identifier of the target armor type.
 *  \return Number The damage multiplier value (float).
 */
int Lua_Get_Damage_Multiplier(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		lua_pushnumber(L, ArmorWarheadManager::Get_Damage_Multiplier(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
		return 1;
	}
	return 0;
}

/*!\details Retrieves the shield absorption value for a given warhead and armor type.
 *  \par Command Usage Example
 *          absorb = Get_Shield_Absorbsion(WarheadID, ArmorType)
 *
 *  \param WarheadID Integer identifier of the warhead.
 *  \param ArmorType Integer identifier of the target armor type.
 *  \return Number The shield absorption value (float).
 */
int Lua_Get_Shield_Absorbsion(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		lua_pushnumber(L, ArmorWarheadManager::Get_Shield_Absorbsion(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
		return 1;
	}
	return 0;
}

/*!\details Retrieves the sight range of a smart game object (e.g., soldier or vehicle).
 *  \par Command Usage Example
 *          range = Get_Sight_Range(Object)
 *
 *  \param Object Integer ID of the target smart game object.
 *  \return Number The current sight range in world units, or nil if not found.
 */
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

/*!\details Sets the sight range of a smart game object.
 *  \par Command Usage Example
 *          Set_Sight_Range(Object, Range)
 *
 *  \param Object Integer ID of the target smart game object.
 *  \param Range Float new sight range value in world units.
 */
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

/*!\details Retrieves the sight arc (field of view angle) for a smart game object.
 *  \par Command Usage Example
 *          arc = Get_Sight_Arc(Object)
 *
 *  \param Object Integer ID of the target smart game object.
 *  \return Number The current sight arc in degrees, or nil if not found.
 */
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

/*!\details Sets the sight arc of a smart game object.
 *  \par Command Usage Example
 *          Set_Sight_Arc(Object, Arc)
 *
 *  \param Object Integer ID of the target smart game object.
 *  \param Arc Float new sight arc value in degrees.
 */
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

/*!\details Retrieves the boolean flag indicating whether a building definition hides its battlefield information from a team.
 *  \par Command Usage Example
 *          hidden = Get_Hide_Team_Battlefield_Information(DefID)
 *
 *  \param DefID Integer ID of the building definition to query.
 *  \return Number `true` if the building’s battlefield info is hidden; otherwise `false`.
 */
int Lua_Get_Hide_Team_Battlefield_Information(lua_State* L)
{
#ifdef DAPLUGIN
	if (lua_gettop(L) == 1)
	{
		BuildingGameObjDef* def = (BuildingGameObjDef*)Find_Definition(luaL_checknumber(L, 1));
		if (def)
		{
			lua_pushboolean(L, def->HideTeamBattlefieldInformation);
			return 1;
		}
	}
#endif
	return 0;
}

/*!\details Sets the boolean flag that controls whether a building’s battlefield information is hidden from teams.
 *  \par Command Usage Example
 *          HideTeamBattlefieldInformation(DefID, Enable)
 *
 *  \param DefID Integer ID of the building definition to modify.
 *  \param Enable Boolean (`true` to hide; `false` to show).
 */
int Lua_HideTeamBattlefieldInformation(lua_State* L)
{
#ifdef DAPLUGIN
	if (lua_gettop(L) == 2)
	{
		BuildingGameObjDef* def = (BuildingGameObjDef*)Find_Definition(luaL_checknumber(L, 1));
		if (def)
		{
			def->HideTeamBattlefieldInformation = lua_tobooleanCPP(L, 2);
		}
	}
#endif
	return 0;
}

/*!\details Triggers a smooth resize of a soldier’s skeleton width over time.
 *  \par Command Usage Example
 *          Trigger_Smooth_Skeleton_Width_Resize(Soldier, TargetWidth, Duration)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param TargetWidth Float desired width value to animate towards.
 *  \param Duration Float duration in seconds for the animation.
 */
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

/*!\details Triggers a smooth resize of a soldier’s skeleton height over time.
 *  \par Command Usage Example
 *          Trigger_Smooth_Skeleton_Height_Resize(Soldier, TargetHeight, Duration)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param TargetHeight Float desired height value to animate towards.
 *  \param Duration Float duration in seconds for the animation.
 */
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

/*!\details Sets the target point for a smart game object and optionally activates targeting.
 *  \par Command Usage Example
 *          Set_Targeting(Object, Position, Active)
 *
 *  \param Object Integer ID of the target smart game object (soldier or vehicle).
 *  \param Position LuaVector3 world coordinates to set as the new targeting point.
 *  \param Active Boolean (`true` to enable targeting toward that point; `false` to disable).
 */
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

/*!\details Retrieves the sub-state (e.g., animation or action phase) of a soldier’s human state.
 *  \par Command Usage Example
 *          sub = Get_Human_Sub_State(Soldier)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \return Number The current human sub-state identifier, or nil if not found.
 */
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

/*!\details Retrieves the world position of a soldier’s muzzle for a given weapon index.
 *  \par Command Usage Example
 *          pos = Get_Muzzle_Position(Soldier, WeaponIndex)
 *
 *  \param Soldier Integer ID of the target soldier.
 *  \param WeaponIndex Integer index of the weapon whose muzzle is queried.
 *  \return Table A `Vector3` table containing the X, Y and Z coordinates of the muzzle point.
 */
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

/*!\details Performs a raycast intersection against the physics scene and returns the ID of any hit damageable object.
 *  \par Command Usage Example
 *          id = Intersects_Object(Origin, Direction, Length, CollisionGroup)
 *
 *  \param Origin LuaVector3 start point of the ray.
 *  \param Direction LuaVector3 direction vector (normalized or un-scaled).
 *  \param Length Float maximum length of the ray segment.
 *  \param CollisionGroup Integer value representing the collision group to test against.
 *  \return Number ID of the first damageable object intersected, or nil if no hit occurs.
 */
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

/*!\details Determines whether a damageable object is currently targetable.
 *  \par Command Usage Example
 *          targetable = Is_Targetable(Object)
 *
 *  \param Object Integer ID of the target damageable game object.
 *  \return Number `true` if the object can be targeted; otherwise `false`.
 */
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

/*!\details Checks if the health bar overlay is currently shown for a damageable object.
 *  \par Command Usage Example
 *          displayed = Is_Health_Bar_Displayed(Object)
 *
 *  \param Object Integer ID of the target damageable game object.
 *  \return Number `true` if the health bar is visible; otherwise `false`.
 */
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

/*!\details Increments the flood counter for a player.
 *  \par Command Usage Example
 *          Increment_Flood_Counter(PlayerID)
 *
 *  \param PlayerID Integer ID of the target player whose counter is increased.
 */
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

/*!\details Decrements the flood counter for a player.
 *  \par Command Usage Example
 *          Decrement_Flood_Counter(PlayerID)
 *
 *  \param PlayerID Integer ID of the target player whose counter is decreased.
 */
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

/*!\details Returns a table of all simple game objects currently active in the world.
 *  \par Command Usage Example
 *          simples = Get_All_Simple_Objects()
 *
 *  \return Table List of integer IDs for each simple object found.
 */
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

/*!\details Enables or disables the ability of a damageable object to die.
 *  \par Command Usage Example
 *          Set_Can_Object_Die(ObjectID, CanDie)
 *
 *  \param ObjectID Integer ID of the target damageable game object.
 *  \param CanDie Boolean (`true` to allow death; `false` to prevent it).
 */
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

/*!\details Retrieves the parent object of a given physical game object.
 *  \par Command Usage Example
 *          parent = Get_Parent_Object(Object)
 *
 *  \param Object Integer ID of the target physical object.
 *  \return Number The ID of its parent object, or nil if none exists.
 */
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

/*!\details Determines whether a physical game object is currently rendered.
 *  \par Command Usage Example
 *          rendered = Is_Rendered(Object)
 *
 *  \param Object Integer ID of the target physical object.
 *  \return Number `true` if the object is visible; otherwise `false`.
 */
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

/*!\details Retrieves the list of interior mesh identifiers belonging to a building object.
 *  \par Command Usage Example
 *          meshes = Get_Interior_Building_Meshes(Building)
 *
 *  \param Building Integer ID of the target building object.
 *  \return Table List of integer IDs for each interior mesh in the building.
 */
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

/*!\details Retrieves the list of exterior mesh identifiers belonging to a building object.
 *  \par Command Usage Example
 *          meshes = Get_Exterior_Building_Meshes(Building)
 *
 *  \param Building Integer ID of the target building object.
 *  \return Table List of integer IDs for each exterior mesh in the building.
 */
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

/*!\details Checks whether a building has any interior meshes.
 *  \par Command Usage Example
 *          has = Has_Interior_Building_Meshes(Building)
 *
 *  \param Building Integer ID of the target building object.
 *  \return Number `true` if at least one interior mesh exists; otherwise `false`.
 */
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

/*!\details Checks whether a building has any exterior meshes.
 *  \par Command Usage Example
 *          has = Has_Exterior_Building_Meshes(Building)
 *
 *  \param Building Integer ID of the target building object.
 *  \return Number `true` if at least one exterior mesh exists; otherwise `false`.
 */
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

/*!\details Displays the HUD notification that a player has received a weapon grant.
*  \par Command Usage Example
*          Display_HUD_Weapon_Grant_Player(PlayerID, WeaponID, Rounds)
*
*  \param PlayerID Integer ID of the target player (must be a star).
*  \param WeaponID Integer identifier of the weapon granted.
*  \param Rounds Integer number of rounds delivered for the weapon.
*/
int Lua_Display_HUD_Weapon_Grant_Player(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	int wID = luaL_checkinteger(L, 2);
	int rounds = luaL_checkinteger(L, 3);
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	Display_HUD_Weapon_Grant_Player(obj, wID, rounds);
	return 0;
}

/*!\details Displays the HUD notification that a player has received an ammo grant.
*  \par Command Usage Example
*          Display_HUD_Ammo_Grant_Player(PlayerID, AmmoType, Amount)
*
*  \param PlayerID Integer ID of the target player (must be a star).
*  \param AmmoType Integer identifier of the ammo type granted.
*  \param Amount Integer amount of ammo delivered.
*/
int Lua_Display_HUD_Ammo_Grant_Player(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	int aType = luaL_checkinteger(L, 2);
	int amount = luaL_checknumber(L, 3);
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	Display_HUD_Ammo_Grant_Player(obj, aType, amount);
	return 0;
}

/*!
 *  \details Retrieves the multiplayer spawn location for a given player and team.
 *  \par Command Usage Example
 *          pos = Get_Multiplayer_Spawn_Location(Team, PlayerID)
 *
 *  \param Team Integer team identifier.
 *  \param PlayerID Integer ID of the target player (must be a star).
 *  \return Table A `Vector3` table with X, Y and Z components of the spawn location.
 */
int Lua_Get_Multiplayer_Spawn_Location(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int team = luaL_checkinteger(L, 1);
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 2));
	if (!obj || !obj->As_SoldierGameObj()) return 0;
	Matrix3D matrix;
	Get_Multiplayer_Spawn_Location(team, obj->As_SoldierGameObj(), matrix);
	Vector3 pos = matrix.Get_Translation();
	LUA_Vector3* a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);
	return 1;
}

int Lua_Set_Subobject_Animation_Player(lua_State* L)
{
    if (lua_gettop(L) < 8) return 0;
    GameObject* player    = Commands->Find_Object(luaL_checknumber(L, 1));
	GameObject* obj = Commands->Find_Object(luaL_checknumber(L, 2));
    const char* animation = luaL_checkstring(L, 3);
    bool looping          = lua_toboolean(L, 4);
    const char* subobject = luaL_checkstring(L, 5);
    float startFrame      = (float)luaL_checknumber(L, 6);
    float endFrame        = (float)luaL_checknumber(L, 7);
	bool blended = lua_toboolean(L, 8);
    if (!player || !obj) return 0;
    int result = Set_Subobject_Animation_Player(player, obj, animation, looping, subobject, startFrame, endFrame, blended);
	lua_pushboolean(L, result);
    return 1;
}

int Lua_Set_Subobject_Animation(lua_State* L)
{
	if (lua_gettop(L) < 7) return 0;

    GameObject* obj       = Commands->Find_Object(luaL_checkinteger(L, 1));
    const char* animation = luaL_checkstring(L, 2);
    bool looping          = lua_toboolean(L, 3);
    const char* subobject = luaL_checkstring(L, 4);
    float startFrame      = (float)luaL_checknumber(L, 5);
    float endFrame        = (float)luaL_checknumber(L, 6);
    bool blended          = lua_toboolean(L, 7);
    if (!obj) return 0;

	int result = Set_Subobject_Animation(obj, animation, looping, subobject, startFrame, endFrame, blended);
	lua_pushboolean(L, result);
    return 1;
}

int Lua_Say_Dynamic_Dialogue(lua_State* L)
{
    if (lua_gettop(L) < 2) return 0;

	int id = luaL_checkinteger(L, 1);
    GameObject* speaker  = Commands->Find_Object(luaL_checkinteger(L, 2));
	int sound_id = luaL_checkinteger(L, 1);

	if (!speaker && !speaker->As_SoldierGameObj()) return 0;

	Say_Dynamic_Dialogue(id, speaker->As_SoldierGameObj(), &sound_id);
    return 0;
}

/*!
 *  \details Plays a dynamic dialogue line to a specific player.
 *  \par Command Usage Example
 *          Say_Dynamic_Dialogue_Player(PlayerID, TextID, SpeakerID)
 *
 *  \param PlayerID Integer ID of the target player receiving the dialogue.
 *  \param TextID Integer identifier of the dialogue text.
 *  \param SpeakerID Integer ID of the star that speaks the line (must be a star).
 */
int Lua_Say_Dynamic_Dialogue_Player(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;

	int playerId = luaL_checkinteger(L, 1);
	int text_id = luaL_checkinteger(L, 2);
	int id = 0;
	GameObject* speaker = Commands->Find_Object(luaL_checkinteger(L, 3));

	if (!speaker || !speaker->As_SoldierGameObj()) return 0;

	Say_Dynamic_Dialogue_Player(Commands->Find_Object(playerId), text_id, speaker->As_SoldierGameObj(), &id);
	return 0;
}

/*!
 *  \details Plays a dynamic dialogue line to an entire team.
 *  \par Command Usage Example
 *          Say_Dynamic_Dialogue_Team(TextID, SpeakerID, Team)
 *
 *  \param TextID Integer identifier of the dialogue text.
 *  \param SpeakerID Integer ID of the star that speaks the line (must be a star).
 *  \param Team Integer ID of the team that should hear the dialogue.
 */
int Lua_Say_Dynamic_Dialogue_Team(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;

	int text_id = luaL_checkinteger(L, 1);
	int id = 0;
	GameObject* speaker = Commands->Find_Object(luaL_checkinteger(L, 2));

	if (!speaker || !speaker->As_SoldierGameObj()) return 0;

	Say_Dynamic_Dialogue_Team(text_id, speaker->As_SoldierGameObj(), &id, luaL_checkinteger(L, 3));
	return 0;
}

/*!
 *  \details Uses the internal BuildingGameObj::Find_Closest_Poly method to calculate the closest polygon position to a given location on the building's surface.
 *  \par Command Usage Example
 *          Find_Closest_Poly_Position(BuildingID, Vector3)
 *
 *  \param BuildingID Integer ID of the building whose poly structure is checked.
 *  \param Location The target world coordinates to find the closest poly position from.
 *  \return Vector3 Returns a Vector3 object containing the closest polygon position, or nothing if the building is invalid.
 */
int Lua_Find_Closest_Poly_Position(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(luaL_checkinteger(L, 1));
	if (!obj) return 0;
	BuildingGameObj* building = obj->As_BuildingGameObj();
	if (!building)
	{
		return 0;
	}
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 location(a->X(), a->Y(), a->Z());
	Vector3 polyPos;
	Find_Closest_Poly_Position(building, location, &polyPos);

	// Build the Vector3 result using the globally registered Vector3 constructor
	// (registered as the "Vector3" global by LuaVector3::Register).
	lua_getglobal(L, "Vector3");
	lua_pushnumber(L, polyPos.X);
	lua_pushnumber(L, polyPos.Y);
	lua_pushnumber(L, polyPos.Z);
	if (lua_pcall(L, 3, 1, 0) != 0)
	{
		lua_pop(L, 1);
		return 0;
	}
	return 1;
}

/*!details Returns whether kill messages are currently disabled.
 *  \par Command Usage Example
 *          disabled = Kill_Messages_Disabled()
 *
 *  \return Number `true` if kill messages are disabled; otherwise `false`.
 */
int Lua_Kill_Messages_Disabled(lua_State* L)
{
	bool disabled = Kill_Messages_Disabled();
	lua_pushboolean(L, disabled);
	return 1;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define REGFUNC(func) lua_register(L, #func, Lua_##func);

void AddFunctions(lua_State *L)
{

/* TODO
	Set_Emote_Icon?
	Get_Radio_Command_String
	Get_Closest_Pathfind_Spot_Size
	Get_Closest_Pathfind_Spot
	
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
	REGFUNC(Set_Subobject_Animation)
	REGFUNC(Say_Dynamic_Dialogue)
	REGFUNC(Say_Dynamic_Dialogue_Player)
	REGFUNC(Say_Dynamic_Dialogue_Team)
	REGFUNC(Find_Closest_Poly_Position)
	REGFUNC(Kill_Messages_Disabled)
	REGFUNC(Display_HUD_Ammo_Grant_Player)
	REGFUNC(Get_Multiplayer_Spawn_Location)
	REGFUNC(Set_Subobject_Animation_Player)
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
	REGFUNC(Set_Emot_Icon)
	REGFUNC(Write_File_Async)
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
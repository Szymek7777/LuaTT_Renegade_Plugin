/*	Lua DA Functions
Renegade Dragonade Lua wrapper (DA Only)
by Stan "sla.ro" Laurentiu Alexandru
Copyright 2010-2022 Sla Studios (http://slastudios.net)

This file is part of the LuaTT
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "General.h"
#include "luatt.h"
#include "LuaLib/lua.hpp"
#include "LuaFunctionsDA.h"
#include "LuaManager.h"
#include "Lua_Class.h"
#include "hooks.h"

// DA Includes
#include "da.h"
#include "da_gameobj.h"
#include "da_ban.h"
#include "da_settings.h"
#include "da_vehicleownership.h"
#include "da_gamemode.h"
#include "da_log.h"
#include "engine_da.h"
#include "engine_player.h"
#include "da_cratemanager.h"
#include "da_game.h"
#include "da_player.h"
#include "da_team.h"
#include "da_vehicle.h"
#include "da_translation.h"
#include "da_damagelog.h"

#include "engine_def.h"
#include "engine_phys.h"
#include "MoveablePhysClass.h"

uint DA_API Send_Object_Update(NetworkObjectClass* Object, int ID);

#endif

#pragma warning(disable: 4244 4800)

static std::vector<DANullGameModeRegistrant*> Lua_NullGameModes;

/*!\details Sets a gameobject invisible
*  \par Lua Command
*  \code{.lua}
*			DA_Set_GameObj_Invisible(Object)
*  \endcode
*  \par		Example
*  \code{.lua}
*			DA_Set_GameObj_Invisible(Get_GameObj(1)) -- Player 1 is now invisible to everyone (including himself)
*  \endcode
*  \param Object Number
*/
int LuaDA_Set_GameObj_Invisible(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr)
	{
		DAGameObjManager::Set_GameObj_Invisible(obj);
	}

	return 1;
}

/*!\details Sets object as pending removal
*  \par Lua Command
*  \code{.lua}
*			DA_Set_GameObj_Delete_Pending(Object)
*  \endcode
*  \par		Example
*  \code{.lua}
*			DA_Set_GameObj_Delete_Pending(Get_GameObj(1)) -- Player gets killed
*  \endcode
*  \param Object Number
*/
int LuaDA_Set_GameObj_Delete_Pending(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr)
	{
		DAGameObjManager::Set_GameObj_Delete_Pending(obj);
	}

	return 1;
}

/*!\details Sets object visible to TT players only
*  \par Lua Command
*  \code{.lua}
*			DA_Set_GameObj_TT_Only(Object)
*  \endcode
*  \par		Example
*  \code{.lua}
*			DA_Set_GameObj_TT_Only(Get_GameObj(1)) -- Player 1 is only visible to TT players
*  \endcode
*  \param Object Number
*/
int LuaDA_Set_GameObj_TT_Only(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr)
	{
		DAGameObjManager::Set_GameObj_TT_Only(obj);
	}

	return 1;
}

/*!\details Sets object visible to Stock players only
*  \par Lua Command
*  \code{.lua}
*			DA_Set_GameObj_Stock_Only(Object)
*  \endcode
*  \par		Example
*  \code{.lua}
*			DA_Set_GameObj_Stock_Only(Get_GameObj(1)) -- Player 1 is only visible to Stock Players
*  \endcode
*  \param Object Number
*/
int LuaDA_Set_GameObj_Stock_Only(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr)
	{
		DAGameObjManager::Set_GameObj_Stock_Only(obj);
	}

	return 1;
}

/*!\details Sets a gameobject invisible but without delete
*  \par Lua Command
*  \code{.lua}
*			DA_Set_GameObj_Invisible_No_Delete(Object)
*  \endcode
*  \par		Example
*  \code{.lua}
*			DA_Set_GameObj_Invisible_No_Delete(Get_GameObj(1)) -- Player 1 is now invisible to everyone (including himself)
*  \endcode
*  \param Object Number
*/
int LuaDA_Set_GameObj_Invisible_No_Delete(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr)
	{
		DAGameObjManager::Set_GameObj_Invisible_No_Delete(obj);
	}

	return 1;
}


/*!\details Returns a list of bans from DA Ban List
*  \par Lua Command
*  \code{.lua}
*			banlist = DA_Get_Ban_List()
*  \endcode
*  \par		Example
*  \code{.lua}
*			return #DA_Get_Ban_List()  -- returns: 0 or the number of bans in list
*  \endcode
*  \return list Table
*/
int LuaDA_Get_Ban_List(lua_State* L)
{
	DABanListClass* banlist = DABanManager::Get_Ban_List();
	lua_gettop(L);
	lua_createtable(L, banlist->Get_Count(), 0);

	for (int a = 0; a < banlist->Get_Count(); a++)
	{
		DABanEntryClass* p = banlist->Peek_Entry(a);
		lua_pushnumber(L, a + 1);
		lua_createtable(L, 0, 4);

		lua_pushstring(L, p->Get_IP());
		lua_setfield(L, -2, "IP");
		lua_pushstring(L, p->Get_Name());
		lua_setfield(L, -2, "Name");
		lua_pushstring(L, p->Get_Reason());
		lua_setfield(L, -2, "Reason");
		lua_pushstring(L, p->Get_Serial());
		lua_setfield(L, -2, "Serial");

		lua_settable(L, -3);
	}

	return 1;
}

/*!\details Returns a list of ban exceptions from DA Ban Exception List
*  \par Lua Command
*  \code{.lua}
*			banlist = DA_Get_Ban_Exception_List()
*  \endcode
*  \par		Example
*  \code{.lua}
*			return #DA_Get_Ban_Exception_List()  -- returns: 0 or the number of exception in list
*  \endcode
*  \return list Table
*/
int LuaDA_Get_Ban_Exception_List(lua_State* L)
{
	DABanListClass* banlist = DABanManager::Get_Ban_Exception_List();
	lua_gettop(L);
	lua_createtable(L, banlist->Get_Count(), 0);

	for (int a = 0; a < banlist->Get_Count(); a++)
	{
		DABanEntryClass* p = banlist->Peek_Entry(a);
		lua_pushnumber(L, a + 1);
		lua_createtable(L, 0, 4);

		lua_pushstring(L, p->Get_IP());
		lua_setfield(L, -2, "IP");
		lua_pushstring(L, p->Get_Name());
		lua_setfield(L, -2, "Name");
		lua_pushstring(L, p->Get_Reason());
		lua_setfield(L, -2, "Reason");
		lua_pushstring(L, p->Get_Serial());
		lua_setfield(L, -2, "Serial");

		lua_settable(L, -3);
	}

	return 1;
}

/*!\details Returns a list of ForceTT players
*  \par Lua Command
*  \code{.lua}
*			banlist = DA_Get_ForceTT_List()
*  \endcode
*  \par		Example
*  \code{.lua}
*			return #DA_Get_ForceTT_List()  -- returns: 0 or the number of forcett players
*  \endcode
*  \return list Table
*/
int LuaDA_Get_ForceTT_List(lua_State* L)
{
	DABanListClass* banlist = DABanManager::Get_ForceTT_List();
	lua_gettop(L);
	lua_createtable(L, banlist->Get_Count(), 0);

	for (int a = 0; a < banlist->Get_Count(); a++)
	{
		DABanEntryClass* p = banlist->Peek_Entry(a);
		lua_pushnumber(L, a + 1);
		lua_createtable(L, 0, 4);

		lua_pushstring(L, p->Get_IP());
		lua_setfield(L, -2, "IP");
		lua_pushstring(L, p->Get_Name());
		lua_setfield(L, -2, "Name");
		lua_pushstring(L, p->Get_Reason());
		lua_setfield(L, -2, "Reason");
		lua_pushstring(L, p->Get_Serial());
		lua_setfield(L, -2, "Serial");

		lua_settable(L, -3);
	}

	return 1;
}

/*!\details Returns a list of ForceTT exception players
*  \par Lua Command
*  \code{.lua}
*			banlist = DA_Get_ForceTT_Exception_List()
*  \endcode
*  \par		Example
*  \code{.lua}
*			return #DA_Get_ForceTT_Exception_List()  -- returns: 0 or the number of forcett exception players
*  \endcode
*  \return list Table
*/
int LuaDA_Get_ForceTT_Exception_List(lua_State* L)
{
	DABanListClass* banlist = DABanManager::Get_ForceTT_Exception_List();
	lua_gettop(L);
	lua_createtable(L, banlist->Get_Count(), 0);

	for (int a = 0; a < banlist->Get_Count(); a++)
	{
		DABanEntryClass* p = banlist->Peek_Entry(a);
		lua_pushnumber(L, a + 1);
		lua_createtable(L, 0, 4);

		lua_pushstring(L, p->Get_IP());
		lua_setfield(L, -2, "IP");
		lua_pushstring(L, p->Get_Name());
		lua_setfield(L, -2, "Name");
		lua_pushstring(L, p->Get_Reason());
		lua_setfield(L, -2, "Reason");
		lua_pushstring(L, p->Get_Serial());
		lua_setfield(L, -2, "Serial");

		lua_settable(L, -3);
	}

	return 1;
}

/*!\details Returns a list of ForceTT exception players
*  \par Lua Command
*  \code{.lua}
*			DA_Register_Chat_Command(Callback, ChatTrigger, ParamsTotal, AccessLevel, ChatType)
*  \endcode
*  \par		Example
*  \code{.lua}
*			function MyChat() Console_Input("msg Hello World!"); end
*			DA_Register_Chat_Command("MyChat", "!hey", 0)  -- registers the !hey command, ingame you can do !hey to trigger it
*  \endcode
*  \param Callback String (Function name)
*  \param ChatTrigger String (Command name)
*  \param ParamsTotal Number
*  \param AccessLevel Number
*  \param ChatType Number
*/
int LuaDA_Register_Chat_Command(lua_State* L)
{
	if (lua_gettop(L) < 2)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	const char* callback = lua_tostring(L, 1);
	std::string std_callback = std::string(callback);

	for (auto it = LuaTT_DAChatList.begin(); it != LuaTT_DAChatList.end(); ++it)
	{
		LuaTT_DAChatCommand* obj = (*it);
		if (obj != nullptr
			&& std_callback == obj->getFunc())
		{
			lua_pushboolean(L, false);
			return 1; // We should throw error because user did call this again..
		}
	}

	// Callback, Triggers, Params, AccessLevel, ChatType, 

	const char* Triggers = lua_tostring(L, 2);

	int Parameters = 0;
	if (lua_gettop(L) >= 3)
	{
		Parameters = lua_tonumber(L, 3);
	}

	DAAccessLevel::Level AccessLevel = DAAccessLevel::NONE;
	if (lua_gettop(L) >= 4)
	{
		AccessLevel = static_cast<DAAccessLevel::Level>((int)lua_tonumber(L, 4));
	}

	DAChatType::Type ChatType = DAChatType::ALL;
	if (lua_gettop(L) >= 5)
	{
		ChatType = static_cast<DAChatType::Type>((int)lua_tonumber(L, 5));
	}

	LuaTT_DAChatCommand* instance = new LuaTT_DAChatCommand; // if we are not carefull, this can cause memleak
	instance->setupLua(std_callback, std::string(Triggers));
	LuaTT_DAChatList.push_back(instance);

	DAChatCommandManager::Register_Chat_Command(instance, Triggers, Parameters, AccessLevel, ChatType);
	lua_pushboolean(L, true);
	return 1;
}


int LuaDA_Unregister_Chat_Command(lua_State* L)
{
	if (lua_gettop(L) < 1)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	const char* callback = lua_tostring(L, 1);
	std::string std_callback = std::string(callback);

	for (auto it = LuaTT_DAChatList.begin(); it != LuaTT_DAChatList.end(); ++it)
	{
		LuaTT_DAChatCommand* obj = (*it);
		if (std_callback == obj->getFunc())
		{
			LuaTT_DAChatList.erase(it);
			DAChatCommandManager::Unregister_Chat_Command(obj->getCommand().c_str());
			delete obj;
			obj = nullptr;

			lua_pushboolean(L, true);
			return 1;
		}
	}

	lua_pushboolean(L, false);
	return 1;
}


/*!\details Unstucks an object
*  \par Lua Command
*  \code{.lua}
*			DA_Fix_Stuck_Object(Object, Range)
*  \endcode
*  \par		Example
*  \code{.lua}
*			DA_Fix_Stuck_Object(Get_GameObj(1), 30) -- Player 1 will be unstuck if were stuck
*  \endcode
*  \param Object Number
*  \param Range Number (Float)
*/
int LuaDA_Fix_Stuck_Object(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr
		&& obj->As_PhysicalGameObj())
	{
		Fix_Stuck_Object(obj->As_PhysicalGameObj(), lua_tonumber(L, 2));
	}

	return 1;
}

int LuaDA_Is_Stealth_Unit(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr)
	{
		lua_pushboolean(L, Is_Stealth_Unit(obj));
	}

	return 1;
}

/*int LuaDA_Is_Stealth_Enabled2(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj != nullptr)
	{
		lua_pushboolean(L, Is_Stealth_Enabled2(obj));
	}

	return 1;
}*/

int LuaDA_Get_Ground_Vehicle_Count(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	int team = lua_tonumber(L, 1);
	lua_pushnumber(L, Get_Ground_Vehicle_Count(team));
	return 1;
}

int LuaDA_Get_Air_Vehicle_Count(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	int team = lua_tonumber(L, 1);
	lua_pushnumber(L, Get_Air_Vehicle_Count(team));
	return 1;
}

int LuaDA_Get_Naval_Vehicle_Count(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	int team = lua_tonumber(L, 1);
	lua_pushnumber(L, Get_Naval_Vehicle_Count(team));
	return 1;
}

int LuaDA_Setup_Send_Message_Fake(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* nick = lua_tostring(L, 1);
	int id = (lua_gettop(L) >= 2) ? (int)lua_tonumber(L, 2) : 0;
	lua_pushnumber(L, Setup_Send_Message_Fake(nick, id));
	return 1;
}

int LuaDA_Setup_Send_Message_Team_Fake(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const char* nick = lua_tostring(L, 1);
	int team = lua_tonumber(L, 2);
	int id = (lua_gettop(L) >= 3) ? (int)lua_tonumber(L, 3) : 0;
	lua_pushnumber(L, Setup_Send_Message_Team_Fake(nick, team, id));
	return 1;
}

int LuaDA_Restore_Send_Message_Fake(lua_State* L)
{
	Restore_Send_Message_Fake();
	return 0;
}

int LuaDA_Send_Announcement(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	Send_Announcement(lua_tostring(L, 1));
	return 0;
}

int LuaDA_Send_Announcement_Version_Less_Than(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Send_Announcement_Version_Less_Than(lua_tostring(L, 1), lua_tonumber(L, 2));
	return 0;
}

int LuaDA_Send_Announcement_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Send_Announcement_Player(lua_tonumber(L, 1), lua_tostring(L, 2));
	return 0;
}

int LuaDA_Send_Announcement_Player_Version_Less_Than(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	Send_Announcement_Player_Version_Less_Than(lua_tonumber(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}

int LuaDA_Send_Announcement_Team(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Send_Announcement_Team(lua_tonumber(L, 1), lua_tostring(L, 2));
	return 0;
}

int LuaDA_Send_Announcement_Team_Version_Less_Than(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	Send_Announcement_Team_Version_Less_Than(lua_tonumber(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3));
	return 0;
}

int LuaDA_Send_Message_Team_With_Team_Color(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Send_Message_Team_With_Team_Color(lua_tonumber(L, 1), lua_tostring(L, 2));
	return 0;
}

int LuaDA_Send_Message_Player_By_ID(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	Send_Message_Player_By_ID(lua_tonumber(L, 1), (unsigned int)lua_tonumber(L, 2), (unsigned int)lua_tonumber(L, 3), (unsigned int)lua_tonumber(L, 4), lua_tostring(L, 5));
	return 0;
}

int LuaDA_Create_2D_WAV_Sound_Player_By_ID(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Create_2D_WAV_Sound_Player_By_ID(lua_tonumber(L, 1), lua_tostring(L, 2));
	return 0;
}


int LuaDA_Get_Distance_To_Closest_Building(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 1);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	int team = lua_tonumber(L, 2);
	float dist = Get_Distance_To_Closest_Building(pos, team);
	lua_pushnumber(L, dist);
	return 1;
}

int LuaDA_SettingsManager_Get_Bool(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	if (lua_gettop(L) >= 3)
	{
		lua_pushboolean(L, DASettingsManager::Get_Bool(lua_tostring(L, 1), lua_tostring(L, 2), lua_toboolean(L, 3)));
	}
	else
	{
		lua_pushboolean(L, DASettingsManager::Get_Bool(lua_tostring(L, 1), lua_toboolean(L, 2)));
	}

	return 1;
}

int LuaDA_SettingsManager_Get_Int(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	if (lua_gettop(L) >= 3)
	{
		lua_pushnumber(L, DASettingsManager::Get_Int(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3)));
	}
	else
	{
		lua_pushnumber(L, DASettingsManager::Get_Int(lua_tostring(L, 1), lua_tonumber(L, 2)));
	}
	return 1;
}

int LuaDA_SettingsManager_Get_Float(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	if (lua_gettop(L) >= 3)
	{
		lua_pushnumber(L, DASettingsManager::Get_Float(lua_tostring(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3)));
	}
	else
	{
		lua_pushnumber(L, DASettingsManager::Get_Float(lua_tostring(L, 1), lua_tonumber(L, 2)));
	}
	return 1;
}

int LuaDA_SettingsManager_Get_String(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	StringClass buff;
	if (lua_gettop(L) >= 3)
	{
		DASettingsManager::Get_String(buff, lua_tostring(L, 1), lua_tostring(L, 2), lua_tostring(L, 3));
	}
	else
	{
		DASettingsManager::Get_String(buff, lua_tostring(L, 1), lua_tostring(L, 2));
	}

	lua_pushstring(L, buff.Peek_Buffer());
	return 1;
}

int LuaDA_SettingsManager_Get_Vector3(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	Vector3 res;
	if (lua_gettop(L) >= 3)
	{
		LUA_Vector3* a = LuaVector3::GetInstance(L, 3);
		Vector3 defaultOpt = Vector3(a->X(), a->Y(), a->Z());
		DASettingsManager::Get_Vector3(res, lua_tostring(L, 1), lua_tostring(L, 2), defaultOpt);
	}
	else
	{
		LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
		Vector3 defaultOpt = Vector3(a->X(), a->Y(), a->Z());
		DASettingsManager::Get_Vector3(res, lua_tostring(L, 1), defaultOpt);
	}

	LUA_Vector3* lvec = new LUA_Vector3(res.X, res.Y, res.Z);
	lua_boxpointer(L, lvec);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);

	return 1;
}


int LuaDA_SettingsManager_Get_Section(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	INISection* Section = DASettingsManager::Get_Section(lua_tostring(L, 1));
	if (Section)
	{
		lua_newtable(L);
		for (INIEntry* i = Section->EntryList.First(); i && i->Is_Valid(); i = i->Next())
		{
			lua_pushstring(L, i->Value);
			lua_setfield(L, -2, i->Entry);
		}
		return 1;
	}
	else
	{
		return 0;
	}

}

int LuaDA_SettingsManager_Add_Settings(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = lua_tostring(L, 1);
	DASettingsManager::Add_Settings(str);
	return 1;
}

int LuaDA_Log_Gamelog(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = lua_tostring(L, 1);
	SSGMGameLog::Log_Gamelog(str);
	return 1;
}

int LuaDA_Write_GameLog(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = lua_tostring(L, 1);
	DALogManager::Write_GameLog(str);
	return 1;
}

int LuaDA_Write_Log(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const char* header = lua_tostring(L, 1);
	const char* str = lua_tostring(L, 2);
	DALogManager::Write_Log(header, str);
	return 1;
}

int LuaDA_Host_Message(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = lua_tostring(L, 1);
	DA::Host_Message(str);
	return 1;
}

#if RENEGADE_VERSION == 1038
REF_DEF2(float, TimeScale, 0, 0x008242B8);
#else
REF_DEF2(float, TimeScale, 0x00811E60, 0x00811038);
#endif

int LuaDA_TimeScale(lua_State* L)
{
	if (lua_gettop(L) < 1)
	{
		lua_pushnumber(L, TimeScale);
	}
	else
	{
		char str[50];
		sprintf(str, "timescale %f", static_cast<float>(lua_tonumber(L, 1)));
		Console_Input(str);
	}
	return 1;
}

int LuaDA_Owns_Vehicle(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
		if (obj && obj->As_SoldierGameObj())
		{
			DAVehicleOwnershipObserverClass* Observer = DAVehicleOwnershipGameFeature->Get_Vehicle_Data(obj->As_SoldierGameObj());
			if (Observer)
			{
				VehicleGameObj* Vehicle = Observer->Get_Vehicle();
				if (Vehicle)
				{
					lua_pushnumber(L, Commands->Get_ID(Vehicle));
					return 1;
				}
			}
		}
	}
	return 0;
}

int LuaDA_Exit_Vehicle(lua_State* L)
{
	if (lua_gettop(L) < 1)
	{
		lua_pushboolean(L, false);
		return 1;
	}
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj && obj->As_SoldierGameObj())
	{
		lua_pushboolean(L, Exit_Vehicle(obj->As_SoldierGameObj()));
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}

int LuaDA_Reverse_Damage(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj)
	{
		Reverse_Damage(obj, static_cast<float>(lua_tonumber(L, 2)));
	}
	return 1;
}

int LuaDA_Set_Emot_Icon(lua_State* L)
{
	if (lua_gettop(L) < 3) return 0;
	int ID = static_cast<int>(lua_tonumber(L, 1));
	const char* Model = lua_tostring(L, 2);
	int Team = static_cast<int>(lua_tonumber(L, 3));
	if (Model)
	{
		Set_Emote_Icon(ID, Model, Team);
	}
	return 1;
}

int LuaDA_Color_Message_With_Team_Color(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const int i = luaL_checknumber(L, 1);
	const char* str = luaL_checkstring(L, 2);
	DA::Color_Message_With_Team_Color(i, str);
	return 1;
}

int LuaDA_C4_Set_Player_Data(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
		if (obj && obj->As_PhysicalGameObj()->As_C4GameObj())
		{
			C4GameObj* C4 = (C4GameObj*)obj;
			if (C4)
			{
				cPlayer* P = Find_Player(luaL_checknumber(L, 2));
				C4->Set_Player_Data(P);
				return 1;
			}
		}
	}
	return 0;
}

int LuaDA_C4_Set_Owner(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
		if (obj && obj->As_PhysicalGameObj()->As_C4GameObj())
		{
			C4GameObj* C4 = (C4GameObj*)obj;
			if (C4)
			{
				GameObject* obj2 = Commands->Find_Object(lua_tonumber(L, 2));
				if (obj2 && Commands->Is_A_Star(obj2))
				{
					C4->Set_Owner(obj2);
					return 1;
				}
			}
		}
	}
	return 0;
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS

bool LuaTT_DAChatCommand::Activate(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType)
{
	bool ret = true;
	const char* func = this->func.c_str();
	for (auto L : LuaManager::Lua)
	{
		lua_getglobal(L, func);
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Player->PlayerId);
			lua_pushstring(L, Text[0].Peek_Buffer());
			lua_pushnumber(L, ChatType);
			LuaManager::Report_Errors(L, lua_pcall(L, 3, 1, 0));
			ret = !ret ? ret : (bool)lua_toboolean(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, func);
		}
	}

	return ret;
}


int LuaDA_Page_Player(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const int i = luaL_checknumber(L, 1);
	const char* str = luaL_checkstring(L, 2);
	DA::Page_Player(i, str);
	return 1;
}

int LuaDA_Register_Null_Game_Mode(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const char* shortname = luaL_checkstring(L, 1);
	const char* longname = luaL_checkstring(L, 2);
	DANullGameModeRegistrant* obj = new DANullGameModeRegistrant(_strdup(shortname), _strdup(longname));
	Lua_NullGameModes.push_back(obj);
	return 0;
}

int LuaDA_CreateC4(lua_State* L)
{
	/*
		Code based on CreateC4 C# code from https://github.com/Neijwiert/RenSharp
		Ported and Adapted by sla.ro(master) to C++ and Lua programming languages
	*/

	if (lua_gettop(L) < 5) return 0;
	/* Lua Part */
	DefinitionClass* def = Find_Named_Definition(luaL_checkstring(L, 1));
	if (def == nullptr)
	{
		return 0;
	}
	int playerId = luaL_checknumber(L, 4);
	LUA_Vector3* a = LuaVector3::GetInstance(L, 2);
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());

	LUA_Vector3* b = LuaVector3::GetInstance(L, 3);
	Vector3 velocity = Vector3(b->X(), b->Y(), b->Z());
	PlayerDataClass* playerData = Find_Player(playerId);

	int detonationMode = luaL_checknumber(L, 5);

	/* C4 Part */
	AmmoDefinitionClass* ammoDef = static_cast<AmmoDefinitionClass*>(def);
	Matrix3D transform = Matrix3D(pos);

	if (ammoDef == nullptr)
	{
		return 0;
	}
	else if (detonationMode < 1 || detonationMode > 3)
	{
		return 0;
	}

	int c4PresetId = ammoDef->C4Preset;
	if (c4PresetId <= 0)
	{
		c4PresetId = Get_Definition_ID("Tossed C4");
	}

	C4GameObjDef* c4ObjDefPtr = static_cast<C4GameObjDef*>(DefinitionMgrClass::Find_Definition(c4PresetId, false));
	if (c4ObjDefPtr == nullptr)
	{
		return 0;
	}

	BaseGameObj* baseObj = static_cast<BaseGameObj*>(c4ObjDefPtr->Create());
	if (baseObj == nullptr)
	{
		return 0;
	}

	PhysicalGameObj* physicalObj = baseObj->As_PhysicalGameObj();
	if (physicalObj == nullptr)
	{
		baseObj->Set_Delete_Pending();
		return 0;
	}

	C4GameObj* c4Obj = physicalObj->As_C4GameObj();
	if (c4Obj == nullptr)
	{
		baseObj->Set_Delete_Pending();
		return 0;
	}

	c4Obj->Set_Transform(transform);
	PhysClass* physObj = c4Obj->Peek_Physical_Object();

	if (!ammoDef->ModelName.Is_Empty())
	{
		Commands->Set_Model(c4Obj, ammoDef->ModelName.Peek_Buffer());
	}

	/* We access the memory location of c4 definition */
	C4GameObjHax* c4ObjHaxed = (C4GameObjHax*)c4Obj;

	AmmoDefinitionClass** c4AmmoDef = &c4ObjHaxed->AmmoDef;
	int* c4DetonationMode = &c4ObjHaxed->DetonationMode;
	PlayerDataClass** c4Player = &c4ObjHaxed->Player;
	float* c4TriggerTime = &c4ObjHaxed->TriggerTime;
	ReferencerClass* c4Owner = &c4ObjHaxed->Owner;
	bool* c4Attached = &c4ObjHaxed->attached;
	ReferencerClass* c4AttachObject = &c4ObjHaxed->attachObject;
	Vector3* c4AttachLocation = &c4ObjHaxed->AttachLocation;
	int* c4AttachBoneIndex = &c4ObjHaxed->AttachBoneIndex;
	bool* c4IsAttachedToMCT = &c4ObjHaxed->IsAttachedToMCT;
	bool* c4AttachedToDynamic = &c4ObjHaxed->attachedToDynamic;

	*c4AmmoDef = ammoDef;
	*c4DetonationMode = detonationMode;
	*c4Player = playerData;

	if (ammoDef->AmmoType != AmmoDefinitionClass::AMMO_TYPE_C4_REMOTE)
	{
		switch (detonationMode)
		{
		case 2:
			*c4TriggerTime = ammoDef->C4TriggerTime2;
			break;
		case 3:
			*c4TriggerTime = ammoDef->C4TriggerTime3;
			break;
		default:
			*c4TriggerTime = ammoDef->C4TriggerTime1;
			break;
		}
	}

	*c4Attached = false;
	*c4AttachObject = nullptr;
	*c4AttachLocation = Vector3();
	*c4AttachBoneIndex = 0;
	*c4IsAttachedToMCT = false;
	*c4AttachedToDynamic = false;

	PhysClass* physC4Obj = c4Obj->Peek_Physical_Object();
	physC4Obj->Set_Collision_Group(Collision_Group_Type::C4_COLLISION_GROUP);

	if (c4Player != nullptr)
	{
		c4Obj->Set_Player_Type(Get_Team(playerId));
		c4Owner = reinterpret_cast<ReferencerClass*>(Get_GameObj(playerId));
	}

	if (ammoDef->C4TimingSound1ID != 0)
	{
		const char* c4TimingSound1Name = Get_Definition_Name(ammoDef->C4TimingSound1ID);
		if (c4TimingSound1Name != nullptr)
		{
			Commands->Create_Sound(c4TimingSound1Name, transform.Get_Translation(), Get_GameObj(playerId));
		}
	}

	if (physC4Obj->As_ProjectileClass() != nullptr)
	{
		physC4Obj->As_MoveablePhysClass()->Set_Velocity(velocity);
	}

	c4Obj->Set_Object_Dirty_Bit(NetworkObjectClass::BIT_RARE, true);
	c4Obj->Post_Re_Init();
	lua_pushnumber(L, Commands->Get_ID(c4Obj));
	return 1;
}

int LuaDA_Team_Host_Message(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const char* str = luaL_checkstring(L, 2);
	int team = luaL_checknumber(L, 1);
	DA::Team_Host_Message(team,str);
	return 1;
}

int LuaDA_Private_Host_Message(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		const char* str = luaL_checkstring(L, 2);
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			DA::Private_Host_Message(P, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Admin_Message(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = luaL_checkstring(L, 1);
	DA::Admin_Message(str);
	return 1;
}

int LuaDA_Team_Admin_Message(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	const char* str = luaL_checkstring(L, 2);
	int team = luaL_checknumber(L, 1);
	DA::Team_Admin_Message(team, str);
	return 1;
}

int LuaDA_Private_Admin_Message(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		const char* str = luaL_checkstring(L, 2);
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			DA::Private_Admin_Message(P, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Player_Message(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		const char* str = luaL_checkstring(L, 2);
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			DA::Player_Message(P, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Team_Player_Message(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		const char* str = luaL_checkstring(L, 2);
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			DA::Team_Player_Message(P, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Page_Team(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int team = luaL_checknumber(L, 1);
	const char* str = luaL_checkstring(L, 2);
	DA::Page_Team(team, str);
	return 1;
}


int LuaDA_Page_Team_Except(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		int team = luaL_checknumber(L, 1);
		cPlayer* P = Find_Player(luaL_checknumber(L, 2));
		const char* str = luaL_checkstring(L, 3);
		if (P)
		{
			DA::Page_Team_Except(team, P, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Color_Message(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;
	int r = luaL_checknumber(L, 1);
	int g = luaL_checknumber(L, 2);
	int b = luaL_checknumber(L, 3);
	const char* str = luaL_checkstring(L, 4);
	DA::Color_Message(r, g, b, str);
	return 1;
}

int LuaDA_Team_Color_Message(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	int team = luaL_checknumber(L, 1);
	int r = luaL_checknumber(L, 2);
	int g = luaL_checknumber(L, 3);
	int b = luaL_checknumber(L, 4);
	const char* str = luaL_checkstring(L, 5);
	DA::Team_Color_Message(team, r, g, b, str);
	return 1;
}

int LuaDA_Team_Color_Message_With_Team_Color(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int team = luaL_checknumber(L, 1);
	const char* str = luaL_checkstring(L, 2);
	DA::Team_Color_Message_With_Team_Color(team, str);
	return 1;
}

int LuaDA_Private_Color_Message(lua_State* L)
{
	if (lua_gettop(L) == 5)
	{
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			int r = luaL_checknumber(L, 2);
			int g = luaL_checknumber(L, 3);
			int b = luaL_checknumber(L, 4);
			const char* str = luaL_checkstring(L, 5);
			DA::Private_Color_Message(P, r, g, b, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Private_Color_Message_With_Team_Color(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			int team = luaL_checknumber(L, 2);
			const char* str = luaL_checkstring(L, 3);
			DA::Private_Color_Message_With_Team_Color(P, team, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Create_2D_Sound(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = luaL_checkstring(L, 1);
	DA::Create_2D_Sound(str);
	return 1;
}

int LuaDA_Create_2D_Sound_Team(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int team = luaL_checknumber(L, 1);
	const char* str = luaL_checkstring(L, 2);
	DA::Create_2D_Sound_Team(team, str);
	return 1;
}

int LuaDA_Create_2D_Sound_Player(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			const char* str = luaL_checkstring(L, 2);
			DA::Create_2D_Sound_Player(P, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_HUD_Message(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;
	int r = luaL_checknumber(L, 1);
	int g = luaL_checknumber(L, 2);
	int b = luaL_checknumber(L, 3);
	const char* str = luaL_checkstring(L, 4);
	DA::HUD_Message(r, g, b, str);
	return 1;
}

int LuaDA_Team_HUD_Message(lua_State* L)
{
	if (lua_gettop(L) < 5) return 0;
	int team = luaL_checknumber(L, 1);
	int r = luaL_checknumber(L, 2);
	int g = luaL_checknumber(L, 3);
	int b = luaL_checknumber(L, 4);
	const char* str = luaL_checkstring(L, 5);
	DA::Team_HUD_Message(team, r, g, b, str);
	return 1;
}


int LuaDA_Private_HUD_Message(lua_State* L)
{
	if (lua_gettop(L) == 5)
	{
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			int r = luaL_checknumber(L, 2);
			int g = luaL_checknumber(L, 3);
			int b = luaL_checknumber(L, 4);
			const char* str = luaL_checkstring(L, 5);
			DA::Private_HUD_Message(P, r, g, b, str);
			return 1;
		}
	}
	return 0;
}

int LuaDA_Get_Version(lua_State* L)
{
	lua_pushstring(L, DA::Get_Version());
	return 1;
}


int LuaDA_Set_Emote_Icon(lua_State* L)
{
	if (lua_gettop(L) == 3)
	{
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			const char* str = luaL_checkstring(L, 2);
			int team = luaL_checknumber(L, 3);
			Set_Emote_Icon(P->Get_Id(),str,team);
			return 1;
		}
	}
	return 0;
}



int LuaDA_Crate_Can_Activate(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L,1));
		if (Crate)
		{
			cPlayer* P = Find_Player(luaL_checknumber(L, 2));
			if (P)
			{
				lua_pushboolean(L, Crate->Can_Activate(P));
				return 1;
			}
		}
	}
	return 0;
}



int LuaDA_Crate_Check_Type(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L, 1));
		if (Crate)
		{
			cPlayer* P = Find_Player(luaL_checknumber(L, 2));
			if (P)
			{
				lua_pushboolean(L, Crate->Check_Type(P));
				return 1;
			}
		}
	}
	return 0;
}


int LuaDA_Crate_Activate(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L, 1));
		if (Crate)
		{
			cPlayer* P = Find_Player(luaL_checknumber(L, 2));
			if (P)
			{
				if (!Crate->Check_Type(P) || !Crate->Can_Activate(P))
				{
					DA::Page_Player(P, "You cannot currently receive that crate.");
					return 0;
				}
				Crate->Activate(P);
				return 1;
			}
		}
	}
	return 0;
}



int LuaDA_Crate_Adjust_Odds(lua_State* L)
{
	if (lua_gettop(L) == 2)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L, 1));
		if (Crate)
		{
			float scale = luaL_checknumber(L, 2);
			Crate->Adjust_Odds(scale);
			return 1;
		}
	}
	return 0;
}


int LuaDA_Crate_Get_Base_Odds(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L, 1));
		if (Crate)
		{
			lua_pushnumber(L, Crate->Get_Base_Odds());
			return 1;
		}
	}
	return 0;
}


int LuaDA_Crate_Get_Cap_Odds(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L, 1));
		if (Crate)
		{
			lua_pushboolean(L, Crate->Get_Cap_Odds());
			return 1;
		}
	}
	return 0;
}

int LuaDA_Crate_Get_Final_Odds(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L, 1));
		if (Crate)
		{
			lua_pushnumber(L, Crate->Get_Final_Odds());
			return 1;
		}
	}
	return 0;
}


int LuaDA_Crate_Get_Modified_Odds(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		DACrateClass* Crate = DACrateManager::Get_Crate(luaL_checkstring(L, 1));
		if (Crate)
		{
			lua_pushnumber(L, Crate->Get_Modified_Odds());
			return 1;
		}
	}
	return 0;
}

int LuaDA_Get_Game_Mode_Long_Name(lua_State* L)
{
	lua_pushstring(L, DAGameManager::Get_Game_Mode_Long_Name());
	return 1;
}

int LuaDA_Get_Game_Mode_Short_Name(lua_State* L)
{
	lua_pushstring(L, DAGameManager::Get_Game_Mode_Short_Name());
	return 1;
}

int LuaDA_Get_Access_Level(lua_State* L)
{
	if (lua_gettop(L) == 1)
	{
		cPlayer* P = Find_Player(luaL_checknumber(L, 1));
		if (P)
		{
			lua_pushnumber(L, P->Get_DA_Player()->Get_Access_Level());
			return 1;
		}
	}
	return 0;
}

int LuaDA_Is_Game_Mode(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* name = lua_tostring(L, 1);
	if (!name) return 0;
	lua_pushboolean(L, DAGameManager::Is_Game_Mode(name));
	return 1;
}

int LuaDA_Is_Shutdown_Pending(lua_State* L)
{
	lua_pushboolean(L, DAGameManager::Is_Shutdown_Pending());
	return 1;
}

int LuaDA_Send(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* data = lua_tostring(L, 1);
	if (!data) return 0;
	DALogManager::Send(data);
	return 0;
}

int LuaDA_Get_Disable_Kill_Messages(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Kill_Messages());
	return 1;
}

int LuaDA_Get_Disable_Kill_Counter(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Kill_Counter());
	return 1;
}

int LuaDA_Get_Disable_Team_Kill_Counter(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Team_Kill_Counter());
	return 1;
}

int LuaDA_Get_Disable_Death_Counter(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Death_Counter());
	return 1;
}

int LuaDA_Get_Disable_Team_Death_Counter(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Team_Death_Counter());
	return 1;
}

int LuaDA_Get_Disable_Damage_Points(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Damage_Points());
	return 1;
}

int LuaDA_Get_Disable_Death_Points(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Death_Points());
	return 1;
}

int LuaDA_Get_Disable_Team_Score_Counter(lua_State* L)
{
	lua_pushboolean(L, DAPlayerManager::Get_Disable_Team_Score_Counter());
	return 1;
}

int LuaDA_Get_Credits_Multiplier(lua_State* L)
{
	lua_pushnumber(L, DAPlayerManager::Get_Credits_Multiplier());
	return 1;
}

int LuaDA_Set_Disable_Kill_Messages(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Kill_Messages(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Disable_Kill_Counter(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Kill_Counter(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Disable_Team_Kill_Counter(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Team_Kill_Counter(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Disable_Death_Counter(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Death_Counter(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Disable_Team_Death_Counter(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Team_Death_Counter(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Disable_Damage_Points(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Damage_Points(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Disable_Death_Points(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Death_Points(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Disable_Team_Score_Counter(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Disable_Team_Score_Counter(lua_toboolean(L, 1));
	return 0;
}

int LuaDA_Set_Credits_Multiplier(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DAPlayerManager::Set_Credits_Multiplier((float)lua_tonumber(L, 1));
	return 0;
}

int LuaDA_Set_Force_Team(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	DATeamManager::Set_Force_Team((int)lua_tointeger(L, 1));
	return 0;
}

int LuaDA_Get_Force_Team(lua_State* L)
{
	lua_pushnumber(L, DATeamManager::Get_Force_Team());
	return 1;
}

int LuaDA_Is_Free_Team_Changing_Enabled(lua_State* L)
{
	lua_pushboolean(L, DATeamManager::Is_Free_Team_Changing_Enabled());
	return 1;
}

int LuaDA_Team_Remix(lua_State* L)
{
	DATeamManager::Remix();
	return 0;
}

int LuaDA_Team_Rebalance(lua_State* L)
{
	DATeamManager::Rebalance();
	return 0;
}

int LuaDA_Team_Swap(lua_State* L)
{
	DATeamManager::Swap();
	return 0;
}

int LuaDA_Get_Percent_Team_Damage(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	lua_pushnumber(L, DADamageLog::Get_Percent_Team_Damage(obj, (int)lua_tointeger(L, 2)));
	return 1;
}

int LuaDA_Get_Percent_Other_Team_Damage(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	lua_pushnumber(L, DADamageLog::Get_Percent_Other_Team_Damage(obj, (int)lua_tointeger(L, 2)));
	return 1;
}

int LuaDA_Air_Drop_Vehicle(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;
	int team = (int)lua_tointeger(L, 1);
	const char* vehicle = lua_tostring(L, 2);
	if (vehicle == nullptr) return 0;
	LUA_Vector3* a = LuaVector3::GetInstance(L, 3);
	if (a == nullptr) return 0;
	Vector3 pos = Vector3(a->X(), a->Y(), a->Z());
	float facing = (float)lua_tonumber(L, 4);
	VehicleGameObj* result = DAVehicleManager::Air_Drop_Vehicle(team, vehicle, pos, facing);
	if (result == nullptr) return 0;
	lua_pushnumber(L, Commands->Get_ID(result));
	return 1;
}

int LuaDA_Translate_Soldier(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	StringClass result = DATranslationManager::Translate_Soldier(obj);
	lua_pushstring(L, result.Peek_Buffer());
	return 1;
}

int LuaDA_Translate_Weapon(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	const char* result = DATranslationManager::Translate_Weapon(obj);
	if (result == nullptr) return 0;
	lua_pushstring(L, result);
	return 1;
}

int LuaDA_Translate_With_Team_Name(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	GameObject* obj = Commands->Find_Object(lua_tonumber(L, 1));
	if (obj == nullptr) return 0;
	StringClass result = DATranslationManager::Translate_With_Team_Name(obj);
	lua_pushstring(L, result.Peek_Buffer());
	return 1;
}

int LuaDA_Disable_Building_Type(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int team = lua_tonumber(L, 1);
	BuildingConstants::BuildingType type = static_cast<BuildingConstants::BuildingType>((int)lua_tonumber(L, 2));
	Disable_Building_Type(team, type);
	return 0;
}

int LuaDA_Get_Hash(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = lua_tostring(L, 1);
	if (!str) return 0;
	unsigned int hash = Get_Hash(str);
	lua_pushnumber(L, hash);
	return 1;
}

int LuaDA_Fix_Stuck_Objects(lua_State* L)
{
	if (lua_gettop(L) < 6) return 0;
	float x = lua_tonumber(L, 1);
	float y = lua_tonumber(L, 2);
	float z = lua_tonumber(L, 3);
	float checkrange = lua_tonumber(L, 4);
	float range = lua_tonumber(L, 5);
	bool destroy = lua_gettop(L) >= 6 ? lua_toboolean(L, 6) : true;
	Vector3 pos(x, y, z);
	Fix_Stuck_Objects(pos, checkrange, range, destroy);
	return 0;
}

int LuaDA_Clean_Model_Name(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	const char* str = lua_tostring(L, 1);
	if (!str) return 0;
	StringClass result = Clean_Model_Name(StringClass(str));
	lua_pushstring(L, result.Peek_Buffer());
	return 1;
}

int LuaDA_Get_Building_Count(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int team = lua_tonumber(L, 1);
	bool destroyed = lua_toboolean(L, 2);
	int count = Get_Building_Count(team, destroyed);
	lua_pushnumber(L, count);
	return 1;
}

int LuaDA_Enable_HUD(lua_State* L)
{
	if (lua_gettop(L) < 1) return 0;
	bool enable = lua_toboolean(L, 1);
	Enable_HUD(enable);
	return 0;
}

int LuaDA_Enable_HUD_Player_By_ID(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int id = lua_tonumber(L, 1);
	bool enable = lua_toboolean(L, 2);
	Enable_HUD_Player_By_ID(id, enable);
	return 0;
}

int LuaDA_Set_Fog_Enable_Player_By_ID(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	int id = lua_tonumber(L, 1);
	bool enable = lua_toboolean(L, 2);
	Set_Fog_Enable_Player_By_ID(id, enable);
	return 0;
}

int LuaDA_Set_Fog_Range_Player_By_ID(lua_State* L)
{
	if (lua_gettop(L) < 4) return 0;
	int id = lua_tonumber(L, 1);
	float startdistance = lua_tonumber(L, 2);
	float enddistance = lua_tonumber(L, 3);
	float transition = lua_tonumber(L, 4);
	Set_Fog_Range_Player_By_ID(id, startdistance, enddistance, transition);
	return 0;
}

void AddFunctionsDA(lua_State* L)
{
	lua_register(L, "DA_Get_Access_Level", LuaDA_Get_Access_Level);
	lua_register(L, "DA_Get_Game_Mode_Short_Name", LuaDA_Get_Game_Mode_Short_Name);
	lua_register(L, "DA_Get_Game_Mode_Long_Name", LuaDA_Get_Game_Mode_Long_Name);
	lua_register(L, "DA_Is_Game_Mode", LuaDA_Is_Game_Mode);
	lua_register(L, "DA_Is_Shutdown_Pending", LuaDA_Is_Shutdown_Pending);
	lua_register(L, "DA_Send", LuaDA_Send);

	// da_player.h
	lua_register(L, "DA_Get_Disable_Kill_Messages", LuaDA_Get_Disable_Kill_Messages);
	lua_register(L, "DA_Get_Disable_Kill_Counter", LuaDA_Get_Disable_Kill_Counter);
	lua_register(L, "DA_Get_Disable_Team_Kill_Counter", LuaDA_Get_Disable_Team_Kill_Counter);
	lua_register(L, "DA_Get_Disable_Death_Counter", LuaDA_Get_Disable_Death_Counter);
	lua_register(L, "DA_Get_Disable_Team_Death_Counter", LuaDA_Get_Disable_Team_Death_Counter);
	lua_register(L, "DA_Get_Disable_Damage_Points", LuaDA_Get_Disable_Damage_Points);
	lua_register(L, "DA_Get_Disable_Death_Points", LuaDA_Get_Disable_Death_Points);
	lua_register(L, "DA_Get_Disable_Team_Score_Counter", LuaDA_Get_Disable_Team_Score_Counter);
	lua_register(L, "DA_Get_Credits_Multiplier", LuaDA_Get_Credits_Multiplier);
	lua_register(L, "DA_Set_Disable_Kill_Messages", LuaDA_Set_Disable_Kill_Messages);
	lua_register(L, "DA_Set_Disable_Kill_Counter", LuaDA_Set_Disable_Kill_Counter);
	lua_register(L, "DA_Set_Disable_Team_Kill_Counter", LuaDA_Set_Disable_Team_Kill_Counter);
	lua_register(L, "DA_Set_Disable_Death_Counter", LuaDA_Set_Disable_Death_Counter);
	lua_register(L, "DA_Set_Disable_Team_Death_Counter", LuaDA_Set_Disable_Team_Death_Counter);
	lua_register(L, "DA_Set_Disable_Damage_Points", LuaDA_Set_Disable_Damage_Points);
	lua_register(L, "DA_Set_Disable_Death_Points", LuaDA_Set_Disable_Death_Points);
	lua_register(L, "DA_Set_Disable_Team_Score_Counter", LuaDA_Set_Disable_Team_Score_Counter);
	lua_register(L, "DA_Set_Credits_Multiplier", LuaDA_Set_Credits_Multiplier);

	// da_team.h
	lua_register(L, "DA_Set_Force_Team", LuaDA_Set_Force_Team);
	lua_register(L, "DA_Get_Force_Team", LuaDA_Get_Force_Team);
	lua_register(L, "DA_Is_Free_Team_Changing_Enabled", LuaDA_Is_Free_Team_Changing_Enabled);
	lua_register(L, "DA_Team_Remix", LuaDA_Team_Remix);
	lua_register(L, "DA_Team_Rebalance", LuaDA_Team_Rebalance);
	lua_register(L, "DA_Team_Swap", LuaDA_Team_Swap);

	// da_damagelog.h
	lua_register(L, "DA_Get_Percent_Team_Damage", LuaDA_Get_Percent_Team_Damage);
	lua_register(L, "DA_Get_Percent_Other_Team_Damage", LuaDA_Get_Percent_Other_Team_Damage);

	// da_vehicle.h
	lua_register(L, "DA_Air_Drop_Vehicle", LuaDA_Air_Drop_Vehicle);

	// da_translation.h
	lua_register(L, "DA_Translate_Soldier", LuaDA_Translate_Soldier);
	lua_register(L, "DA_Translate_Weapon", LuaDA_Translate_Weapon);
	lua_register(L, "DA_Translate_With_Team_Name", LuaDA_Translate_With_Team_Name);

	lua_register(L, "DA_CreateC4", LuaDA_CreateC4);

	// da_cratemanager.h
	lua_register(L, "DA_Crate_Get_Modified_Odds", LuaDA_Crate_Get_Modified_Odds);
	lua_register(L, "DA_Crate_Get_Final_Odds", LuaDA_Crate_Get_Final_Odds);
	lua_register(L, "DA_Crate_Get_Cap_Odds", LuaDA_Crate_Get_Cap_Odds);
	lua_register(L, "DA_Crate_Get_Base_Odds", LuaDA_Crate_Get_Base_Odds);
	lua_register(L, "DA_Crate_Adjust_Odds", LuaDA_Crate_Adjust_Odds);
	lua_register(L, "DA_Crate_Activate", LuaDA_Crate_Activate);
	lua_register(L, "DA_Crate_Check_Type", LuaDA_Crate_Check_Type);
	lua_register(L, "DA_Crate_Can_Activate", LuaDA_Crate_Can_Activate);

	// da.h
	lua_register(L, "DA_Get_Version", LuaDA_Get_Version);
	lua_register(L, "DA_Private_HUD_Message", LuaDA_Private_HUD_Message);
	lua_register(L, "DA_Team_HUD_Message", LuaDA_Team_HUD_Message);
	lua_register(L, "DA_HUD_Message", LuaDA_HUD_Message);
	lua_register(L, "DA_Create_2D_Sound_Player", LuaDA_Create_2D_Sound_Player);
	lua_register(L, "DA_Create_2D_Sound_Team", LuaDA_Create_2D_Sound_Team);
	lua_register(L, "DA_Create_2D_Sound", LuaDA_Create_2D_Sound);
	lua_register(L, "DA_Private_Color_Message_With_Team_Color", LuaDA_Private_Color_Message_With_Team_Color);
	lua_register(L, "DA_Private_Color_Message", LuaDA_Private_Color_Message);
	lua_register(L, "DA_Team_Color_Message_With_Team_Color", LuaDA_Team_Color_Message_With_Team_Color);
	lua_register(L, "DA_Team_Color_Message", LuaDA_Team_Color_Message);
	lua_register(L, "DA_Color_Message", LuaDA_Color_Message);
	lua_register(L, "DA_Page_Team_Except", LuaDA_Page_Team_Except);
	lua_register(L, "DA_Page_Team", LuaDA_Page_Team);
	lua_register(L, "DA_Team_Player_Message", LuaDA_Team_Player_Message);
	lua_register(L, "DA_Player_Message", LuaDA_Player_Message);
	lua_register(L, "DA_Private_Admin_Message", LuaDA_Private_Admin_Message);
	lua_register(L, "DA_Team_Admin_Message", LuaDA_Team_Admin_Message);
	lua_register(L, "DA_Admin_Message", LuaDA_Admin_Message);
	lua_register(L, "DA_Private_Host_Message", LuaDA_Private_Host_Message);
	lua_register(L, "DA_Team_Host_Message", LuaDA_Team_Host_Message);

	// da_settings.h
	lua_register(L, "DA_SettingsManager_Add_Settings", LuaDA_SettingsManager_Add_Settings);
	lua_register(L, "DA_SettingsManager_Get_Section", LuaDA_SettingsManager_Get_Section);
	lua_register(L, "DA_SettingsManager_Get_Vector3", LuaDA_SettingsManager_Get_Vector3);
	lua_register(L, "DA_SettingsManager_Get_String", LuaDA_SettingsManager_Get_String);
	lua_register(L, "DA_SettingsManager_Get_Float", LuaDA_SettingsManager_Get_Float);
	lua_register(L, "DA_SettingsManager_Get_Int", LuaDA_SettingsManager_Get_Int);
	lua_register(L, "DA_SettingsManager_Get_Bool", LuaDA_SettingsManager_Get_Bool);

	lua_register(L, "DA_Get_Distance_To_Closest_Building", LuaDA_Get_Distance_To_Closest_Building);
	lua_register(L, "DA_Log_Gamelog", LuaDA_Log_Gamelog);
	lua_register(L, "DA_Write_GameLog", LuaDA_Write_GameLog);
	lua_register(L, "DA_Write_Log", LuaDA_Write_Log);
	lua_register(L, "DA_Host_Message", LuaDA_Host_Message);

	// da_gameobj.h
	//Set_Observer_Delete_Pending TODO
	lua_register(L, "DA_Set_GameObj_Invisible_No_Delete", LuaDA_Set_GameObj_Invisible_No_Delete);
	lua_register(L, "DA_Set_GameObj_Stock_Only", LuaDA_Set_GameObj_Stock_Only);
	lua_register(L, "DA_Set_GameObj_TT_Only", LuaDA_Set_GameObj_TT_Only);
	lua_register(L, "DA_Set_GameObj_Delete_Pending", LuaDA_Set_GameObj_Delete_Pending);
	lua_register(L, "DA_Set_GameObj_Invisible", LuaDA_Set_GameObj_Invisible);

	// da_ban.h
	lua_register(L, "DA_Get_Ban_List", LuaDA_Get_Ban_List);
	lua_register(L, "DA_Get_Ban_Exception_List", LuaDA_Get_Ban_Exception_List);
	lua_register(L, "DA_Get_ForceTT_List", LuaDA_Get_ForceTT_List);
	lua_register(L, "DA_Get_ForceTT_Exception_List", LuaDA_Get_ForceTT_Exception_List);

	// da_chatcommands.h
	lua_register(L, "DA_Register_Chat_Command", LuaDA_Register_Chat_Command);
	lua_register(L, "DA_Unregister_Chat_Command", LuaDA_Unregister_Chat_Command);

	// engine_da.h
	lua_register(L, "DA_Set_Emote_Icon", LuaDA_Set_Emote_Icon);
	lua_register(L, "DA_Fix_Stuck_Object", LuaDA_Fix_Stuck_Object);
	lua_register(L, "DA_Is_Stealth_Unit", LuaDA_Is_Stealth_Unit);
	//lua_register(L, "DA_Is_Stealth_Enabled2", LuaDA_Is_Stealth_Enabled2);
	lua_register(L, "DA_Get_Ground_Vehicle_Count", LuaDA_Get_Ground_Vehicle_Count);
	lua_register(L, "DA_Get_Air_Vehicle_Count", LuaDA_Get_Air_Vehicle_Count);
	lua_register(L, "DA_Get_Naval_Vehicle_Count", LuaDA_Get_Naval_Vehicle_Count);
	lua_register(L, "DA_Setup_Send_Message_Fake", LuaDA_Setup_Send_Message_Fake);
	lua_register(L, "DA_Setup_Send_Message_Team_Fake", LuaDA_Setup_Send_Message_Team_Fake);
	lua_register(L, "DA_Restore_Send_Message_Fake", LuaDA_Restore_Send_Message_Fake);
	lua_register(L, "DA_Send_Announcement", LuaDA_Send_Announcement);
	lua_register(L, "DA_Send_Announcement_Version_Less_Than", LuaDA_Send_Announcement_Version_Less_Than);
	lua_register(L, "DA_Send_Announcement_Player", LuaDA_Send_Announcement_Player);
	lua_register(L, "DA_Send_Announcement_Player_Version_Less_Than", LuaDA_Send_Announcement_Player_Version_Less_Than);
	lua_register(L, "DA_Send_Announcement_Team", LuaDA_Send_Announcement_Team);
	lua_register(L, "DA_Send_Announcement_Team_Version_Less_Than", LuaDA_Send_Announcement_Team_Version_Less_Than);
	lua_register(L, "DA_Send_Message_Team_With_Team_Color", LuaDA_Send_Message_Team_With_Team_Color);
	lua_register(L, "DA_Send_Message_Player_By_ID", LuaDA_Send_Message_Player_By_ID);
	lua_register(L, "DA_Create_2D_WAV_Sound_Player_By_ID", LuaDA_Create_2D_WAV_Sound_Player_By_ID);
	lua_register(L, "DA_TimeScale", LuaDA_TimeScale);

	lua_register(L, "DA_Register_Crate", LuaTT_DACrateClass::Lua_Register_Crate);
	lua_register(L, "DA_Register_Console_Function", LuaTT_DAConsoleFunction::Lua_Register_Console_Function);
	lua_register(L, "DA_Owns_Vehicle", LuaDA_Owns_Vehicle);
	lua_register(L, "DA_Exit_Vehicle", LuaDA_Exit_Vehicle);
	lua_register(L, "DA_Reverse_Damage", LuaDA_Reverse_Damage);
	lua_register(L, "DA_Set_Emot_Icon", LuaDA_Set_Emot_Icon);

	lua_register(L, "DA_Page_Player", LuaDA_Page_Player);

	lua_register(L, "DA_C4_Set_Owner", LuaDA_C4_Set_Owner);
	lua_register(L, "DA_C4_Set_Player_Data", LuaDA_C4_Set_Player_Data);
	lua_register(L, "DA_Color_Message_With_Team_Color", LuaDA_Color_Message_With_Team_Color);

	lua_register(L, "DA_Register_Null_Game_Mode", LuaDA_Register_Null_Game_Mode);
	lua_register(L, "DA_Disable_Building_Type", LuaDA_Disable_Building_Type);
	lua_register(L, "DA_Get_Hash", LuaDA_Get_Hash);
	lua_register(L, "DA_Fix_Stuck_Objects", LuaDA_Fix_Stuck_Objects);
	lua_register(L, "DA_Clean_Model_Name", LuaDA_Clean_Model_Name);
	lua_register(L, "DA_Get_Building_Count", LuaDA_Get_Building_Count);
	lua_register(L, "DA_Enable_HUD", LuaDA_Enable_HUD);
	lua_register(L, "DA_Enable_HUD_Player_By_ID", LuaDA_Enable_HUD_Player_By_ID);
	lua_register(L, "DA_Set_Fog_Enable_Player_By_ID", LuaDA_Set_Fog_Enable_Player_By_ID);
	lua_register(L, "DA_Set_Fog_Range_Player_By_ID", LuaDA_Set_Fog_Range_Player_By_ID);

	/*
	DA_API bool Is_Starting_Weapon(const SoldierGameObjDef *Soldier,const PowerUpGameObjDef *PowerUp);
	DA_API bool Is_Starting_Weapon(SoldierGameObj *Soldier,const PowerUpGameObjDef *PowerUp);
	DA_API StringClass Get_Weapon_PowerUp_Model(const WeaponDefinitionClass *Weapon);
	*/

	/*
	DAChatCommandManager
	DA_API static void Register_Event_Chat_Command(DAEventClass *Base,DAECC Func,const char *Triggers,int Parameters = 0,DAAccessLevel::Level AccessLevel = DAAccessLevel::NONE,DAChatType::Type ChatType = DAChatType::ALL);
	DA_API static void Register_Key_Hook(DAKeyHookClass *Base,const char *Triggers);
	DA_API static void Register_Event_Key_Hook(DAEventClass *Base,DAEKH Func,const char *Triggers);
	DA_API static void Unregister_Chat_Command(const char *Trigger);
	DA_API static void Unregister_Event_Chat_Command(DAEventClass *Base,const char *Trigger);
	DA_API static void Unregister_Key_Hook(const char *Trigger);
	DA_API static void Unregister_Event_Key_Hook(DAEventClass *Base,const char *Trigger);
	DA_API static void Clear_Event_Chat_Commands(DAEventClass *Base);
	DA_API static void Clear_Event_Key_Hooks(DAEventClass *Base);
	*/

	// da_damagelog.h
	/*
	DA_API static DADamageLogObserverClass *Get_Damage_Log(GameObject *obj);

	DA_API static const SList<DADamageEventStruct> *Get_Damage_Events(GameObject *obj);

	DA_API static float Compile_Damage_Table_Team(DADamageTableStruct *DamageTable,GameObject *obj,int Team);
	DA_API static float Compile_Damage_Table_Other_Team(DADamageTableStruct *DamageTable,GameObject *obj,int Team);
	DA_API static void Get_Damagers_By_Percent_Team(DynamicVectorClass<DADamageTableStruct> &Buffer,GameObject *obj,int Team,float MinimumPercentTotal,float MinimumPercentHighest);
	DA_API static void Get_Damagers_By_Percent_Other_Team(DynamicVectorClass<DADamageTableStruct> &Buffer,GameObject *obj,int Team,float MinimumPercentTotal,float MinimumPercentHighest);
	DA_API static cPlayer *Get_Highest_Damager_Team(GameObject *obj,int Team);
	DA_API static cPlayer *Get_Highest_Damager_Other_Team(GameObject *obj,int Team);
	DA_API static float Get_Percent_Player_Damage(GameObject *obj,cPlayer *Player);
	DA_API static const DADamageEventStruct *Get_Last_Damage_Event(GameObject *obj);

	DA_API static float Compile_Repair_Table_Team(DADamageTableStruct *DamageTable,GameObject *obj,int Team);
	DA_API static float Compile_Repair_Table_Other_Team(DADamageTableStruct *DamageTable,GameObject *obj,int Team);
	DA_API static void Get_Repairers_By_Percent_Team(DynamicVectorClass<DADamageTableStruct> &Buffer,GameObject *obj,int Team,float MinimumPercentTotal,float MinimumPercentHighest);
	DA_API static void Get_Repairers_By_Percent_Other_Team(DynamicVectorClass<DADamageTableStruct> &Buffer,GameObject *obj,int Team,float MinimumPercentTotal,float MinimumPercentHighest);
	DA_API static cPlayer *Get_Highest_Repairer_Team(GameObject *obj,int Team);
	DA_API static cPlayer *Get_Highest_Repairer_Other_Team(GameObject *obj,int Team);
	DA_API static float Get_Percent_Team_Repairs(GameObject *obj,int Team);
	DA_API static float Get_Percent_Other_Team_Repairs(GameObject *obj,int Team);
	DA_API static float Get_Percent_Player_Repairs(GameObject *obj,cPlayer *Player);
	DA_API static const DADamageEventStruct *Get_Last_Repair_Event(GameObject *obj);
	*/

	// da_disable.h
	/*
	DA_API static bool Is_Preset_Disabled(GameObject *obj);
	DA_API static bool Is_Preset_Disabled(const char *Preset);
	DA_API static bool Is_Preset_Disabled(const DefinitionClass *Def);
	DA_API static bool Is_Preset_Disabled(unsigned int DefID);
	DA_API static bool Is_Preset_Disabled_For_Character(const DefinitionClass *Character,const DefinitionClass *Def);
	DA_API static bool Is_Preset_Disabled_For_Character(const DefinitionClass *Character,const char *Preset);
	DA_API static bool Is_Preset_Disabled_For_Character(const DefinitionClass *Character,unsigned int DefID);
	DA_API static bool Can_Character_Enter_Vehicles(const DefinitionClass *Character);
	DA_API static bool Can_Character_Drive_Vehicles(const DefinitionClass *Character);
	DA_API static bool Can_Character_Steal_Vehicles(const DefinitionClass *Character);

	*/

	// da_game.h
	/*
	DA_API static DAGameModeFactoryClass *Get_Game_Mode();
	DA_API static DAGameModeFactoryClass *Find_Game_Mode(const char *Name);
	DA_API static DAGameFeatureFactoryClass *Find_Game_Feature(const char *Name);
	DA_API static void Add_Game_Mode(DAGameModeFactoryClass *Factory);
	DA_API static void Add_Game_Feature(DAGameFeatureFactoryClass* Factory);
	DA_API static const DynamicVectorClass<DAGameFeatureFactoryClass*> &Get_Game_Features();
	*/

	// da_player.h
	/*
	DA_API static void Add_Data_Factory(DAPlayerDataFactoryClass *Factory);
	DA_API static void Remove_Data_Factory(DAPlayerDataFactoryClass *Factory);
	*/

	// da_soldier.h
	/*
	DA_API static HashTemplateClass<unsigned int,DynamicVectorClass<const WeaponDefinitionClass*>>  &Get_Exclusive_Weapons();
	DA_API static HashTemplateClass<unsigned int,const WeaponDefinitionClass*> &Get_Replace_Weapons();
	DA_API static HashTemplateClass<unsigned int,DynamicVectorClass<const WeaponDefinitionClass*>>  &Get_Remove_Weapons();
	*/

	// da_translation.h
	/*
	DA_API static const char *Translate(GameObject *obj); //Returns the translated name of a GameObject, Weapon, or Preset.
	DA_API static const char *Translate(WeaponClass *Weap); //da.ini translation > strings.tdb translation > preset name
	DA_API static const char *Translate(const DefinitionClass *Def);
	DA_API static const char *Translate(const char *Preset);
	DA_API static const char *Translate(unsigned int DefID);
	*/

	// da_vehicle.h
	/*
	DA_API static bool Check_Limit_For_Player(cPlayer *Player);
	DA_API static DAVehicleObserverClass *Get_Vehicle_Data(GameObject *obj);
	DA_API static cPlayer *Get_Vehicle_Owner(GameObject *obj);
	*/
}
#endif
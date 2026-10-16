/* Lua Manager
The Lua hooks and wrappers manager for Renegade
by Stan "sla.ro" Laurentiu Alexandru
Copyright 2010-2022 Sla Studios (http://slastudios.net)

This file is part of the LuaTT
*/

#include <stdarg.h>

#include "General.h"
#include "luatt.h"
#include "engine.h"
#include "Definition.h"
#include "PurchaseSettingsDefClass.h"
#include "SoldierGameObj.h"
#include "engine_tt.h"
#include "engine_ttdef.h"
#include "engine_io.h"
#include "GameObjManager.h"
#include "ScriptableGameObj.h"
#include "ConnectionRequest.h"
#include "Iterator.h"
#include "weaponmgr.h"
#include "CommandLineParser.h"

#include "LuaManager.h"
#include "LuaFunctions.h"

#ifdef DAPLUGIN
#include "LuaFunctionsDA.h"
#else
#include "gmgame.h"
#endif

#ifndef LUATT_EXPORTS
extern "C"
{
#include "lsqlite3.h"
#include "LuaSocket\libluasocket\luasocket.h"
#include "LuaSocket\libluasocket\mime.h"
}
#endif

#include "LuaHooks.h"
#include "Lua_Class.h"
#include "LuaThread.h"


///////////////////////////////////
//	Script Manager
///////////////////////////////////

std::vector<LuaScriptRegistrant<LuaScriptManager::LuaScript>*> LuaScriptManager::Scripts;

void LuaScriptManager::Load()
{
	Scripts.reserve(128);
}

ScriptImpClass *LuaScriptManager::Get_Script(GameObject *obj, int scriptID)
{
	const SimpleDynVecClass<GameObjObserverClass *> *observers = &((ScriptableGameObj *)obj)->Get_Observers();
	int x = observers->Count();
	for (int i = 0; i < x; i++)
	{
		if ((*observers)[i]->Get_ID() == scriptID)
		{
			return ((ScriptImpClass*)(*observers)[i]);
		}
	}
	return 0;
}

std::vector<ScriptImpClass *> LuaScriptManager::Get_ScriptByName(GameObject *obj, const char *name)
{
	std::vector<ScriptImpClass *> list;

	const SimpleDynVecClass<GameObjObserverClass *> *observers = &((ScriptableGameObj *)obj)->Get_Observers();
	int x = observers->Count();
	for (int i = 0; i < x; i++)
	{
		if (strcmp((*observers)[i]->Get_Name(), name) == 0)
		{
			list.push_back((ScriptImpClass*)(*observers)[i]);
		}
	}

	return list;
}

void LuaScriptManager::Lua_DestroyScript(GameObject *obj, int scriptID)
{
	if(!obj)
	{
		Console_Output("LuaTT: Invalid object to destroy! ScriptID: %d\n", scriptID);
		return;
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if(!script)
	{
		Console_Output("LuaTT: Error while destroying script! Object: %d ScriptID: %d\n", obj->Get_ID(), scriptID);
		return;
	}
	script->Destroy_Script();
}

float LuaScriptManager::Lua_Get_Float_Parameter(GameObject *obj, int scriptID, const char *Name)
{
	if(!obj)
	{
		Console_Output("LuaTT: Invalid object! ScriptID: %d Name: %s\n", scriptID, Name);
		return 0;
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if(!script)
	{
		Console_Output("LuaTT: Error while trying to get float parameter! Object: %d ScriptID: %d Name: %s\n", obj->Get_ID(), scriptID, Name);
		return 0;
	}
	return script->Get_Float_Parameter(Name);
}

Vector3 LuaScriptManager::Lua_Get_Vector3_Parameter(GameObject *obj, int scriptID, const char *Name)
{
	if (!obj)
	{
		Console_Output("LuaTT: Invalid object! ScriptID: %d Name: %s\n", scriptID, Name);
		return Vector3();
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if (!script)
	{
		Console_Output("LuaTT: Error while trying to get float parameter! Object: %d ScriptID: %d Name: %s\n", obj->Get_ID(), scriptID, Name);
		return Vector3();
	}
	return script->Get_Vector3_Parameter(Name);
}

int LuaScriptManager::Lua_Get_Int_Parameter(GameObject *obj, int scriptID, const char *Name)
{
	if(!obj)
	{
		Console_Output("LuaTT: Invalid object! ScriptID: %d Name: %s\n", scriptID, Name);
		return 0;
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if(!script)
	{
		Console_Output("LuaTT: Error while trying to get int parameter! Object: %d ScriptID: %d Name: %s\n", obj->Get_ID(), scriptID, Name);
		return 0;
	}
	return script->Get_Int_Parameter(Name);
}

bool LuaScriptManager::Lua_Get_Bool_Parameter(GameObject *obj, int scriptID, const char *Name)
{
	if(!obj)
	{
		Console_Output("LuaTT: Invalid object! ScriptID: %d Name: %s\n", scriptID, Name);
		return 0;
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if(!script)
	{
		Console_Output("LuaTT: Error while trying to get bool parameter! Object: %d ScriptID: %d Name: %s\n", obj->Get_ID(), scriptID, Name);
		return 0;
	}
	return script->Get_Bool_Parameter(Name);
}

const char *LuaScriptManager::Lua_Get_String_Parameter(GameObject *obj, int scriptID, const char *Name)
{
	if(!obj)
	{
		Console_Output("LuaTT: Invalid object! ScriptID: %d Name: %s\n", scriptID, Name);
		return "ERROR OBJECT";
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if(!script)
	{
		Console_Output("LuaTT: Error while trying to get string parameter! Object: %d ScriptID: %d Name: %s\n", obj->Get_ID(), scriptID, Name);
		return "ERROR";
	}
	return script->Get_Parameter(Name);
}

int LuaScriptManager::Register_Script(lua_State *L)
{
	if(!lua_istable(L, -1))
	{
		luaL_error(L, "Bad argument #1 to Register_Script. Expected table.");
		return 0;
	}

	const char *Name = luaL_checkstring(L, -3);
	if(!Name)
	{
		luaL_error(L, "Bad argument #2 to Register_Script. Expected Script Name.");
		return 0;
	}

	const char *Parameters = luaL_checkstring(L, -2);
	if(!Parameters)
	{
		luaL_error(L, "Bad argument #3 to Register_Script. Expected Script Params.");
		return 0;
	}

	std::string std_Name = std::string(Name);

	for (auto script_it = Scripts.begin(); script_it != Scripts.end(); ++script_it)
	{
		LuaScriptRegistrant<LuaScript> *s = (*script_it);
		if (s != nullptr)
		{
			if (s->getName() == std_Name)
			{
				Console_Output("[Lua-Warning] You cannot register more than one script with the same name.\n");
				return 0;
			}
		}
	}
	int uRef = luaL_ref(L, LUA_REGISTRYINDEX);


	const char *copy_Name = _strdup(Name);
	const char *copy_Parameters = _strdup(Parameters);

	LuaScriptRegistrant<LuaScript> *s = new LuaScriptRegistrant<LuaScript>(copy_Name, copy_Parameters, L, uRef);
	Scripts.push_back(s);
	return 0;
}

void LuaScriptManager::Lua_Start_Timer(GameObject *obj, int scriptID, float Time, int Number)
{
	if(!obj)
	{
		return;
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if(!script)
	{
		return;
	}
	Commands->Start_Timer(obj, script, Time, Number);
}

void LuaScriptManager::Cleanup()
{
	if (Scripts.size() > 0)
	{
		for (auto script_it = Scripts.rbegin(); script_it != Scripts.rend(); ++script_it)
		{
			LuaScriptRegistrant<LuaScript> *s = (*script_it);
			if (s != nullptr)
			{
				delete s;
				s = nullptr;
			}
		}
	}
	Scripts.clear();
}

void LuaScriptManager::RegisterScriptFunctions(lua_State *L)
{
	lua_register(L, "DestroyScript", LuaScriptManager::Lua_DestroyScript_Wrap);
	lua_register(L, "Get_Float_Parameter", LuaScriptManager::Lua_Get_Float_Parameter_Wrap);
	lua_register(L, "Get_Vector3_Parameter_Wrap", LuaScriptManager::Lua_Get_Vector3_Parameter_Wrap);
	lua_register(L, "Get_Int_Parameter", LuaScriptManager::Lua_Get_Int_Parameter_Wrap);
	lua_register(L, "Get_Bool_Parameter", LuaScriptManager::Lua_Get_Bool_Parameter_Wrap);
	lua_register(L, "Get_String_Parameter", LuaScriptManager::Lua_Get_String_Parameter_Wrap);
	lua_register(L, "Start_Timer", LuaScriptManager::Lua_Start_Timer_Wrap);
	lua_register(L, "Register_Script", LuaScriptManager::Register_Script);
	lua_register(L, "Install_Hook", LuaScriptManager::Lua_Install_Hook_Warp);
	lua_register(L, "Remove_Hook", LuaScriptManager::Lua_Remove_Hook_Warp);
	lua_register(L, "Get_Script_Name", LuaScriptManager::Lua_Get_Script_Name);
}


int LuaScriptManager::Lua_Get_Script_Name(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 2)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	GameObject *obj = Commands->Find_Object(objID);
	if (!obj)
	{
		return 0;
	}
	ScriptImpClass *script = Get_Script(obj, scriptID);
	if (!script)
	{
		return 0;
	}
	lua_pushstring(L, script->Get_Name());
	return 1;
}


int LuaScriptManager::Lua_DestroyScript_Wrap(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc < 2)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	LuaScriptManager::Lua_DestroyScript(Commands->Find_Object(objID), scriptID);
	return 0;
}

int LuaScriptManager::Lua_Get_Float_Parameter_Wrap(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc < 3)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	const char *name = lua_tostring(L, 3);
	lua_pushnumber(L, LuaScriptManager::Lua_Get_Float_Parameter(Commands->Find_Object(objID), scriptID, name));
	return 1;
}

int LuaScriptManager::Lua_Get_Vector3_Parameter_Wrap(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 3)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	const char *name = lua_tostring(L, 3);
	Vector3 pos = Lua_Get_Vector3_Parameter(Commands->Find_Object(objID), scriptID, name);
	LUA_Vector3 *a = new LUA_Vector3(pos.X, pos.Y, pos.Z);
	lua_boxpointer(L, a);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);

	return 1;
}


int LuaScriptManager::Lua_Get_Int_Parameter_Wrap(lua_State *L)
{
	if (lua_gettop(L) < 3)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	const char *name = lua_tostring(L, 3);
	lua_pushnumber(L, LuaScriptManager::Lua_Get_Int_Parameter(Commands->Find_Object(objID), scriptID, name));
	return 1;
}

int LuaScriptManager::Lua_Get_Bool_Parameter_Wrap(lua_State *L)
{
	if (lua_gettop(L) < 3)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	const char *name = lua_tostring(L, 3);
	lua_pushboolean(L, LuaScriptManager::Lua_Get_Bool_Parameter(Commands->Find_Object(objID), scriptID, name));
	return 1;
}

int LuaScriptManager::Lua_Get_String_Parameter_Wrap(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc < 3)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	const char *name = lua_tostring(L, 3);
	lua_pushstring(L, LuaScriptManager::Lua_Get_String_Parameter(Commands->Find_Object(objID), scriptID, name));
	return 1;
}

int LuaScriptManager::Lua_Start_Timer_Wrap(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc < 4)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int obj = (int)lua_tonumber(L, 2);
	float Time = (float)lua_tonumber(L, 3);
	int Number = (int)lua_tonumber(L, 4);
	LuaScriptManager::Lua_Start_Timer(Commands->Find_Object(obj), scriptID, Time, Number);
	return 0;
}

int LuaScriptManager::Lua_Install_Hook_Warp(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc < 3)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);
	const char *TheKey = lua_tostring(L, 3);
	
	GameObject* obj = Commands->Find_Object(objID);
	if (!obj) return 0;

	LuaScript* script = (LuaScript*)Get_Script(obj, scriptID);
	if (!script) return 0;
	script->InstallHook(TheKey, obj);
	return 0;
}

int LuaScriptManager::Lua_Remove_Hook_Warp(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 2)
	{
		return 0;
	}
	int scriptID = (int)lua_tonumber(L, 1);
	int objID = (int)lua_tonumber(L, 2);

	GameObject* obj = Commands->Find_Object(objID);
	if (!obj) return 0;

	LuaScript* script = (LuaScript*)Get_Script(obj, scriptID);
	if (!script) return 0;
	script->RemoveHook();
	return 0;
}

void LuaScriptManager::LuaScript::Created(GameObject *obj)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Created");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		LuaManager::Report_Errors(L, lua_pcall(L, 3, 0, 0));
	}
	lua_pop(L, -1);
}


void LuaScriptManager::LuaScript::Destroyed(GameObject *obj)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Destroyed");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		LuaManager::Report_Errors(L, lua_pcall(L, 3, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Detach(GameObject *obj)
{

	if (L == nullptr)
		return;

	if (myFactory)
	{
		myFactory->removeScript(Get_ID());
		myFactory = nullptr;
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	
	lua_getfield(L, -1, "Detach");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		LuaManager::Report_Errors(L, lua_pcall(L, 3, 0, 0));
	}
	lua_pop(L, -1);
	ScriptImpClass::Detach(obj);
	Destroy_Script();
}


void LuaScriptManager::LuaScript::KeyHook()
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "KeyHook");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(Owner()));
		LuaManager::Report_Errors(L, lua_pcall(L, 3, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Killed(GameObject *obj,GameObject *shooter)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Killed");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, Commands->Get_ID(shooter));
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Damaged(GameObject *obj,GameObject *damager,float damage)
{
	if (L == nullptr)
		return;


	lua_rawgeti(L, LUA_REGISTRYINDEX, uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Damaged");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, Commands->Get_ID(damager));
		lua_pushnumber(L, damage);
		LuaManager::Report_Errors(L, lua_pcall(L, 5, 0, 0));
	}
	lua_pop(L, -1);

}

void LuaScriptManager::LuaScript::Custom(GameObject *obj,int message,int param,GameObject *sender)
{
	if (L == nullptr)
		return;


	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Custom");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, message);
		lua_pushnumber(L, param);
		lua_pushnumber(L, Commands->Get_ID(sender));
		LuaManager::Report_Errors(L, lua_pcall(L, 6, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Enemy_Seen(GameObject *obj,GameObject *seen)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Enemy_Seen");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, Commands->Get_ID(seen));
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}


void LuaScriptManager::LuaScript::Sound_Heard(GameObject *obj, const CombatSound & sound)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, uRef);
	if (!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Sound_Heard");
	if (lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, Commands->Get_ID(sound.Creator));

		LUA_Vector3 *a = new LUA_Vector3(sound.Position.X, sound.Position.Y, sound.Position.Z);
		lua_boxpointer(L, a);
		luaL_getmetatable(L, "Vector3");
		lua_setmetatable(L, -2);

		lua_pushnumber(L, sound.sound);
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Action_Complete(GameObject *obj,int action, ActionCompleteReason reason)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Action_Complete");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, action);
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Timer_Expired(GameObject *obj,int number)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Timer_Expired");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, number);
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Animation_Complete(GameObject *obj,const char *anim)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Animation_Complete");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushstring(L, anim);
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Poked(GameObject *obj,GameObject *poker)
{
	if (L == nullptr)
		return;


	lua_rawgeti(L, LUA_REGISTRYINDEX,uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Poked");
	if (lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, Commands->Get_ID(poker));
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Entered(GameObject *obj,GameObject *enter)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Entered");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, Commands->Get_ID(enter));
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

void LuaScriptManager::LuaScript::Exited(GameObject *obj,GameObject *exit)
{
	if (L == nullptr)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, uRef);
	if(!lua_istable(L, -1))
	{
		return;
	}

	lua_getfield(L, -1, "Exited");
	if(lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -2);
		lua_pushnumber(L, Get_ID());
		lua_pushnumber(L, Commands->Get_ID(obj));
		lua_pushnumber(L, Commands->Get_ID(exit));
		LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
	}
	lua_pop(L, -1);
}

std::vector<lua_State*> LuaManager::Lua;
std::vector<std::string> LuaManager::LuaNames;

///////////////////////////////////
//	SSGM Hooks
///////////////////////////////////

bool LuaManager::Call_Chat_Hook(int ID, int Type, const wchar_t *Msg, int Target)
{
	int ret = 1;
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnChat");
		if (lua_isfunction(L, -1))
		{
			const char *Msg2 = WideCharToChar(Msg);
			lua_pushnumber(L, ID);
			lua_pushnumber(L, Type);
			lua_pushstring(L, Msg2);
			lua_pushnumber(L, Target);
			LuaManager::Report_Errors(L, lua_pcall(L, 4, 1, 0));
			ret = !ret ? ret : (int)lua_tonumber(L, -1);
			delete[]Msg2;
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "OnChat");
		}
	}

	return ret != 0 ? 1 : 0;
}

const char* LuaManager::ConAc(const char* ip, const char* nick, const char* serial, const char* version, int id)
{
	const char *ret = "CONTINUE";
	for (auto L : Lua)
	{
		lua_getglobal(L, "ConnectionAcceptance");
		if (lua_isfunction(L, -1))
		{
			lua_pushstring(L, nick);
			lua_pushstring(L, ip);
			lua_pushstring(L, serial);
			lua_pushstring(L, version);
			lua_pushnumber(L, id);
			LuaManager::Report_Errors(L, lua_pcall(L, 5, 1, 0));

			ret = (const char *)lua_tostring(L, -1);
			if (!ret) { ret = "CONTINUE"; }
			lua_pop(L, lua_gettop(L));
			if (strcmp(ret, "CONTINUE")) break;
		}
		else
		{
			lua_setglobal(L, "ConnectionAcceptance");
		}
	}
	
	return ret;
}


bool LuaManager::RadioHook(int PlayerType, int PlayerID, int AnnouncementID, int IconID, AnnouncementEnum AnnouncementType)
{
	int ret = 1;
	for (auto L : Lua)
	{
		lua_getglobal(L, "RadioHook");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, PlayerType);
			lua_pushnumber(L, PlayerID);
			lua_pushnumber(L, AnnouncementID);
			lua_pushnumber(L, IconID);
			lua_pushnumber(L, AnnouncementType);
			LuaManager::Report_Errors(L, lua_pcall(L, 5, 1, 0));
			ret = !ret ? ret : (int)lua_tonumber(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "RadioHook");
		}
	}
	return ret != 0 ? 1 : 0;
}

bool LuaManager::Refill_Hook(GameObject *purchaser)
{
	if (!purchaser) { return true; }
	int ret = 1;
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnRefill");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Commands->Get_ID(purchaser));
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 1, 0));
			ret = !ret ? ret : (int)lua_tonumber(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "OnRefill");
		}
	}
	return ret != 0 ? 1 : 0;
}

bool LuaManager::Call_Host_Hook(int PlayerID,TextMessageEnum Type,const char *Message)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnHostMessage");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, PlayerID);
			lua_pushnumber(L, Type);
			lua_pushstring(L, Message);
			LuaManager::Report_Errors(L, lua_pcall(L, 3, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnHostMessage");
		}
	}
	return true;
}
void LuaManager::Call_Player_Join_Hook(int ID, const char *Nick)
{
	if (!ID) { return; }
	if (!Nick) { return; }
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnPlayerJoin");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, ID);
			lua_pushstring(L, Nick);
			LuaManager::Report_Errors(L, lua_pcall(L, 2, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnPlayerJoin");
		}
	}
}

void LuaManager::Call_On_Dialog(int PlayerID, int DialogID, int ControlID, DialogMessageType MessageType)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnDialog");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, PlayerID);
			lua_pushnumber(L, DialogID);
			lua_pushnumber(L, ControlID);
			lua_pushnumber(L, MessageType);
			LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnDialog");
		}
	}
}

void LuaManager::Call_Player_Leave_Hook(int ID)
{
	if (!ID) { return; }
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnPlayerLeave");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, ID);
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnPlayerLeave");
		}
	}
}
void LuaManager::Call_Level_Loaded_Hook()
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnLevelLoaded");
		if (lua_isfunction(L, -1))
		{
			LuaManager::Report_Errors(L, lua_pcall(L, 0, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnLevelLoaded");
		}
	}
}
void LuaManager::Call_GameOver_Hook()
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnGameOver");
		if (lua_isfunction(L, -1))
		{
			LuaManager::Report_Errors(L, lua_pcall(L, 0, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnGameOver");
		}
	}
}
void LuaManager::Call_Console_Output_Hook(const char *Output)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnConsoleOutput");
		if (lua_isfunction(L, -1))
		{
			lua_pushstring(L, Output);
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnConsoleOutput");
		}
	}
}

void LuaManager::Purchase_Hook(BaseControllerClass *base, GameObject *purchaser, unsigned int cost, unsigned int preset,unsigned int purchaseret,const char *data)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, data);
		if (lua_isfunction(L, -1))
		{
			if (!Commands->Get_ID(purchaser)) { return; }
			lua_pushnumber(L, Commands->Get_ID(purchaser));
			lua_pushnumber(L, cost);
			lua_pushnumber(L, preset);
			lua_pushnumber(L, purchaseret);
			LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
		}
		else
		{
			lua_setglobal(L, data);
		}
	}
}

int LuaManager::Purchase_Hook2(BaseControllerClass *base,GameObject *purchaser,unsigned int cost,unsigned int preset,const char *data)
{
	int ret = -1;

	for (auto L : Lua)
	{
		lua_getglobal(L, "VehicleTTHook");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Commands->Get_ID(purchaser));
			lua_pushnumber(L, cost);
			lua_pushnumber(L, preset);
			LuaManager::Report_Errors(L, lua_pcall(L, 3, 1, 0));
			int rety = (int)lua_tonumber(L, -1);
			if (rety) ret = rety;
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "VehicleTTHook");
		}
	}

	return ret;
}

bool LuaManager::Stock_Damage(PhysicalGameObj *Damager, PhysicalGameObj *Target, float Damage, uint Warhead)
{
	int ret = 1;
	for (auto L : Lua)
	{
		lua_getglobal(L, "StockDamageHook");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Commands->Get_ID(Damager));
			lua_pushnumber(L, Commands->Get_ID(Target));
			lua_pushnumber(L, Damage);
			lua_pushnumber(L, Warhead);
			LuaManager::Report_Errors(L, lua_pcall(L, 4, 1, 0));
			ret = !ret ? ret : (int)lua_tonumber(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "StockDamageHook");
		}
	}
	return ret != 0 ? 1 : 0;
}

bool LuaManager::TT_Damage_Hook(PhysicalGameObj* damager, PhysicalGameObj* target, const AmmoDefinitionClass* ammo, const char* bone)
{
	int ret = 1;
	for (auto L : Lua)
	{
		lua_getglobal(L, "TTDamageHook");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Commands->Get_ID(damager));
			lua_pushnumber(L, Commands->Get_ID(target));
			lua_pushstring(L, ammo->Get_Name());
			lua_pushnumber(L, ammo->Warhead());
			lua_pushstring(L, bone);
			lua_pushlightuserdata(L, (void*)ammo);
			lua_pushnumber(L, ammo->Damage);
			LuaManager::Report_Errors(L, lua_pcall(L, 7, 1, 0));
			ret = !ret ? ret : (int)lua_tonumber(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "TTDamageHook");
		}
	}
	return ret != 0 ? 1 : 0;
}


bool LuaManager::Report_Errors(lua_State *L, int status)
{
	if(status != 0) 
	{
		const char *err = lua_tostring(L, -1);
		lua_pop(L, 1);
		Console_Output("[Lua_Error] %s\n", err);
		lua_getglobal(L, "OnError");
		if(lua_isfunction(L, -1))
		{
			lua_pushstring(L, err);
			if(lua_pcall(L, 1, 0, 0) != 0)
			{
				lua_pop(L, 1);
			}
		}
		return 1;
	}

	return 0;
}


void LuaManager::UnloadingLua()
{
	for (auto L : Lua)
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

 Delete_Console_Function("Lua");
}


void LuaManager::Call_Object_Hook(void *data, GameObject *obj)
{
	if (!obj) { return; }
	Attach_Script_Once(obj,"Lua_Script_Hook","");

	for (auto L : Lua)
	{
		lua_getglobal(L, "OnObjectCreate");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Commands->Get_ID(obj));
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnObjectCreate");
		}
	}
}

void LuaManager::Adress_IP(const char *addr)
{
	if (!addr) { return; }
	for (auto L : Lua)
	{
		lua_getglobal(L, "Adress_IP");
		if (lua_isfunction(L, -1))
		{
			lua_pushstring(L, addr);
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 0, 0));
		}
		else
		{
			lua_setglobal(L, "Adress_IP");
		}
	}
}

bool LuaManager::Reload_Flag=false;

void LuaManager::Call_Think_Hook()
{
	LuaThread::Lock_Check();
	if(Reload_Flag)
	{
		Reload_All();
		Reload_Flag = false;
		return;
	}
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnThink");
		if (lua_isfunction(L, -1))
		{
			LuaManager::Report_Errors(L, lua_pcall(L, 0, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnThink");
		}
	}
}

void LuaManager::CallInvoke(const char *Function, const char *Arg)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, Function);
		if (lua_isfunction(L, -1))
		{
			lua_pushstring(L, Arg);
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 0, 0));
		}
	}
}

///////////////////////////////////
//	LuaManager Functions
///////////////////////////////////

void LuaManager::Load()
{

#ifndef LUATT_EXPORTS
	LoadLua();
#endif
}

#ifdef LUATT_EXPORTS
void LuaManager::ShowBanner()
{
	Console_Output("|-----------------------------------------------|\n");
#ifdef DAPLUGIN
	Console_Output("| LuaTT (DA) 6 by sla.ro (C) 2014-2026          |\n");
#else
	Console_Output("| LuaTT (SSGM) 6 by sla.ro (C) 2014-2026        |\n");
#endif
	Console_Output("| Original LuaV4 by jnz                         |\n");
	Console_Output("|-----------------------------------------------|\n");
	Console_Output("LuaTT 6 %s revision\n", LUATT_BUILD);
}
#endif

#ifndef LUATT_EXPORTS
void LuaManager::LoadLua()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
	lsqlite3(L);
	AddFunctions(L);
	LoadLuaSocket(L);
	LuaScriptManager::RegisterScriptFunctions(L);
#ifdef DAPLUGIN
	AddFunctionsDA(L);
#endif
	#include "Bot\loader.Lua.h"
	LoadModule(L, luaJIT_BC_loader, luaJIT_BC_loader_SIZE, "loader.lua");
	Lua.push_back(L);
	LuaNames.push_back("loader");
}

void LuaManager::LoadModule(lua_State *L, const char *data, size_t size, const char *name)
{
	if (luaL_loadbuffer(L, data, size, name) == 0)
	{
		LuaManager::Report_Errors(L, lua_pcall(L, 0, 0, 0));
	}
	else
	{
		const char *err = lua_tostring(L, -1);
		lua_pop(L, 1);
		Console_Output("[Lua_Error] %s\n", err);
	}
}

#else

void LuaManager::LoadLua(const char *LuaFile)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	AddFunctions(L);
	LuaScriptManager::RegisterScriptFunctions(L);
#ifdef DAPLUGIN
	AddFunctionsDA(L);
#endif

	int s = luaL_loadfile(L, LuaFile);
	if (s == 0)
	{
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	if (Report_Errors(L, s))
	{
		lua_close(L);
		Console_Output("[Lua] Failed to load plugin: %s\n", LuaFile);
		return;
	}
	Console_Output("[Lua] Loaded plugin: %s\n", LuaFile);

	std::string filename(LuaFile);
	const size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	Lua.push_back(L);
	LuaNames.push_back(filename);
}

#endif

void LuaManager::Cleanup()
{
#ifdef DAPLUGIN
	for (auto it = LuaTT_DAChatList.begin(); it != LuaTT_DAChatList.end(); ++it)
	{
		auto obj = (*it);
		DAChatCommandManager::Unregister_Chat_Command(obj->getCommand().c_str());
		delete obj;
		obj = nullptr;
	}

	LuaTT_DAChatList.clear();
#endif


	for (auto L : Lua)
	{
		lua_close(L);
		L = nullptr;
	}
	Lua.clear();
}

void LuaManager::Reload_All()
{
	LuaScriptManager::Cleanup();
	LuaManager::Cleanup();

	LuaManager::Load();
#ifdef LUATT_EXPORTS
	LoadLuaPlugins();
#else
	// TODO onLuaRestart function
#endif
}

#ifdef LUATT_EXPORTS
void LuaManager::LoadLuaPlugins()
{
	char basepath[128];
	GetCurrentDirectory(128, basepath);
	std::string std_basepath = std::string(basepath);
	std::string std_pluginsdir = std_basepath + std::string("\\LuaPlugins\\*.lua");
	std::vector<std::string> plugins;

	GetFiles(std_pluginsdir, &plugins);

	for (std::string Name : plugins)
	{
		std::string path = std_basepath + "\\LuaPlugins\\" + Name;
		LuaManager::LoadLua(path.c_str());
	}
}
#endif


void GetFiles(std::string Path, std::vector<std::string> *Data)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(Path.c_str(), &FindFileData);
	if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
	{
		char *file = new char[strlen(FindFileData.cFileName) + 1];
		strcpy(file, FindFileData.cFileName);
		Data->push_back(std::string(file));
		delete []file;
	}

	while (FindNextFile(hFind, &FindFileData) != 0)
	{
		if (FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			char *file = new char[strlen(FindFileData.cFileName) + 1];
			strcpy(file, FindFileData.cFileName);
			Data->push_back(std::string(file));
			delete []file;
		}
	}

	FindClose(hFind);
}


///////////////////////////////////
//	LuaTT Script Hooking
///////////////////////////////////

void LuaManager::KilledHook(int obj,int shooter)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnKilled");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			lua_pushnumber(L, shooter);
			LuaManager::Report_Errors(L, lua_pcall(L, 2, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnKilled");
		}
	}
}

void LuaManager::ZoneEnterHook(int obj, int enter)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnZoneEnter");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			lua_pushnumber(L, enter);
			LuaManager::Report_Errors(L, lua_pcall(L, 2, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnZoneEnter");
		}
	}
}

void LuaManager::ZoneExitHook(int obj, int exit)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnZoneExit");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			lua_pushnumber(L, exit);
			LuaManager::Report_Errors(L, lua_pcall(L, 2, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnZoneExit");
		}
	}
}


void LuaManager::DestroyedHook(int obj)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "OnDestroyed");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 0, 0));
		}
		else
		{
			lua_setglobal(L, "OnDestroyed");
		}
	}
}

void LuaManager::CustomHook(int obj,int message,int param,int sender)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "CustomHook");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			lua_pushnumber(L, message);
			lua_pushnumber(L, param);
			lua_pushnumber(L, sender);
			LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
		}
		else
		{
			lua_setglobal(L, "CustomHook");
		}
	}
}

void LuaManager::DamageHook(int obj,int damager,float damage)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "DamageHook");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			lua_pushnumber(L, damager);
			lua_pushnumber(L, damage);
			lua_pushnumber(L, Get_Damage_Warhead());
			LuaManager::Report_Errors(L, lua_pcall(L, 4, 0, 0));
		}
		else
		{
			lua_setglobal(L, "DamageHook");
		}
	}
}

void LuaManager::PokedHook(int obj,int damager)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "PokedHook");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			lua_pushnumber(L, damager);
			LuaManager::Report_Errors(L, lua_pcall(L, 2, 0, 0));
		}
		else
		{
			lua_setglobal(L, "PokedHook");
		}
	}
}

void LuaManager::KeyHook(int obj, const char *key, const char *callback)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, callback);
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, obj);
			lua_pushstring(L, key);
			LuaManager::Report_Errors(L, lua_pcall(L, 2, 0, 0));
		}
		else
		{
			lua_setglobal(L, callback);
		}
	}
}

void Lua_Script_Hook::Killed(GameObject *obj, GameObject *shooter)
{
   LuaManager::KilledHook(Commands->Get_ID(obj), Commands->Get_ID(shooter));
}

void Lua_Script_Hook::Entered(GameObject* obj, GameObject* enter)
{
	LuaManager::ZoneEnterHook(Commands->Get_ID(obj), Commands->Get_ID(enter));
}

void Lua_Script_Hook::Exited(GameObject* obj, GameObject* exit)
{
	LuaManager::ZoneExitHook(Commands->Get_ID(obj), Commands->Get_ID(exit));
}


void Lua_Script_Hook::Destroyed(GameObject *obj)
{
   LuaManager::DestroyedHook(Commands->Get_ID(obj));
}

void Lua_Script_Hook::Custom(GameObject *obj,int message,int param,GameObject *sender)
{
   LuaManager::CustomHook(Commands->Get_ID(obj),message,param,Commands->Get_ID(sender));
}

void Lua_Script_Hook::Damaged(GameObject *obj,GameObject *damager,float damage)
{
   LuaManager::DamageHook(Commands->Get_ID(obj),Commands->Get_ID(damager),damage);
   
}

void Lua_Script_Hook::Poked(GameObject *obj,GameObject *poker)
{
   LuaManager::PokedHook(Commands->Get_ID(obj),Commands->Get_ID(poker));
}

ScriptRegistrant<Lua_Script_Hook> Lua_Script_Hook_Registrant("Lua_Script_Hook","");

///////////////////////////////////
// LuaTT KeyHook
///////////////////////////////////

void Lua_Key_Hook::Created(GameObject *obj)
{
	myobj = Commands->Get_ID(obj);
	this->InstallHook(Get_Parameter("key"), obj);
}

void Lua_Key_Hook::KeyHook()
{
	LuaManager::KeyHook(myobj, Get_Parameter("key"), Get_Parameter("callback"));
}

ScriptRegistrant<Lua_Key_Hook> Lua_Key_Hookk_Registrant("Lua_Key_Hook", "key=none:string,callback=FunctionName:string");




#ifdef DAPLUGIN
///////////////////////////////////
//	DA LuaTT Hooks
///////////////////////////////////

bool LuaManager::DA_Suicide_Event(cPlayer *Player)
{
	if (Player == nullptr)
	{
		return true;
	}
	bool ret = true;
	for (auto L : Lua)
	{
		lua_getglobal(L, "DA_Suicide_Event");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Player->Get_Id());
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 1, 0));
			ret = !ret ? ret : (bool)lua_toboolean(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "DA_Suicide_Event");
		}
	}

	return ret;
}

bool LuaManager::DA_Refill_Event(cPlayer *Player)
{
	if (Player == nullptr)
	{
		return true;
	}
	bool ret = true;
	for (auto L : Lua)
	{
		lua_getglobal(L, "DA_Refill_Event");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Player->Get_Id());
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 1, 0));
			ret = !ret ? ret : (bool)lua_toboolean(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "DA_Refill_Event");
		}
	}

	return ret;
}

bool LuaManager::DA_Team_Change_Request_Event(cPlayer *Player)
{
	if (Player == nullptr)
	{
		return true;
	}
	bool ret = true;
	for (auto L : Lua)
	{
		lua_getglobal(L, "DA_Team_Change_Request_Event");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, Player->Get_Id());
			LuaManager::Report_Errors(L, lua_pcall(L, 1, 1, 0));
			ret = !ret ? ret : (bool)lua_toboolean(L, -1);
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "DA_Team_Change_Request_Event");
		}
	}

	return ret;
}

void LuaManager::DA_Player_Pre_Join_Event(ConnectionRequest& Request)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "DA_Player_Pre_Join_Event");
		if (lua_isfunction(L, -1))
		{
			StringClass Address = inet_ntoa(Request.clientAddress.sin_addr);
			StringClass PlayerName = WideCharToChar(Request.clientName.Peek_Buffer());
			StringClass PassW = WideCharToChar(Request.password.Peek_Buffer());
			char version[55];
			sprintf(version, "%f", Request.clientVersion);

			lua_pushnumber(L, Request.clientId);
			lua_pushstring(L, PlayerName);
			lua_pushstring(L, Address);
			lua_pushstring(L, Request.clientSerialHash);
			lua_pushstring(L, version);
			lua_pushnumber(L, Request.clientRevisionNumber);
			lua_pushstring(L, PassW);
			lua_pushnumber(L, Request.clientExeKey);


			LuaManager::Report_Errors(L, lua_pcall(L, 8, 0, 0));
		}
		else
		{
			lua_setglobal(L, "DA_Player_Pre_Join_Event");
		}
	}
}

void LuaManager::DA_Log_Event(const char *Header, const char *Output)
{
	for (auto L : Lua)
	{
		lua_getglobal(L, "DA_Log_Event");
		if (lua_isfunction(L, -1))
		{
			lua_pushstring(L, Header);
			lua_pushstring(L, Output);
			LuaManager::Report_Errors(L, lua_pcall(L, 2, 0, 0));
		}
		else
		{
			lua_setglobal(L, "DA_Log_Event");
		}
	}
}

const char* LuaManager::DA_Connection_Request_Event(ConnectionRequest& Request, WideStringClass& RefusalMessage)
{
	const char* ret = "CONTINUE";
	for (auto L : Lua)
	{
		lua_getglobal(L, "DA_Connection_Request_Event");
		if (lua_isfunction(L, -1))
		{
			StringClass Address = inet_ntoa(Request.clientAddress.sin_addr);
			StringClass PlayerName = WideCharToChar(Request.clientName.Peek_Buffer());
			StringClass PassW = WideCharToChar(Request.password.Peek_Buffer());
			char version[55];
			sprintf(version, "%f", Request.clientVersion);

			lua_pushnumber(L, Request.clientId);
			lua_pushstring(L, PlayerName);
			lua_pushstring(L, Address);
			lua_pushstring(L, Request.clientSerialHash);
			lua_pushstring(L, version);
			lua_pushnumber(L, Request.clientRevisionNumber);
			lua_pushstring(L, PassW);
			lua_pushnumber(L, Request.clientExeKey);
			LuaManager::Report_Errors(L, lua_pcall(L, 8, 1, 0));

			ret = (const char*)lua_tostring(L, -1);
			if (!ret) { ret = "CONTINUE"; }
			lua_pop(L, lua_gettop(L));
			if (strcmp(ret, "CONTINUE")) break;
		}
		else
		{
			lua_setglobal(L, "DA_Connection_Request_Event");
		}
	}
	return ret;
}


bool LuaManager::DA_Damage_Request_Event(DamageableGameObj *Victim, ArmedGameObj *Damager, float &Damage, unsigned int &Warhead, float Scale, int Type)
{
	bool ret = true;
	int VictimID = 0;
	if (Victim != nullptr)
	{
		VictimID = Victim->Get_ID();
	}

	int DamagerID = 0;
	if (Damager != nullptr)
	{
		DamagerID = Damager->Get_ID();
	}

	int stack_size;
	int num_returns;

	for (auto L : Lua)
	{
		stack_size = lua_gettop(L);
		lua_getglobal(L, "DA_Damage_Request_Event");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, VictimID);
			lua_pushnumber(L, DamagerID);
			lua_pushnumber(L, Damage);
			lua_pushnumber(L, Warhead);
			lua_pushnumber(L, Scale);
			lua_pushnumber(L, Type);

			LuaManager::Report_Errors(L, lua_pcall(L, 6, LUA_MULTRET, 0));
			num_returns = lua_gettop(L) - stack_size;

			if (num_returns >= 1)
			{
				ret = !ret ? ret : lua_toboolean(L, -1);
			}
			
			if (num_returns >= 2 && lua_isnumber(L, -2))
			{
				Damage = static_cast<float>(lua_tonumber(L, -2));
			}
			if (num_returns >= 3 && lua_isnumber(L, -3))
			{
				Warhead = static_cast<unsigned int>(lua_tonumber(L, -3));
			}
			lua_pop(L, lua_gettop(L));
		}
		else
		{
			lua_setglobal(L, "DA_Damage_Request_Event");
		}
	}

	return ret;
}


int LuaManager::GenericHelper_getTotalArguments(GenericParamList params)
{
	int t = 0;

	if (params.p1 != USE_NOTHING)
		t++;
	if (params.p2 != USE_NOTHING)
		t++;
	if (params.p3 != USE_NOTHING)
		t++;
	if (params.p4 != USE_NOTHING)
		t++;
	if (params.p5 != USE_NOTHING)
		t++;
	if (params.p6 != USE_NOTHING)
		t++;

	return t;
}

void GenericHelper_pushStack(GenericParam type, void *data, lua_State *L)
{
	if (type == USE_BOOL)
	{
		lua_pushboolean(L, reinterpret_cast<bool>(data));
	}
	else if (type == USE_STRING)
	{
		lua_pushstring(L, reinterpret_cast<const char *>(data));
	}
	else if (type == USE_INT)
	{
		lua_pushnumber(L, reinterpret_cast<int>(data));
	}
	else if (type == USE_DOUBLE)
	{
		lua_pushnumber(L, *(double*)data);
	}
	else if (type == USE_FLOAT)
	{
		lua_pushnumber(L, *(float*)data);
	}
}

int LuaManager::Generic_Hook(const char *func, GenericParamList params, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6)
{
	int totalArgs = GenericHelper_getTotalArguments(params);
	int ret = 0;
	int usingReturn = 0;
	if (params.ret != USE_NOTHING)
	{
		ret = params.defaultReturn;
		usingReturn = 1;
	}


	for (auto L : Lua)
	{
		lua_getglobal(L, func);
		if (lua_isfunction(L, -1))
		{
			GenericHelper_pushStack(params.p1, p1, L);
			GenericHelper_pushStack(params.p2, p2, L);
			GenericHelper_pushStack(params.p3, p3, L);
			GenericHelper_pushStack(params.p4, p4, L);
			GenericHelper_pushStack(params.p5, p5, L);
			GenericHelper_pushStack(params.p6, p6, L);
			LuaManager::Report_Errors(L, lua_pcall(L, totalArgs, usingReturn, 0));
			if (params.ret == USE_BOOL)
			{
				ret = !ret ? ret : (bool)lua_toboolean(L, -1);
			}
			else if (params.ret == USE_INT)
			{
				const char* str = lua_tostring(L, -1);
				if (str) 
				{
					ret = lua_tointeger(L, -1);
				}
			}
			lua_pop(L, -1);
		}
		else
		{
			lua_setglobal(L, func);
		}
	}

	return ret;
}


#endif // End of DAPLUGIN

#ifndef LUATT_EXPORTS

// Quick macro for adding functions to 
// the preloder.
#define PRELOAD(name, function) \
	lua_getglobal(L, "package"); \
	lua_getfield(L, -1, "preload"); \
	lua_pushcfunction(L, function); \
	lua_setfield(L, -2, name); \
	lua_pop(L, 2);


int __open_luasocket_socket(lua_State * L)
{
#include "LuaSocket\libluasocket/socket.lua.h"
	lua_getglobal(L, "socket");
	return 1;
}

int __open_luasocket_ftp(lua_State * L)
{
#include "LuaSocket\libluasocket/ftp.lua.h"
	lua_getglobal(L, "socket.ftp");
	return 1;
}

int __open_luasocket_http(lua_State * L)
{
#include "LuaSocket\libluasocket/http.lua.h"
	lua_getglobal(L, "socket.http");
	return 1;
}

int __open_luasocket_ltn12(lua_State * L)
{
#include "LuaSocket\libluasocket/ltn12.lua.h"
	lua_getglobal(L, "ltn12");
	return 1;
}

int __open_luasocket_mime(lua_State * L)
{
#include "LuaSocket\libluasocket/mime.lua.h"
	lua_getglobal(L, "mime");
	return 1;
}

int __open_luasocket_smtp(lua_State * L)
{
#include "LuaSocket\libluasocket/smtp.lua.h"
	lua_getglobal(L, "socket.smtp");
	return 1;
}

int __open_luasocket_tp(lua_State * L)
{
#include "LuaSocket\libluasocket/tp.lua.h"
	lua_getglobal(L, "socket.tp");
	return 1;
}

int __open_luasocket_url(lua_State * L)
{
#include "LuaSocket\libluasocket/url.lua.h"
	lua_getglobal(L, "socket.url");
	return 1;
}

int LoadLuaSocket(lua_State * L)
{
	// Preload code from LuaSocket.
	PRELOAD("socket.core", luaopen_socket_core);
	PRELOAD("mime.core", luaopen_mime_core);
	PRELOAD("socket", __open_luasocket_socket);
	PRELOAD("socket.ftp", __open_luasocket_ftp)
		PRELOAD("socket.http", __open_luasocket_http);
	PRELOAD("ltn12", __open_luasocket_ltn12);
	PRELOAD("mime", __open_luasocket_mime)
		PRELOAD("socket.smtp", __open_luasocket_smtp);
	PRELOAD("socket.tp", __open_luasocket_tp)
		PRELOAD("socket.url", __open_luasocket_url)

		// No need to register garbage collector function.
		return 0;
}
#endif
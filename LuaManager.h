/*	Lua Manager header
by Stan "sla.ro" Laurentiu Alexandru
Copyright 2010-2022 Sla Studios (http://slastudios.net)

This file is part of the LuaTT
*/

#ifndef INCLUDE_LUAMANAGER
#define INCLUDE_LUAMANAGER
#include "LuaLib/lua.hpp"
#include "scripts.h"
#include <list>
#include <vector>
#include <string>

enum GenericParam
{
	USE_NOTHING=0,
	USE_INT,
	USE_FLOAT,
	USE_STRING,
	USE_BOOL,
	USE_DOUBLE
};

struct GenericParamList
{
	GenericParam ret = USE_NOTHING;
	GenericParam p1 = USE_NOTHING;
	GenericParam p2 = USE_NOTHING;
	GenericParam p3 = USE_NOTHING;
	GenericParam p4 = USE_NOTHING;
	GenericParam p5 = USE_NOTHING;
	GenericParam p6 = USE_NOTHING;
	int defaultReturn = 1;
};


void KeyHookBaseHook(void *data);
struct ConnectionRequest;
#pragma warning(disable: 4127 4291)
void GetFiles(std::string Path, std::vector<std::string> *Data);

template <class T> class LuaScriptRegistrant : public ScriptFactory
{
public:
	LuaScriptRegistrant(const char* name, const char* param, lua_State *r_Lua, int r_uRef) : ScriptFactory(name, param)
		, str_name(name)
		, str_params(param)
		, luaHandler(r_Lua)
		, uRef(r_uRef)
	{
	}

	virtual ScriptImpClass* Create(void)
	{
		T* script = new T;
		script->SetFactory(this);
		script->Register_Auto_Save_Variables();
		script->setLua(luaHandler, uRef, this);
		scripts.push_back(script);
		return script;
	}

	std::string getName() { return str_name; }

	int getURef() { return uRef; }

	~LuaScriptRegistrant()
	{
		for (auto script : scripts)
		{
			if (script)
			{
				script->setLua(nullptr, 0, this);
				script->Destroy_Script();
			}
		}
		delete []str_name;
		delete []str_params;
		str_name = nullptr;
		str_params = nullptr;
		scripts.clear();
	}

	void removeScript(int scriptID)
	{
		if (scripts.begin() != scripts.end())
		{
			for (auto it = scripts.begin(); it != scripts.end(); ++it)
			{
				if ((*it) != nullptr)
				{
					if ((*it)->Get_ID() == scriptID)
					{
						scripts.erase(it);
						return;
					}
				}
			}
		}

	}
private:
	lua_State *luaHandler = nullptr;
	const char *str_name;
	const char *str_params;
	int uRef = 0;
	std::vector<T*> scripts;
};

class LuaScriptManager
{
public:	
	class LuaScript : public JFW_Key_Hook_Base
	{	
		void Created(GameObject *obj);
		void Destroyed(GameObject *obj);
		void Killed(GameObject *obj, GameObject *shooter);
		void Damaged(GameObject *obj, GameObject *damager, float damage);
		void Custom(GameObject *obj, int message, int param, GameObject *sender);
		void Enemy_Seen(GameObject *obj, GameObject *seen);
		void Action_Complete(GameObject *obj, int action, ActionCompleteReason reason);
		void Timer_Expired(GameObject *obj, int number);
		void Animation_Complete(GameObject *obj, const char *anim);
		void Poked(GameObject *obj, GameObject *poker);
		void Entered(GameObject *obj, GameObject *enter);
		void Exited(GameObject *obj, GameObject *exit);
		void Detach(GameObject *obj);
		void Sound_Heard(GameObject *obj, const CombatSound & sound);
		void KeyHook();

	public:
		void setLua(lua_State *lua, int uref, LuaScriptRegistrant<LuaScript> *theFactory) { L = lua; uRef = uref; myFactory = theFactory; };
		int getURef() { return uRef; }
	private:
		lua_State *L = nullptr;
		int uRef = 0;
		LuaScriptRegistrant<LuaScript> *myFactory=nullptr;
	};
	
	static std::vector<LuaScriptRegistrant<LuaScript>*> Scripts;

	static ScriptImpClass *Get_Script(GameObject *obj, int scriptID);
	static std::vector<ScriptImpClass *> Get_ScriptByName(GameObject *obj, const char *name);
	static void RegisterScriptFunctions(lua_State *L);

	static int Lua_DestroyScript_Wrap(lua_State *L);
	static int Lua_Get_Float_Parameter_Wrap(lua_State *L);
	static int Lua_Get_Int_Parameter_Wrap(lua_State *L);
	static int Lua_Get_Bool_Parameter_Wrap(lua_State *L);
	static int Lua_Get_String_Parameter_Wrap(lua_State *L);
	static int Lua_Start_Timer_Wrap(lua_State *L);
	static int Lua_Install_Hook_Warp(lua_State *L);
	static int Lua_Remove_Hook_Warp(lua_State *L);
	static int Lua_Get_Script_Name(lua_State *L);
	static int Lua_Get_Vector3_Parameter_Wrap(lua_State *L);
	
	static void Load();
	static int Register_Script(lua_State *L);
	static void Lua_DestroyScript(GameObject *obj, int scriptID);
	static float Lua_Get_Float_Parameter(GameObject *obj, int scriptID, const char *Name);
	static Vector3 Lua_Get_Vector3_Parameter(GameObject *obj, int scriptID, const char *Name);
	static int Lua_Get_Int_Parameter(GameObject *obj, int scriptID, const char *Name);
	static bool Lua_Get_Bool_Parameter(GameObject *obj, int scriptID, const char *Name);
	static const char *Lua_Get_String_Parameter(GameObject *obj, int scriptID, const char *Name);
	static void Lua_Start_Timer(GameObject *obj, int scriptID, float Time, int Number);
	static void Cleanup();
};



class LuaManager
{	
public:
	static std::vector<lua_State*> Lua;
	static std::vector<std::string> LuaNames;
	static void Load();
	static void ShowBanner();
#ifndef LUATT_EXPORTS
	static void LoadLua();
	static void LoadModule(lua_State *L, const char *data, size_t size, const char *name);
#else
	static void LoadLua(const char *LuaFile);
	static void LoadLuaPlugins();
#endif
	static void Cleanup();
	static char LuaFiles[256];
	static bool Call_Chat_Hook(int ID, int Type, const wchar_t *Msg, int Target);
	static const char *ConAc(const char *ip, const char *nick, const char *serial, const char *version, int id);
	static bool Call_Host_Hook(int PlayerID,TextMessageEnum Type,const char *Message);
	static void Call_Player_Join_Hook(int ID, const char *Nick);
	static bool Refill_Hook(GameObject *purchaser);
	static void Call_Player_Leave_Hook(int ID);
	static void Call_Level_Loaded_Hook();
	static void Call_GameOver_Hook();
	static void Call_Console_Output_Hook(const char *Output);
	static bool Report_Errors(lua_State *L, int status);
	static void Purchase_Hook(BaseControllerClass *base, GameObject *purchaser, unsigned int cost, unsigned int preset,unsigned int purchaseret,const char *data);
	static void Call_Object_Hook(void *data, GameObject *obj);
	static void Call_Think_Hook();
	static int Purchase_Hook2(BaseControllerClass *base,GameObject *purchaser,unsigned int cost,unsigned int preset,const char *data);
	static void CallInvoke(const char *Function, const char *Arg);
	static bool RadioHook(int PlayerType, int PlayerID, int AnnouncementID, int IconID, AnnouncementEnum AnnouncementType);
	static void UnloadingLua();
	static void Reload_All();
	static void KilledHook(int obj,int shooter);
	static void ZoneEnterHook(int obj, int enter);
	static void ZoneExitHook(int obj, int exit);
	static void DestroyedHook(int obj);
	static void CustomHook(int obj,int message,int param,int sender);
	static void DamageHook(int obj,int damager,float damage);
	static void PokedHook(int obj,int damager);
	static void KeyHook(int obj, const char *key, const char *callback);
	static bool Stock_Damage(PhysicalGameObj *Damager, PhysicalGameObj *Target, float Damage, uint Warhead);
	static bool TT_Damage_Hook(PhysicalGameObj* damager, PhysicalGameObj* target, const AmmoDefinitionClass* ammo, const char* bone);
	static void Call_On_Dialog(int PlayerID, int DialogID, int ControlID, DialogMessageType MessageType);
    static void Adress_IP(const char *addr);
	static bool Reload_Flag;

#ifdef DAPLUGIN // New DA Hooks
	static bool DA_Suicide_Event(cPlayer *Player);
	static bool DA_Refill_Event(cPlayer *Player);
	static bool DA_Team_Change_Request_Event(cPlayer *Player);
	static void DA_Log_Event(const char *Header, const char *Output);
	static void DA_Player_Pre_Join_Event(ConnectionRequest& Request);
	static const char* DA_Connection_Request_Event(ConnectionRequest& Request, WideStringClass& RefusalMessage);
	static bool DA_Damage_Request_Event(DamageableGameObj *Victim, ArmedGameObj *Damager, float &Damage, unsigned int &Warhead, float Scale, int Type);
#endif // End of DAPLUGIN
	static int GenericHelper_getTotalArguments(GenericParamList params);
	static int Generic_Hook(const char *func, GenericParamList params, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6);
};

class Lua_Script_Hook : public ScriptImpClass 
{
 void Killed(GameObject *obj, GameObject *shooter);
 void Destroyed(GameObject *obj);
 void Custom(GameObject *obj,int message,int param,GameObject *sender);
 void Damaged(GameObject *obj,GameObject *damager,float damage);
 void Poked(GameObject *obj,GameObject *poker);
 void Entered(GameObject* obj, GameObject* enter);
 void Exited(GameObject* obj, GameObject* exit);
};

class Lua_Key_Hook : public JFW_Key_Hook_Base 
{
	void Created(GameObject *obj);
	void KeyHook();
	int myobj;
};

// LuaConsole commands? TODO

#ifndef LUATT_EXPORTS
int LoadLuaSocket(lua_State * L);
int __open_luasocket_socket(lua_State * L);
int __open_luasocket_ftp(lua_State * L);
int __open_luasocket_http(lua_State * L);
int __open_luasocket_ltn12(lua_State * L);
int __open_luasocket_mime(lua_State * L);
int __open_luasocket_smtp(lua_State * L);
int __open_luasocket_tp(lua_State * L);
int __open_luasocket_url(lua_State * L);
#endif

#endif
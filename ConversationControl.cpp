#include "General.h"
#include "engine.h"
#include "ConversationClass.h"
#include "ConversationControl.h"
#include "SoldierGameObj.h"
#include "SoldierGameObjDef.h"
#include "GameObjManager.h"
#include "DefinitionMgrClass.h"

#define RENEGADE_FUNCTION  __declspec(naked)

__declspec(dllexport) int RenegadeStockVersionSelector()
{
	return 1;
};

__declspec(dllexport) void* HookupStock(void* a, void* b, void* c, void* patch_start, void* patch_end, int (*version_selector)())
{
	static const char relJumpOp = '\xE9';					
	static const char breakpointOp = '\xCC';

	int versel = version_selector(); 
	void* function = (versel == 0) ? a : ((versel == 1) ? b : ((versel == 2) ? c : 0));
	TT_ASSERT(function != 0);

	char* function_rel = (char*)((char *)function - (char *)patch_start - 5);
	size_t patch_size = (char*) patch_end - (char*) patch_start;
	char* current_addr = (char*) patch_start;

	DWORD oldProtection;
	HANDLE process = GetCurrentProcess();
	VirtualProtectEx(process, patch_start, patch_size, PAGE_EXECUTE_READWRITE, &oldProtection);

	WriteProcessMemory(process, current_addr, &relJumpOp, sizeof(relJumpOp), NULL);
	current_addr += sizeof(relJumpOp);
	WriteProcessMemory(process, current_addr, &function_rel, sizeof(function_rel), NULL);
	current_addr += sizeof(function_rel);
	
	for (; current_addr < patch_end; current_addr += sizeof(breakpointOp))
	{
		WriteProcessMemory(process, current_addr, &breakpointOp, sizeof(breakpointOp), NULL);
	};

	VirtualProtectEx(process, patch_start, patch_size, oldProtection, &oldProtection);
	FlushInstructionCache(process, patch_start, patch_size);

	return function;
};

#define RENEGADESTOCK(client, server)				\
{													\
	__asm AT3PatchStart:							\
	__asm push ecx									\
	__asm push edx									\
	__asm push RenegadeStockVersionSelector			\
	__asm push offset AT3PatchEnd					\
	__asm push offset AT3PatchStart					\
	__asm push 0									\
	__asm push server								\
	__asm push client								\
	__asm call HookupStock							\
	__asm AT3PatchEnd:								\
	__asm add esp, 24								\
	__asm pop edx									\
	__asm pop ecx									\
	__asm jmp eax									\
}

RENEGADE_FUNCTION
ConversationClass *ConversationControl::Find_Conversation(int ConversationID)
#if RENEGADE_VERSION == 1038
RENEGADESTOCK(0,0x00556160);
#else
RENEGADESTOCK(0x006D6C90, 0x006D6530);
#endif

RENEGADE_FUNCTION
ConversationClass *ConversationControl::Find_Conversation(const char *Conversation_Name)
#if RENEGADE_VERSION == 1038
RENEGADESTOCK(0,0x00556100);
#else
RENEGADESTOCK(0x006D6C90, 0x006D6530);
#endif

void AI_Dialogue_Script::Created(GameObject *obj)
{
	if(!obj->As_SoldierGameObj())
	{
		Destroy_Script();
		return;
	}

	this->_Acting = false;
	this->_LastState = Get_Current_State();
	this->_SawEnemy = false;
	this->_SawFriend = false;
	this->_EnemyKilled = false;

	if(!Commands->Is_A_Star(obj))
	{
		//Commands->Enable_Enemy_Seen(obj, true);
		Commands->Start_Timer(obj, this, 1, 1);
	}
}

void AI_Dialogue_Script::Damaged(GameObject *obj, GameObject *damager, float amount)
{
	if(obj->Is_Hibernating() || !damager || amount <= 0 || amount>=Get_Hitpoints(obj))
		return;

	if(obj->As_DamageableGameObj()->Is_Enemy(damager->As_DamageableGameObj()))
	{
		Act(TAKE_DAMAGE_FROM_ENEMY);
		Commands->Send_Custom_Event(obj, damager, CUSTOM_EVENT_DAMAGED_ENEMY, 0, 0);
	}
	else
	{
		Act(TAKE_DAMAGE_FROM_FRIEND);
		Commands->Send_Custom_Event(obj, damager, CUSTOM_EVENT_DAMAGED_FRIEND, 0, 0);
	}
}

void AI_Dialogue_Script::Killed(GameObject *obj, GameObject *killer)
{
	if(obj->Is_Hibernating() || !killer)
		return;

	if(obj->As_DamageableGameObj()->Is_Enemy(killer->As_DamageableGameObj()))
		Commands->Send_Custom_Event(obj, killer, CUSTOM_EVENT_KILLED_ENEMY, 0, 0);
	else
		Commands->Send_Custom_Event(obj, killer, CUSTOM_EVENT_KILLED_FRIEND, 0, 0);

	this->_Acting=false;
	Act(DIE);
}

void AI_Dialogue_Script::Poked(GameObject *obj, GameObject *poker)
{
	if(obj->Is_Hibernating())
		return;

	if(this->_LastState == AI_STATE_IDLE)
		Act(POKE_IDLE);
	else if(this->_LastState == AI_STATE_SEARCH)
		Act(POKE_SEARCH);
	else if(this->_LastState == AI_STATE_COMBAT)
		Act(POKE_COMBAT);
}

void AI_Dialogue_Script::Timer_Expired(GameObject *obj, int number)
{
	if(number == 1)
	{
		State_Changed(this->_LastState, Get_Current_State());

		if(!obj->Is_Hibernating() && !this->_SawFriend)
		{
			for(SLNode<SoldierGameObj> *SoldierNode = GameObjManager::StarGameObjList.Head(); SoldierNode; SoldierNode = SoldierNode->Next())
			{
				SoldierGameObj *Soldier = SoldierNode->Data();
				if(Soldier && Soldier->Is_Human_Controlled() && !Soldier->Is_Enemy(obj->As_SmartGameObj()) && obj->As_SmartGameObj()->Is_Obj_Visible(Soldier))
				{
					RenderObjClass *Model = Soldier->Peek_Model();
					if(Model && Model->Is_Hidden())
						continue;

					Act(SAW_FRIEND);
					break;
				}
			}
		}

		Commands->Start_Timer(obj, this, 2, 1);
	}
	else if(number == 2)
	{
		this->_Acting = false;
		if(this->_EnemyKilled)
		{
			Act(KILLED_ENEMY);
			this->_EnemyKilled=false;
		}
	}

	else if(number == 3)
	{
		this->_SawEnemy = false;
	}
	else if(number == 4)
	{
		this->_SawFriend = false;
	}
}

void AI_Dialogue_Script::Custom(GameObject *obj, int type, int param, GameObject *sender)
{
	if(type == CUSTOM_EVENT_DAMAGED_ENEMY)
		Act(DAMAGE_ENEMY);
	else if(type == CUSTOM_EVENT_DAMAGED_FRIEND)
		Act(DAMAGE_FRIEND);
	else if(type == CUSTOM_EVENT_KILLED_ENEMY)
	{
		if(!this->_Acting)
			Act(KILLED_ENEMY);
		else
			this->_EnemyKilled=true;
	}
	else if(type == CUSTOM_EVENT_KILLED_FRIEND)
		Act(KILLED_FRIEND);
}

void AI_Dialogue_Script::Enemy_Seen(GameObject *obj, GameObject *enemy)
{
	if(!obj->Is_Hibernating() && !this->_SawEnemy)
	{
		//Commands->Enable_Enemy_Seen(obj, false);

		Act(SAW_ENEMY);
	}
}

void AI_Dialogue_Script::State_Changed(SoldierAIState From, SoldierAIState To)
{
	if(From != To)
	{
		this->_LastState = To;

		if(!Owner()->Is_Hibernating())
		{
			if(From == AI_STATE_IDLE && To == AI_STATE_COMBAT)
				Act(IDLE_TO_COMBAT);
			else if(From == AI_STATE_IDLE && To == AI_STATE_SEARCH)
				Act(IDLE_TO_SEARCH);
			else if(From == AI_STATE_SEARCH && To == AI_STATE_COMBAT)
				Act(SEARCH_TO_COMBAT);
			else if(From == AI_STATE_SEARCH && To == AI_STATE_IDLE)
				Act(SEARCH_TO_IDLE);
			else if(From == AI_STATE_COMBAT && To == AI_STATE_SEARCH)
				Act(COMBAT_TO_SEARCH);
			else if(From == AI_STATE_COMBAT && To == AI_STATE_IDLE)
				Act(COMBAT_TO_IDLE);
		}
	}
}

SoldierAIState AI_Dialogue_Script::Get_Current_State()
{
	SoldierGameObj *Soldier = Owner()->As_SoldierGameObj();
	if(Soldier)
		return Soldier->Get_AI_State();
	else
		return this->_LastState;
}

void AI_Dialogue_Script::Act(DialogueType Type)
{
	if(Owner()->Is_Hibernating() || Type < 0 || Type >= DIALOGUE_TYPE_NONE || this->_Acting || Commands->Is_A_Star(Owner()))
		return;

	if(Owner()->As_SoldierGameObj()->Is_Dead() && Type != DIE )
	{
		return;
	}

	if(Type == SAW_FRIEND)
	{
		this->_SawFriend = true;
		Commands->Start_Timer(Owner(),this,3,4);
	}
	else if(Type == SAW_ENEMY)
	{
		this->_SawEnemy = true;
		Commands->Start_Timer(Owner(),this,3,3);
	}

	ConversationClass *Conversation = Get_Random_Conversation(Get_Dialogue(Type));
	if(!Conversation)
		return;

	this->_Acting = true;
	char sound[100];
	sprintf(sound, "%s.wav", Conversation->Get_Name());
	Commands->Create_3D_WAV_Sound_At_Bone(sound, Owner(), "c head");
	
	Commands->Start_Timer(Owner(), this, 2, 2);
}

DialogueClass &AI_Dialogue_Script::Get_Dialogue(DialogueType Type)
{
	SoldierGameObjDef *SoldierDef = (SoldierGameObjDef *)DefinitionMgrClass::Find_Definition(Owner()->As_SoldierGameObj()->Get_Definition().Get_ID(), false);

	return SoldierDef->Get_Dialog_List()[Type];
}

ConversationClass *AI_Dialogue_Script::Get_Random_Conversation(DialogueClass &Dialogue)
{
	// If it complaints about not having permissions to read the values of the DialogueClass. Make it temporary public and then build this plugin. You do not need to build scripts.dll or change the dll in your server folder to the new one.
	SimpleDynVecClass<int> ConversationIDs;
	SimpleDynVecClass<float> Weights;

	float TotalWeight = 0;

	int OptionCount = Dialogue.Get_Option_List().Count();
	for(int x = 0; x < OptionCount; x++) 
	{
		DialogueOptionClass *CurrentOption = Dialogue.Get_Option_List()[x];
		if(CurrentOption)
		{
			ConversationIDs.Add(CurrentOption->Get_Conversation_ID());
			Weights.Add(CurrentOption->Get_Weight());

			TotalWeight += CurrentOption->Get_Weight();
		}
	}

	float Rand = Commands->Get_Random(0, TotalWeight + Dialogue.Get_Silence_Weight());
	if(Rand >= Dialogue.Get_Silence_Weight())
	{
		Rand -= Dialogue.Get_Silence_Weight(); 
		float CurrentWeight = 0;

		OptionCount = Weights.Count();
		for(int x = 0; x < OptionCount; x++) 
		{
			CurrentWeight += Weights[x]; 
			if(Rand <= CurrentWeight)
				return ConversationControl::Find_Conversation(ConversationIDs[x]);
		}
	}

	return 0;
}

ScriptRegistrant<AI_Dialogue_Script> AI_Dialogue_Script_Registrant("AI_Dialogue","");
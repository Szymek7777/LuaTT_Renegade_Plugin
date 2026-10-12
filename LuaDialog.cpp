/*	Lua Dialog wrapper
Wraps Renegade Dialog to Lua
by Stan "sla.ro" Laurentiu Alexandru
Copyright 2010-2022 Sla Studios (http://slastudios.net)

This file is part of the LuaTT
*/

#include "General.h"
#include "luatt.h"
#include "LuaDialog.h"

#define method(class, name) {#name, class::name}

const luaL_Reg LuaScriptedControlClass::methods[] = {
	method(LuaScriptedControlClass, Get_Control_ID),
	method(LuaScriptedControlClass, Get_Location),
	method(LuaScriptedControlClass, Get_Size),
	method(LuaScriptedControlClass, Is_Enabled),
	method(LuaScriptedControlClass, Is_Dirty),
	method(LuaScriptedControlClass, Set_Location),
	method(LuaScriptedControlClass, Set_Size),
	method(LuaScriptedControlClass, Set_Enabled),
	method(LuaScriptedControlClass, Set_Dirty),
	method(LuaScriptedControlClass, Get_Label_Text),
	method(LuaScriptedControlClass, Get_Text_Color),
	method(LuaScriptedControlClass, Get_Style),
	method(LuaScriptedControlClass, Get_Orientation),
	method(LuaScriptedControlClass, Set_Label_Text),
	method(LuaScriptedControlClass, Set_Text_Color),
	method(LuaScriptedControlClass, Set_Style),
	method(LuaScriptedControlClass, Set_Orientation),
	method(LuaScriptedControlClass, Get_Image_Name),
	method(LuaScriptedControlClass, Set_Image_Name),
	method(LuaScriptedControlClass, Set_Button_Text),
	method(LuaScriptedControlClass, Set_Button_Up_Image_Name),
	method(LuaScriptedControlClass, Set_Button_Style),
	method(LuaScriptedControlClass, Set_Button_Down_Image_Name),
	method(LuaScriptedControlClass, Get_Button_Style),
	method(LuaScriptedControlClass, Get_Button_Down_Image_Name),
	method(LuaScriptedControlClass, Get_Button_Up_Image_Name),
	method(LuaScriptedControlClass, Get_Button_Text),
	method(LuaScriptedControlClass, Get_Text_Limit),
	method(LuaScriptedControlClass, Is_Password_Field),
	method(LuaScriptedControlClass, Is_Numeric_Field),
	method(LuaScriptedControlClass, Is_Automatic_Horizontal_Scroll),
	method(LuaScriptedControlClass, Get_Text_Length),
	method(LuaScriptedControlClass, Set_Checked),
	method(LuaScriptedControlClass, Is_Checked),
	method(LuaScriptedControlClass, Get_Text),
	method(LuaScriptedControlClass, Set_Text),
	method(LuaScriptedControlClass, Set_Automatic_Horizontal_Scroll),
	method(LuaScriptedControlClass, Set_Numeric_Field),
	method(LuaScriptedControlClass, Set_Password_Field),
	method(LuaScriptedControlClass, Set_Text_Limit),
	method(LuaScriptedControlClass, Get_Selected_Index),
	method(LuaScriptedControlClass, Get_Item_Count),
	method(LuaScriptedControlClass, Get_Selected_Item_Text),
	method(LuaScriptedControlClass, Set_Selected_Index),
	method(LuaScriptedControlClass, Remove_Item),
	method(LuaScriptedControlClass, Insert_Item),
	method(LuaScriptedControlClass, Clear_Items),
	method(LuaScriptedControlClass, Get_Minimum),
	method(LuaScriptedControlClass, Get_Maximum),
	method(LuaScriptedControlClass, Get_Value),
	method(LuaScriptedControlClass, Set_Value),
	method(LuaScriptedControlClass, Set_Maximum),
	method(LuaScriptedControlClass, Set_Minimum),
	method(LuaScriptedControlClass, Get_Progress),
	method(LuaScriptedControlClass, Get_Step_Count),
	method(LuaScriptedControlClass, Increment_Value),
	method(LuaScriptedControlClass, Decrement_Value),
	method(LuaScriptedControlClass, Set_Progress),
	method(LuaScriptedControlClass, Set_Step_Count),
	method(LuaScriptedControlClass, Get_Item),
	method(LuaScriptedControlClass, Add_Item),

	{ 0, 0 }
};

void LuaScriptedControlClass::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);

	luaL_newmetatable(L, "ScriptedControlClass");
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	/*lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, DeleteInstance);
	lua_settable(L, metatable);
	*/

	lua_pop(L, 1);

	luaL_openlib(L, 0, methods, 0);
	lua_pop(L, 1);

	//lua_register(L, "ScriptedControlClass", RegisterTable);
}


int LuaScriptedControlClass::Get_Control_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushnumber(L, a->Get_Control_ID());
		return 1;
	}
	return 0;
}

int LuaScriptedControlClass::Get_Location(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		Vector2 pos = a->Get_Location();
		lua_pushnumber(L, pos.X);
		lua_pushnumber(L, pos.Y);
		return 2;
	}
	return 0;
}

int LuaScriptedControlClass::Get_Size(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		Vector2 sz = a->Get_Size();
		lua_pushnumber(L, sz.X);
		lua_pushnumber(L, sz.Y);
		return 2;
	}
	return 0;
}

int LuaScriptedControlClass::Is_Enabled(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushboolean(L, a->Is_Enabled());
		return 1;
	}
	return 0;
}

int LuaScriptedControlClass::Is_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushboolean(L, a->Is_Dirty());
		return 1;
	}
	return 0;
}

int LuaScriptedControlClass::Set_Location(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int x = static_cast<int>(luaL_checknumber(L, 2));
		int y = static_cast<int>(luaL_checknumber(L, 3));
		a->Set_Location(x, y);
	}
	return 0;
}

int LuaScriptedControlClass::Set_Size(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int w = static_cast<int>(luaL_checknumber(L, 2));
		int h = static_cast<int>(luaL_checknumber(L, 3));
		a->Set_Size(w, h);
	}
	return 0;
}

int LuaScriptedControlClass::Set_Enabled(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		bool bl = lua_toboolean(L, 2);
		a->Set_Enabled(bl);
	}
	return 0;
}

int LuaScriptedControlClass::Set_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		bool bl = lua_toboolean(L, 2);
		a->Set_Dirty(bl);
	}
	return 0;
}

int LuaScriptedControlClass::Get_Label_Text(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			auto text = label->Get_Label_Text();
			StringClass str;
			text.Convert_To(str);
			lua_pushstring(L, str.Peek_Buffer());
			return 1;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Text_Color(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			auto vec = label->Get_Text_Color();
			lua_pushnumber(L, vec.X);
			lua_pushnumber(L, vec.Y);
			lua_pushnumber(L, vec.Z);
			return 3;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Style(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			auto is = label->Get_Style();
			lua_pushnumber(L, is);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Orientation(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			auto is = label->Get_Orientation();
			lua_pushnumber(L, is);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Label_Text(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			const char *text = lua_tostring(L, 2);
			WideStringClass wch(text);
			label->Set_Label_Text(wch);
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Text_Color(lua_State *L)
{
	if (lua_gettop(L) < 4) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			label->Set_Text_Color(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Style(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			label->Set_Style(static_cast<TextStyle>(lua_tointeger(L, 2)));
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Orientation(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedLabelControlClass();
		if (label != nullptr)
		{
			int num = lua_tointeger(L, 2);
			label->Set_Orientation(static_cast<TextOrientation>(num));
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Image_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto image = a->As_ScriptedImageControlClass();
		if (image != nullptr)
		{
			auto name = image->Get_Image_Name();
			lua_pushstring(L, name);
			return 1;
		}
	}
	return 0;
}


int LuaScriptedControlClass::Set_Image_Name(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto image = a->As_ScriptedImageControlClass();
		if (image != nullptr)
		{
			image->Set_Image_Name(lua_tostring(L, 2));
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Button_Text(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto button = a->As_ScriptedButtonControlClass();
		if (button != nullptr)
		{
			auto text = button->Get_Button_Text();
			StringClass str;
			text.Convert_To(str);
			lua_pushstring(L, str.Peek_Buffer());
			return 1;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Button_Up_Image_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto button = a->As_ScriptedButtonControlClass();
		if (button != nullptr)
		{
			auto text = button->Get_Button_Up_Image_Name();
			lua_pushstring(L, text);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Button_Down_Image_Name(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto button = a->As_ScriptedButtonControlClass();
		if (button != nullptr)
		{
			auto text = button->Get_Button_Down_Image_Name();
			lua_pushstring(L, text);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Button_Style(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto button = a->As_ScriptedButtonControlClass();
		if (button != nullptr)
		{
			auto num = button->Get_Button_Style();
			lua_pushnumber(L, num);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Button_Text(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedButtonControlClass();
		if (label != nullptr)
		{
			const char *text = lua_tostring(L, 2);
			WideStringClass wch(text);
			label->Set_Button_Text(wch);
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Button_Down_Image_Name(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedButtonControlClass();
		if (label != nullptr)
		{
			const char *text = lua_tostring(L, 2);
			label->Set_Button_Down_Image_Name(text);
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Button_Up_Image_Name(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedButtonControlClass();
		if (label != nullptr)
		{
			const char *text = lua_tostring(L, 2);
			label->Set_Button_Up_Image_Name(text);
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Button_Style(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedButtonControlClass();
		if (label != nullptr)
		{
			int style = lua_tointeger(L, 2);
			label->Set_Button_Style(static_cast<ButtonStyle>(style));
		}
	}
	return 0;
}

int LuaScriptedControlClass::Set_Text(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto textarea = a->As_ScriptedTextAreaControlClass();
		if (textarea != nullptr)
		{
			const char *text = lua_tostring(L, 2);
			WideStringClass wch(text);
			textarea->Set_Text(wch);
		}
		else
		{
			auto checkbox = a->As_ScriptedCheckBoxControlClass();
			if (checkbox)
			{
				const char *text = lua_tostring(L, 2);
				WideStringClass wch(text);
				checkbox->Set_Text(wch);
			}
		}

	}
	return 0;
}

int LuaScriptedControlClass::Get_Text(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto textarea = a->As_ScriptedTextAreaControlClass();
		if (textarea != nullptr)
		{
			auto w = textarea->Get_Text();
			StringClass str;
			w.Convert_To(str);
			lua_pushstring(L, str.Peek_Buffer());
			return 1;
		}
		else
		{
			auto checkbox = a->As_ScriptedCheckBoxControlClass();
			if (checkbox)
			{
				auto w = checkbox->Get_Text();
				StringClass str;
				w.Convert_To(str);
				lua_pushstring(L, str.Peek_Buffer());
				return 1;
			}
		}

	}
	return 0;
}

int LuaScriptedControlClass::Insert_Item(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto label = a->As_ScriptedComboBoxControlClass();
		if (label != nullptr)
		{
			const char *text = lua_tostring(L, 2);
			int index = lua_tointeger(L, 3);
			WideStringClass wch(text);
			label->Insert_Item(index, wch);
		}
	}
	return 0;
}

int LuaScriptedControlClass::Get_Item(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedControlClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto button = a->As_ScriptedComboBoxControlClass();
		if (button != nullptr)
		{
			int index = lua_tointeger(L, 2);
			auto text = button->Get_Item(index);
			StringClass str;
			text.Convert_To(str);
			lua_pushstring(L, str.Peek_Buffer());
			return 1;
		}
	}
	return 0;
}

/************************ ScriptedDialogClass *************************/

const luaL_Reg LuaScriptedDialogClass::methods[] = {
	method(LuaScriptedDialogClass, Get_Dialog_Type),
	method(LuaScriptedDialogClass, Get_Dialog_ID),
	method(LuaScriptedDialogClass, Get_Client_ID),
	method(LuaScriptedDialogClass, Is_Dirty),
	method(LuaScriptedDialogClass, Set_Dirty),
	method(LuaScriptedDialogClass, Focus_Control),
	method(LuaScriptedDialogClass, Add_Control),
	method(LuaScriptedDialogClass, Create_Control),
	method(LuaScriptedDialogClass, Find_Control),
	method(LuaScriptedDialogClass, Get_Control_Count),
	method(LuaScriptedDialogClass, Get_Control_Index),
	method(LuaScriptedDialogClass, Remove_Control),
	method(LuaScriptedDialogClass, Clear_Controls),

	method(LuaScriptedDialogClass, Get_Orientation),
	method(LuaScriptedDialogClass, Get_Dialog_Size),
	method(LuaScriptedDialogClass, Get_Dialog_Location),
	method(LuaScriptedDialogClass, Get_Dialog_Title),
	method(LuaScriptedDialogClass, Set_Dialog_Size),
	method(LuaScriptedDialogClass, Set_Dialog_Location),
	method(LuaScriptedDialogClass, Set_Dialog_Title),
	method(LuaScriptedDialogClass, Set_Orientation),
	{ 0, 0 }
};

void LuaScriptedDialogClass::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);

	luaL_newmetatable(L, "ScriptedDialogClass");
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	/*lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, DeleteInstance);
	lua_settable(L, metatable);
	*/

	lua_pop(L, 1);

	luaL_openlib(L, 0, methods, 0);
	lua_pop(L, 1);

	//lua_register(L, "ScriptedControlClass", RegisterTable);
}

int LuaScriptedDialogClass::Get_Dialog_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto dialog = a->Get_Dialog_Type();
		lua_pushnumber(L, dialog);
		return 1;
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Dialog_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto dialog = a->Get_Dialog_ID();
		lua_pushnumber(L, dialog);
		return 1;
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Client_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto dialog = a->Get_Client_ID();
		lua_pushnumber(L, dialog);
		return 1;
	}
	return 0;
}

int LuaScriptedDialogClass::Is_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto dialog = a->Is_Dirty();
		lua_pushboolean(L, dialog);
		return 1;
	}
	return 0;
}

int LuaScriptedDialogClass::Set_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		a->Set_Dirty(lua_toboolean(L, 2));
		return 1;
	}
	return 0;
}

int LuaScriptedDialogClass::Add_Control(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		ScriptedControlClass *control = LuaScriptedControlClass::GetInstance(L, 2);
		if (control != nullptr)
		{
			a->Add_Control(control);
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Focus_Control(lua_State* L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass* a = GetInstance(L, 1);
	if (a != nullptr)
	{
		ScriptedControlClass* control = LuaScriptedControlClass::GetInstance(L, 2);
		if (control != nullptr)
		{
			a->Focus_Control(control);
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Create_Control(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int type = lua_tointeger(L, 2);
		auto control = a->Create_Control(static_cast<ControlType>(type));
		if (control != nullptr)
		{
			LuaScriptedControlClass::Wrap(L, control);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Find_Control(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int id = lua_tointeger(L, 2);
		auto control = a->Find_Control(id);
		if (control != nullptr)
		{
			LuaScriptedControlClass::Wrap(L, control);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Control_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto dialog = a->Get_Control_Count();
		lua_pushnumber(L, dialog);
		return 1;
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Control_Index(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int id = lua_tointeger(L, 2);
		auto control = a->Get_Control_Index(id);
		if (control != nullptr)
		{
			LuaScriptedControlClass::Wrap(L, control);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Remove_Control(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		void *ud = luaL_checkudata(L, 2, "ScriptedControlClass");
		if (!ud)
		{
			int id = lua_tointeger(L, 2);
			a->Remove_Control(id);
		}
		else
		{
			ScriptedControlClass *control = LuaScriptedControlClass::GetInstance(L, 2);
			if (control)
				a->Remove_Control(control);
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Clear_Controls(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		a->Clear_Controls();
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Orientation(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			auto control = popup->Get_Orientation();
			lua_pushnumber(L, control);
			return 1;
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Dialog_Size(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			auto control = popup->Get_Dialog_Size();
			lua_pushnumber(L, control.X);
			lua_pushnumber(L, control.Y);
			return 2;
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Dialog_Location(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			auto control = popup->Get_Dialog_Location();
			lua_pushnumber(L, control.X);
			lua_pushnumber(L, control.Y);
			return 2;
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Get_Dialog_Title(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			auto title = popup->Get_Dialog_Title();
			StringClass str;
			title.Convert_To(str);
			lua_pushstring(L, str.Peek_Buffer());
			return 1;
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Set_Dialog_Size(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			int w = lua_tointeger(L, 2);
			int h = lua_tointeger(L, 3);
			popup->Set_Dialog_Size(w, h);
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Set_Dialog_Location(lua_State *L)
{
	if (lua_gettop(L) < 3) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			int x = lua_tointeger(L, 2);
			int y = lua_tointeger(L, 3);
			popup->Set_Dialog_Location(x, y);
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Set_Dialog_Title(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			const char *text = lua_tostring(L, 2);
			WideStringClass title(text);
			popup->Set_Dialog_Title(title);
		}
	}
	return 0;
}

int LuaScriptedDialogClass::Set_Orientation(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	ScriptedDialogClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		auto popup = a->As_ScriptedPopupDialogClass();
		if (popup != nullptr)
		{
			int orientation = lua_tointeger(L, 2);
			popup->Set_Orientation(static_cast<DialogOrientation>(orientation));
		}
	}
	return 0;
}


/************************ LuaHUDSurfaceClass *************************/

const luaL_Reg LuaHUDSurfaceClass::methods[] = {
	method(LuaHUDSurfaceClass, Get_Surface_ID),
	method(LuaHUDSurfaceClass, Get_Client_ID),
	method(LuaHUDSurfaceClass, Get_Boundary_Area),
	method(LuaHUDSurfaceClass, Get_Aspect_Ratio),
	method(LuaHUDSurfaceClass, Get_Surface_Texture),
	method(LuaHUDSurfaceClass, Set_Surface_Texture),
	method(LuaHUDSurfaceClass, Is_Dirty),
	method(LuaHUDSurfaceClass, Set_Dirty),

	method(LuaHUDSurfaceClass, Create_Element),
	method(LuaHUDSurfaceClass, Find_Element),
	method(LuaHUDSurfaceClass, Get_Element_Count),
	method(LuaHUDSurfaceClass, Get_Element_Index),
	method(LuaHUDSurfaceClass, Remove_Element),
	method(LuaHUDSurfaceClass, Clear_Elements),
	{ 0, 0 }
};

void LuaHUDSurfaceClass::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);

	luaL_newmetatable(L, "HUDSurfaceClass");
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);
	lua_pop(L, 1);

	luaL_openlib(L, 0, methods, 0);
	lua_pop(L, 1);
}

int LuaHUDSurfaceClass::Get_Surface_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushnumber(L, a->Get_Surface_ID());
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Get_Client_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushnumber(L, a->Get_Client_ID());
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Get_Boundary_Area(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		RectClass rec = a->Get_Boundary_Area();
		lua_pushnumber(L, rec.Left);
		lua_pushnumber(L, rec.Top);
		lua_pushnumber(L, rec.Right);
		lua_pushnumber(L, rec.Bottom);
		return 4;
	}
	return 0;
}

int LuaHUDSurfaceClass::Get_Aspect_Ratio(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushnumber(L, a->Get_Aspect_Ratio());
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Get_Surface_Texture(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushstring(L, a->Get_Surface_Texture());
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Set_Surface_Texture(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		a->Set_Surface_Texture(lua_tostring(L, 2));
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Is_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushboolean(L, a->Is_Dirty());
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Set_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		a->Set_Dirty(lua_toboolean(L, 2));
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Create_Element(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		HUDElementType type = static_cast<HUDElementType>(lua_tointeger(L, 2));
		HUDElementClass *obj = a->Create_Element(type);
		switch (type)
		{
			case HUD_ELEMENT_LINE:
				LuaHUDLineElementClass::Wrap(L, static_cast<HUDLineElementClass*>(obj));
				return 1;
			case HUD_ELEMENT_RECTANGLE:
				LuaHUDRectangleElementClass::Wrap(L, static_cast<HUDRectangleElementClass*>(obj));
				return 1;
			case HUD_ELEMENT_OUTLINE:
				LuaHUDOutlineElementClass::Wrap(L, static_cast<HUDOutlineElementClass*>(obj));
				return 1;
			case HUD_ELEMENT_TEXT:
				LuaHUDTextElementClass::Wrap(L, static_cast<HUDTextElementClass*>(obj));
				return 1;
			default:
				break;
		}
	}
	return 0;
}

int LuaHUDSurfaceClass::Find_Element(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int id = lua_tointeger(L, 2);
		HUDElementClass *obj = a->Find_Element(id);
		HUDElementType type = obj->Get_Element_Type();
		switch (type)
		{
		case HUD_ELEMENT_LINE:
			LuaHUDLineElementClass::Wrap(L, static_cast<HUDLineElementClass*>(obj));
			return 1;
		case HUD_ELEMENT_RECTANGLE:
			LuaHUDRectangleElementClass::Wrap(L, static_cast<HUDRectangleElementClass*>(obj));
			return 1;
		case HUD_ELEMENT_OUTLINE:
			LuaHUDOutlineElementClass::Wrap(L, static_cast<HUDOutlineElementClass*>(obj));
			return 1;
		case HUD_ELEMENT_TEXT:
			LuaHUDTextElementClass::Wrap(L, static_cast<HUDTextElementClass*>(obj));
			return 1;
		default:
			break;
		}
	}
	return 0;
}

int LuaHUDSurfaceClass::Get_Element_Count(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int count = a->Get_Element_Count();
		lua_pushnumber(L, count);
		return 1;
	}
	return 0;
}

int LuaHUDSurfaceClass::Get_Element_Index(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int id = lua_tointeger(L, 2);
		HUDElementClass *obj = a->Get_Element_Index(id);
		HUDElementType type = obj->Get_Element_Type();
		switch (type)
		{
		case HUD_ELEMENT_LINE:
			LuaHUDLineElementClass::Wrap(L, static_cast<HUDLineElementClass*>(obj));
			return 1;
		case HUD_ELEMENT_RECTANGLE:
			LuaHUDRectangleElementClass::Wrap(L, static_cast<HUDRectangleElementClass*>(obj));
			return 1;
		case HUD_ELEMENT_OUTLINE:
			LuaHUDOutlineElementClass::Wrap(L, static_cast<HUDOutlineElementClass*>(obj));
			return 1;
		case HUD_ELEMENT_TEXT:
			LuaHUDTextElementClass::Wrap(L, static_cast<HUDTextElementClass*>(obj));
			return 1;
		default:
			break;
		}
	}
	return 0;
}

int LuaHUDSurfaceClass::Remove_Element(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		int type = lua_type(L, 2);
		if (type == LUA_TNUMBER)
		{
			int id = lua_tointeger(L, 2);
			a->Remove_Element(id);
		}
		else if (type == LUA_TUSERDATA)
		{
			HUDElementClass *element = *reinterpret_cast<HUDElementClass **>(lua_touserdata(L, 2));
			if (element)
			{
				a->Remove_Element(element);
			}
		}
	}
	return 0;
}

int LuaHUDSurfaceClass::Clear_Elements(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDSurfaceClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		a->Clear_Elements();
	}
	return 0;
}

/************************ LuaHUDElementClass *************************/


void LuaHUDElementClass::Register(lua_State *L)
{
	LuaHUDLineElementClass::Register(L);
	LuaHUDRectangleElementClass::Register(L);
	LuaHUDOutlineElementClass::Register(L);
	LuaHUDTextElementClass::Register(L);
}


int LuaHUDElementClass::Get_Element_Type(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDElementClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushnumber(L, a->Get_Element_Type());
		return 1;
	}
	return 0;
}

int LuaHUDElementClass::Get_Element_ID(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDElementClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushnumber(L, a->Get_Element_ID());
		return 1;
	}
	return 0;
}

int LuaHUDElementClass::Is_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDElementClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushboolean(L, a->Is_Dirty());
		return 1;
	}
	return 0;
}

int LuaHUDElementClass::Set_Dirty(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDElementClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		a->Set_Dirty(lua_toboolean(L, 2));
		return 1;
	}
	return 0;
}

int LuaHUDElementClass::Is_Rendered(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDElementClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		lua_pushboolean(L, a->Is_Rendered());
		return 1;
	}
	return 0;
}

int LuaHUDElementClass::Set_Rendered(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDElementClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		a->Set_Rendered(lua_toboolean(L, 2));
		return 1;
	}
	return 0;
}

int LuaHUDElementClass::Get_UV_Range(lua_State *L)
{
	if (lua_gettop(L) < 1) return 0;
	HUDElementClass *a = GetInstance(L, 1);
	if (a != nullptr)
	{
		RectClass rect = a->Get_UV_Range();
		lua_newtable(L);
		int tbl = lua_gettop(L);
		int index = 1;
		lua_pushnumber(L, rect.Left);
		lua_rawseti(L, tbl, 1);
		lua_pushnumber(L, rect.Top);
		lua_rawseti(L, tbl, 2);
		lua_pushnumber(L, rect.Right);
		lua_rawseti(L, tbl, 3);
		lua_pushnumber(L, rect.Bottom);
		lua_rawseti(L, tbl, 4);
		return 1;
	}
	return 0;
}

int LuaHUDElementClass::Set_UV_Range(lua_State *L)
{
	if (lua_gettop(L) < 2) return 0;
	HUDElementClass *obj = GetInstance(L, 1);
	if (obj != nullptr)
	{
		if (!lua_istable(L, 2))
		{
			luaL_error(L, "Invalid argument #2. Expected a table.");
			return 0;
		}

		float point[4];
		for (int a = 1; a <= 4; a++)
		{
			lua_rawgeti(L, 2, a);
			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				point[a - 1] = lua_tonumber(L, -1);
			}
			lua_pop(L, 1);
		}

		RectClass rect(point[0], point[1], point[2], point[3]);
		obj->Set_UV_Range(rect);
	}
	return 0;
}

/************************ LuaHUDLineElementClass *************************/

const luaL_Reg LuaHUDLineElementClass::methods[] = {
	method(LuaHUDElementClass, Get_Element_Type),
	method(LuaHUDElementClass, Get_Element_ID),
	method(LuaHUDElementClass, Is_Dirty),
	method(LuaHUDElementClass, Set_Dirty),
	method(LuaHUDElementClass, Is_Rendered),
	method(LuaHUDElementClass, Set_Rendered),
	method(LuaHUDElementClass, Get_UV_Range),
	method(LuaHUDElementClass, Set_UV_Range),

	method(LuaHUDLineElementClass, Get_P0),
	method(LuaHUDLineElementClass, Get_P1),
	method(LuaHUDLineElementClass, Set_P0),
	method(LuaHUDLineElementClass, Set_P1),
	method(LuaHUDLineElementClass, Set_Color),
	method(LuaHUDLineElementClass, Get_Color),
	method(LuaHUDLineElementClass, Get_Thickness),
	method(LuaHUDLineElementClass, Set_Thickness),
	method(LuaHUDLineElementClass, Get_Length),

	{ 0, 0 }
};

/************************ LuaHUDRectangleElementClass *************************/

const luaL_Reg LuaHUDRectangleElementClass::methods[] = {
	method(LuaHUDElementClass, Get_Element_Type),
	method(LuaHUDElementClass, Get_Element_ID),
	method(LuaHUDElementClass, Is_Dirty),
	method(LuaHUDElementClass, Set_Dirty),
	method(LuaHUDElementClass, Is_Rendered),
	method(LuaHUDElementClass, Set_Rendered),
	method(LuaHUDElementClass, Get_UV_Range),
	method(LuaHUDElementClass, Set_UV_Range),

	method(LuaHUDRectangleElementClass, Set_Color),
	method(LuaHUDRectangleElementClass, Get_Color),
	method(LuaHUDRectangleElementClass, Get_Rect),
	method(LuaHUDRectangleElementClass, Set_Rect),
	method(LuaHUDRectangleElementClass, Get_Area),
	method(LuaHUDRectangleElementClass, Get_Diagonal_Length),

	{ 0, 0 }
};

/************************ LuaHUDOutlineElementClass *************************/

const luaL_Reg LuaHUDOutlineElementClass::methods[] = {
	method(LuaHUDElementClass, Get_Element_Type),
	method(LuaHUDElementClass, Get_Element_ID),
	method(LuaHUDElementClass, Is_Dirty),
	method(LuaHUDElementClass, Set_Dirty),
	method(LuaHUDElementClass, Is_Rendered),
	method(LuaHUDElementClass, Set_Rendered),
	method(LuaHUDElementClass, Get_UV_Range),
	method(LuaHUDElementClass, Set_UV_Range),

	method(LuaHUDOutlineElementClass, Set_Color),
	method(LuaHUDOutlineElementClass, Get_Color),
	method(LuaHUDOutlineElementClass, Get_Rect),
	method(LuaHUDOutlineElementClass, Set_Rect),
	method(LuaHUDOutlineElementClass, Get_Area),
	method(LuaHUDOutlineElementClass, Get_Diagonal_Length),
	method(LuaHUDOutlineElementClass, Set_Thickness),
	method(LuaHUDOutlineElementClass, Get_Thickness),

	{ 0, 0 }
};

/************************ LuaHUDTextElementClass *************************/

const luaL_Reg LuaHUDTextElementClass::methods[] = {
	method(LuaHUDElementClass, Get_Element_Type),
	method(LuaHUDElementClass, Get_Element_ID),
	method(LuaHUDElementClass, Is_Dirty),
	method(LuaHUDElementClass, Set_Dirty),
	method(LuaHUDElementClass, Is_Rendered),
	method(LuaHUDElementClass, Set_Rendered),
	method(LuaHUDElementClass, Get_UV_Range),
	method(LuaHUDElementClass, Set_UV_Range),

	method(LuaHUDTextElementClass, Set_Color),
	method(LuaHUDTextElementClass, Get_Color),
	method(LuaHUDTextElementClass, Set_Clipping_Area),
	method(LuaHUDTextElementClass, Get_Clipping_Area),
	method(LuaHUDTextElementClass, Get_Clipping_Area_Size),
	method(LuaHUDTextElementClass, Get_Clipping_Area_Diagonal_Length),
	method(LuaHUDTextElementClass, Set_Text),
	method(LuaHUDTextElementClass, Get_Text),
	method(LuaHUDTextElementClass, Get_Font),
	method(LuaHUDTextElementClass, Set_Font),

	{ 0, 0 }
};



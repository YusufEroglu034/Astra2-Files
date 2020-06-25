#include "stdafx.h"
#include <sstream>
#include "constants.h"
#include "char.h"
#include "char_manager.h"
#include "log.h"
#include "questmanager.h"
#include "questlua.h"
#include "questevent.h"
#include "config.h"
#include "mining.h"
#include "fishing.h"
#include "priv_manager.h"
#include "utils.h"
#include "p2p.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "start_position.h"
#include "over9refine.h"
#include "OXEvent.h"
#include "regen.h"
#include "cmd.h"
#include "guild.h"
#include "guild_manager.h"
#include "sectree_manager.h"

#ifdef __MOUNT__
#include "mountsystem.h"
#endif

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)

#endif
#ifdef ENABLE_NEWSTUFF //@correction232
#include "db.h"
#endif
extern ACMD(do_block_chat);

namespace quest
{
	int32_t _get_locale(lua_State* L)
	{
		lua_pushstring(L, g_stLocale.c_str());
		return 1;
	}

	int32_t _number(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, number((int32_t)lua_tonumber(L, 1), (int32_t)lua_tonumber(L, 2)));
		return 1;
	}

	int32_t _time_to_str(lua_State* L)
	{
		time_t curTime = (time_t)lua_tonumber(L, -1);
		lua_pushstring(L, asctime(gmtime(&curTime)));
		return 1;
	}

	int32_t _say(lua_State* L)
	{
		ostringstream s;
#ifdef __MULTI_LANGUAGE__
		if (lua_isnumber(L, 1))
		{
			s << "{";
			int32_t idx = (int32_t)lua_tonumber(L, 1);
			s << idx;
			if (lua_isstring(L, 2))
				s << ", " << lua_tostring(L, 2);
			CQuestManager::Instance().AddScript(s.str() + "}[ENTER]");
		}
		else
		{
			combine_lua_string(L, s);
			CQuestManager::Instance().AddScript(s.str() + "[ENTER]");
		}
#else
		combine_lua_string(L, s);
		CQuestManager::Instance().AddScript(s.str() + "[ENTER]");
#endif
		return 0;
	}

	int32_t _chat(lua_State* L)
	{
		ostringstream s;
		combine_lua_string(L, s);

		if (CQuestManager::Instance().GetCurrentCharacterPtr())
			CQuestManager::Instance().GetCurrentCharacterPtr()->ChatPacket(CHAT_TYPE_TALKING, "%s", s.str().c_str());
		return 0;
	}

	int32_t _cmdchat(lua_State* L)
	{
		ostringstream s;
		combine_lua_string(L, s);
		if (CQuestManager::Instance().GetCurrentCharacterPtr())
			CQuestManager::Instance().GetCurrentCharacterPtr()->ChatPacket(CHAT_TYPE_COMMAND, "%s", s.str().c_str());
		return 0;
	}

	int32_t _syschat(lua_State* L)
	{
		ostringstream s;
		combine_lua_string(L, s);
		if (CQuestManager::Instance().GetCurrentCharacterPtr())
			CQuestManager::Instance().GetCurrentCharacterPtr()->ChatPacket(CHAT_TYPE_INFO, "%s", s.str().c_str());
		return 0;
	}

	int32_t _notice(lua_State* L)
	{
		ostringstream s;
		combine_lua_string(L, s);
		if (CQuestManager::Instance().GetCurrentCharacterPtr())
			CQuestManager::Instance().GetCurrentCharacterPtr()->ChatPacket(CHAT_TYPE_NOTICE, "%s", s.str().c_str());
		return 0;
	}

	int32_t _left_image(lua_State* L)
	{
		if (lua_isstring(L, -1))
		{
			string s = lua_tostring(L,-1);
			CQuestManager::Instance().AddScript("[LEFTIMAGE src;"+s+"]");
		}
		return 0;
	}

	int32_t _top_image(lua_State* L)
	{
		if (lua_isstring(L, -1))
		{
			string s = lua_tostring(L,-1);
			CQuestManager::Instance().AddScript("[TOPIMAGE src;"+s+"]");
		}
		return 0;
	}

	int32_t _set_skin(lua_State* L)
	{
		if (lua_isnumber(L, -1))
		{
			CQuestManager::Instance().SetSkinStyle((int32_t)rint(lua_tonumber(L,-1)));
		}
		else
		{
			sys_err("QUEST wrong skin index");
		}

		return 0;
	}

	int32_t _set_server_timer(lua_State* L)
	{
		int32_t n = lua_gettop(L);
		if ((n != 2 || !lua_isnumber(L, 2) || !lua_isstring(L, 1)) &&
				(n != 3 || !lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)))
		{
			sys_err("QUEST set_server_timer argument count wrong.");
			return 0;
		}

		const char * name = lua_tostring(L, 1);
		double t = lua_tonumber(L, 2);
		uint32_t arg = 0;

		CQuestManager & q = CQuestManager::instance();

		if (lua_isnumber(L, 3))
			arg = (uint32_t) lua_tonumber(L, 3);

		int32_t timernpc = q.LoadTimerScript(name);

		LPEVENT event = quest_create_server_timer_event(name, t, timernpc, false, arg);
		q.AddServerTimer(name, arg, event);
		return 0;
	}

	int32_t _set_server_loop_timer(lua_State* L)
	{
		int32_t n = lua_gettop(L);
		if ((n != 2 || !lua_isnumber(L, 2) || !lua_isstring(L, 1)) &&
				(n != 3 || !lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)))
		{
			sys_err("QUEST set_server_timer argument count wrong.");
			return 0;
		}
		const char * name = lua_tostring(L, 1);
		double t = lua_tonumber(L, 2);
		uint32_t arg = 0;
		CQuestManager & q = CQuestManager::instance();

		if (lua_isnumber(L, 3))
			arg = (uint32_t) lua_tonumber(L, 3);

		int32_t timernpc = q.LoadTimerScript(name);

		LPEVENT event = quest_create_server_timer_event(name, t, timernpc, true, arg);
		q.AddServerTimer(name, arg, event);
		return 0;
	}

	int32_t _clear_server_timer(lua_State* L)
	{
		CQuestManager & q = CQuestManager::instance();
		const char * name = lua_tostring(L, 1);
		uint32_t arg = (uint32_t) lua_tonumber(L, 2);
        if (name && arg) // @correction016
            q.ClearServerTimer(name, arg);
        else
            sys_err("LUA PREVENT: Wrong argument on ClearServerTimer!");
		return 0;
	}

	int32_t _set_named_loop_timer(lua_State* L)
	{
		int32_t n = lua_gettop(L);

		if (n != 2 || !lua_isnumber(L, -1) || !lua_isstring(L, -2))
			sys_err("QUEST set_timer argument count wrong.");
		else
		{
			const char * name = lua_tostring(L, -2);
			double t = lua_tonumber(L, -1);

			CQuestManager & q = CQuestManager::instance();
			int32_t timernpc = q.LoadTimerScript(name);
			q.GetCurrentPC()->AddTimer(name, quest_create_timer_event(name, q.GetCurrentCharacterPtr()->GetPlayerID(), t, timernpc, true));
		}

		return 0;
	}

	int32_t _get_server_timer_arg(lua_State* L)
	{
		lua_pushnumber(L, CQuestManager::instance().GetServerTimerArg());
		return 1;
	}

	int32_t _set_timer(lua_State* L)
	{
		if (lua_gettop(L) != 1 || !lua_isnumber(L, -1))
			sys_err("QUEST invalid argument.");
		else
		{
			double t = lua_tonumber(L, -1);

			CQuestManager& q = CQuestManager::instance();
			quest_create_timer_event("", q.GetCurrentCharacterPtr()->GetPlayerID(), t);
		}

		return 0;
	}

	int32_t _set_named_timer(lua_State* L)
	{
		int32_t n = lua_gettop(L);

		if (n != 2 || !lua_isnumber(L, -1) || !lua_isstring(L, -2))
		{
			sys_err("QUEST set_timer argument count wrong.");
		}
		else
		{
			const char * name = lua_tostring(L,-2);
			double t = lua_tonumber(L, -1);

			CQuestManager & q = CQuestManager::instance();
			int32_t timernpc = q.LoadTimerScript(name);
			q.GetCurrentPC()->AddTimer(name, quest_create_timer_event(name, q.GetCurrentCharacterPtr()->GetPlayerID(), t, timernpc));
		}

		return 0;
	}

	int32_t _timer(lua_State * L)
	{
		if (lua_gettop(L) == 1)
			return _set_timer(L);
		else
			return _set_named_timer(L);
	}

	int32_t _clear_named_timer(lua_State* L)
	{
		int32_t n = lua_gettop(L);

		if (n != 1 || !lua_isstring(L, -1))
			sys_err("QUEST set_timer argument count wrong.");
		else
		{
			CQuestManager & q = CQuestManager::instance();
			q.GetCurrentPC()->RemoveTimer(lua_tostring(L, -1));
		}

		return 0;
	}

	int32_t _getnpcid(lua_State * L)
	{
		const char * name = lua_tostring(L, -1);
		CQuestManager & q = CQuestManager::instance();
		lua_pushnumber(L, q.FindNPCIDByName(name));
		return 1;
	}

	int32_t _is_test_server(lua_State * L)
	{
		lua_pushboolean(L, test_server);
		return 1;
	}

	int32_t _is_speed_server(lua_State * L)
	{
		lua_pushboolean(L, speed_server);
		return 1;
	}

	int32_t _raw_script(lua_State* L)
	{
		if ( test_server )
			sys_log ( 0, "_raw_script : %s ", lua_tostring(L,-1));
		if (lua_isstring(L, -1))
			CQuestManager::Instance().AddScript(lua_tostring(L,-1));
		else
			sys_err("QUEST wrong argument: questname: %s", CQuestManager::instance().GetCurrentQuestName().c_str());

		return 0;
	}

	int32_t _char_log(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch) // @correction017
		{
			sys_err("Null ptr on ch");
			return 0;
		}

		uint32_t what = 0;
		const char* how = "";
		const char* hint = "";

		if (lua_isnumber(L, 1))
			what = (uint32_t)lua_tonumber(L, 1);
		if (lua_isstring(L, 2))
			how = lua_tostring(L, 2);
		if (lua_tostring(L, 3))
			hint = lua_tostring(L, 3);

		LogManager::instance().CharLog(ch, what, how, hint);
		return 0;
	}

	int32_t _item_log(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		uint32_t dwItemID = 0;
		const char* how = "";
		const char* hint = "";

		if ( lua_isnumber(L, 1) ) dwItemID = (uint32_t)lua_tonumber(L, 1);
		if ( lua_isstring(L, 2) ) how = lua_tostring(L, 2);
		if ( lua_tostring(L, 3) ) hint = lua_tostring(L, 3);

		LPITEM item = ITEM_MANAGER::instance().Find(dwItemID);

		if (item)
			LogManager::instance().ItemLog(ch, item, how, hint);

		return 0;
	}

	int32_t _syslog(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
			return 0;

		if (lua_tonumber(L, 1) >= 1)
		{
			if (!test_server)
				return 0;
		}

		PC* pc = CQuestManager::instance().GetCurrentPC();

		if (!pc)
			return 0;

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		sys_log(0, "QUEST: quest: %s player: %s : %s", pc->GetCurrentQuestName().c_str(), ch->GetName(), lua_tostring(L, 2));

		if (true == test_server)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "QUEST_SYSLOG %s", lua_tostring(L, 2));
		}

		return 0;
	}

	int32_t _syserr(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		// @correction021 START
		PC* pc = CQuestManager::instance().GetCurrentPC();
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!pc || !ch)
			return 0;

		sys_err("FROM LUA: %s player: %s : %s", pc->GetCurrentQuestName().c_str(), ch->GetName(), lua_tostring(L, 1));
		// @correction021 END
		return 0;
	}

	int32_t _set_bgm_volume_enable(lua_State* L)
	{
		CHARACTER_SetBGMVolumeEnable();

		return 0;
	}

	int32_t _add_bgm_info(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
			return 0;

		int32_t mapIndex		= (int32_t)lua_tonumber(L, 1);

		const char*	bgmName	= lua_tostring(L, 2);
		if (!bgmName)
			return 0;

		float bgmVol = lua_isnumber(L, 3) ? lua_tonumber(L, 3) : (1.0f/5.0f)*0.1f;

		CHARACTER_AddBGMInfo(mapIndex, bgmName, bgmVol);

		return 0;
	}

	int32_t _add_goto_info(lua_State* L)
	{
		const char* name = lua_tostring(L, 1);

		int32_t empire 	= (int32_t)lua_tonumber(L, 2);
		int32_t mapIndex 	= (int32_t)lua_tonumber(L, 3);
		int32_t x 		= (int32_t)lua_tonumber(L, 4);
		int32_t y 		= (int32_t)lua_tonumber(L, 5);

		if (!name)
			return 0;

		CHARACTER_AddGotoInfo(name, empire, mapIndex, x, y);
		return 0;
	}

	int32_t _refine_pick(lua_State* L)
	{
		uint16_t wCell = (uint16_t) lua_tonumber(L,-1);

		CQuestManager& q = CQuestManager::instance();

		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		LPITEM item = ch->GetInventoryItem(wCell);

		int32_t ret = mining::RealRefinePick(ch, item);
		lua_pushnumber(L, ret);
		return 1;
	}

	int32_t _fish_real_refine_rod(lua_State* L)
	{
		uint16_t wCell = (uint16_t) lua_tonumber(L,-1);

		CQuestManager& q = CQuestManager::instance();

		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		LPITEM item = ch->GetInventoryItem(wCell);

		int32_t ret = fishing::RealRefineRod(ch, item);
		lua_pushnumber(L, ret);
		return 1;
	}

	int32_t _give_char_privilege(lua_State* L)
	{
		int32_t pid = CQuestManager::instance().GetCurrentCharacterPtr()->GetPlayerID();
		int32_t type = (int32_t)lua_tonumber(L, 1);
		int32_t value = (int32_t)lua_tonumber(L, 2);

		if (MAX_PRIV_NUM <= type)
		{
			sys_err("PRIV_MANAGER: _give_char_privilege: wrong empire priv type(%u)", type);
			return 0;
		}

		CPrivManager::instance().RequestGiveCharacterPriv(pid, type, value);

		return 0;
	}

	int32_t _give_empire_privilege(lua_State* L)
	{
		int32_t empire = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L, 2);
		int32_t value = (int32_t)lua_tonumber(L, 3);
		int32_t time = (int32_t) lua_tonumber(L,4);
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (MAX_PRIV_NUM <= type)
		{
			sys_err("PRIV_MANAGER: _give_empire_privilege: wrong empire priv type(%u)", type);
			return 0;
		}

		if (ch)
			sys_log(0, "_give_empire_privileage(empire=%d, type=%d, value=%d, time=%d), by quest, %s",
					empire, type, value, time, ch->GetName());
		else
			sys_log(0, "_give_empire_privileage(empire=%d, type=%d, value=%d, time=%d), by quest, nullptr",
					empire, type, value, time);

		CPrivManager::instance().RequestGiveEmpirePriv(empire, type, value, time);
		return 0;
	}

	int32_t _give_guild_privilege(lua_State* L)
	{
		int32_t guild_id = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L, 2);
		int32_t value = (int32_t)lua_tonumber(L, 3);
		int32_t time = (int32_t)lua_tonumber( L, 4 );

		if (MAX_PRIV_NUM <= type)
		{
			sys_err("PRIV_MANAGER: _give_guild_privilege: wrong empire priv type(%u)", type);
			return 0;
		}

		sys_log(0, "_give_guild_privileage(empire=%d, type=%d, value=%d, time=%d)",
				guild_id, type, value, time);

		CPrivManager::instance().RequestGiveGuildPriv(guild_id,type,value,time);

		return 0;
	}

	int32_t _get_empire_privilege_string(lua_State* L)
	{
		int32_t empire = (int32_t) lua_tonumber(L, 1);
		ostringstream os;
		bool found = false;

		for (int32_t type = PRIV_NONE + 1; type < MAX_PRIV_NUM; ++type)
		{
			CPrivManager::SPrivEmpireData* pkPrivEmpireData = CPrivManager::instance().GetPrivByEmpireEx(empire, type);

			if (pkPrivEmpireData && pkPrivEmpireData->m_value)
			{
				if (found)
					os << ", ";

				os << LC_TEXT(c_apszPrivNames[type]) << " : " <<
					pkPrivEmpireData->m_value << "%" << " (" <<
					((pkPrivEmpireData->m_end_time_sec-get_global_time())/3600.0f) << " hours)" << endl;
				found = true;
			}
		}

		if (!found)
			os << "None!" << endl;

		lua_pushstring(L, os.str().c_str());
		return 1;
	}

	int32_t _get_empire_privilege(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushnumber(L,0);
			return 1;
		}
		int32_t empire = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L,2);
		int32_t value = CPrivManager::instance().GetPrivByEmpire(empire,type);
		lua_pushnumber(L, value);
		return 1;
	}

	int32_t _get_guild_privilege_string(lua_State* L)
	{
		int32_t guild = (int32_t) lua_tonumber(L,1);
		ostringstream os;
		bool found = false;

		for (int32_t type = PRIV_NONE+1; type < MAX_PRIV_NUM; ++type)
		{
			const CPrivManager::SPrivGuildData* pPrivGuildData = CPrivManager::instance().GetPrivByGuildEx( guild, type );

			if (pPrivGuildData && pPrivGuildData->value)
			{
				if (found)
					os << ", ";

				os << LC_TEXT(c_apszPrivNames[type]) << " : " << pPrivGuildData->value << "%"
					<< " (" << ((pPrivGuildData->end_time_sec - get_global_time()) / 3600.0f) << " hours)" << endl;
				found = true;
			}
		}

		if (!found)
			os << "None!" << endl;

		lua_pushstring(L, os.str().c_str());
		return 1;
	}

	int32_t _get_guildid_byname( lua_State* L )
	{
		if ( !lua_isstring( L, 1 ) ) {
			sys_err( "_get_guildid_byname() - invalud argument" );
			lua_pushnumber( L, 0 );
			return 1;
		}

		const char* pszGuildName = lua_tostring( L, 1 );
		CGuild* pFindGuild = CGuildManager::instance().FindGuildByName( pszGuildName );
		if ( pFindGuild )
			lua_pushnumber( L, pFindGuild->GetID() );
		else
			lua_pushnumber( L, 0 );

		return 1;
	}

	int32_t _get_guild_privilege(lua_State* L)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushnumber(L,0);
			return 1;
		}
		int32_t guild = (int32_t)lua_tonumber(L,1);
		int32_t type = (int32_t)lua_tonumber(L,2);
		int32_t value = CPrivManager::instance().GetPrivByGuild(guild,type);
		lua_pushnumber(L, value);
		return 1;
	}

	int32_t _item_name(lua_State* L)
	{
		if (lua_isnumber(L,1))
		{
			uint32_t dwVnum = (uint32_t)lua_tonumber(L,1);
			TItemTable* pTable = ITEM_MANAGER::instance().GetTable(dwVnum);
			if (pTable)
				lua_pushstring(L,pTable->szLocaleName);
			else
				lua_pushstring(L,"");
		}
		else
			lua_pushstring(L,"");
		return 1;
	}

	int32_t _mob_name(lua_State* L)
	{
		if (lua_isnumber(L, 1))
		{
			uint32_t dwVnum = (uint32_t) lua_tonumber(L,1);
			const CMob * pkMob = CMobManager::instance().Get(dwVnum);

			if (pkMob)
				lua_pushstring(L, pkMob->m_table.szLocaleName);
			else
				lua_pushstring(L, "");
		}
		else
			lua_pushstring(L,"");

		return 1;
	}

	int32_t _mob_vnum(lua_State* L)
	{
		if (lua_isstring(L,1))
		{
			const char* str = lua_tostring(L, 1);
			const CMob* pkMob = CMobManager::instance().Get(str, false);
			if (pkMob)
				lua_pushnumber(L,pkMob->m_table.dwVnum);
			else
				lua_pushnumber(L,0);
		}
		else
			lua_pushnumber(L,0);

		return 1;
	}

	int32_t _get_global_time(lua_State* L)
	{
		lua_pushnumber(L, get_global_time());
		return 1;
	}


	int32_t _get_channel_id(lua_State* L)
	{
		lua_pushnumber(L, g_bChannel);

		return 1;
	}

	int32_t _do_command(lua_State* L)
	{
		if (!lua_isstring(L, 1))
			return 0;

		const char * str = lua_tostring(L, 1);
		size_t len = strlen(str);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		::interpret_command(ch, str, len);
		return 0;
	}

	int32_t _find_pc(lua_State* L)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		const char * name = lua_tostring(L, 1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(name);
		lua_pushnumber(L, tch ? tch->GetVID() : 0);
		return 1;
	}

	int32_t _find_pc_cond(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		int32_t iMinLev = (int32_t) lua_tonumber(L, 1);
		int32_t iMaxLev = (int32_t) lua_tonumber(L, 2);
		uint32_t uiJobFlag = (uint32_t) lua_tonumber(L, 3);

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		LPCHARACTER tch;

		if (test_server)
		{
			sys_log(0, "find_pc_cond map=%d, job=%d, level=%d~%d",
					ch->GetMapIndex(),
					uiJobFlag,
					iMinLev, iMaxLev);
		}

		tch = CHARACTER_MANAGER::instance().FindSpecifyPC(uiJobFlag,
				ch->GetMapIndex(),
				ch,
				iMinLev,
				iMaxLev);

		lua_pushnumber(L, tch ? tch->GetVID() : 0);
		return 1;
	}

	int32_t _find_npc_by_vnum(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t race = (uint32_t) lua_tonumber(L, 1);

		CharacterVectorInteractor i;

		if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(race, i))
		{
			CharacterVectorInteractor::iterator it = i.begin();

			while (it != i.end())
			{
				LPCHARACTER tch = *(it++);

				if (tch->GetMapIndex() == CQuestManager::instance().GetCurrentCharacterPtr()->GetMapIndex())
				{
					lua_pushnumber(L, tch->GetVID());
					return 1;
				}
			}
		}


		lua_pushnumber(L, 0);
		return 1;
	}

	int32_t _set_quest_state(lua_State* L)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::instance();
		QuestState * pqs = q.GetCurrentState();
		PC* pPC = q.GetCurrentPC();
		if (L!=pqs->co)
		{
			luaL_error(L, "running thread != current thread???");
			sys_log(0,"running thread != current thread???");
			return -1;
		}
		if (pPC)
		{
			const string stQuestName(lua_tostring(L, 1));
			const string stStateName(lua_tostring(L, 2));
			if ( test_server )
				sys_log(0,"set_state %s %s ", stQuestName.c_str(), stStateName.c_str() );
			if (pPC->GetCurrentQuestName() == stQuestName)
			{
				pqs->st = q.GetQuestStateIndex(pPC->GetCurrentQuestName(), lua_tostring(L, -1));
				pPC->SetCurrentQuestStateName(lua_tostring(L,-1));
			}
			else
			{
				pPC->SetQuestState(stQuestName, stStateName);
			}
		}
		return 0;
	}

	int32_t _get_quest_state(lua_State* L)
	{
		if (!lua_isstring(L, 1) )
			return 0;

		CQuestManager& q = CQuestManager::instance();
		PC* pPC = q.GetCurrentPC();

		if (pPC)
		{
			std::string stQuestName	= lua_tostring(L, 1);
			stQuestName += ".__status";

			int32_t nRet = pPC->GetFlag( stQuestName.c_str() );

			lua_pushnumber(L, nRet );

			if ( test_server )
				sys_log(0,"Get_quest_state name %s value %d", stQuestName.c_str(), nRet );
		}
		else
		{
			if ( test_server )
				sys_log(0,"PC == 0 ");

			lua_pushnumber(L, 0);
		}
		return 1;
	}

#ifdef ENABLE_FULL_NOTICE // @correction188
	int32_t _big_notice(lua_State* L)
	{
		ostringstream s;
		combine_lua_string(L, s);
		CQuestManager::Instance().GetCurrentCharacterPtr()->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", s.str().c_str());
		return 0;
	}

	int32_t _big_notice_in_map( lua_State* L )
	{
		const LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
		{
			SendNoticeMap(lua_tostring(L,1), pChar->GetMapIndex(), true);
		}

		return 0;
	}

	int32_t _big_notice_all( lua_State* L )
	{
		ostringstream s;
		combine_lua_string(L, s);

		TPacketGGNotice p;
		p.bHeader = HEADER_GG_BIG_NOTICE;
		p.lSize = strlen(s.str().c_str()) + 1;

		TEMP_BUFFER buf;
		buf.write(&p, sizeof(p));
		buf.write(s.str().c_str(), p.lSize);

		P2P_MANAGER::instance().Send(buf.read_peek(), buf.size());

		SendNotice(s.str().c_str(), true);
		return 1;
	}
#endif

	int32_t _notice_all( lua_State* L )
	{
		ostringstream s;
		combine_lua_string(L, s);

		TPacketGGNotice p;
		p.bHeader = HEADER_GG_NOTICE;
		p.lSize = strlen(s.str().c_str()) + 1;

		TEMP_BUFFER buf;
		buf.write(&p, sizeof(p));
		buf.write(s.str().c_str(), p.lSize);

		P2P_MANAGER::instance().Send(buf.read_peek(), buf.size());

		SendNotice(s.str().c_str());
		return 1;
	}

	EVENTINFO(warp_all_to_village_event_info)
	{
		uint32_t dwWarpMapIndex;

		warp_all_to_village_event_info()
		: dwWarpMapIndex( 0 )
		{
		}
	};

	struct FWarpAllToVillage
	{
		FWarpAllToVillage() {};
		void operator()(LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER) ent;
				if (ch && ch->IsPC() && !ch->IsGM()) // @correction020
				{
					uint8_t bEmpire =  ch->GetEmpire();
					if ( bEmpire == 0 )
					{
						sys_err( "Unkonwn Empire %s %d ", ch->GetName(), ch->GetPlayerID() );
						return;
					}

					ch->WarpSet( g_start_position[bEmpire][0], g_start_position[bEmpire][1] );
				}
			}
		}
	};

	EVENTFUNC(warp_all_to_village_event)
	{
		warp_all_to_village_event_info * info = dynamic_cast<warp_all_to_village_event_info *>(event->info);

		if ( info == nullptr )
		{
			sys_err( "warp_all_to_village_event> <Factor> Null pointer" );
			return 0;
		}

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap( info->dwWarpMapIndex );

		if (nullptr != pSecMap)
		{
			FWarpAllToVillage f;
			pSecMap->for_each( f );
		}

		return 0;
	}

	int32_t _warp_all_to_village( lua_State * L )
	{
		int32_t iMapIndex 	= static_cast<int32_t>(lua_tonumber(L, 1));
		int32_t iSec		= static_cast<int32_t>(lua_tonumber(L, 2));

		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();

		info->dwWarpMapIndex = iMapIndex;

		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(iSec));

		SendNoticeMap(LC_TEXT("잠시후 모두 마을로 이동됩니다."), iMapIndex, false);

		return 0;
	}

	int32_t _warp_to_village( lua_State * L )
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (nullptr != ch)
		{
			uint8_t bEmpire = ch->GetEmpire();
			ch->WarpSet( g_start_position[bEmpire][0], g_start_position[bEmpire][1] );
		}

		return 0;
	}

	int32_t _say_in_map( lua_State * L )
	{
		int32_t iMapIndex 		= static_cast<int32_t>(lua_tonumber( L, 1 ));
		std::string Script(lua_tostring( L, 2 ));

		Script += "[ENTER]";
		Script += "[DONE]";

		struct ::packet_script packet_script;

		packet_script.header = HEADER_GC_SCRIPT;
		packet_script.skin = CQuestManager::QUEST_SKIN_NORMAL;
		packet_script.src_size = Script.size();
		packet_script.size = packet_script.src_size + sizeof(struct packet_script);

		FSendPacket f;
		f.buf.write(&packet_script, sizeof(struct packet_script));
		f.buf.write(&Script[0], Script.size());

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap( iMapIndex );

		if ( pSecMap )
		{
			pSecMap->for_each( f );
		}

		return 0;
	}

	struct FKillSectree2
	{
		void operator () (LPENTITY ent)
		{
			if (ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = (LPCHARACTER) ent;

				if (!ch->IsPC() && !ch->IsPet()
#ifdef __MOUNT__
&& !ch->IsMountSystem()
#endif
#ifdef __GROWTH_PET__
&& !ch->IsNewPet()
#endif
)
					ch->Dead();
			}
		}
	};

	int32_t _kill_all_in_map ( lua_State * L )
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap( lua_tonumber(L,1) );

		if (nullptr != pSecMap)
		{
			FKillSectree2 f;
			pSecMap->for_each( f );
		}

		return 0;
	}

	int32_t _regen_in_map( lua_State * L )
	{
		int32_t iMapIndex = static_cast<int32_t>(lua_tonumber(L, 1));
		std::string szFilename(lua_tostring(L, 2));

		LPSECTREE_MAP pkMap = SECTREE_MANAGER::instance().GetMap(iMapIndex);

		if (pkMap != nullptr)
		{
			regen_load_in_file( szFilename.c_str(), iMapIndex, pkMap->m_setting.iBaseX ,pkMap->m_setting.iBaseY );
		}

		return 0;
	}

	int32_t _enable_over9refine( lua_State* L )
	{
		if ( lua_isnumber(L, 1) == true && lua_isnumber(L, 2) == true )
		{
			uint32_t dwVnumFrom = (uint32_t)lua_tonumber(L, 1);
			uint32_t dwVnumTo = (uint32_t)lua_tonumber(L, 2);

			COver9RefineManager::instance().enableOver9Refine(dwVnumFrom, dwVnumTo);
		}

		return 0;
	}

	int32_t _add_ox_quiz(lua_State* L)
	{
		int32_t level = (int32_t)lua_tonumber(L, 1);
		const char* quiz = lua_tostring(L, 2);
		bool answer = lua_toboolean(L, 3);

		if ( COXEventManager::instance().AddQuiz(level, quiz, answer) == false )
		{
			sys_log(0, "OXEVENT : Cannot add quiz. %d %s %d", level, quiz, answer);
		}

		return 1;
	}

	EVENTFUNC(warp_all_to_map_my_empire_event)
	{
		warp_all_to_map_my_empire_event_info * info = dynamic_cast<warp_all_to_map_my_empire_event_info *>(event->info);

		if ( info == nullptr )
		{
			sys_err( "warp_all_to_map_my_empire_event> <Factor> Null pointer" );
			return 0;
		}

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap( info->m_lMapIndexFrom );

		if (pSecMap)
		{
			FWarpEmpire f;

			f.m_lMapIndexTo = info->m_lMapIndexTo;
			f.m_x			= info->m_x;
			f.m_y			= info->m_y;
			f.m_bEmpire		= info->m_bEmpire;

			pSecMap->for_each(f);
		}

		return 0;
	}

	int32_t _block_chat(lua_State* L)
	{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (pChar != nullptr)
		{
			if (lua_isstring(L, 1) != true && lua_isstring(L, 2) != true)
			{
				lua_pushboolean(L, false);
				return 1;
			}

			std::string strName(lua_tostring(L, 1));
			std::string strTime(lua_tostring(L, 2));

			std::string strArg = strName + " " + strTime;

			do_block_chat(pChar, const_cast<char*>(strArg.c_str()), 0, 0);

			lua_pushboolean(L, true);
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

#ifdef ENABLE_NEWSTUFF //@correction232
	int32_t _spawn_mob0(lua_State* L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			lua_pushnumber(L, -1);
			return 1;
		}
		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const int32_t lMapIndex = static_cast<int32_t>(lua_tonumber(L, 2));
		const uint32_t dwX = static_cast<uint32_t>(lua_tonumber(L, 3));
		const uint32_t dwY = static_cast<uint32_t>(lua_tonumber(L, 4));

		const CMob* pMonster = CMobManager::instance().Get(dwVnum);
		if (!pMonster)
		{
			lua_pushnumber(L, -2);
			return 1;
		}
		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);
		if (!pkSectreeMap)
		{
			lua_pushnumber(L, -3);
			return 1;
		}
		const LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(dwVnum, lMapIndex, pkSectreeMap->m_setting.iBaseX+dwX*100, pkSectreeMap->m_setting.iBaseY+dwY*100, 0, false, -1);
		lua_pushnumber(L, (ch)?ch->GetVID():0);
		return 1;
	}
#endif

	int32_t _spawn_mob(lua_State* L)
	{
		if( false == lua_isnumber(L, 1) || false == lua_isnumber(L, 2) || false == lua_isboolean(L, 3) )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const size_t count = MINMAX(1, static_cast<size_t>(lua_tonumber(L, 2)), 10);
		const bool isAggresive = static_cast<bool>(lua_toboolean(L, 3));
		size_t SpawnCount = 0;

		const CMob* pMonster = CMobManager::instance().Get( dwVnum );

		if( nullptr != pMonster )
		{
			const LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

			for( size_t i=0 ; i < count ; ++i )
			{
				const LPCHARACTER pSpawnMonster = CHARACTER_MANAGER::instance().SpawnMobRange( dwVnum,
						pChar->GetMapIndex(),
						pChar->GetX() - number(200, 750),
						pChar->GetY() - number(200, 750),
						pChar->GetX() + number(200, 750),
						pChar->GetY() + number(200, 750),
						true,
						pMonster->m_table.bType == CHAR_TYPE_STONE,
						isAggresive );

				if( nullptr != pSpawnMonster )
				{
					++SpawnCount;
				}
			}

			sys_log(0, "QUEST Spawn Monstster: VNUM(%u) COUNT(%u) isAggresive(%b)", dwVnum, SpawnCount, isAggresive);
		}

		lua_pushnumber(L, SpawnCount);

		return 1;
	}

#ifdef ENABLE_NEWSTUFF //@correction232
	int32_t _spawn_mob_in_map(lua_State* L)
	{
		if( false == lua_isnumber(L, 1) || false == lua_isnumber(L, 2) || false == lua_isboolean(L, 3) || false == lua_isnumber(L, 4) || false == lua_isnumber(L, 5) || false == lua_isnumber(L, 6) )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const size_t count = MINMAX(1, static_cast<size_t>(lua_tonumber(L, 2)), 10);
		const bool isAggressive = static_cast<bool>(lua_toboolean(L, 3));
		const int32_t iMapIndex = static_cast<int32_t>(lua_tonumber(L, 4));
		const int32_t iMapX = static_cast<int32_t>(lua_tonumber(L, 5));
		const int32_t iMapY = static_cast<int32_t>(lua_tonumber(L, 6));
		size_t SpawnCount = 0;
		sys_log(0, "QUEST _spawn_mob_in_map: VNUM(%u) COUNT(%u) isAggressive(%b) MapIndex(%d) MapX(%d) MapY(%d)", dwVnum, count, isAggressive, iMapIndex, iMapX, iMapY);

		PIXEL_POSITION pos;
		if (!SECTREE_MANAGER::instance().GetMapBasePositionByMapIndex(iMapIndex, pos))
		{
			sys_err("QUEST _spawn_mob_in_map: cannot find base position in this map %d", iMapIndex);
			lua_pushnumber(L, 0);
			return 1;
		}

		const CMob* pMonster = CMobManager::instance().Get( dwVnum );

		if( nullptr != pMonster )
		{
			for( size_t i=0 ; i < count ; ++i )
			{
				const LPCHARACTER pSpawnMonster = CHARACTER_MANAGER::instance().SpawnMobRange(dwVnum,
						iMapIndex,
						pos.x - number(200, 750) + (iMapX * 100),
						pos.y - number(200, 750) + (iMapY * 100),
						pos.x + number(200, 750) + (iMapX * 100),
						pos.y + number(200, 750) + (iMapY * 100),
						true,
						pMonster->m_table.bType == CHAR_TYPE_STONE,
						isAggressive
				);

				if( nullptr != pSpawnMonster )
				{
					++SpawnCount;
				}
			}

			sys_log(0, "QUEST Spawn Monster: VNUM(%u) COUNT(%u) isAggressive(%b)", dwVnum, SpawnCount, isAggressive);
		}

		lua_pushnumber(L, SpawnCount);

		return 1;
	}
#endif

	int32_t _notice_in_map( lua_State* L )
	{
		const LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
		{
			SendNoticeMap( lua_tostring(L,1), pChar->GetMapIndex(), lua_toboolean(L,2) );
		}

		return 0;
	}

	int32_t _get_locale_base_path( lua_State* L )
	{
		lua_pushstring( L, LocaleService_GetBasePath().c_str() );

		return 1;
	}

	struct FPurgeArea
	{
		int32_t x1, y1, x2, y2;
		LPCHARACTER ExceptChar;

		FPurgeArea(int32_t a, int32_t b, int32_t c, int32_t d, LPCHARACTER p)
			: x1(a), y1(b), x2(c), y2(d),
			ExceptChar(p)
		{}

		void operator () (LPENTITY ent)
		{
			if (true == ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

				if (pChar == ExceptChar)
					return;

				if (!pChar->IsPet() && (true == pChar->IsMonster() || true == pChar->IsStone())
#ifdef __MOUNT__
&& !pChar->IsMountSystem()
#endif
#ifdef __GROWTH_PET__
&& !pChar->IsNewPet()
#endif
)
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						M2_DESTROY_CHARACTER(pChar);
					}
				}
			}
		}
	};

	int32_t _purge_area( lua_State* L )
	{
		int32_t x1 = lua_tonumber(L, 1);
		int32_t y1 = lua_tonumber(L, 2);
		int32_t x2 = lua_tonumber(L, 3);
		int32_t y2 = lua_tonumber(L, 4);

		const int32_t mapIndex = SECTREE_MANAGER::instance().GetMapIndex( x1, y1 );

		if (0 == mapIndex)
		{
			sys_err("_purge_area: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(mapIndex);

		if (nullptr != pSectree)
		{
			FPurgeArea func(x1, y1, x2, y2, CQuestManager::instance().GetCurrentNPCCharacterPtr());

			pSectree->for_each(func);
		}

		return 0;
	}

	struct FWarpAllInAreaToArea
	{
		int32_t from_x1, from_y1, from_x2, from_y2;
		int32_t to_x1, to_y1, to_x2, to_y2;
		size_t warpCount;

		FWarpAllInAreaToArea(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int32_t h)
			: from_x1(a), from_y1(b), from_x2(c), from_y2(d),
			to_x1(e), to_y1(f), to_x2(g), to_y2(h),
			warpCount(0)
		{}

		void operator () (LPENTITY ent)
		{
			if (true == ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

				if (true == pChar->IsPC())
				{
					if (from_x1 <= pChar->GetX() && pChar->GetX() <= from_x2 && from_y1 <= pChar->GetY() && pChar->GetY() <= from_y2)
					{
						++warpCount;

						pChar->WarpSet( number(to_x1, to_x2), number(to_y1, to_y2) );
					}
				}
			}
		}
	};

	int32_t _warp_all_in_area_to_area( lua_State* L )
	{
		int32_t from_x1 = lua_tonumber(L, 1);
		int32_t from_y1 = lua_tonumber(L, 2);
		int32_t from_x2 = lua_tonumber(L, 3);
		int32_t from_y2 = lua_tonumber(L, 4);

		int32_t to_x1 = lua_tonumber(L, 5);
		int32_t to_y1 = lua_tonumber(L, 6);
		int32_t to_x2 = lua_tonumber(L, 7);
		int32_t to_y2 = lua_tonumber(L, 8);

		const int32_t mapIndex = SECTREE_MANAGER::instance().GetMapIndex( from_x1, from_y1 );

		if (0 == mapIndex)
		{
			sys_err("_warp_all_in_area_to_area: cannot get a map index with (%u, %u)", from_x1, from_y1);
			lua_pushnumber(L, 0);
			return 1;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(mapIndex);

		if (nullptr != pSectree)
		{
			FWarpAllInAreaToArea func(from_x1, from_y1, from_x2, from_y2, to_x1, to_y1, to_x2, to_y2);

			pSectree->for_each(func);

			lua_pushnumber(L, func.warpCount);
			sys_log(0, "_warp_all_in_area_to_area: %u character warp", func.warpCount);
			return 1;
		}
		else
		{
			lua_pushnumber(L, 0);
			sys_err("_warp_all_in_area_to_area: no sectree");
			return 1;
		}
	}

#ifdef ENABLE_NEWSTUFF //@correction232
	int32_t _get_special_item_group( lua_State* L )
	{
		if (!lua_isnumber (L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::instance().GetSpecialItemGroup((uint32_t)lua_tonumber(L, 1));

		if (!pItemGroup)
		{
			sys_err("cannot find special item group %d", (uint32_t)lua_tonumber(L, 1));
			return 0;
		}

		int32_t count = pItemGroup->GetGroupSize();

		for (int32_t i = 0; i < count; i++)
		{
			lua_pushnumber(L, (int32_t)pItemGroup->GetVnum(i));
			lua_pushnumber(L, (int32_t)pItemGroup->GetCount(i));
		}

		return count*2;
	}

	int32_t _get_table_postfix(lua_State* L)
	{
		lua_pushstring(L, get_table_postfix());
		return 1;
	}

#ifdef _MSC_VER
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
#endif
	int32_t _mysql_direct_query(lua_State* L)
	{

		if (!lua_isstring(L, 1))
			return 0;

		int32_t i=0, m=1;
		MYSQL_ROW row;
		MYSQL_FIELD * field;
		MYSQL_RES * result;
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(lua_tostring(L, 1)));
		if (pMsg.get())
		{
			lua_pushnumber(L, pMsg->Get()->uiAffectedRows);
			lua_newtable(L);
			if ((result = pMsg->Get()->pSQLResult) &&
					!(pMsg->Get()->uiAffectedRows == 0 || pMsg->Get()->uiAffectedRows == (uint32_t)-1))
			{

				while((row = mysql_fetch_row(result)))
				{
					lua_pushnumber(L, m);
					lua_newtable(L);
					while((field = mysql_fetch_field(result)))
					{
						lua_pushstring(L, field->name);
						if (!(field->flags & NOT_NULL_FLAG) && (row[i]==nullptr))
						{
							lua_pushnil(L);
						}
						else if (IS_NUM(field->type))
						{
							double val = NAN;
							lua_pushnumber(L, (sscanf(row[i],"%lf",&val)==1)?val:NAN);
						}
						else if (field->type == MYSQL_TYPE_BLOB)
						{
							lua_newtable(L);
							for (uint32_t iBlob=0; iBlob < field->max_length; iBlob++)
							{
								lua_pushnumber(L, row[i][iBlob]);
								lua_rawseti(L, -2, iBlob+1);
							}
						}
						else
							lua_pushstring(L, row[i]);
						lua_rawset(L, -3);
						i++;
					}
					mysql_field_seek(result, 0);
					i=0;

					lua_rawset(L, -3);
					m++;
				}
			}
		}
		else
		{
			lua_pushnumber(L, 0);
			lua_newtable(L);
		}

		return 2;
	}

	int32_t _mysql_escape_string(lua_State* L)
	{
		char szQuery[1024] = {0};

		if (!lua_isstring(L, 1))
			return 0;
		DBManager::instance().EscapeString(szQuery, sizeof(szQuery), lua_tostring(L, 1), strlen(lua_tostring(L, 1)));
		lua_pushstring(L, szQuery);
		return 1;
	}

	int32_t _mysql_password(lua_State* L)
	{
		lua_pushstring(L, mysql_hash_password(lua_tostring(L, 1)).c_str());
		return 1;
	}
#endif

#ifdef __TEMPLE_OCHAO__
	int32_t _add_restart_city_pos(lua_State* L)
	{
		int32_t iMapIndex = (int32_t)lua_tonumber(L, 1);
		int32_t iEmpire = (int32_t)lua_tonumber(L, 2);
		int32_t iX = (int32_t)lua_tonumber(L, 3);
		int32_t iY = (int32_t)lua_tonumber(L, 4);
		int32_t iZ = (int32_t)lua_tonumber(L, 5);
		SECTREE_MANAGER::instance().AddRestartCityPos(iMapIndex, iEmpire, iX, iY, iZ);
		return 0;
	}
#endif

#ifdef __12ZI__
	struct FPurgeAreaZodiac
	{
		int32_t x1, y1, x2, y2;
		LPCHARACTER ExceptChar;

		FPurgeAreaZodiac(int32_t a, int32_t b, int32_t c, int32_t d, LPCHARACTER p)
			: x1(a), y1(b), x2(c), y2(d),
			ExceptChar(p)
		{}

		void operator () (LPENTITY ent)
		{
			if (true == ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

				if (pChar == ExceptChar)
					return;

				if (!pChar->IsPet() && (true == pChar->IsMonster() || true == pChar->IsStone() || true == pChar->IsNPC()))
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						M2_DESTROY_CHARACTER(pChar);
					}
				}
			}
		}
	};
	int32_t _purge_area_zodiac(lua_State* L)
	{
		int32_t x1 = lua_tonumber(L, 1);
		int32_t y1 = lua_tonumber(L, 2);
		int32_t x2 = lua_tonumber(L, 3);
		int32_t y2 = lua_tonumber(L, 4);

		const int32_t mapIndex = SECTREE_MANAGER::instance().GetMapIndex(x1, y1);

		if (0 == mapIndex)
		{
			sys_err("_purge_area_zodiac: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(mapIndex);

		if (nullptr != pSectree)
		{
			FPurgeAreaZodiac func(x1, y1, x2, y2, CQuestManager::instance().GetCurrentNPCCharacterPtr());

			pSectree->for_each(func);
		}

		return 0;
	}
#endif
	void RegisterGlobalFunctionTable(lua_State* L)
	{
		extern int32_t quest_setstate(lua_State* L);

		luaL_reg global_functions[] =
		{
			{	"sys_err",					_syserr					},
			{	"sys_log",					_syslog					},
			{	"char_log",					_char_log				},
			{	"item_log",					_item_log				},
			{	"set_state",				quest_setstate			},
			{	"set_skin",					_set_skin				},
			{	"setskin",					_set_skin				},
			{	"time_to_str",				_time_to_str			},
			{	"say",						_say					},
			{	"chat",						_chat					},
			{	"cmdchat",					_cmdchat				},
			{	"syschat",					_syschat				},
			{	"get_locale",				_get_locale				},
			{	"setleftimage",				_left_image				},
			{	"settopimage",				_top_image				},
			{	"server_timer",				_set_server_timer		},
			{	"clear_server_timer",		_clear_server_timer		},
			{	"server_loop_timer",		_set_server_loop_timer	},
			{	"get_server_timer_arg",		_get_server_timer_arg	},
			{	"timer",					_timer					},
			{	"loop_timer",				_set_named_loop_timer	},
			{	"cleartimer",				_clear_named_timer		},
			{	"getnpcid",					_getnpcid				},
			{	"is_test_server",			_is_test_server			},
			{	"is_speed_server",			_is_speed_server		},
			{	"raw_script",				_raw_script				},
			{	"number",					_number	   				},

			{	"set_bgm_volume_enable",	_set_bgm_volume_enable	},
			{	"add_bgm_info",				_add_bgm_info			},

			{	"add_goto_info",			_add_goto_info			},

			{	"__refine_pick",			_refine_pick			},

			{	"add_ox_quiz",					_add_ox_quiz					},
			{	"__fish_real_refine_rod",		_fish_real_refine_rod			},
			{	"__give_char_priv",				_give_char_privilege			},
			{	"__give_empire_priv",			_give_empire_privilege			},
			{	"__give_guild_priv",			_give_guild_privilege			},
			{	"__get_empire_priv_string",		_get_empire_privilege_string	},
			{	"__get_empire_priv",			_get_empire_privilege			},
			{	"__get_guild_priv_string",		_get_guild_privilege_string		},
			{	"__get_guildid_byname",			_get_guildid_byname				},
			{	"__get_guild_priv",				_get_guild_privilege			},
			{	"item_name",					_item_name						},
			{	"mob_name",						_mob_name						},
			{	"mob_vnum",						_mob_vnum						},
			{	"get_time",						_get_global_time				},
			{	"get_global_time",				_get_global_time				},
			{	"get_channel_id",				_get_channel_id					},
			{	"command",						_do_command						},
			{	"find_pc_cond",					_find_pc_cond					},
			{	"find_pc_by_name",				_find_pc						},
			{	"find_npc_by_vnum",				_find_npc_by_vnum				},
			{	"set_quest_state",				_set_quest_state				},
			{	"get_quest_state",				_get_quest_state				},
			{	"notice",						_notice							},
			{	"notice_all",					_notice_all						},
			{	"notice_in_map",				_notice_in_map					},
#ifdef ENABLE_FULL_NOTICE // @correction188
			{	"big_notice",					_big_notice						},
			{	"big_notice_all",				_big_notice_all					},
			{	"big_notice_in_map",			_big_notice_in_map				},
#endif
			{	"warp_all_to_village",			_warp_all_to_village			},
			{	"warp_to_village",				_warp_to_village				},
			{	"say_in_map",					_say_in_map						},
			{	"kill_all_in_map",				_kill_all_in_map				},
			{	"regen_in_map",					_regen_in_map					},
			{	"enable_over9refine",			_enable_over9refine				},
			{	"block_chat",					_block_chat						},
			{	"spawn_mob",					_spawn_mob						},
			{	"get_locale_base_path",			_get_locale_base_path			},
			{	"purge_area",					_purge_area						},
			{	"warp_all_in_area_to_area",		_warp_all_in_area_to_area		},
#ifdef ENABLE_NEWSTUFF //@correction232
			{	"get_special_item_group",		_get_special_item_group			},
			{	"spawn_mob0",					_spawn_mob0						},
			{	"spawn_mob_in_map",				_spawn_mob_in_map				},
			{	"get_table_postfix",			_get_table_postfix				},
			{	"mysql_direct_query",			_mysql_direct_query				},
			{	"mysql_escape_string",			_mysql_escape_string			},
			{	"mysql_password",				_mysql_password					},
#endif
#ifdef __TEMPLE_OCHAO__
			{"add_restart_city_pos", _add_restart_city_pos},
#endif
#ifdef __12ZI__
			{	"purge_area_zodiac",					_purge_area_zodiac		},
#endif
			{	nullptr,	nullptr	}
		};

		int32_t i = 0;

		while (global_functions[i].name != nullptr)
		{
			lua_register(L, global_functions[i].name, global_functions[i].func);
			++i;
		}
	}
}


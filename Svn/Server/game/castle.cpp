#define _castle_cpp_

#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "char_manager.h"
#include "castle.h"
#include "start_position.h"
#include "monarch.h"
#include "questlua.h"
#include "log.h"
#include "char.h"
#include "sectree_manager.h"

#define EMPIRE_NONE		0
#define EMPIRE_RED		1
#define EMPIRE_YELLOW	2
#define EMPIRE_BLUE		3


#define SIEGE_EVENT_PULSE	PASSES_PER_SEC(60*5)


#define GET_CAHR_MANAGER()								CHARACTER_MANAGER::instance()
#define GET_CASTLE(empire)								(s_castle+(empire))
#define GET_GUARD(empire, region_index, guard_index)	(GET_CASTLE(empire)->guard[region_index][guard_index])
#define GET_GUARD_REGION(empire, region_index)			(s_guard_region[empire][region_index])
#define GET_GUARD_GROUP(empire, region_index, guard_index)	(GET_CASTLE(empire)->guard_group[region_index][guard_index])
#define GET_FROG(empire, index)							(GET_CASTLE(empire)->frog[index])
#define GET_FROG_POS(empire, index)						(s_frog_pos[empire][index])
#define GET_TOWER(empire, index)							(GET_CASTLE(empire)->tower[index])

#define DO_ALL_EMPIRE(empire)	for (int32_t empire = 1; empire < 4; ++empire)
#define DO_ALL_TOWER(i)			for (int32_t i = 0; i < MAX_CASTLE_TOWER; ++i)
#define DO_ALL_FROG(i)			for (int32_t i = 0; i < MAX_CASTLE_FROG; ++i)


#define GET_SIEGE_STATE()			s_siege_state
#define GET_SIEGE_EMPIRE()			s_sige_empire
#define GET_SIEGE_EVENT(empire)		(GET_CASTLE(empire)->siege_event)
#define GET_STONE_EVENT(empire)		(GET_CASTLE(empire)->stone_event)

#define GET_TOWER_REGION(empire)	s_tower_region[empire]
#define GET_STONE_REGION(empire)	s_tower_region[empire]


static CASTLE_DATA	*s_castle		= nullptr;
static CASTLE_STATE	s_siege_state	= CASTLE_SIEGE_NONE;
static int32_t			s_sige_empire	= EMPIRE_NONE;


struct POSITION
{
	int32_t	x, y;
};

static POSITION	s_frog_pos[4][MAX_CASTLE_FROG] = {
	{
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },

		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },

		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },

		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 }
	},
	{
		{ 225, 45 },
		{ 231, 45 },
		{ 237, 45 },
		{ 243, 45 },
		{ 249, 45 },

		{ 225, 50 },
		{ 231, 50 },
		{ 237, 50 },
		{ 243, 50 },
		{ 249, 50 },

		{ 261, 45 },
		{ 267, 45 },
		{ 273, 45 },
		{ 279, 45 },
		{ 285, 45 },

		{ 261, 50 },
		{ 267, 50 },
		{ 273, 50 },
		{ 279, 50 },
		{ 285, 50 }
	},
	{
		{ 221, 36 },
		{ 227, 36 },
		{ 233, 36 },
		{ 239, 36 },
		{ 245, 36 },

		{ 269, 36 },
		{ 275, 36 },
		{ 281, 36 },
		{ 287, 36 },
		{ 293, 36 },

		{ 221, 41 },
		{ 227, 41 },
		{ 233, 41 },
		{ 239, 41 },
		{ 245, 41 },

		{ 269, 41 },
		{ 275, 41 },
		{ 281, 41 },
		{ 287, 41 },
		{ 293, 41 }
	},
	{
		{ 225, 45 },
		{ 231, 45 },
		{ 237, 45 },
		{ 243, 45 },
		{ 249, 45 },

		{ 225, 50 },
		{ 231, 50 },
		{ 237, 50 },
		{ 243, 50 },
		{ 249, 50 },

		{ 261, 45 },
		{ 267, 45 },
		{ 273, 45 },
		{ 279, 45 },
		{ 285, 45 },

		{ 261, 50 },
		{ 267, 50 },
		{ 273, 50 },
		{ 279, 50 },
		{ 285, 50 }
	}
};


struct GUARD_REGION
{
	int32_t	sx, sy, ex, ey;
};

static GUARD_REGION s_guard_region[4][4] = {
	{
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 }
	},
	{
		{ 74, 170, 96, 180 },
		{ 237, 135, 270, 146 },
		{ 235, 260, 278, 273 },
		{ 421, 167, 435, 205 }
	},
	{
		{ 109, 172, 128, 202 },
		{ 237, 140, 282, 153 },
		{ 232, 261, 279, 276 },
		{ 390, 173, 403, 205 },
	},
	{
		{ 74, 170, 96, 120 },
		{ 237, 135, 270, 146 },
		{ 235, 260, 278, 273 },
		{ 421, 167, 435, 205 }
	}
};

static GUARD_REGION s_tower_region[4] = {
	{ 0, 0, 0, 0 },
	{ 85, 135, 420, 265 },
	{ 120, 130, 405, 276 },
	{ 85, 135, 420, 265 }
};


static int32_t FN_castle_map_index(int32_t empire);

EVENTINFO(castle_event_info)
{
	int32_t		empire;
	int32_t		pulse;

	castle_event_info()
	: empire( 0 )
	, pulse( 0 )
	{
	}
};

EVENTFUNC(castle_siege_event)
{
	char	buf[1024] = {0};
	struct castle_event_info	*info = dynamic_cast<castle_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "castle_siege_event> <Factor> Null pointer" );
		return 0;
	}

	info->pulse += SIEGE_EVENT_PULSE;

	if (info->pulse < PASSES_PER_SEC(30*60))
	{
		snprintf(buf, sizeof(buf), LC_TEXT("%s에서 봉화를 둘러싸고 전투가 진행중입니다."),
				EMPIRE_NAME(GET_SIEGE_EMPIRE()));
		BroadcastNotice(buf);

		return SIEGE_EVENT_PULSE;
	}

	switch (GET_SIEGE_STATE())
	{
		case CASTLE_SIEGE_NONE:
			break;

		case CASTLE_SIEGE_STRUGGLE:
			{
				snprintf(buf, sizeof(buf), LC_TEXT("%s이 수성에 성공했습니다."), EMPIRE_NAME(GET_SIEGE_EMPIRE()));
				BroadcastNotice(buf);

				snprintf(buf, sizeof(buf), LC_TEXT("지금부터 %s은 30분간 봉화를 파괴하여 보상을 획득 할 수 있습니다."), EMPIRE_NAME(GET_SIEGE_EMPIRE()));
				BroadcastNotice(buf);

				GET_SIEGE_STATE() = CASTLE_SIEGE_END;

				return PASSES_PER_SEC(60*30);
			}
			break;
		case CASTLE_SIEGE_END:
			BroadcastNotice(LC_TEXT("30분이 경과했습니다.. 봉화가 사라집니다."));
			castle_end_siege();
			break;
	}
	return 0;
}


static uint32_t FN_random_stone()
{
	uint32_t	vnum[7] = {
		8012,
		8013,
		8014,
		8024,
		8025,
		8026,
		8027
	};

	int32_t	index = number(0, 6);

	return vnum[index];
}

EVENTINFO(castle_stone_event_info)
{
	int32_t	empire;
	int32_t	spawn_count;

	castle_stone_event_info()
	: empire( 0 )
	, spawn_count( 0 )
	{
	}
};


EVENTFUNC(castle_stone_event)
{
	struct castle_stone_event_info	*info = dynamic_cast<castle_stone_event_info*>( event->info );

	if (info == nullptr)
	{
		sys_err( "castle_stone_event> <Factor> Null pointer" );
		return 0;
	}

	int32_t	map_index	= FN_castle_map_index(GET_SIEGE_EMPIRE());

	SECTREE_MAP	*sectree_map = SECTREE_MANAGER::instance().GetMap(map_index);

	if (nullptr == sectree_map)
		return 0;

	const int32_t SPAWN_COUNT = 15;

	if (info->spawn_count < (SPAWN_COUNT * 2))
	{
		for (int32_t i = 0; i < SPAWN_COUNT; ++i)
		{
			uint32_t	sx = sectree_map->m_setting.iBaseX + 100 * GET_TOWER_REGION(info->empire).sx;
			uint32_t	sy = sectree_map->m_setting.iBaseY + 100 * GET_TOWER_REGION(info->empire).sy;
			uint32_t	ex = sectree_map->m_setting.iBaseX + 100 * GET_TOWER_REGION(info->empire).ex;
			uint32_t	ey = sectree_map->m_setting.iBaseY + 100 * GET_TOWER_REGION(info->empire).ey;

			CHARACTER_MANAGER::instance().SpawnMobRange(FN_random_stone(),
														FN_castle_map_index(info->empire),
														sx, sy, ex, ey);
		}

		info->spawn_count += SPAWN_COUNT;

		if (info->spawn_count < (SPAWN_COUNT * 2))
			return PASSES_PER_SEC(30 * 60);
		else
			return 0;
	}

	return 0;
}



LPCHARACTER castle_spawn_frog_force(int32_t empire, int32_t empty_index);

static int32_t FN_castle_map_index(int32_t empire)
{
	switch (empire)
	{
		case EMPIRE_RED:	return 181;
		case EMPIRE_YELLOW:	return 183;
		case EMPIRE_BLUE:	return 182;
	}
	return 0;
}

static int32_t FN_empty_frog_index(int32_t empire)
{
	DO_ALL_FROG(i)
	{
		if (nullptr == GET_FROG(empire, i))
			return i;
	}
	return (-1);
}

static POSITION* FN_empty_frog_pos(int32_t empire)
{
	int32_t	frog_index = FN_empty_frog_index(empire);

	if (frog_index < 0)
		return nullptr;

	switch (empire)
	{
		case EMPIRE_RED:
		case EMPIRE_YELLOW:
		case EMPIRE_BLUE:
			return &GET_FROG_POS(empire, frog_index);
	}

	return nullptr;
}

static int32_t FN_empty_guard_pos(int32_t empire, int32_t region_index)
{
	for (int32_t i = 0; i < MAX_CASTLE_GUARD_PER_REGION; ++i)
	{
		if (nullptr == GET_GUARD(empire, region_index, i))
		{
			return i;
		}
	}

	return -1;
}

static bool FN_is_castle_map(int32_t map_index)
{
	switch (map_index)
	{
		case 181:
		case 182:
		case 183:
			return true;
	}
	return false;
}


bool castle_boot()
{
	FILE	*fp;
	char	one_line[256];
	const char	*delim			= " \t\r\n";
	char	*v;
	int32_t		empire			= 0;
	int32_t		guard_region	= 0;

	CREATE(s_castle, CASTLE_DATA, 4);

	const char	*castle_file = "castle_data.txt";

	if ((fp = fopen(castle_file, "r")) == 0)
		return false;

	while (fgets(one_line, 256, fp))
	{
		int32_t value = 0;

		if (one_line[0] == '#')
			continue;

		const char* token_string = strtok(one_line, delim);

		if (nullptr == token_string)
			continue;

		TOKEN("section")
		{
			continue;
		}
		else TOKEN("empire")
		{
			v = strtok(nullptr, delim);
			if (v)
			{
				str_to_number(empire, v);
			}
			else
			{
				fclose(fp);
				sys_err("wrong empire number is null");
				return false;
			}
		}
		else TOKEN("frog")
		{
			int32_t	pos = 0;

			while ((v = strtok(nullptr, delim)))
			{
				str_to_number(value, v);
				if (value)
				{
					castle_spawn_frog_force(empire, pos);
				}
				++pos;
			}
		}
		else TOKEN("guard")
		{
			int32_t	group_vnum		= 0;

			while ((v = strtok(nullptr, delim)))
			{
				str_to_number(group_vnum, v);
				if (group_vnum)
				{
					castle_spawn_guard(empire, group_vnum, guard_region);
				}
			}

			++guard_region;
		}
		else TOKEN("end")
		{
			guard_region = 0;
		}
	}

	fclose(fp);

	return true;
}

void castle_save()
{
	if (nullptr == s_castle)
		return;

	const char	*castle_file = "castle_data.txt";
	FILE		*fp;

	fp = fopen(castle_file, "w");

	if (nullptr == fp)
	{
		sys_err("<FAIL> fopen(%s)", castle_file);
		return;
	}

	DO_ALL_EMPIRE(empire)
	{
		fprintf(fp, "section\n");

		fprintf(fp, "\tempire %d\n", empire);

		fprintf(fp, "\tfrog ");
		for (int32_t i = 0; i < MAX_CASTLE_FROG; ++i)
		{
			fprintf(fp, " %d", GET_FROG(empire, i) ? 1 : 0);
		}
		fprintf(fp, "\n");

		for (int32_t region_index = 0; region_index < MAX_CASTLE_GUARD_REGION; ++region_index)
		{
			fprintf(fp, "\tguard ");
			for (int32_t guard_index = 0; guard_index < MAX_CASTLE_GUARD_PER_REGION; ++guard_index)
			{
				fprintf(fp, " %u", GET_GUARD_GROUP(empire, region_index, guard_index));
			}
			fprintf(fp, "\n");
		}
		fprintf(fp, "end\n");
	}

	fclose(fp);
}

int32_t castle_siege(int32_t empire, int32_t tower_count)
{
	{
		if (nullptr == SECTREE_MANAGER::instance().GetMap(181)) return 0;
		if (nullptr == SECTREE_MANAGER::instance().GetMap(182)) return 0;
		if (nullptr == SECTREE_MANAGER::instance().GetMap(183)) return 0;
	}

	switch (GET_SIEGE_STATE())
	{
		case CASTLE_SIEGE_NONE:
			castle_start_siege(empire, tower_count);
			return 1;
			break;

		case CASTLE_SIEGE_STRUGGLE:
		case CASTLE_SIEGE_END:
			castle_end_siege();
			return 2;
			break;
	}

	return 0;
}

void castle_start_siege(int32_t empire, int32_t tower_count)
{
	if (CASTLE_SIEGE_NONE != GET_SIEGE_STATE())
		return;

	GET_SIEGE_STATE()	= CASTLE_SIEGE_STRUGGLE;
	GET_SIEGE_EMPIRE()	= empire;

	castle_spawn_tower(empire, tower_count);

	{
		castle_event_info* info = AllocEventInfo<castle_event_info>();

		info->empire = empire;
		info->pulse	= 0;

		GET_SIEGE_EVENT(empire) = event_create(castle_siege_event, info, SIEGE_EVENT_PULSE);
	}

	{
		castle_stone_event_info* info = AllocEventInfo<castle_stone_event_info>();

		info->spawn_count = 0;
		info->empire = empire;

		GET_STONE_EVENT(empire) = event_create(castle_stone_event, info, PASSES_PER_SEC(1));
	}
}

void castle_end_siege()
{
	GET_SIEGE_EMPIRE()	= EMPIRE_NONE;
	GET_SIEGE_STATE()	= CASTLE_SIEGE_NONE;

	DO_ALL_EMPIRE(empire)
	{
		if (GET_SIEGE_EVENT(empire))
		{
			event_cancel(&GET_SIEGE_EVENT(empire));
		}

		DO_ALL_TOWER(i)
		{
			if (GET_TOWER(empire, i))
			{
				LPCHARACTER	npc = GET_TOWER(empire, i);
				M2_DESTROY_CHARACTER(npc);
				GET_TOWER(empire, i) = nullptr;
			}
		}
	}
}


LPCHARACTER castle_spawn_frog(int32_t empire)
{
	int32_t		dir = 1;
	int32_t	map_index	= FN_castle_map_index(empire);

	POSITION	*empty_pos = FN_empty_frog_pos(empire);
	if (nullptr == empty_pos)
		return nullptr;

	SECTREE_MAP	*sectree_map = SECTREE_MANAGER::instance().GetMap(map_index);
	if (nullptr == sectree_map)
		return nullptr;
	uint32_t x = sectree_map->m_setting.iBaseX + 100*empty_pos->x;
	uint32_t y = sectree_map->m_setting.iBaseY + 100*empty_pos->y;

	LPCHARACTER frog = CHARACTER_MANAGER::instance().SpawnMob(CASTLE_FROG_VNUM, map_index,
															x, y, 0 ,
															false, dir);
	if (frog)
	{
		frog->SetEmpire(empire);
		int32_t empty_index	= FN_empty_frog_index(empire);
		GET_FROG(empire, empty_index) = frog;
		return frog;
	}
	return nullptr;
}

LPCHARACTER castle_spawn_frog_force(int32_t empire, int32_t empty_index)
{
	int32_t		dir = 1;
	int32_t	map_index	= FN_castle_map_index(empire);

	POSITION	*empty_pos = &GET_FROG_POS(empire, empty_index);

	SECTREE_MAP	*sectree_map = SECTREE_MANAGER::instance().GetMap(map_index);
	if (nullptr == sectree_map)
	{
		return nullptr;
	}
	uint32_t x = sectree_map->m_setting.iBaseX + 100*empty_pos->x;
	uint32_t y = sectree_map->m_setting.iBaseY + 100*empty_pos->y;

	LPCHARACTER frog = CHARACTER_MANAGER::instance().SpawnMob(CASTLE_FROG_VNUM, map_index,
															x, y, 0 ,
															false, dir);
	if (frog)
	{
		frog->SetEmpire(empire);
		GET_FROG(empire, empty_index) = frog;
		return frog;
	}
	return nullptr;
}


LPCHARACTER castle_spawn_guard(int32_t empire, uint32_t group_vnum, int32_t region_index)
{
	LPCHARACTER	mob;
	int32_t	sx, sy, ex, ey;
	int32_t	map_index	= FN_castle_map_index(empire);

	SECTREE_MAP	*sectree_map = SECTREE_MANAGER::instance().GetMap(map_index);
	if (nullptr == sectree_map)
		return nullptr;

	if (castle_guard_count(empire, region_index) >= MAX_CASTLE_GUARD_PER_REGION)
		return nullptr;

	sx = sectree_map->m_setting.iBaseX + 100*GET_GUARD_REGION(empire, region_index).sx;
	sy = sectree_map->m_setting.iBaseY + 100*GET_GUARD_REGION(empire, region_index).sy;
	ex = sectree_map->m_setting.iBaseX + 100*GET_GUARD_REGION(empire, region_index).ex;
	ey = sectree_map->m_setting.iBaseY + 100*GET_GUARD_REGION(empire, region_index).ey;

	mob = CHARACTER_MANAGER::instance().SpawnGroup(group_vnum, map_index,
													sx, sy, ex, ey);
	if (mob)
	{
		mob->SetEmpire(empire);

		int32_t	pos = FN_empty_guard_pos(empire, region_index);
		GET_GUARD(empire, region_index, pos) = mob;
		GET_GUARD_GROUP(empire, region_index, pos) = group_vnum;
	}

	return mob;
}


static uint32_t FN_random_tower()
{
	uint32_t vnum[5] =
	{
		11506,
		11507,
		11508,
		11509,
		11510
	};

	int32_t index = number(0, 4);
	return vnum[index];
}

static void FN_spawn_tower(int32_t empire, LPSECTREE_MAP sectree_map)
{
	DO_ALL_TOWER(i)
	{
		if (GET_TOWER(empire, i))
			continue;

		uint32_t	sx = sectree_map->m_setting.iBaseX + 100 * GET_TOWER_REGION(empire).sx;
		uint32_t	sy = sectree_map->m_setting.iBaseY + 100 * GET_TOWER_REGION(empire).sy;
		uint32_t	ex = sectree_map->m_setting.iBaseX + 100 * GET_TOWER_REGION(empire).ex;
		uint32_t	ey = sectree_map->m_setting.iBaseY + 100 * GET_TOWER_REGION(empire).ey;

		GET_TOWER(empire, i) =
			CHARACTER_MANAGER::instance().SpawnMobRange(FN_random_tower(),
														FN_castle_map_index(empire),
														sx, sy, ex, ey);
		GET_TOWER(empire, i)->SetEmpire(empire);
		return;
	}
}

bool castle_spawn_tower(int32_t empire, int32_t tower_count)
{
	int32_t	map_index = FN_castle_map_index(empire);
	SECTREE_MAP	*sectree_map = SECTREE_MANAGER::instance().GetMap(map_index);
	if (nullptr == sectree_map)
		return false;

	DO_ALL_TOWER(i)
	{
		if (GET_TOWER(empire, i))
				GET_TOWER(empire, i)->Dead(nullptr, true);
		GET_TOWER(empire, i) = nullptr;
	}

	int32_t	spawn_count = MINMAX(MIN_CASTLE_TOWER, tower_count, MAX_CASTLE_TOWER);

	for (int32_t j = 0; j < spawn_count; ++j)
	{
		FN_spawn_tower(empire, sectree_map);
	}

	{
		char buf[1024];
		snprintf(buf, sizeof(buf), LC_TEXT("%s에 전쟁의 시작을 알리는 봉화가 나타났습니다."), EMPIRE_NAME(empire));
		BroadcastNotice(buf);
	}
	return true;
}

void castle_guard_die(LPCHARACTER ch, LPCHARACTER killer)
{
	int32_t	empire = ch->GetEmpire();

	for (int32_t region_index = 0; region_index < MAX_CASTLE_GUARD_REGION; ++region_index)
	{
		for (int32_t i = 0; i < MAX_CASTLE_GUARD_PER_REGION; ++i)
		{
			if (GET_GUARD(empire, region_index, i) == ch)
			{
				GET_GUARD(empire, region_index, i) = nullptr;
				GET_GUARD_GROUP(empire, region_index, i) = 0;
			}
		}
	}

	castle_save();
}


void castle_frog_die(LPCHARACTER ch, LPCHARACTER killer)
{
	if (nullptr == ch || nullptr == killer)
		return;

	int32_t	empire = ch->GetEmpire();

	DO_ALL_FROG(i)
	{
		if (ch == GET_FROG(empire, i))
		{
			GET_FROG(empire, i) = nullptr;

			killer->ChangeGold(true, 10000000, true);
			castle_save();
			return;
		}
	}
}

void castle_tower_die(LPCHARACTER ch, LPCHARACTER killer)
{
	char	buf[1024] = {0};

	if (nullptr == ch || nullptr == killer)
		return;

	int32_t	killer_empire = killer->GetEmpire();

	switch (GET_SIEGE_STATE())
	{
		case CASTLE_SIEGE_NONE:
			break;

		case CASTLE_SIEGE_STRUGGLE:
		case CASTLE_SIEGE_END:
			{
				int32_t	siege_end = true;
				snprintf(buf, sizeof(buf), LC_TEXT("%s이 봉화를 파괴했습니다."), EMPIRE_NAME(killer_empire));
				BroadcastNotice(buf);

				LogManager::instance().CharLog(killer, 0, "CASTLE_TORCH_KILL", "");

				DO_ALL_TOWER(i)
				{
					if (ch == GET_TOWER(GET_SIEGE_EMPIRE(), i))
						GET_TOWER(GET_SIEGE_EMPIRE(), i) = nullptr;
				}

				DO_ALL_TOWER(i)
				{
					if (GET_TOWER(GET_SIEGE_EMPIRE(), i))
						siege_end = false;
				}

				if (siege_end)
				{
					if (GET_SIEGE_STATE() == CASTLE_SIEGE_STRUGGLE)
					{
						snprintf(buf, sizeof(buf), LC_TEXT("%s이 수성에 실패하여 전쟁에 패배하였습니다.."), EMPIRE_NAME(GET_SIEGE_EMPIRE()));
						BroadcastNotice(buf);
					}
					else
					{
						snprintf(buf, sizeof(buf), LC_TEXT("%s이 모든 봉화를 파괴하였습니다."), EMPIRE_NAME(GET_SIEGE_EMPIRE()));
						BroadcastNotice(buf);
					}
					castle_end_siege();
				}
			}
			break;
	}
}


int32_t castle_guard_count(int32_t empire, int32_t region_index)
{
	int32_t count = 0;

	for (int32_t i = 0; i < MAX_CASTLE_GUARD_PER_REGION; ++i)
	{
		if ( GET_GUARD(empire, region_index, i) )
			++count;
	}
	return count;
}


int32_t castle_frog_count(int32_t empire)
{
	int32_t		count = 0;
	DO_ALL_FROG(i)
	{
		if (GET_FROG(empire, i))
			++count;
	}
	return count;
}

bool castle_is_guard_vnum(uint32_t vnum)
{
	switch (vnum)
	{
		case 11112:
		case 11114:
		case 11116:
		case 11106:
		case 11108:
		case 11110:
		case 11100:
		case 11102:
		case 11104:
		case 11113:
		case 11115:
		case 11117:
		case 11107:
		case 11109:
		case 11111:
		case 11101:
		case 11103:
		case 11105:
			return true;
	}

	return false;
}

int32_t castle_cost_of_hiring_guard(uint32_t group_vnum)
{
	switch (group_vnum)
	{
		case 9501:
		case 9511:
		case 9521:

		case 9502:
		case 9512:
		case 9522:
			return (100*10000);

		case 9503:
		case 9513:
		case 9523:

		case 9504:
		case 9514:
		case 9524:
			return (300*10000);

		case 9505:
		case 9515:
		case 9525:

		case 9506:
		case 9516:
		case 9526:
			return (1000*10000);
	}

	return 0;
}

bool castle_can_attack(LPCHARACTER ch, LPCHARACTER victim)
{
	if (nullptr == ch || nullptr == victim)
		return false;

	if (false == FN_is_castle_map(ch->GetMapIndex()))
		return false;

	if (ch->IsPC() && victim->IsPC())
		return true;

	if (CASTLE_SIEGE_END == GET_SIEGE_STATE())
	{
		if (castle_is_tower_vnum(victim->GetRaceNum()))
		{
			if (ch->GetEmpire() == victim->GetEmpire())
				return true;
			else
				return false;
		}
	}

	if (ch->GetEmpire() == victim->GetEmpire())
		return false;

	return true;
}

bool castle_frog_to_empire_money(LPCHARACTER ch)
{
	if (nullptr == ch)
		return false;

	int32_t empire = ch->GetEmpire();

	DO_ALL_FROG(i)
	{
		if (nullptr == GET_FROG(empire, i))
			continue;

		LPCHARACTER	npc = GET_FROG(empire, i);

		if (false == CMonarch::instance().SendtoDBAddMoney(CASTLE_FROG_PRICE, empire, ch))
			return false;

		GET_FROG(empire, i) = nullptr;
		npc->Dead(nullptr, true);
		return true;
	}

	return false;
}

bool castle_is_my_castle(int32_t empire, int32_t map_index)
{
	switch (empire)
	{
		case EMPIRE_RED:	return (181 == map_index);
		case EMPIRE_YELLOW: return (183 == map_index);
		case EMPIRE_BLUE:	return (182 == map_index);
	}
	return false;
}

bool castle_is_tower_vnum(uint32_t vnum)
{
	switch (vnum)
	{
		case 11506:
		case 11507:
		case 11508:
		case 11509:
		case 11510:
			return true;
	}
	return false;
}


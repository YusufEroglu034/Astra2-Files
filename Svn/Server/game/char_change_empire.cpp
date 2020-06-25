
#include "stdafx.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "db.h"
#include "guild_manager.h"
#include "marriage.h"

int32_t CHARACTER::ChangeEmpire(uint8_t empire)
{
	if (GetEmpire() == empire)
		return 1;

	char szQuery[1024+1];
	uint32_t dwAID;
	uint32_t dwPID[4];
	memset(dwPID, 0, sizeof(dwPID));

	{
#ifdef ENABLE_PLAYER_PER_ACCOUNT5 // @correction112
		snprintf(szQuery, sizeof(szQuery),
				"SELECT id, pid1, pid2, pid3, pid4, pid5 FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
				get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#else
		snprintf(szQuery, sizeof(szQuery),
				"SELECT id, pid1, pid2, pid3, pid4 FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u AND empire=%u",
				get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#endif
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(szQuery));

		if (pMsg->Get()->uiNumRows == 0)
		{
			return 0;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		str_to_number(dwAID, row[0]);
		str_to_number(dwPID[0], row[1]);
		str_to_number(dwPID[1], row[2]);
		str_to_number(dwPID[2], row[3]);
		str_to_number(dwPID[3], row[4]);
	}

	const int32_t loop = 4;

	{
		uint32_t dwGuildID[4];
		CGuild * pGuild[4];
		SQLMsg * pMsg2 = nullptr;

		for (int32_t i = 0; i < loop; ++i)
		{
			snprintf(szQuery, sizeof(szQuery), "SELECT guild_id FROM guild_member%s WHERE pid=%u", get_table_postfix(), dwPID[i]);

			pMsg2 = DBManager::instance().DirectQuery(szQuery);

			if (pMsg2)
			{
				if (pMsg2->Get()->uiNumRows > 0)
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg2->Get()->pSQLResult);

					str_to_number(dwGuildID[i], row[0]);

					pGuild[i] = CGuildManager::instance().FindGuild(dwGuildID[i]);

					if (pGuild[i] != nullptr)
					{
						M2_DELETE(pMsg2);
						return 2;
					}
				}
				else
				{
					dwGuildID[i] = 0;
					pGuild[i] = nullptr;
				}

				M2_DELETE(pMsg2);
				pMsg2 = nullptr; // @correction023(because the pointer can still point to a place in memory)
			}
		}
	}

	{
		for (int32_t i = 0; i < loop; ++i)
		{
			if (marriage::CManager::instance().IsEngagedOrMarried(dwPID[i]) == true)
				return 3;
		}
	}

	{
		snprintf(szQuery, sizeof(szQuery), "UPDATE player_index%s SET empire=%u WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u AND empire=%u",
				get_table_postfix(), empire, GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
		std::unique_ptr<SQLMsg> pMsg2(DBManager::instance().DirectQuery(szQuery));

		if (pMsg2->Get()->uiAffectedRows > 0)
		{
			SetChangeEmpireCount();
// @correction127 BEIGN
			SetEmpire(empire);
			UpdatePacket();
// @correction127 END
			return 999;
		}
	}

	return 0;
}

int32_t CHARACTER::GetChangeEmpireCount() const
{
	char szQuery[1024+1];
	uint32_t dwAID = GetAID();

	if (dwAID == 0)
		return 0;

	snprintf(szQuery, sizeof(szQuery), "SELECT change_count FROM change_empire WHERE account_id = %u", dwAID);

	SQLMsg * pMsg = DBManager::instance().DirectQuery(szQuery);

	if (pMsg != nullptr)
	{
		if (pMsg->Get()->uiNumRows == 0)
		{
			M2_DELETE(pMsg);
			return 0;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		uint32_t count = 0;
		str_to_number(count, row[0]);

		M2_DELETE(pMsg);

		return count;
	}

	return 0;
}

void CHARACTER::SetChangeEmpireCount()
{
	char szQuery[1024+1];

	uint32_t dwAID = GetAID();

	if (dwAID == 0) return;

	int32_t count = GetChangeEmpireCount();

	if (count == 0)
	{
		count++;
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO change_empire VALUES(%u, %d, NOW())", dwAID, count);
	}
	else
	{
		count++;
		snprintf(szQuery, sizeof(szQuery), "UPDATE change_empire SET change_count=%d WHERE account_id=%u", count, dwAID);
	}

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(szQuery));
}

uint32_t CHARACTER::GetAID() const
{
	char szQuery[1024+1];
	uint32_t dwAID = 0;
#ifdef ENABLE_PLAYER_PER_ACCOUNT5 // @correction112
	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
			get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#else
	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u AND empire=%u",
			get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#endif
	SQLMsg* pMsg = DBManager::instance().DirectQuery(szQuery);

	if (pMsg != nullptr)
	{
		if (pMsg->Get()->uiNumRows == 0)
		{
			M2_DELETE(pMsg);
			return 0;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		str_to_number(dwAID, row[0]);

		M2_DELETE(pMsg);

		return dwAID;
	}
	else
	{
		return 0;
	}
}


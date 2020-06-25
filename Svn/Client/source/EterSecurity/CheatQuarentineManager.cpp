#include "StdAfx.h"
#include "CheatQuarentineManager.h"
#include "CheatQueueManager.h"
#include "AnticheatManager.h"

CCheatQuarentineMgr::CCheatQuarentineMgr()
{
	m_vProcessMd5List.clear();
}

void CCheatQuarentineMgr::ProcessQuarentineList(int32_t iIdx, bool bSingle)
{
	if (iIdx == NET_SCAN_ID_PROCESS_HASH)
	{
		CheckProcessMd5Hashes(bSingle);
	}
}

void CCheatQuarentineMgr::AppendProcessMd5(const std::string & strProcessName, const std::string & strMd5, bool bBlacklisted)
{
	if (bBlacklisted)
	{
		for (const auto & pCurrBlacklistItem : m_vProcessMd5Blacklist)
		{
			if (!strcmp(pCurrBlacklistItem->szProcessMd5, strMd5.c_str()))
			{
				return;
			}
		}
	}

	auto pScanData = std::make_shared<SProcessMd5ScanData>();
	if (!pScanData || !pScanData.get())
		return;

	strcpy_s(pScanData->szProcessName, strProcessName.c_str());
	strcpy_s(pScanData->szProcessMd5, strMd5.c_str());

	if (bBlacklisted)
		m_vProcessMd5Blacklist.emplace_back(pScanData);
	else
		m_vProcessMd5List.emplace_back(pScanData);
}

void CCheatQuarentineMgr::CheckProcessMd5Hashes(bool bSingle)
{
	if (m_vProcessMd5Blacklist.empty())
	{
		return;
	}

	for (const auto & pCurrScanCtx : m_vProcessMd5Blacklist)
	{
		if (pCurrScanCtx && pCurrScanCtx.get())
		{

			for (const auto & pCurrProcessCtx : m_vProcessMd5List)
			{
				if (pCurrProcessCtx && pCurrProcessCtx.get())
				{
					if (!strcmp(pCurrScanCtx->szProcessMd5, pCurrProcessCtx->szProcessMd5))
					{

						CCheatDetectQueueMgr::Instance().AppendDetection(BAD_PROCESS_DETECT, 0 , pCurrScanCtx->szProcessMd5);

						m_vProcessMd5Blacklist.erase(std::remove(m_vProcessMd5Blacklist.begin(), m_vProcessMd5Blacklist.end(), pCurrScanCtx), m_vProcessMd5Blacklist.end());
						m_vProcessMd5List.erase(std::remove(m_vProcessMd5List.begin(), m_vProcessMd5List.end(), pCurrProcessCtx), m_vProcessMd5List.end());

						break;
					}
				}
			}
		}
	}

	if (bSingle == false)
	{
		m_vProcessMd5List.clear();
	}
}


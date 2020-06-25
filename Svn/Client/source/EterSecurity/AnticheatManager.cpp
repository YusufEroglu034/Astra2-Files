#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "../eterBase/SimpleTimer.h"
#include "../UserInterface/Locale_inc.h"
#include "../eterBase/MappedFile.h"

#include <XORstr.h>
#include <Wtsapi32.h>
#include <sstream>

#include "DataBuffer.hpp"
#include "md5.h"
#include "Exception.h"
#include "ProtectionMacros.h"

static auto gs_pWatchdogCheckTimer = std::make_unique <CSimpleTimer <std::chrono::milliseconds> >();

void NTAPI OnSyncWait(PVOID pParam, BOOLEAN)
{
	static auto bBlocked = false;
	if (bBlocked)
		return;

	if (gs_pWatchdogCheckTimer->diff() > 30000)
	{
		if (CAnticheatManager::Instance().GetWatchdogStatus() == false)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(WATCHDOG_SIGNAL_LOST, 0, "");
			bBlocked = true;
		}
		CAnticheatManager::Instance().SendWatchdogSignal(false);

		gs_pWatchdogCheckTimer->reset();
	}
}

CAnticheatManager::CAnticheatManager() :
	m_hWndCheckThread(INVALID_HANDLE_VALUE), m_hWndThreadWaitObj(INVALID_HANDLE_VALUE),
	m_hSyncProc(INVALID_HANDLE_VALUE), m_hSyncWaitObj(INVALID_HANDLE_VALUE),
	m_pDumpGuardMem(nullptr)
{
}

void CAnticheatManager::InitializeAnticheatRoutines(HINSTANCE hInstance, HWND hWnd)
{
#ifndef ENABLE_ANTICHEAT
	return;
#else
	m_bAggressiveMode = ENABLE_ANTICHEAT_AGGRESSIVE_MODE;
#endif

#if !defined(_DEBUG)
#if USE_ENIGMA_SDK == 1
	if (EP_CheckupIsProtected() && EP_CheckupIsEnigmaOk() == false)
#elif USE_VMPROTECT_SDK == 1
	if (VMProtectIsValidImageCRC() == false)
#elif USE_SHIELDEN_SDK == 1
	if (SECheckProtection() == false)
#else
	if (false)
#endif
	{
		ExitByAnticheat(PROTECTION_CRACK, 0, 0, true);
		return;
	}
#endif

	m_pQuerantineMgr = std::make_shared<CCheatQuarentineMgr>();

	m_hInstance = hInstance;
	m_hWnd = hWnd;
	m_dwMainThreadId = GetCurrentThreadId();
	m_wndpOldProc = nullptr;
	m_hkMessageHook = nullptr;

 	m_mapKeyStatusList[0] = false;
 	m_mapKeyStatusList[1] = false;
 	m_mapKeyStatusList[2] = false;

	SetupSectionMonitorContainer();

#if !defined(_DEBUG) && !defined(ENABLE_DEBUGGER_ALLOW_BY_ANTICHEAT)
	SetAntiTrace(GetCurrentThread());
	CheckExceptionTrap();
	SimpleDebugCheck();
	DebugCheckBugControl();
	DebugObjectHandleCheck();
	SharedUserDataCheck();
	GlobalFlagsCheck();
#endif

	CheckMainFolderFiles();
	CheckMilesFolderForMilesPlugins();
	CheckLibFolderForPythonLibs();
	CheckYmirFolder();

	BlockAccess(GetCurrentProcess());
	BlockAccess(GetCurrentThread());
	DenyProcessAccess();
	DenyThreadAccess();

	InitializeExceptionFilters();
	InitializeAnticheatWatchdog();

	

	LoadDllFilter();

	ParentProcessCheck(XOR("astra2 patcher.exe"));

	InitializeWindowWatcherThread();

	InitSectionHashes();

	m_hSyncProc = OpenProcess(SYNCHRONIZE, false, GetCurrentProcessId());
	if (m_hSyncProc && m_hSyncProc != INVALID_HANDLE_VALUE)
	{
		gs_pWatchdogCheckTimer->reset();

		auto bRegisterWaitRet = RegisterWaitForSingleObject(&m_hSyncWaitObj, m_hSyncProc, OnSyncWait, nullptr, 12000, WT_EXECUTEDEFAULT);
		if (bRegisterWaitRet == false)
		{
			CCheatDetectQueueMgr::Instance().AppendDetection(SYNC_PROC_REGISTER_FAIL, GetLastError(), "");
		}
	}
}

void CAnticheatManager::FinalizeAnticheatRoutines()
{
#ifndef ENABLE_ANTICHEAT
	return;
#endif

	if (m_hSyncWaitObj && m_hSyncWaitObj != INVALID_HANDLE_VALUE)
		UnregisterWait(m_hSyncWaitObj);
	if (m_hSyncProc && m_hSyncProc != INVALID_HANDLE_VALUE)
		CloseHandle(m_hSyncProc);

	RemoveAnticheatWatchdog();
	RemoveExceptionHandlers();

	DestroyWindowWatcher();

	
}

void CAnticheatManager::ParseCheatBlacklist(const std::string & szContent)
{
	int32_t iScanID = -1;

#if 0
	try
	{
		std::istringstream iss(szContent);

		boost::property_tree::ptree root;
		boost::property_tree::json_parser::read_json(iss, root);

		

		auto headerTree = root.get_child(XOR("Header"));
		if (headerTree.empty())
		{
#ifdef _DEBUG
			TraceError("Blacklist header not found!");
#endif
			return;
		}

		auto scanId = headerTree.get<std::string>(XOR("ScanID"));
		auto rowCount = headerTree.get<std::string>(XOR("RowCount"));
		auto fieldCount = headerTree.get<std::string>(XOR("FieldCount"));

		if (scanId.empty() || rowCount.empty() || fieldCount.empty())
		{
#ifdef _DEBUG
			TraceError("Blacklist header context not found! ID: %s Row: %s Field: %s", scanId.c_str(), rowCount.c_str(), fieldCount.c_str());
#endif
			return;
		}

#ifdef _DEBUG
		TraceError("Blacklist process started! ScanID: %s Row Count: %s Field Count: %s", scanId.c_str(), rowCount.c_str(), fieldCount.c_str());
#endif

		iScanID = std::stoi(scanId);
		if (!(iScanID > NET_SCAN_ID_NULL && iScanID < NET_SCAN_ID_MAX))
		{
#ifdef _DEBUG
			TraceError("Unallowed scan ID: %d", iScanID);
#endif
			return;
		}

		auto nRowCount = std::stoi(rowCount);
		for (auto i = 1; i <= nRowCount; ++i)
		{

			auto strCurrNode = std::to_string(i);
			auto currNode = root.get_child(strCurrNode);
			if (currNode.empty())
			{
#ifdef _DEBUG
				TraceError("Current node: %s is empty!", strCurrNode.c_str());
#endif
				continue;
			}

			switch (iScanID)
			{
				case NET_SCAN_ID_PROCESS_HASH:
				{
					auto strMd5Hash = currNode.get<std::string>(XOR("Md5Hash"));

					CCheatQuarentineMgr::Instance().AppendProcessMd5(strCurrNode, strMd5Hash, true);

				} break;

				default:
#ifdef _DEBUG
					TraceError("Unknown scan ID: %d", iScanID);
#endif
					break;
			}
		}
	}
	catch (const std::exception & e)
	{
#ifdef _DEBUG
		TraceError("Blacklist json parse exception: %s", e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}

	if (iScanID != -1)
	{
		CCheatQuarentineMgr::Instance().ProcessQuarentineList(iScanID, false);
	}
#endif
}

uint32_t CAnticheatManager::ServiceMessageBox(const std::string & szTitle, const std::string & szMessage, uint16_t wType)
{
	BOOL bRet = false;
	auto dwResponse = 0UL;

	bRet = WTSSendMessageA(
		WTS_CURRENT_SERVER_HANDLE,
		WTSGetActiveConsoleSessionId(),
		const_cast<LPSTR>(szTitle.c_str()),
		static_cast<uint32_t>(strlen(szTitle.c_str())),
		const_cast<LPSTR>(szMessage.c_str()),
		static_cast<uint32_t>(strlen(szMessage.c_str())),
		wType,
		0,
		&dwResponse,
		false
	);

	return dwResponse;
}

bool CAnticheatManager::IsFileExist(const std::string & szFileName)
{
	auto dwAttrib = GetFileAttributesA(szFileName.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool CAnticheatManager::IsDirExist(const std::string & szDirName)
{
	auto dwAttrib = GetFileAttributesA(szDirName.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::string CAnticheatManager::GetMd5(const uint8_t* pData, std::size_t nSize)
{
	auto strHash = std::string("");

	try
	{
		MD5 md5buf;
		md5buf.add(pData, nSize);
		md5buf.calculate();

		strHash = md5buf.get_hash();
	}
	catch (Exception & e)
	{
#ifdef _DEBUG
		TraceError("CMd5|BuildHash exception! Data: %p Size: %u Reason: %s", pData, nSize, e.what());
#else
		UNREFERENCED_PARAMETER(e);
#endif
	}
	catch (...)
	{
#ifdef _DEBUG
		TraceError("CMd5|BuildHash unhandled exception!");
#endif
	}

	std::transform(strHash.begin(), strHash.end(), strHash.begin(), tolower);
	return strHash;
}

std::string CAnticheatManager::GetFileMd5(const std::string & szName)
{
	std::string szOutput;
	__PROTECTOR_START__("GetMd5")
	CMappedFile pMappedFile;
	const uint8_t* c_pbMap;
	if (!pMappedFile.Create(szName.c_str(), (const void **)&c_pbMap, 0, 0))
		return szOutput;

	szOutput = GetMd5(c_pbMap, pMappedFile.Size());
	__PROTECTOR_END__("GetMd5")

	pMappedFile.Close();
	return szOutput;
}

void CAnticheatManager::ExitByAnticheat(uint32_t dwErrorCode, uint32_t dwSubCode1, uint32_t dwSubCode2, bool bForceClose, const std::string& stSubMessage)
{
	__PROTECTOR_START__("ExitByAnticheat")
	if (m_bAggressiveMode || bForceClose)
	{
#ifdef DEBUG

		auto stMessage = XOR("Anticheat violation detected! Error code: ") +
			std::to_string(dwErrorCode) + "-" + std::to_string(dwSubCode1) + "-" + std::to_string(dwSubCode2) +
			" " + stSubMessage;

		//TraceError(stMessage.c_str());
		ServiceMessageBox(XOR("Anticheat"), stMessage, MB_ICONERROR);

#endif // DEBUG
		__fastfail(-1);
	}
	__PROTECTOR_END__("ExitByAnticheat")
}

#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "dirent_win.h"
#include <XORstr.h>

inline static int32_t getdir(const std::string & dir, std::vector <std::string> & files)
{
	DIR * dp;
	if ((dp = opendir(dir.c_str())) == nullptr)
	{
		return errno;
	}

	struct dirent *dirp;
	while ((dirp = readdir(dp)) != nullptr)
	{
		auto file = std::string(dirp->d_name);
		if (file != "." && file != "..")
			files.emplace_back(std::string(dirp->d_name));
	}

	closedir(dp);
	return 0;
}

void CAnticheatManager::CheckMainFolderFiles()
{
	auto dir = std::string(".");
	auto files = std::vector<std::string>();

	getdir(dir, files);

	for (size_t i = 0; i < files.size(); i++)
	{
		std::string file = files[i];
		std::transform(file.begin(), file.end(), file.begin(), tolower);

		if (file.substr(file.find_last_of(".") + 1) == XOR("mix") ||
			file.substr(file.find_last_of(".") + 1) == XOR("flt") ||
			file.substr(file.find_last_of(".") + 1) == XOR("asi") ||
			file.substr(file.find_last_of(".") + 1) == XOR("m3d") ||
			file.substr(file.find_last_of(".") + 1) == XOR("def") ||
			file.substr(file.find_last_of(".") + 1) == XOR("py"))
		{
			// TraceError("Unallowed file found on main folder! File: %s", file.c_str());
			abort();
		}

		if (file == XOR("mss32.dll") && GetFileMd5(file) != XOR("6400e224b8b44ece59a992e6d8233719"))
		{
			// TraceError("mss32.dll file is corrupted! Please delete it and restart game");
			abort();
		}

		if (file == XOR("speedtreert.dll") && GetFileMd5(file) != XOR("1ac3d612389fa679f5ca3c6bab855145"))
		{
			// TraceError("speedtreert.dll file is corrupted! Please delete it and restart game");
			abort();
		}

		if (file == XOR("discord-rpc.dll") && GetFileMd5(file) != XOR("b6fd682381b825d2ceba5995c296dde3"))
		{
			// TraceError("speedtreert.dll file is corrupted! Please delete it and restart game");
			abort();
		}

		if (file == XOR("devil.dll") && GetFileMd5(file) != XOR("26eec5cc3d26cb38c93de01a3eb84cff"))
		{
			// TraceError("speedtreert.dll file is corrupted! Please delete it and restart game");
			abort();
		}
	}
}

void CAnticheatManager::CheckLibFolderForPythonLibs()
{
	if (IsDirExist(XOR("stdlib")))
	{
		TraceError("Please delete stdlib folder and restart game.");
		abort();
	}
}

void CAnticheatManager::CheckMilesFolderForMilesPlugins()
{
	auto dir = std::string(XOR("miles"));
	auto files = std::vector<std::string>();

	getdir(dir, files);

	static std::map <std::string , std::string > mapKnownFiles =
	{
		{ XOR("mssa3d.m3d"),	XOR("e089ce52b0617a6530069f22e0bdba2a") },
		{ XOR("mssds3d.m3d"),	XOR("85267776d45dbf5475c7d9882f08117c") },
		{ XOR("mssdsp.flt"),	XOR("cb71b1791009eca618e9b1ad4baa4fa9") },
		{ XOR("mssdx7.m3d"),	XOR("2727e2671482a55b2f1f16aa88d2780f") },
		{ XOR("msseax.m3d"),	XOR("788bd950efe89fa5166292bd6729fa62") },
		{ XOR("mssmp3.asi"),	XOR("189576dfe55af3b70db7e3e2312cd0fd") },
		{ XOR("mssrsx.m3d"),	XOR("7fae15b559eb91f491a5f75cfa103cd4") },
		{ XOR("msssoft.m3d"),	XOR("bdc9ad58ade17dbd939522eee447416f") },
		{ XOR("mssvoice.asi"),	XOR("3d5342edebe722748ace78c930f4d8a5") },
		{ XOR("mss32.dll"),		XOR("6400e224b8b44ece59a992e6d8233719") }
	};

	if (files.size() > mapKnownFiles.size())
	{
		// TraceError("Unknown file detected on miles folder! Please delete miles folder and restart game.");
		abort();
	}

	for (const auto & strCurrFolderFile : files)
	{
		auto strCurrFileLower = strCurrFolderFile;
		std::transform(strCurrFileLower.begin(), strCurrFileLower.end(), strCurrFileLower.begin(), tolower);

		auto it = mapKnownFiles.find(strCurrFileLower);
		if (it == mapKnownFiles.end())
		{
			// TraceError("Unknown file detected on miles folder! File: %s", strCurrFolderFile.c_str());
			abort();
		}

		std::string szPath = XOR("miles/");
		auto strCurrentMd5 = GetFileMd5(szPath + strCurrFileLower);
		auto strCorrectMd5 = it->second;
		if (strCurrentMd5 != strCorrectMd5)
		{
			// TraceError("Corrupted file detected on miles folder! File: %s", strCurrFolderFile.c_str());
			abort();
		}
	}
}

void CAnticheatManager::CheckYmirFolder()
{
	if (IsDirExist(XOR("d:/ymir work")) || IsDirExist(XOR("d:\\ymir work")))
	{
		TraceError("Unallowed folder: 'd:/ymir work' detected! Please delete it and restart game");
		abort();
	}
}


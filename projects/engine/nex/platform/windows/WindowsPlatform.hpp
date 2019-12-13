#pragma once

#ifdef WIN32
#include <vld.h>
#include <nex/platform/windows/CrashHandlerWin32.hpp>
#include <string>

std::wstring utf8ToUtf16(const std::filesystem::path& path)
{
	std::string utf8Str = path.generic_u8string();

	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), utf8Str.length(), NULL, 0);
	if (len > 0)
	{
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), utf8Str.length(), &ret[0], len);
	}
	return ret;
}

#endif
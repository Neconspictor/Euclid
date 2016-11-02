#ifndef UTIL_HPP
#define UTIL_HPP
#include <string>
#include <iomanip>
#include <vector>
#include <filesystem>
#include <sstream>

namespace platform
{
	namespace util
	{
		std::string backSlasheshToForwards(const std::string& source);

		std::string makeAbsolute(std::string path);

		std::string splitString(const std::string& source, const std::string& split);

		std::vector<std::string> tokenize(const std::string& source, const std::string& pattern);

		std::vector<std::wstring> tokenize(const std::wstring& source, const std::wstring& pattern);

		bool isParentFolderOf(const std::experimental::filesystem::path& parent,
			const std::experimental::filesystem::path& child);

		std::string relativePathToBuildDirectory(const std::string& source);

		std::string stringFromRegex(const std::string& source, const std::string& pattern, int splitIndex);


		inline std::string getCurrentTime()
		{
			auto t = time(nullptr);
			tm t2;
			localtime_s(&t2, &t);

			std::ostringstream oss;
			oss << std::put_time(&t2, "[%H:%M:%S]");
			return oss.str();
		}
	}
}


namespace util
{
	namespace global_path {
		static const std::string TEXTURE_PATH = "./_work/data/textures/";
		static const std::string SHADER_PATH = "./shaders/";
	}
}

#endif UTIL_HPP
#ifndef PLATFORM_UTIL_STRING_UTILS_HPP
#define PLATFORM_UTIL_STRING_UTILS_HPP

#include <string>
#include <vector>
#include <filesystem>

namespace platform {
	namespace util {
		std::string backSlasheshToForwards(const std::string& source);

		std::string makeAbsolute(std::string path);

		std::string splitString(const std::string& source, const std::string& split);

		std::vector<std::string> tokenize(const std::string& source, const std::string& pattern);

		std::vector<std::wstring> tokenize(const std::wstring& source, const std::wstring& pattern);

		bool isParentFolderOf(const std::experimental::filesystem::path& parent,
			const std::experimental::filesystem::path& child);

		std::string relativePathToBuildDirectory(const std::string& source);

		std::string stringFromRegex(const std::string& source, const std::string& pattern, int splitIndex);
	}
}
#endif
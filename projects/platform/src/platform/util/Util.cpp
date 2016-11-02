#include <platform/util/Util.hpp>
#include <regex>
#include <iostream>

namespace platform
{
	std::string util::backSlasheshToForwards(const std::string& source)
	{
		std::regex pattern("\\\\");
		return regex_replace(source, pattern, "/");
	}

	std::string util::makeAbsolute(std::string path)
	{
		std::experimental::filesystem::path systemPath(path);
		systemPath = canonical(systemPath);
		if (!systemPath.is_absolute())
		{
			std::cerr << "Couldn't convert file path: " << path << std::endl;
		}

		return systemPath.string();
	}

	std::string util::splitString(const std::string& source, const std::string& split)
	{
		// Unify source and split
		std::string result = backSlasheshToForwards(source);
		std::string splitCpy = backSlasheshToForwards(split);
		std::regex pattern(splitCpy);
		return regex_replace(result, pattern, "");
	}

	std::vector<std::string> util::tokenize(const std::string& source, const std::string& pattern)
	{
		std::regex re(pattern);
		std::sregex_token_iterator
			first{ source.begin(), source.end(), re, -1 },
			last;
		return{ first, last };
	}

	std::vector<std::wstring> util::tokenize(const std::wstring& source, const std::wstring& pattern)
	{
		std::wregex re(pattern);
		std::wsregex_token_iterator
			first{ source.begin(), source.end(), re, -1 },
			last;
		return{ first, last };
	}

	bool util::isParentFolderOf(const std::experimental::filesystem::path& parent, const std::experimental::filesystem::path& child)
	{
		std::wstring childStr = canonical(child).generic_wstring();
		std::wstring parentStr = canonical(parent).generic_wstring();

		// make paths case insensitive
		transform(childStr.begin(), childStr.end(), childStr.begin(), tolower);
		transform(parentStr.begin(), parentStr.end(), parentStr.begin(), tolower);

		std::vector<std::wstring> childTokens = tokenize(childStr, L"/");
		std::vector<std::wstring> parentTokens = tokenize(parentStr, L"/");

		for (int i = 0; i < parentTokens.size(); ++i)
		{
			if (parentTokens[i].compare(childTokens[i]) != 0)
			{
				//parent cannot be a parent folder of child!
				return false;
			}
		}

		return true;
	}

	std::string util::relativePathToBuildDirectory(const std::string& source)
	{
		if (!isParentFolderOf(NEC_BUILD_PATH, source))
		{
			// to do log error!
			std::cerr << "error!!!!!!!!!!!!!!!!!!" << std::endl;
			return source;
		}

		std::string buildStr = NEC_BUILD_PATH;
		std::string sourceCpy = source;
		transform(buildStr.begin(), buildStr.end(), buildStr.begin(), tolower);
		transform(sourceCpy.begin(), sourceCpy.end(), sourceCpy.begin(), tolower);

		return splitString(sourceCpy, buildStr);
	}

	std::string util::stringFromRegex(const std::string& source, const std::string& pattern, int splitIndex)
	{
		std::regex reg(pattern);
		std::smatch match;
		if (!regex_match(source, match, reg)) {
			return "";
		}
		return match[splitIndex + 1];
	}
}

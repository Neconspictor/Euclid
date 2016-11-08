#include <platform/util/StringUtils.hpp>
#include <regex>
#include <iostream>
#include <sstream>

using namespace std;

namespace platform {
	string util::backSlasheshToForwards(const string& source)
	{
		regex pattern("\\\\");
		return regex_replace(source, pattern, "/");
	}

	string util::makeAbsolute(string path)
	{
		experimental::filesystem::path systemPath(path);
		systemPath = canonical(systemPath);
		if (!systemPath.is_absolute())
		{
			cerr << "Couldn't convert file path: " << path << endl;
		}

		return systemPath.string();
	}

	string util::splitString(const string& source, const string& split)
	{
		// Unify source and split
		string result = backSlasheshToForwards(source);
		string splitCpy = backSlasheshToForwards(split);
		regex pattern(splitCpy);
		return regex_replace(result, pattern, "");
	}

	vector<string> util::tokenize(const string& source, const string& pattern)
	{
		regex re(pattern);
		sregex_token_iterator
			first{ source.begin(), source.end(), re, -1 },
			last;
		return{ first, last };
	}

	vector<wstring> util::tokenize(const wstring& source, const wstring& pattern)
	{
		wregex re(pattern);
		wsregex_token_iterator
			first{ source.begin(), source.end(), re, -1 },
			last;
		return{ first, last };
	}

	bool util::isParentFolderOf(const experimental::filesystem::path& parent, const experimental::filesystem::path& child)
	{
		wstring childStr = canonical(child).generic_wstring();
		wstring parentStr = canonical(parent).generic_wstring();

		// make paths case insensitive
		transform(childStr.begin(), childStr.end(), childStr.begin(), ::tolower);
		transform(parentStr.begin(), parentStr.end(), parentStr.begin(), ::tolower);

		vector<wstring> childTokens = tokenize(childStr, L"/");
		vector<wstring> parentTokens = tokenize(parentStr, L"/");

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

	string util::relativePathToBuildDirectory(const string& source)
	{
		if (!isParentFolderOf(NEC_BUILD_PATH, source))
		{
			stringstream ss;
			ss << "util::relativePathToBuildDirectory() : " << NEC_BUILD_PATH <<
				" is no parent directory of " << source << endl;
			throw runtime_error(ss.str());
		}

		string buildStr = NEC_BUILD_PATH;
		string sourceCpy = source;
		transform(buildStr.begin(), buildStr.end(), buildStr.begin(), ::tolower);
		transform(sourceCpy.begin(), sourceCpy.end(), sourceCpy.begin(), ::tolower);

		return splitString(sourceCpy, buildStr);
	}

	string util::stringFromRegex(const string& source, const string& pattern, int splitIndex)
	{
		regex reg(pattern);
		smatch match;
		if (!regex_match(source, match, reg)) {
			return "";
		}
		return match[splitIndex + 1];
	}
}
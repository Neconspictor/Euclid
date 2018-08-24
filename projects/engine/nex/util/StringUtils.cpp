#include <nex/util/StringUtils.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <regex>
#include <iostream>
#include <sstream>

using namespace std;

namespace nex::util {
	string backSlasheshToForwards(const string& source)
	{
		regex pattern("\\\\");
		return regex_replace(source, pattern, "/");
	}

	string makeAbsolute(string path)
	{
		experimental::filesystem::path systemPath(path);
		systemPath = canonical(systemPath);
		if (!systemPath.is_absolute())
		{
			cerr << "Couldn't convert file path: " << path << endl;
		}

		return systemPath.string();
	}

	string splitPath(const string& source, const string& split)
	{
		//TODO specification isn't implmeneted yet!
		// Unify source and split
		string result = backSlasheshToForwards(source);
		string splitCpy = backSlasheshToForwards(split);
		regex pattern(splitCpy);
		return regex_replace(result, pattern, "");
	}

	vector<string> tokenize(const string& source, const string& pattern)
	{
		regex re(pattern);
		sregex_token_iterator
			first{ source.begin(), source.end(), re, -1 },
			last;
		return{ first, last };
	}

	vector<wstring> tokenize(const wstring& source, const wstring& pattern)
	{
		wregex re(pattern);
		wsregex_token_iterator
			first{ source.begin(), source.end(), re, -1 },
			last;
		return{ first, last };
	}

	bool isParentFolderOf(const experimental::filesystem::path& parent, const experimental::filesystem::path& child)
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

	string relativePathToBuildDirectory(const string& source)
	{
		// TODO match specification!
		if (!isParentFolderOf(NEC_BUILD_PATH, source))
		{
			stringstream ss;
			ss << "util::relativePathToBuildDirectory() : " << NEC_BUILD_PATH <<
				" is no parent directory of " << source << endl;
			throw_with_trace(runtime_error(ss.str()));
		}

		string buildStr = NEC_BUILD_PATH;
		string sourceCpy = source;
		transform(buildStr.begin(), buildStr.end(), buildStr.begin(), ::tolower);
		transform(sourceCpy.begin(), sourceCpy.end(), sourceCpy.begin(), ::tolower);

		return splitPath(sourceCpy, buildStr);
	}

	string stringFromRegex(const string& source, const string& pattern, int splitIndex)
	{
		regex reg(pattern);
		smatch match;
		if (!regex_match(source, match, reg)) {
			return "";
		}
		return match[splitIndex + 1];
	}
}
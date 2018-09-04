#include <nex/util/StringUtils.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <regex>
#include <iostream>
#include <sstream>

using namespace std;

namespace nex::util {
	string backSlashesToForwards(const string& source)
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
		string result = backSlashesToForwards(source);
		string splitCpy = backSlashesToForwards(split);
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

	std::string removeComments_copy(const std::string& source)
	{
		CommentStateTracker tracker;
		std::string result;

		for (char c : source)
		{
			CommentState lastState = tracker.getCurrentState();
			tracker.update(c);
			CommentState currentState = tracker.getCurrentState();

			if (currentState == CommentState::LINE_COMMENT && !tracker.isCommentState(lastState))
			{
				result.pop_back();
			}
			else if (currentState == CommentState::MULTI_LINE_COMMENT && !tracker.isCommentState(lastState))
			{
				result.pop_back();
			}

			if ((!tracker.isCommentActive()
				&& !(tracker.isCommentState(lastState)
					&& !tracker.isCommentState(currentState)))
				|| c == '\n')
			{
				result.push_back(c);
			}
		}
		return result;
	}

	void removeComments(std::string& source)
	{
		CommentStateTracker tracker;

		std::vector<std::string::iterator> toErase;

		for (auto it = source.begin(); it != source.end(); )
		{
			CommentState lastState = tracker.getCurrentState();
			tracker.update(*it);
			CommentState currentState = tracker.getCurrentState();

			bool erased = false;

			if (currentState == CommentState::LINE_COMMENT && !tracker.isCommentState(lastState))
			{
				it = source.erase(it - 1); // erases the first slash
				it = source.erase(it); // erases the second slash
				erased = true;
			}
			else if (currentState == CommentState::MULTI_LINE_COMMENT && !tracker.isCommentState(lastState))
			{
				it = source.erase(it - 1); // erases the slash
				it = source.erase(it); // erases the asterix
				erased = true;
			}
			else if ((tracker.isCommentState(currentState) || tracker.isCommentState(lastState)) && *it != '\n')
			{
				it = source.erase(it);
				erased = true;
			}

			if (!erased)++it;
		}
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

	CommentStateTracker::CommentStateTracker(CommentState initialState): mState(initialState)
	{
	}

	void CommentStateTracker::update(char c)
	{
		bool isSlash = c == '/';
		bool isNewLine = c == '\n';
		bool isAsterix = c == '*';

		switch (mState)
		{
		case CommentState::NO_COMMENT:
			if (isSlash)
				mState = CommentState::NO_COMMENT_FIRST_SLASH;
			break;
		case CommentState::NO_COMMENT_FIRST_SLASH:
			if (isNewLine)
				mState = CommentState::NO_COMMENT;

			if (isSlash)
				mState = CommentState::LINE_COMMENT;
			else if (isAsterix)
				mState = CommentState::MULTI_LINE_COMMENT;
			else
				mState = CommentState::NO_COMMENT;

			break;
		case CommentState::LINE_COMMENT:
			if (isNewLine)
				mState = CommentState::NO_COMMENT;


			break;
		case CommentState::MULTI_LINE_COMMENT:
			if (isAsterix)
				mState = CommentState::MULTI_LINE_COMMENT_ASTERIX;

			break;
		case CommentState::MULTI_LINE_COMMENT_ASTERIX:

			// We continue multi line state if the current character is not a slash
			if (isSlash)
			{
				mState = CommentState::NO_COMMENT;
			}
			else if (!isAsterix)
			{
				mState = CommentState::MULTI_LINE_COMMENT;
			}

			break;
		default: throw std::runtime_error("Not registered CommentState: " + to_underlying(mState));
		}
	}

	CommentState CommentStateTracker::getCurrentState() const
	{
		return mState;
	}

	bool CommentStateTracker::isCommentActive() const
	{
		return mState != CommentState::NO_COMMENT
			&& mState != CommentState::NO_COMMENT_FIRST_SLASH;
	}

	bool CommentStateTracker::isCommentState(CommentState state) const
	{
		return state == CommentState::LINE_COMMENT
			|| state == CommentState::MULTI_LINE_COMMENT
			|| state == CommentState::MULTI_LINE_COMMENT_ASTERIX;
	}

	StringMatcher::StringMatcher(const char* text, std::string::iterator end): mText(text), mEnd(end)
	{
		assert(mText != nullptr);
	}

	std::string::iterator StringMatcher::getFirstAfterMatch() const
	{
		if (matched())
			return mAfterMatch;
		return mEnd;
	}

	void StringMatcher::update(const std::string::iterator next)
	{
		if (matched()) return;

		char c = *next;

		if (c == mText[mIndex])
		{
			mAfterMatch = next;
			if (next != mEnd)
				++mAfterMatch;
			++mIndex;
		}
	}

	bool StringMatcher::matched() const
	{
		return mText[mIndex] == '\0';
	}

	void StringMatcher::reset()
	{
		mIndex = 0;
		mAfterMatch = mEnd;
	}

	void Trim::ltrim(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
		{
			return !std::isspace(ch);
		}));
	}

	void Trim::rtrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
		{
			return !std::isspace(ch);
		}).base(), s.end());
	}

	void Trim::trim(std::string& s)
	{
		ltrim(s);
		rtrim(s);
	}

	std::string Trim::ltrim_copy(std::string s)
	{
		ltrim(s);
		return s;
	}

	std::string Trim::rtrim_copy(std::string s)
	{
		rtrim(s);
		return s;
	}

	std::string Trim::trim_copy(std::string s)
	{
		ltrim(s);
		rtrim(s);

		return s;
	}
}
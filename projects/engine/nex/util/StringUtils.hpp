#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <nex/exception/EnumFormatException.hpp>
#include <nex/util/ExceptionHandling.hpp>

#ifndef SID
#define SID(str) nex::util::customSimpleHash(str)
#endif

namespace nex::util {
	/**
	 * Converts back slashes in a string to forward slashes.
	 */
	std::string backSlashesToForwards(const std::string& source);

	/**
	 * Checks if a string ends with some other string.
	 */
	bool endsWith(const std::string& source, const std::string& expectedEnd);
	
	/**
	 * Makes a relative path to an absolute path, but only if the provided string
	 * Represents a valid path.
	 */
	std::string makeAbsolute(std::string path);

	/**
	 * Splits a given path at a given sub path. The result will be a relative path from 
	 * the sub path.
	 * NOTE: It is assumed that both arguments, 'path' and 'subPath' are valid paths and 'subPath'
	 * is indead a sub path from the argument 'path'. So 'path' begins with 'subPath'.
	 * If one of this conditions isn't fulfilled, an empty string will be returned!
	 */
	std::string splitPath(const std::string& path, const std::string& subPath);

	/**
	 * Returns a list of tokens that could be found on a given source string. The argument 'pattern'
	 * specifies the token to search for.
	 */
	std::vector<std::string> tokenize(const std::string& source, const std::string& pattern);

	/**
	* Returns a list of tokens that could be found on a given source string. The argument 'pattern'
	* specifies the token to search for.
	*/
	std::vector<std::wstring> tokenize(const std::wstring& source, const std::wstring& pattern);

	/**
	 * Checks if a given path 'parent' is a sub path of another path 'child'. 
	 */
	bool isParentFolderOf(const std::filesystem::path& parent,
		const std::filesystem::path& child);

	/**
	 * Creates from an absolute path 'source' a relative path using a second path 'from'
	 * NOTE: It is assumed that both arguments, 'source' and 'from' are valid paths.
	 * Additionally it is assumed that, 'from' is a parent folder of 'source'.
	 * If one of these conditions isn't fulfilled, an empty string will be returned.
	 */
	std::string relativePathToBuildDirectory(const std::string& source);

	/**
	 * Removes one-line and multi-line comments from a string.
	 * Produces a copy.
	 */
	std::string removeComments_copy(const std::string& source);

	/**
	* Removes one-line and multi-line comments from a string.
	*/
	void removeComments(std::string& source);

	/**
	 * Splits a given string 'source' by a regex pattern 'pattern'. The splitting creates a list of
	 * possible splittings. With the argument 'splitIndex' one can access a specific result.
	 */
	std::string stringFromRegex(const std::string& source, const std::string& pattern, int splitIndex);


	/**
	 * A class for mapping an enum to a string representation
	 * and vice versa
	 * NOTE: Mappings are supposed to be in upper case!
	 */
	template<class E>
	struct EnumString
	{
		E enumValue;
		std::string strValue;
	};

	/**
	 * Maps a given string 'str' to an enum of type 'E'.
	 * @param str: The string to map
	 * @param converter: An array of enum to string mappings. It is assumed, that
	 * all string mappings are in upper case!
	 * @return: The mapped enum.
	 *
	 * ATTENTION: This function throws a EnumFormatException if the string couldn't
	 * be mapped to an enum.
	 */
	template<class E, size_t S>
	E stringToEnum(const std::string& str, const EnumString<E> (& converter)[S])
	{
		std::string upper = str;
		transform(str.begin(), str.end(), upper.begin(), ::toupper);
		for (int i = 0; i < S; ++i)
		{
			if (upper.compare(converter[i].strValue) == 0)
				return converter[i].enumValue;
		}

		std::stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << " : Couldn't convert string to enum: " << str;
		throw_with_trace(EnumFormatException(ss.str()));

		// won't be reached
		return {};
	}

	template<class E, std::size_t S>
	std::string enumToString(E e, const EnumString<E>(&converter)[S])
	{
		for (int i = 0; i < S; ++i)
		{
			if (converter[i].enumValue == e)
				return converter[i].strValue;
		}

		std::stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << " : Couldn't convert enum to string: Enum class: " << typeid(E).name()
			<< ", enum value: " << e;
		throw_with_trace(EnumFormatException(ss.str()));

		// won't be reached
		return "";
	}


	template <typename E>
	static constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
		return static_cast<typename std::underlying_type<E>::type>(e);
	}

	enum class CommentState
	{
		NO_COMMENT, // We are currently in no comment
		NO_COMMENT_FIRST_SLASH, // There is no comment currently, but the current character is a slash (could be the begin of a comment)
		LINE_COMMENT, // We are in a line comment
		MULTI_LINE_COMMENT, // We are in a multi line comment
		MULTI_LINE_COMMENT_ASTERIX // We are in a multi line comment and the current character is an asterix
	};


	class CommentStateTracker
	{
	public:

		CommentStateTracker(CommentState initialState = CommentState::NO_COMMENT);

		void update(char c);
		CommentState getCurrentState() const;
		bool isCommentActive() const;
		bool isCommentState(CommentState state) const;

	private:

		/**
		* Holds the current state of Comment
		*/
		CommentState mState;
	};

	class StringMatcher
	{
	public:
		StringMatcher(const char* text, std::string::iterator end);

		std::string::iterator getFirstAfterMatch() const;
		void update(const std::string::iterator next);
		bool matched() const;
		void reset();

	private:
		const char* mText;
		unsigned int mIndex = 0;
		std::string::iterator mAfterMatch;
		std::string::iterator mEnd;
	};

	class Trim
	{
	public:
		// trim from start (in place)
		static void ltrim(std::string& s);

		// trim from end (in place)
		static void rtrim(std::string& s);

		// trim from both ends (in place)
		static void trim(std::string& s);

		// trim from start (copying)
		static std::string ltrim_copy(std::string s);

		// trim from end (copying)
		static std::string rtrim_copy(std::string s);

		// trim from both ends (in place)
		static std::string trim_copy(std::string s);
	};

	inline unsigned int customSimpleHash(const std::string &str)
	{
		// Ensure that we have a value (0) that is guaranteed to be not a hash value
		unsigned int hash = 1;

		for (auto& it : str) {
			// NOTE: be sure to use prime numbers
			hash = 37 * hash + 17 * static_cast<char>(it);
		}

		return hash;
	}
	
	inline unsigned int customSimpleHash(const char* cStr)
	{
		std::string str(cStr);
		return customSimpleHash(str);
	}
}
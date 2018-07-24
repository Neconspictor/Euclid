#pragma once

#include <string>
#include <vector>
#include <experimental/filesystem>
#include <boost/current_function.hpp>
#include <sstream>
#include <platform/exception/EnumFormatException.hpp>

namespace platform {
	namespace util {
		/**
		 * Converts back slashes in a string to forward slashes.
		 */
		std::string backSlasheshToForwards(const std::string& source);
		
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
		bool isParentFolderOf(const std::experimental::filesystem::path& parent,
			const std::experimental::filesystem::path& child);

		/**
		 * Creates from an absolute path 'source' a relative path using a second path 'from'
		 * NOTE: It is assumed that both arguments, 'source' and 'from' are valid paths.
		 * Additionally it is assumed that, 'from' is a parent folder of 'source'.
		 * If one of these conditions isn't fulfilled, an empty string will be returned.
		 */
		std::string relativePathToBuildDirectory(const std::string& source);

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
			throw EnumFormatException(ss.str());
		}

		template<class E, size_t S>
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
			throw EnumFormatException(ss.str());
		}
	}
}
#ifndef PLATFORM_UTIL_STRING_UTILS_HPP
#define PLATFORM_UTIL_STRING_UTILS_HPP

#include <string>
#include <vector>
#include <filesystem>

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
	}
}
#endif
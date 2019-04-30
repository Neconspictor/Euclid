#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace nex
{

	class FileSystem
	{
	public:

		FileSystem() = default;
		FileSystem(std::vector<std::filesystem::path> includeDirectories);

		void addIncludeDirectory(const std::filesystem::path& folder);


		std::filesystem::path resolveAbsolute(const std::filesystem::path& path) const;

		/**
		 *
		 */
		std::filesystem::path resolvePath(const std::filesystem::path& path) const;

		std::filesystem::path resolveRelative(const std::filesystem::path& path,
			const std::filesystem::path& base = std::filesystem::current_path()) const;

		static std::filesystem::path makeRelative(const std::filesystem::path& path,
			const std::filesystem::path& root = std::filesystem::current_path());
		static std::filesystem::path getCurrentPath_Relative();

		/**
		 * Loads a file from a given file path into a string.
		 * If the file couldn't successfully read or if another error occurs, the destination string
		 * will be untouched.
		 * @return true if the file loading was successful.
		 */
		static bool loadFileIntoString(const std::string& filePath, std::string* destination);


		/**
		 * @param path The file to open
		 * @param lines the lines to write into to the file.
		 * @param openMode The mode used to open the file
		 *
		 * @throws std::ofstream::failure If an IO error occurs
		 */
		static void writeToFile(const std::string& path, const std::vector<char>& source,
			std::ostream::_Openmode openMode = std::ostream::trunc);


		/**
		* Provides the content of a specified file.
		* The memory will be allocated with new[] and therefore has
		* to be deleted by the caller (using delete[]).
		* NOTE: If the file couldn't be read, a nullptr will be returned. In this case, no memory
		* has to be freed.
		*/
		static char* getBytesFromFile(const std::string& filePath, std::streampos* fileSize);

		static std::streampos getFileSize(const std::string& filePath);

		static std::vector<std::string> getFilesFromFolder(const std::string& folderPath, bool skipSubFolders = true);

	private:
		std::vector<std::filesystem::path> mIncludeDirectories;
	};
}
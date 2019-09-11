#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <nex/common/File.hpp>

namespace nex
{

	class FileSystem
	{
	public:

		struct CompiledPathResult {
			bool fromIncludeDirectory;
			std::filesystem::path path;
		};

		/**
		 * Creates a new file system with a vector of include directories.
		 * @param includeDirectories : the include directories. Has to contain minimal one entry!
		 * @param compiledRootDirectory : The root directory for storing compiled resources
		 * @param compiledFileExtension : The file extension for compiled resources
		 * @throws std::invalid_argument : if size of includeDirectories == 0
		 */
		FileSystem(std::vector<std::filesystem::path> includeDirectories, std::filesystem::path compiledRootDirectory, std::string compiledFileExtension);

		void addIncludeDirectory(const std::filesystem::path& folder);

		static void createDirectories(const std::string& relative,  const std::filesystem::path& root);

		/**
		* Provides the content of a specified file.
		* The memory will be allocated with new[] and therefore has
		* to be deleted by the caller (using delete[]).
		* NOTE: If the file couldn't be read, a nullptr will be returned. In this case, no memory
		* has to be freed.
		*/
		static char* getBytesFromFile(const std::string& filePath, std::streampos* fileSize);

		static std::filesystem::path getCurrentPath_Relative();

		CompiledPathResult getCompiledPath(const std::filesystem::path& path) const;

		static std::streampos getFileSize(const std::string& filePath);

		static std::vector<std::string> getFilesFromFolder(const std::string& folderPath, bool skipSubFolders = true);

		/**
		 * A file system has got minimal one include directory, which is assumed to be the most important one.
		 * This function provides this first include directory.
		 */
		const std::filesystem::path& getFirstIncludeDirectory() const;

		/**
		 * Provides the list of include directories.
		 * Note: the returned vector has minimal one entry.
		 */
		const std::vector<std::filesystem::path>& getIncludeDirectories() const;

		/**
		 * Checks if a given path is contained in a given 'root' directory or in one of the root's sub directories.
		 */
		static bool isContained(const std::filesystem::path & path, const std::filesystem::path& root);

		/**
		 * Loads a file from a given file path into a string.
		 * If the file couldn't successfully read or if another error occurs, the destination string
		 * will be untouched.
		 * @return true if the file loading was successful.
		 */
		static bool loadFileIntoString(const std::string& filePath, std::string* destination);

		template<typename T>
		static void load(const std::filesystem::path& root, const std::filesystem::path& relative, T& out)
		{
			BinStream file;
			file.open(root / relative, std::ios::in);
			file >> out;
		}

		template<typename T>
		static void load(const std::filesystem::path& filePath, T& out)
		{
			BinStream file;
			file.open(filePath, std::ios::in);
			file >> out;
		}

		template<typename T>
		void loadFromCompiled(const std::filesystem::path& resourcePath, 
			const std::function<void(T&)>& resourceLoader,T& resource, bool forceLoad = false)
		{
			auto compiledPath = getCompiledPath(resourcePath).path;

			if (!std::filesystem::exists(compiledPath) || forceLoad)
			{
				//const auto resolvedPath = resolvePath(resourcePath);
				resourceLoader(resource);
				FileSystem::store(compiledPath, resource);
			}
			else
			{
				BinStream file;
				file.open(compiledPath, std::ios::in);
				file >> resource;
			}
		}

		static std::filesystem::path makeRelative(const std::filesystem::path& path,
			const std::filesystem::path& root = std::filesystem::current_path());


		std::filesystem::path resolveAbsolute(const std::filesystem::path& path, const std::filesystem::path& root) const;

		/**
		 *
		 */
		std::filesystem::path resolvePath(const std::filesystem::path& path, 
			const std::filesystem::path& relativeRoot = "./", 
			bool noException = false) const;

		std::filesystem::path resolveRelative(const std::filesystem::path& path,
			const std::filesystem::path& base = std::filesystem::current_path()) const;

		template<typename T>
		static void store(const std::filesystem::path& path, const T& input, bool trunc = true)
		{
			BinStream file;
			auto directory = path.parent_path();
			std::filesystem::create_directories(directory);

			std::ios_base::openmode mode = std::ios::out;
			if (trunc) mode |= std::ios::trunc;

			file.open(path, mode);
			file << input;
		}


		/**
		 * @param path The file to open
		 * @param lines the lines to write into to the file.
		 * @param openMode The mode used to open the file
		 *
		 * @throws std::ofstream::failure If an IO error occurs
		 */
		static void writeToFile(const std::string& path, const std::vector<char>& source,
			std::ostream::_Openmode openMode = std::ostream::trunc);

	private:
		std::vector<std::filesystem::path> mIncludeDirectories;
		std::filesystem::path mCompiledRootDirectory;
		std::string mCompiledFileExtension;
	};
}
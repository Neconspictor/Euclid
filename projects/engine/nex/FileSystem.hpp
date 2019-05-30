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

		static std::streampos getFileSize(const std::string& filePath);

		static std::vector<std::string> getFilesFromFolder(const std::string& folderPath, bool skipSubFolders = true);

		const std::vector<std::filesystem::path>& getIncludeDirectories() const;

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
		void loadFromCompiled(const std::filesystem::path& resourcePath, 
			const std::filesystem::path& compiledPath, 
			const std::filesystem::path& compiledRootPath,
			const std::function<void(T&)>& resourceLoader,T& resource)
		{
			auto resolvedCompiledPath = resolvePath(compiledPath, true);

			if (!std::filesystem::exists(resolvedCompiledPath))
			{
				const auto resolvedPath = resolvePath(resourcePath);
				resourceLoader(resource);
				FileSystem::store(compiledRootPath, compiledPath, resource);
			}
			else
			{
				BinStream file;
				file.open(resolvedCompiledPath, std::ios::in);
				file >> resource;
			}
		}

		static std::filesystem::path makeRelative(const std::filesystem::path& path,
			const std::filesystem::path& root = std::filesystem::current_path());


		std::filesystem::path resolveAbsolute(const std::filesystem::path& path) const;

		/**,
		 *
		 */
		std::filesystem::path resolvePath(const std::filesystem::path& path, bool noException = false) const;

		std::filesystem::path resolveRelative(const std::filesystem::path& path,
			const std::filesystem::path& base = std::filesystem::current_path()) const;

		template<typename T>
		static void store(const std::filesystem::path& root, const std::filesystem::path& relative, const T& input, bool trunc = true)
		{
			BinStream file;
			auto directory = relative.parent_path();
			createDirectories(directory.generic_string(), root);

			std::ios_base::openmode mode = std::ios::out;
			if (trunc) mode |= std::ios::trunc;

			file.open(root / relative, mode);
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
	};
}
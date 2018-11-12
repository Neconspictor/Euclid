#pragma once
#include <string>
#include <vector>
#include <filesystem>


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

private:
	std::vector<std::filesystem::path> mIncludeDirectories;
};
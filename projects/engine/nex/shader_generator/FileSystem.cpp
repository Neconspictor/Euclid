#include "FileSystem.hpp"
#include <fstream>
#include <iostream>

//FileSystem::FileSystem()
//{
//}

FileSystem::FileSystem(std::vector<std::filesystem::path> includeDirectories) :
	mIncludeDirectories(std::move(includeDirectories))
{
}

void FileSystem::addIncludeDirectory(const std::filesystem::path& path)
{
	using namespace std::filesystem;

	std::filesystem::path folder = path;
	if (!exists(folder)) throw std::runtime_error("FileSystem::addIncludeDirectory: Folder doesn't exist: " + folder.generic_string());

	mIncludeDirectories.emplace_back(std::move(folder));
}

std::filesystem::path FileSystem::resolveAbsolute(const std::filesystem::path& path) const
{
	return canonical(resolvePath(path));
}

std::filesystem::path FileSystem::resolvePath(const std::filesystem::path& path) const
{

	static std::string errorBase = "FileSystem::resolvePath: path doesn't exist: ";

	if (path.is_absolute()) {

		if (!exists(path))
			throw std::runtime_error(errorBase + path.generic_string());

		return path;
	}

	for (const auto& item : mIncludeDirectories)
	{
		std::filesystem::path p = item / path;
		if (exists(p)) return p;
	}

	throw std::runtime_error(errorBase + path.generic_string());
}

std::filesystem::path FileSystem::resolveRelative(const std::filesystem::path& path,
	const std::filesystem::path& base) const
{
	auto result = resolvePath(path);
	return relative(result, base);
}

std::filesystem::path FileSystem::makeRelative(const std::filesystem::path& path, const std::filesystem::path& root)
{
	return relative(path, root);
}

std::filesystem::path FileSystem::getCurrentPath_Relative()
{
	return makeRelative(std::filesystem::current_path());
}
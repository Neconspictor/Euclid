#include <nex/FileSystem.hpp>
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "nex/common/Log.hpp"
#include "util/Memory.hpp"
#include "util/ExceptionHandling.hpp"

using namespace nex;

FileSystem::FileSystem(std::vector<std::filesystem::path> includeDirectories) :
	mIncludeDirectories(std::move(includeDirectories))
{
}

void FileSystem::addIncludeDirectory(const std::filesystem::path& path)
{
	using namespace std::filesystem;

	std::filesystem::path folder = path;
	if (!exists(folder)) throw_with_trace(std::runtime_error("FileSystem::addIncludeDirectory: Folder doesn't exist: " + folder.generic_string()));

	mIncludeDirectories.emplace_back(std::move(folder));
}

void FileSystem::createDirectories(const std::string& relative, const std::filesystem::path& root)
{
	std::filesystem::create_directories(root.generic_string() + relative);
}

std::filesystem::path FileSystem::resolveAbsolute(const std::filesystem::path& path) const
{
	return canonical(resolvePath(path));
}

std::filesystem::path FileSystem::resolvePath(const std::filesystem::path& path, bool noException) const
{

	static std::string errorBase = "FileSystem::resolvePath: path doesn't exist: ";

	if (path.is_absolute()) {

		if (!exists(path))
		{
			if (noException) return {};
			throw_with_trace(std::runtime_error(errorBase + path.generic_string()));
		}

		return path;
	}

	for (const auto& item : mIncludeDirectories)
	{
		std::filesystem::path p = item / path;
		if (exists(p)) return p;
	}

	if (!noException)
		throw_with_trace(std::runtime_error(errorBase + path.generic_string()));

	return {};
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

const std::vector<std::filesystem::path>& FileSystem::getIncludeDirectories() const
{
	return mIncludeDirectories;
}


bool FileSystem::loadFileIntoString(const std::string& filePath, std::string* destination)
{
	std::ifstream shaderStreamFile;
	bool loadingWasSuccessful = true;

	// ensure ifstream can throw exceptions!
	shaderStreamFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		shaderStreamFile.open(filePath);
		std::stringstream shaderStream;

		// read file content to stream
		shaderStream << shaderStreamFile.rdbuf();
		*destination = shaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		nex::Logger logger("FileSystem");

		if (shaderStreamFile.fail())
		{
			LOG(logger, nex::Error) << "Couldn't open file: "
				<< filePath;
		}

		if (shaderStreamFile.bad())
		{
			LOG(logger, nex::Error) << "Couldn't read file properly.";
		}
		loadingWasSuccessful = false;
		*destination = "";
	}

	//clear exceptions as close shouldn't throw any exceptions!
	shaderStreamFile.exceptions((std::ios_base::iostate)0);
	shaderStreamFile.close();

	return loadingWasSuccessful;
}

void FileSystem::writeToFile(const std::string& path, const std::vector<char>& source, std::ostream::_Openmode openMode)
{
	std::ofstream out;
	out.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	out.open(path, openMode | std::ostream::binary);

	out.write(source.data(), source.size() - 1);
}

char* FileSystem::getBytesFromFile(const std::string& filePath, std::streampos* fileSize)
{
	std::ifstream file;
	nex::MemGuard content(nullptr);
	*fileSize = 0;
	nex::Logger logger("FileSystem");

	// ensure ifstream can throw exceptions!
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		file.open(filePath, std::ios::binary);
		std::filebuf* buffer = file.rdbuf();

		//get file size
		*fileSize = file.tellg();
		file.seekg(0, std::ios::end);
		*fileSize = file.tellg() - *fileSize;
		buffer->pubseekpos(0, file.in);

		// allocate memory to contain file data
		char* memory = new char[*fileSize];
		content.setContent(memory);

		//copy data to content buffer
		buffer->sgetn(content.get(), *fileSize);
	}
	catch (std::ifstream::failure e)
	{
		if (file.fail())
		{
			LOG(logger, nex::Error) << "Couldn't open file: " << filePath;
		}

		if (file.bad())
		{
			LOG(logger, nex::Error) << "Couldn't read file properly.";
		}
	}
	catch (std::bad_alloc e)
	{
		LOG(logger, nex::Error) << "Couldn't allocate memory of size: " << std::to_string(*fileSize);
	}

	//clear exceptions as close shouldn't throw any exceptions!
	file.exceptions((std::ios_base::iostate)0);
	file.close();

	char* result = content.get();
	content.setContent(nullptr);

	return result;
}

std::streampos  FileSystem::getFileSize(const std::string& filePath)
{
	std::ifstream file;
	std::streampos size = 0;

	// ensure ifstream can throw exceptions!
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		file.open(filePath, std::ios::binary);
		std::filebuf* buffer = file.rdbuf();

		//get file size
		size = buffer->pubseekoff(0, file.end, file.in);
		buffer->pubseekpos(0, file.in);
	}
	catch (std::ifstream::failure e)
	{
		nex::Logger logger("FileSystem");

		if (file.fail())
		{
			LOG(logger, nex::Error) << "Couldn't open file: "
				<< filePath;
		}

		if (file.bad())
		{
			LOG(logger, nex::Error) << "Couldn't read file properly.";
		}
	}

	//clear exceptions as close shouldn't throw any exceptions!
	file.exceptions((std::ios_base::iostate)0);
	file.close();

	return size;
}

std::vector<std::string> FileSystem::getFilesFromFolder(const std::string& folderPath, bool skipSubFolders)
{
	using namespace boost::filesystem;
	path folder(folderPath);

	if (!is_directory(folder) || !exists(folder))
	{
		return std::vector<std::string>();
	}

	std::vector<std::string> result;

	std::vector<std::string> subPaths;

	for (directory_iterator it(folder); it != directory_iterator(); ++it)
	{
		path path = it->path();
		if (!skipSubFolders && is_directory(path))
		{
			subPaths.push_back(path.generic_string());
		}
		else
		{
			result.push_back(path.generic_string());
		}
	}

	for (auto& path : subPaths)
	{
		std::vector<std::string> subResult = getFilesFromFolder(path, false);
		result.insert(result.begin(), subResult.begin(), subResult.end());
	}

	return result;
}
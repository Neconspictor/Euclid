#include <nex/resource/FileSystem.hpp>
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "nex/common/Log.hpp"
#include <nex/util/ExceptionHandling.hpp>
#include <regex>
#include <nex/util/StringUtils.hpp>
using namespace nex;

FileSystem::FileSystem(std::vector<std::filesystem::path> includeDirectories, std::filesystem::path compiledRootDirectory, std::string compiledFileExtension) :
	mIncludeDirectories(std::move(includeDirectories)),
	mCompiledRootDirectory(std::move(compiledRootDirectory)),
	mCompiledFileExtension(std::move(compiledFileExtension))
{
	if (mIncludeDirectories.size() == 0) throw std::invalid_argument("size of include directories must be greater 0!");
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

std::filesystem::path FileSystem::resolveAbsolute(const std::filesystem::path& path, const std::filesystem::path& root) const
{
	return absolute(resolvePath(path, root));
}

std::filesystem::path FileSystem::resolvePath(const std::filesystem::path& path, const std::filesystem::path& root, bool noException) const
{
	static std::string errorBase = "FileSystem::resolvePath: path doesn't exist: ";

	bool isAbsolute = path.is_absolute();

	if (isAbsolute) {
		auto compiledResource = getCompiledPath(path).path;

		if (!exists(path) && !exists(compiledResource))
		{
			if (noException) return {};
			throw_with_trace(std::runtime_error(errorBase + path.generic_string()));
		}

		return path;
	}


	// try to match the path with the specified base directoy.
	auto current = root / path;	 
	auto compiledResult = getCompiledPath(current);

	if (exists(current) || (exists(compiledResult.path) && !compiledResult.fromIncludeDirectory)) return current;

	// try to match the path wih a registered include directory
	for (const auto& item : mIncludeDirectories)
	{
		std::filesystem::path p = item / path;
		auto compiledResource = getCompiledPath(p).path;
		if (exists(p) || exists(compiledResource)) return p;
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

nex::FileSystem::CompiledPathResult nex::FileSystem::getCompiledPath(const std::filesystem::path & path) const
{
	CompiledPathResult result;

	auto relativePath = path;

	if (relativePath.is_absolute()) {
		// make the absolute path relative if it is contained in one include directory
		for (const auto& item : mIncludeDirectories)
		{
			if (isContained(relativePath, item)) {
				relativePath = makeRelative(relativePath, item);
				break;
			}
		}
	}

	if (relativePath.is_absolute()) {
		// A regex that matches the following tokens: /\?*<>|
		std::regex re(":|/|\\\\|\\?|\\*|<|>|\\|");
		auto root = relativePath.root_name().generic_string();
		root = std::regex_replace(root, re, "");

		result.path = mCompiledRootDirectory / root / relativePath.relative_path();
		result.fromIncludeDirectory = false;
	}
		
	else {
		result.path = mCompiledRootDirectory / relativePath;
		result.fromIncludeDirectory = true;
	}

	result.path.replace_extension(mCompiledFileExtension);
	return result;
}

const std::vector<std::filesystem::path>& FileSystem::getIncludeDirectories() const
{
	return mIncludeDirectories;
}

bool FileSystem::isContained(const std::filesystem::path& path, const std::filesystem::path& root)
{
	auto absolutePath = absolute(path);
	// ensure that the root directory ends with a path separator
	// This is needed for getting an unambigious result using std::mismatch
	auto absoluteRoot = absolute(root / ""); 

	//path::preferred_separator

	const auto pair = std::mismatch(absolutePath.begin(), absolutePath.end(), absoluteRoot.begin(), absoluteRoot.end());
	auto last = --absoluteRoot.end();
	return pair.second == last;
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
	std::unique_ptr<char[]> content;
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
		content = std::make_unique<char[]>(*fileSize);

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

	return content.release();
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

std::vector<std::string> nex::FileSystem::filter(const std::vector<std::string>& files, const std::string& extension)
{
	std::vector<std::string> filtered;

	for (const auto& file : files) {
		if (nex::util::endsWith(file, extension))
			filtered.push_back(file);
	}

	return filtered;
}

const std::filesystem::path& FileSystem::getFirstIncludeDirectory() const
{
	return mIncludeDirectories[0];
}
#include <nex/FileSystem.hpp>
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "nex/common/Log.hpp"


namespace nex::filesystem
{

	class MemoryWrapper
	{
	public:
		explicit MemoryWrapper(char* value)
			: _value(value)
		{ }

		char* operator *()
		{
			return _value;
		}

		void setContent(char* content)
		{
			_value = content;
		}

		virtual ~MemoryWrapper()
		{
			if (_value)
			{
				delete[] _value;
				_value = nullptr;
			}
		}
	private:
		char* _value;
	};


	bool loadFileIntoString(const std::string& filePath, std::string* destination)
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
				LOG(logger, Error) << "Couldn't open file: "
					<< filePath;
			}

			if (shaderStreamFile.bad())
			{
				LOG(logger, Error) << "Couldn't read file properly.";
			}
			loadingWasSuccessful = false;
			*destination = "";
		}

		//clear exceptions as close shouldn't throw any exceptions!
		shaderStreamFile.exceptions((std::ios_base::iostate)0);
		shaderStreamFile.close();

		return loadingWasSuccessful;
	}

	void writeToFile(const std::string& path, const std::vector<std::string>& lines, std::ostream::_Openmode openMode)
	{
		std::ofstream out;
		out.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		out.open(path, openMode);

		for (auto& line : lines)
		{
			out << line << std::endl;
		}
	}

	char* getBytesFromFile(const std::string& filePath, std::streampos* fileSize)
	{
		std::ifstream file;
		MemoryWrapper content(nullptr);
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
			buffer->sgetn(*content, *fileSize);
		}
		catch (std::ifstream::failure e)
		{
			if (file.fail())
			{
				LOG(logger, Error) << "Couldn't open file: " << filePath;
			}

			if (file.bad())
			{
				LOG(logger, Error) << "Couldn't read file properly.";
			}
		}
		catch (std::bad_alloc e)
		{
			LOG(logger, Error) << "Couldn't allocate memory of size: " << std::to_string(*fileSize);
		}

		//clear exceptions as close shouldn't throw any exceptions!
		file.exceptions((std::ios_base::iostate)0);
		file.close();

		char* result = *content;
		content.setContent(nullptr);

		return result;
	}

	std::streampos  getFileSize(const std::string& filePath)
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
				LOG(logger, Error) << "Couldn't open file: "
					<< filePath;
			}

			if (file.bad())
			{
				LOG(logger, Error) << "Couldn't read file properly.";
			}
		}

		//clear exceptions as close shouldn't throw any exceptions!
		file.exceptions((std::ios_base::iostate)0);
		file.close();

		return size;
	}

	std::vector<std::string> getFilesFromFolder(const std::string& folderPath, bool skipSubFolders)
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
}

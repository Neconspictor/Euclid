#pragma once
#include <string>
#include <vector>

namespace nex::filesystem
{
	
	/**
	 * Loads a file from a given file path into a string.
	 * If the file couldn't successfully read or if another error occurs, the destination string 
	 * will be untouched.
	 * @return true if the file loading was successful.
	 */
	 bool loadFileIntoString(const std::string& filePath, std::string* destination);

	 /**
	 * Provides the content of a specified file. 
	 * The memory will be allocated with new[] and therefore has
	 * to be deleted by the caller (using delete[]).
	 * NOTE: If the file couldn't be read, a nullptr will be returned. In this case, no memory
	 * has to be freed.
	 */
	 char* getBytesFromFile(const std::string& filePath, std::streampos* fileSize);

	 std::streampos getFileSize(const std::string& filePath);

	 std::vector<std::string> getFilesFromFolder(const std::string& folderPath, bool skipSubFolders = true);
};
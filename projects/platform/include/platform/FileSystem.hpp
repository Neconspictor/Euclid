#pragma once
#include <string>

namespace filesystem
{
	
	/**
	 * Loads a file from a given file path into a string.
	 * If the file couldn't successfully read or if another error occurs, the destination string 
	 * will be untouched.
	 * @return true if the file loading was successful.
	 */
	 bool loadFileIntoString(const std::string& filePath, std::string* destination);

};
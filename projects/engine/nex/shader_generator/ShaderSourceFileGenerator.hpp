#pragma once
#include <vector>
#include <string>
#include <list>
#include <filesystem>
#include <nex/FileSystem.hpp>
#include <nex/common/Log.hpp>


struct IncludeDesc;


struct Replacement {
	// where the include statement is located in the parent source code
	unsigned int begin = 0;
	unsigned int end = 0;

	// Specifies the difference between the byte size from the old code ([begin, end)) and the 
	// included source code
	int diff = 0;
};

std::ostream& operator<<(std::ostream& os, const Replacement& r);

struct FileDesc {

	FileDesc() = default;

	virtual ~FileDesc() = default;

	FileDesc (FileDesc&& o) noexcept;

	FileDesc& operator=(FileDesc&& o) noexcept;

	std::vector<char> source;
	size_t sourceSize = 0;

	// The source code with resolved include statements
	std::vector<char> resolvedSource;
	size_t resolvedSourceSize = 0;

	std::list<IncludeDesc> includes;
	std::string filePath = "";
};

std::ostream& operator<<(std::ostream& os, const FileDesc& include);

struct IncludeDesc : FileDesc {

	IncludeDesc();
	IncludeDesc(IncludeDesc&& o) noexcept;
	IncludeDesc& operator=(IncludeDesc&& o) noexcept;


	virtual ~IncludeDesc() = default;

	// where the include statement is located in the parent source code
	unsigned int begin = 0;
	unsigned int end = 0;

	// Specifies information of position and size of the include source code, 
	// that replaces the include statement in the parent souce code.
	Replacement replacement;
	
	FileDesc* parent = nullptr;
};

std::ostream& operator<<(std::ostream& os, const IncludeDesc& include);

struct ReverseInfo
{
	FileDesc* fileDesc;
	unsigned int position;
};

std::ostream& operator<<(std::ostream& os, const ReverseInfo& r);


struct ProgramDesc
{
	FileDesc root;
	int parseErrorLogOffset;
};

struct ProgramSources
{
	ProgramDesc vertexShaderDesc;
	ProgramDesc fragmentShaderDesc;
	std::filesystem::path sourceFile;
};

class ShaderSourceFileGenerator
{
public:

	ShaderSourceFileGenerator(const FileSystem* fileSystem);

	ProgramSources extractShaderPrograms(const std::filesystem::path& filePath);

	FileDesc generate(const std::filesystem::path& filePath);

	void generate(ProgramDesc* programDesc);

	/**
	 * @param lineNumber: The line number (starting at 0)
	 * @param column: Starts at 0.
	 */
	static unsigned int calcResolvedPosition(const ProgramDesc& desc, int lineNumber, int column);
	static void calcLineColumn(const std::vector<char>& source, unsigned int position, int* lineNumber, int* column);

	static ReverseInfo reversePosition(const FileDesc* fileDesc, unsigned int resolvedPosition);




private:

	void buildShader(FileDesc* fileDesc);

	void generate(FileDesc* root);

	/**
	 * @throws ParseException: If the file couldn't be parsed
	 */
	void parseShaderFile(FileDesc* fileDesc, const std::filesystem::path& filePath);

	void parseShaderSource(FileDesc* fileDesc, const std::filesystem::path& filePath, std::vector<char> source);

	const FileSystem* mFileSystem;

	nex::Logger mLogger;
};
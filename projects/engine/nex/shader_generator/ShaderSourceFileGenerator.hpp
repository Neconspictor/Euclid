#pragma once
#include <vector>
#include <string>
#include <list>
#include <filesystem>
#include <nex/FileSystem.hpp>
#include <nex/common/Log.hpp>


namespace nex
{
	struct IncludeDesc;


	struct Replacement {
		// where the include statement is located in the parent source code
		size_t begin = 0;
		size_t end = 0;

		// Specifies the difference between the resolved source code (resolved include statements) and the original source code
		// The resolved source code can be smaller than the original source code. Thus it is important to use a signed type.
		// Note that in theory the difference between two unsigned numbers (we use size_t for the size of the resolved and original code)
		// can result into an overflow when assigning it to a signed number (diff has to to be signed). As  the difference between the origginal an the
		// resolved source code won't reach really high numbers, this shouldn't be an issue, practically.
		long long diff = 0;
	};

	struct FileDesc {

		FileDesc() = default;

		virtual ~FileDesc() = default;

		FileDesc(FileDesc&& o) noexcept;

		FileDesc& operator=(FileDesc&& o) noexcept;

		std::vector<char> source;
		size_t sourceSize = 0;

		// The source code with resolved include statements
		std::vector<char> resolvedSource;
		size_t resolvedSourceSize = 0;

		std::list<IncludeDesc> includes;
		std::string filePath = "";
	};

	struct IncludeDesc : FileDesc {

		IncludeDesc();
		IncludeDesc(IncludeDesc&& o) noexcept;
		IncludeDesc& operator=(IncludeDesc&& o) noexcept;


		virtual ~IncludeDesc() = default;

		// where the include statement is located in the parent source code
		size_t begin = 0;
		size_t end = 0;

		// Specifies information of position and size of the include source code, 
		// that replaces the include statement in the parent source code.
		Replacement replacement;

		FileDesc* parent = nullptr;
	};


	struct ReverseInfo
	{
		FileDesc* fileDesc;
		size_t position;
	};


	struct ShaderStageDesc
	{
		FileDesc root;
		int parseErrorLogOffset = 0;
	};

	struct ProgramSources
	{
		ShaderStageDesc vertexShaderDesc;
		ShaderStageDesc fragmentShaderDesc;
		std::filesystem::path sourceFile;
	};

	class ShaderSourceFileGenerator
	{
	public:

		ShaderSourceFileGenerator();

		ProgramSources extractShaderPrograms(const std::filesystem::path& filePath) const;

		FileDesc generate(const std::filesystem::path& filePath);

		void generate(ShaderStageDesc* programDesc);

		void init(const FileSystem* fileSystem);


		/**
		 * @param desc
		 * @param lineNumber: The line number (starting at 0)
		 * @param column: Starts at 0.
		 */
		static unsigned int calcResolvedPosition(const ShaderStageDesc& desc, size_t lineNumber, size_t column);
		static void calcLineColumn(const std::vector<char>& source, size_t position, size_t* lineNumber, size_t* column);

		static ReverseInfo reversePosition(const FileDesc* fileDesc, size_t resolvedPosition);


	private:

		static void buildShader(FileDesc* fileDesc);

		void generate(FileDesc* root);

		/**
		 * @throws ParseException: If the file could not be parsed
		 */
		void parseShaderFile(FileDesc* fileDesc, const std::filesystem::path& filePath) const;

		void parseShaderSource(FileDesc* fileDesc, const std::filesystem::path& filePath, std::vector<char> source) const;

		const FileSystem* mFileSystem;

		nex::Logger mLogger;
	};
};

std::ostream& operator<<(std::ostream& os, const nex::Replacement& r);
std::ostream& operator<<(std::ostream& os, const nex::FileDesc& include);
std::ostream& operator<<(std::ostream& os, const nex::IncludeDesc& include);
std::ostream& operator<<(std::ostream& os, const nex::ReverseInfo& r);
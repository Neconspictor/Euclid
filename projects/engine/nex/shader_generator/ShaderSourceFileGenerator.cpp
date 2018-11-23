#include <fstream>
#include <sstream>
#include <set>
#include <queue>
#include <cassert>
#include "ShaderSourceFileGenerator.hpp"
#include "CommentStateTracker.hpp"
#include "SourceFileConsumer.hpp"
#include "SourceReader.hpp"
#include <functional>

std::ostream& operator<<(std::ostream& os, const Replacement& r)
{
	os << "(begin=" << r.begin << ", end=" << r.end << ", diff=" << r.diff << ")";
	return os;
}

FileDesc::FileDesc(FileDesc&& o) noexcept : source(std::move(o.source)),
sourceSize(o.sourceSize), 
resolvedSource(std::move(o.resolvedSource)), 
resolvedSourceSize(o.resolvedSourceSize),
filePath(std::move(o.filePath)),
includes(std::move(o.includes))
{
}

FileDesc& FileDesc::operator=(FileDesc&& o) noexcept
{
	if (this == &o) return *this;
	this->source = std::move(o.source);
	this->sourceSize = o.sourceSize;
	this->resolvedSource = std::move(o.resolvedSource);
	this->resolvedSourceSize = o.resolvedSourceSize;
	this->filePath = std::move(o.filePath);
	this->includes = std::move(o.includes);

	return *this;
}

std::ostream& operator<<(std::ostream& os, const FileDesc& fileDesc)
{
	os << "(filePath='"<< fileDesc.filePath << "', sourceSize= " << fileDesc.sourceSize << ", resolvedSourceSize=" << fileDesc.resolvedSourceSize << ")";
	return os;
}

IncludeDesc::IncludeDesc()
{
}

IncludeDesc::IncludeDesc(IncludeDesc&& o) noexcept :
FileDesc(std::move(o)),
begin(o.begin),
end(o.end),
replacement(std::move(o.replacement)),
parent(o.parent)
{
}

IncludeDesc& IncludeDesc::operator=(IncludeDesc&& o) noexcept
{
	if (this == &o) return *this;
	FileDesc::operator=(std::move(o));
	this->begin = o.begin;
	this->end = o.end;
	this->replacement = std::move(o.replacement);
	this->parent = o.parent;

	return *this;
}

std::ostream& operator<<(std::ostream& os, const IncludeDesc& include)
{
	os << "(FileDesc" << (FileDesc&)include << ", begin=" << include.begin << ", end=" << include.end 
	<< ", parent= " << include.parent <<", replacement=" << include.replacement << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const ReverseInfo& r)
{
	os << "(" << r.fileDesc << ", " << r.position << ")";
	return os;
}

ShaderSourceFileGenerator::ShaderSourceFileGenerator(const FileSystem* fileSystem) : mFileSystem(fileSystem),  mLogger("ShaderGenerator")
{
	assert(mFileSystem != nullptr);
}

ProgramSources ShaderSourceFileGenerator::extractShaderPrograms(const std::filesystem::path& filePath)
{

	auto path = mFileSystem->resolvePath(filePath);

	try
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file.open(path);

		enum class ShaderType
		{
			None = -1, Vertex = 0, Fragment = 1
		};

		std::string line;
		std::stringstream ss[2];
		ProgramDesc parseResult[2];

		ShaderType type = ShaderType::None;

		int lineNumber = 0;

		while (!file.eof() && std::getline(file, line))
		{
			++lineNumber;

			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("vertex") != std::string::npos)
				{
					type = ShaderType::Vertex;
					parseResult[(int)type].parseErrorLogOffset = lineNumber;
				}
				else if (line.find("fragment") != std::string::npos)
				{
					type = ShaderType::Fragment;
					parseResult[(int)type].parseErrorLogOffset = lineNumber;
				}
			}
			else
			{
				ss[(int)type] << line << '\n';
			}
		}

		std::string contents[2];
		contents[0] = ss[0].str();
		contents[1] = ss[1].str();

		std::copy(contents[0].begin(), contents[0].end(), std::back_inserter(parseResult[0].root.source));
		std::copy(contents[1].begin(), contents[1].end(), std::back_inserter(parseResult[1].root.source));

		// Null-byte is needed for shader compilation!
		parseResult[0].root.source.push_back('\0');
		parseResult[0].root.sourceSize = parseResult[0].root.source.size();
		parseResult[0].root.filePath = path.generic_string();
	
		parseResult[1].root.source.push_back('\0');
		parseResult[1].root.sourceSize = parseResult[0].root.source.size();
		parseResult[1].root.filePath = path.generic_string();

		return {	std::move(parseResult[0]), 
					std::move(parseResult[1]), 
					std::move(path)};
	}
	catch (const std::ios_base::failure&)
	{
		throw std::runtime_error("Couldn't extract shader programs from " + path.generic_string());
	}
}

FileDesc ShaderSourceFileGenerator::generate(const std::filesystem::path& filePath)
{
	FileDesc root; 
	parseShaderFile(&root, mFileSystem->resolvePath(filePath));
	generate(&root);
	return root;
}

void ShaderSourceFileGenerator::generate(ProgramDesc* programDesc)
{
	parseShaderSource(&programDesc->root, programDesc->root.filePath, std::move(programDesc->root.source));
	generate(&programDesc->root);
}

unsigned int ShaderSourceFileGenerator::calcResolvedPosition(const ProgramDesc& desc, int lineNumber, int column)
{
	auto& source = desc.root.resolvedSource;

	unsigned int resolvedPosition;

	for (resolvedPosition = 0; resolvedPosition < source.size(); resolvedPosition++)
	{
		if (lineNumber == 0)
		{
			if (column == 0)break;
			column--;
		}

		if (source[resolvedPosition] == '\n') lineNumber--;
	}

	assert(lineNumber == 0);
	assert(column == 0);

	return resolvedPosition;
}

void ShaderSourceFileGenerator::calcLineColumn(const std::vector<char>& source, unsigned int position, int* lineNumber,
	int* column)
{
	int workLineNumber = 0;
	int workColumn = 0;

	for (auto i = 0; i < source.size(); ++i)
	{
		if (i == position) break;

		++workColumn;

		if (source[i] == '\n')
		{
			++workLineNumber;
			workColumn = 0;
		}
	}

	*lineNumber = workLineNumber;
	*column = workColumn;
}

ReverseInfo ShaderSourceFileGenerator::reversePosition(const FileDesc* fileDesc, unsigned int resolvedPosition)
{
	ReverseInfo result;
	result.position = resolvedPosition;
	unsigned int& cursor = result.position;

	using IteratorType = std::list<IncludeDesc>::const_iterator;
	std::queue<IteratorType> queue;

	const FileDesc* current = fileDesc;

	queue.push(current->includes.begin());

	while (!queue.empty())
	{
		auto it = queue.front();
		queue.pop();

		const Replacement& r = it->replacement;

		if (r.end <= resolvedPosition)
		{
			cursor -= r.diff;

			// Next include
			++it;
			if (it != current->includes.end())
				queue.push(it);
		}
		else if (r.begin <= resolvedPosition)
		{
			// the original position lies in an include file
			current = &*it;

			//position -= r.begin;
			// rebase resolved position 
			resolvedPosition -= r.begin;
			cursor = resolvedPosition;

			// Next include
			it = current->includes.begin();
			if (it != current->includes.end())
				queue.push(it);
		}

		// nothing is to do
	}

	result.fileDesc = const_cast<FileDesc*>(current);

	return result;
}

void ShaderSourceFileGenerator::buildShader(FileDesc* fileDesc)
{
	fileDesc->resolvedSource;
	fileDesc->resolvedSource = std::vector<char>(fileDesc->source.begin(), fileDesc->source.end());

	auto& out = fileDesc->resolvedSource;

	static auto cmp = [](const Replacement& a, const Replacement& b) { return a.begin < b.begin; };
	std::set<Replacement, decltype(cmp)> replacements(cmp);

	for (auto& include : fileDesc->includes)
	{
		// recursion
		buildShader(&include);
		const std::vector<char>& sourceCode = include.resolvedSource;

		// Note: We want the change the content of the repalcement in the current include desc!
		Replacement& replacement = include.replacement;

		unsigned int sizeOriginal = include.end - include.begin;
		unsigned int sizeInclude = sourceCode.size();

		replacement.diff = sizeInclude - sizeOriginal;

		// Find insert position
		replacement.begin = include.begin;
		for (auto it = replacements.begin(); it != replacements.end(); ++it)
		{
			if (it->begin >= replacement.begin) break;
			replacement.begin += it->diff;
		}

		replacement.end = replacement.begin + sizeInclude;

		// replace include statement with include source code
		unsigned int eraseEnd = replacement.begin + sizeOriginal;
		out.erase(out.begin() + replacement.begin, out.begin() + eraseEnd);
		out.insert(out.begin() + replacement.begin, sourceCode.begin(), sourceCode.end());


		// Finally add the range of the included source code
		replacements.insert(replacement);
	}

	fileDesc->resolvedSourceSize = fileDesc->resolvedSource.size();
}

void ShaderSourceFileGenerator::generate(FileDesc* root)
{
	using namespace std::filesystem;

	std::queue<IncludeDesc*> queue;
	const auto compare = [](const path& a, const path& b)
	{
		return absolute(a).generic_string() <
			absolute(b).generic_string();
	};
	std::set<path, decltype(compare)> parsedFiles(compare);

	const auto checkAndPush = [this,&parsedFiles, &queue](IncludeDesc& desc)
	{
		path p = mFileSystem->resolvePath(desc.filePath);
		if (parsedFiles.find(p) == parsedFiles.end())
		{
			parsedFiles.insert(p);
			queue.push(&desc);
		} else
		{
			std::stringstream ss;
			ss << "Detected include file is multiple times included: " << p.generic_string() << ":\n"
								<< "Include hierarchy for " << p.generic_string() << ":\n";
			FileDesc* parent = desc.parent;
			while(parent)
			{
				ss << "\t" << mFileSystem->resolvePath(parent->filePath).generic_string() << "\n";

				IncludeDesc* parentInclude = dynamic_cast<IncludeDesc*>(parent);
				if (parentInclude)
				{
					parent = parentInclude->parent;
				} else
				{
					parent = nullptr;
				}

			}

			mLogger.log(nex::Error) << ss.str();

			throw std::runtime_error("File is included multiple times: " + p.generic_string());
		}
	};


	parsedFiles.insert(mFileSystem->resolvePath(root->filePath));
	for (auto& elem : root->includes)
		checkAndPush(elem);

	while (!queue.empty())
	{
		IncludeDesc* include = queue.front();
		queue.pop();

		parseShaderFile(include, mFileSystem->resolvePath(include->filePath));

		for (auto& elem : include->includes)
			checkAndPush(elem);
	}

	buildShader(root);
}

void ShaderSourceFileGenerator::parseShaderFile(FileDesc* fileDesc, const std::filesystem::path& filePath)
{
	SourceFileParser reader(filePath);

	LineCounter lineCounter;
	OffsetIncludeCalculator offsetIncludeCalculator;
	IncludeCollector includeCollector;

	reader.addConsumer(&lineCounter);
	reader.addConsumer(&offsetIncludeCalculator);
	reader.addConsumer(&includeCollector);

	fileDesc->filePath = filePath.generic_string();

	try
	{
		reader.read(&fileDesc->source);

	} catch (const std::exception& e) {
		std::stringstream ss;
		ss << "Couldn't read " << filePath << std::endl;
		ss << "Reason:  " << e.what() << std::endl;
		throw ParseException(ss.str().c_str());
	}

	fileDesc->sourceSize = fileDesc->source.size();

	//std::cout << "Line count root: " << lineCounter.getLineCount() << std::endl;
	//std::cout << "Found include statements" << std::endl;
	for (const auto& it : includeCollector.getIncludes())
	{
		//std::string statement(fileDesc.source.begin() + it.lineBegin, fileDesc.source.begin() + it.lineEnd);
		//std::cout << it << std::endl;

		IncludeDesc include;
		//std::string test = "Hello World!";
		//include.source = std::vector<char>(test.begin(), test.end());
		//include.sourceSize = include.source.size();
		include.filePath = it.filePath;
		include.begin = it.lineBegin;
		include.end = it.lineEnd;
		//include.replacement.diff = include.source.size() - (include.replacement.end - include.replacement.begin);
		include.parent = fileDesc;

		fileDesc->includes.emplace_back(std::move(include));
	}
}

void ShaderSourceFileGenerator::parseShaderSource(FileDesc* fileDesc, const std::filesystem::path& filePath, std::vector<char> source)
{
	fileDesc->filePath = filePath.generic_string();
	fileDesc->source = std::move(source);
	fileDesc->sourceSize = fileDesc->source.size();
	SourceMemoryParser reader(&fileDesc->source);

	LineCounter lineCounter;
	OffsetIncludeCalculator offsetIncludeCalculator;
	IncludeCollector includeCollector;

	reader.addConsumer(&lineCounter);
	reader.addConsumer(&offsetIncludeCalculator);
	reader.addConsumer(&includeCollector);

	try
	{
		reader.read();
	} catch (const std::exception& e) {
		std::stringstream ss;
		ss << "Couldn't parse content from " << filePath << std::endl;
		ss << "Reason:  " << e.what() << std::endl;
		throw ParseException(ss.str().c_str());
	}

	for (const auto& it : includeCollector.getIncludes())
	{
		IncludeDesc include;
		include.filePath = it.filePath;
		include.begin = it.lineBegin;
		include.end = it.lineEnd;
		include.parent = fileDesc;

		fileDesc->includes.emplace_back(std::move(include));
	}
}
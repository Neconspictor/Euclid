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
#include "nex/util/ExceptionHandling.hpp"
#include <vector>

std::ostream& operator<<(std::ostream& os, const nex::Replacement& r)
{
	os << "(begin=" << r.begin << ", end=" << r.end << ", diff=" << r.diff << ")";
	return os;
}

nex::FileDesc::FileDesc(FileDesc&& o) noexcept : source(std::move(o.source)),
                                                 sourceSize(o.sourceSize),
                                                 resolvedSource(std::move(o.resolvedSource)),
                                                 resolvedSourceSize(o.resolvedSourceSize),
                                                 includes(std::move(o.includes)), defineInfo(std::move(o.defineInfo)),
                                                 mIsRoot(o.mIsRoot), 
												 firstLineAfterVersionStatement(o.firstLineAfterVersionStatement),
                                                 filePath(std::move(o.filePath))
{
}

nex::FileDesc& nex::FileDesc::operator=(nex::FileDesc&& o) noexcept
{
	if (this == &o) return *this;
	this->source = std::move(o.source);
	this->sourceSize = o.sourceSize;
	this->resolvedSource = std::move(o.resolvedSource);
	this->resolvedSourceSize = o.resolvedSourceSize;
	this->filePath = std::move(o.filePath);
	this->includes = std::move(o.includes);
	this->mIsRoot = o.mIsRoot;
	this->firstLineAfterVersionStatement = o.firstLineAfterVersionStatement;
	this->defineInfo = std::move(o.defineInfo);

	return *this;
}

std::ostream& operator<<(std::ostream& os, const nex::FileDesc& fileDesc)
{
	os << "(filePath='"<< fileDesc.filePath << "', sourceSize= " << fileDesc.sourceSize << ", resolvedSourceSize=" << fileDesc.resolvedSourceSize << ")";
	return os;
}

nex::IncludeDesc::IncludeDesc()
{
}

nex::IncludeDesc::IncludeDesc(IncludeDesc&& o) noexcept :
FileDesc(std::move(o)),
begin(o.begin),
end(o.end),
replacement(std::move(o.replacement)),
parent(o.parent)
{
}

nex::IncludeDesc& nex::IncludeDesc::operator=(nex::IncludeDesc&& o) noexcept
{
	if (this == &o) return *this;
	FileDesc::operator=(std::move(o));
	this->begin = o.begin;
	this->end = o.end;
	this->replacement = std::move(o.replacement);
	this->parent = o.parent;

	return *this;
}

std::ostream& operator<<(std::ostream& os, const nex::IncludeDesc& include)
{
	os << "(FileDesc" << static_cast<const nex::FileDesc&>(include) << ", begin=" << include.begin << ", end=" << include.end
	<< ", parent= " << include.parent <<", replacement=" << include.replacement << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const nex::ReverseInfo& r)
{
	os << "(" << r.fileDesc << ", " << r.position << ")";
	return os;
}

nex::ShaderSourceFileGenerator::ShaderSourceFileGenerator() : mFileSystem(nullptr),
                                                                                     mLogger("ShaderGenerator")
{
}

nex::ProgramSources nex::ShaderSourceFileGenerator::extractShaderPrograms(const std::filesystem::path& filePath) const
{

	auto path = mFileSystem->resolvePath(filePath);
	ProgramSources result;
	result.sourceFile = filePath;

	// currently vertex and fragment shader
	result.descs.resize(2);
	result.descs[0].type = ShaderStageType::VERTEX;
	result.descs[1].type = ShaderStageType::FRAGMENT;

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
					result.descs[static_cast<int>(type)].parseErrorLogOffset = lineNumber;
				}
				else if (line.find("fragment") != std::string::npos)
				{
					type = ShaderType::Fragment;
					result.descs[static_cast<int>(type)].parseErrorLogOffset = lineNumber;
				}
			}
			else
			{
				ss[static_cast<int>(type)] << line << '\n';
			}
		}

		std::string contents[2];
		contents[0] = ss[0].str();
		contents[1] = ss[1].str();

		std::copy(contents[0].begin(), contents[0].end(), std::back_inserter(result.descs[0].root.source));
		std::copy(contents[1].begin(), contents[1].end(), std::back_inserter(result.descs[1].root.source));

		// Null-byte is needed for shader compilation!
		result.descs[0].root.source.push_back('\0');
		result.descs[0].root.sourceSize = result.descs[0].root.source.size();
		result.descs[0].root.filePath = path.generic_string();
	
		result.descs[1].root.source.push_back('\0');
		result.descs[1].root.sourceSize = result.descs[0].root.source.size();
		result.descs[1].root.filePath = path.generic_string();

		return result;
	}
	catch (const std::ios_base::failure&)
	{
		throw_with_trace(std::runtime_error("Couldn't extract shader programs from " + path.generic_string()));
	}

	//won't be reached
	return ProgramSources();
}

nex::ProgramSources nex::ShaderSourceFileGenerator::generate(const std::vector<UnresolvedShaderStageDesc>& stages) const
{
	ProgramSources result;
	result.descs.resize(stages.size());

	for (auto i = 0; i < stages.size(); ++i)
	{
		auto& unresolved = stages[i];
		auto& resolved = result.descs[i];

		try
		{
			resolved.type = unresolved.type;
			resolved.root = generate(unresolved.filePath, unresolved.defines);
			//writeUnfoldedShaderContentToFile(vertexDesc.root.filePath, vertexDesc.root.resolvedSource);
		} catch (const ParseException& e)
		{
			std::stringstream ss;
			ss << "Couldn't generate unresolved shader stage of type " << unresolved.type << std::endl;
			ss << "File: " << unresolved.filePath << std::endl;
			ss << "Reason: " << e.what();

			throw_with_trace(ParseException(ss.str().c_str()));
		}
	}

	return result;
}

nex::FileDesc nex::ShaderSourceFileGenerator::generate(const std::filesystem::path& filePath, const std::vector<std::string>& defines) const
{
	FileDesc root;
	root.mIsRoot = true;
	parseShaderFile(&root, mFileSystem->resolvePath(filePath));

	// build source code for defines and update dependencies accordingly
	std::stringstream ss;
	for (auto& define : defines)
	{
		ss << define + "\n";
	}

	std::string defineSource = ss.str();
	
	auto oldSize = root.source.size();
	root.source.insert( root.source.begin() + root.firstLineAfterVersionStatement,
						defineSource.begin(), 
						defineSource.end());

	root.sourceSize = root.source.size();

	auto sizeDiff = root.sourceSize - oldSize;

	root.defineInfo.begin = root.firstLineAfterVersionStatement;
	root.defineInfo.size = sizeDiff;

	for (auto& include : root.includes)
	{
		include.begin += sizeDiff;
		include.end += sizeDiff;

		// diff is calculated later
		//include.replacement.diff += sizeDiff;
	}




	generate(&root);
	return root;
}

nex::ShaderSourceFileGenerator* nex::ShaderSourceFileGenerator::get()
{
	static ShaderSourceFileGenerator instance;
	return &instance;
}

void nex::ShaderSourceFileGenerator::init(const FileSystem* fileSystem)
{
	mFileSystem = fileSystem;
}

const nex::FileSystem* nex::ShaderSourceFileGenerator::getFileSystem() const
{
	return mFileSystem;
}

unsigned int nex::ShaderSourceFileGenerator::calcResolvedPosition(const nex::ResolvedShaderStageDesc& desc, size_t lineNumber, size_t column)
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

void nex::ShaderSourceFileGenerator::calcLineColumn(const std::vector<char>& source, size_t position, size_t* lineNumber,
	size_t* column)
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

nex::ReverseInfo nex::ShaderSourceFileGenerator::reversePosition(const FileDesc* fileDesc, size_t resolvedPosition)
{
	ReverseInfo result;
	result.position = resolvedPosition;
	size_t& cursor = result.position;
	result.errorInUserDefineMacro = false;


	using IteratorType = std::list<IncludeDesc>::const_iterator;
	std::queue<IteratorType> queue;

	const FileDesc* current = fileDesc;

	if (current->includes.size() > 0)
	{
		queue.push(current->includes.begin());
	}

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

	if (result.fileDesc->isRoot())
	{
		const auto& pos = result.position;
		const auto& info = result.fileDesc->defineInfo;
		// Adjust position when user set define macros
		if (pos >= info.begin && pos < info.begin + info.size)
			result.errorInUserDefineMacro = true;
		else if (pos >= info.begin + info.size)
			result.position -= result.fileDesc->defineInfo.size;
	}

	return result;
}

void nex::ShaderSourceFileGenerator::buildShader(nex::FileDesc* fileDesc)
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

		// Note: We want the change the content of the replacement in the current include desc!
		Replacement& replacement = include.replacement;

		const size_t sizeOriginal = include.end - include.begin;
		const size_t sizeInclude = sourceCode.size();

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
		const size_t eraseEnd = replacement.begin + sizeOriginal;
		out.erase(out.begin() + replacement.begin, out.begin() + eraseEnd);
		out.insert(out.begin() + replacement.begin, sourceCode.begin(), sourceCode.end());


		// Finally add the range of the included source code
		replacements.insert(replacement);
	}

	fileDesc->resolvedSourceSize = fileDesc->resolvedSource.size();
}

void nex::ShaderSourceFileGenerator::generate(nex::FileDesc* root) const
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

			throw_with_trace(std::runtime_error("File is included multiple times: " + p.generic_string()));
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

void nex::ShaderSourceFileGenerator::parseShaderFile(nex::FileDesc* fileDesc, const std::filesystem::path& filePath) const
{
	SourceFileParser reader(filePath);

	LineCounter lineCounter;
	OffsetIncludeCalculator offsetIncludeCalculator;
	IncludeCollector includeCollector;
	FirstLineAfterVersionStatementSearcher definesBeginPositionSearcher;
	

	reader.addConsumer(&lineCounter);
	reader.addConsumer(&offsetIncludeCalculator);
	reader.addConsumer(&includeCollector);
	reader.addConsumer(&definesBeginPositionSearcher);

	fileDesc->filePath = filePath.generic_string();

	try
	{
		reader.read(&fileDesc->source);

	} catch (const std::exception& e) {
		std::stringstream ss;
		ss << "Couldn't read " << filePath << std::endl;
		ss << "Reason:  " << e.what() << std::endl;
		throw_with_trace(ParseException(ss.str().c_str()));
	}

	fileDesc->sourceSize = fileDesc->source.size();
	fileDesc->firstLineAfterVersionStatement = definesBeginPositionSearcher.getResultPosition();

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

void nex::ShaderSourceFileGenerator::parseShaderSource(nex::FileDesc* fileDesc, const std::filesystem::path& filePath, std::vector<char> source) const
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
		throw_with_trace(ParseException(ss.str().c_str()));
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
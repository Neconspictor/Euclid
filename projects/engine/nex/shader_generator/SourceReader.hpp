#pragma once
#include "SourceFileConsumer.hpp"
#include <filesystem>

namespace nex
{
	class Parser
	{
	public:

		virtual ~Parser() = default;

		void addConsumer(SourceFileConsumer* consumer);

		/**
		 * @throws ParseException: If syntax errors are detected in the source file.
		 */
		void parse(StreamPos& streamPos);

	protected:

		std::vector<SourceFileConsumer*> mConsumers;
	};

	class SourceFileParser : public Parser
	{
	public:
		SourceFileParser(const std::filesystem::path& filePath);
		virtual ~SourceFileParser() = default;
		void read(std::vector<char>* result);

		std::streampos getFileSize(std::ifstream & file);

	private:
		std::filesystem::path mFilePath;
	};

	class SourceMemoryParser : public Parser
	{
	public:
		SourceMemoryParser(const std::vector<char>* source);
		virtual ~SourceMemoryParser() = default;
		void read();

	private:
		const std::vector<char>* mSource;
	};
}
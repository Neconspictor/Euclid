#pragma once
#include <vector>
#include <exception>

namespace nex
{
	struct StreamPos
	{
		const char* start;
		const char* pos;
		const char* end;
	};

	class ParseException : public std::exception
	{
	public:
		ParseException(const char* errorMsg) : std::exception(errorMsg) {}
	};

	class SourceFileConsumer
	{
	public:
		virtual ~SourceFileConsumer() = default;

		struct StreamDesc
		{
			const StreamPos* position;
			bool isComment;
		};

		/**
		 * @throws ParseException: If syntax errors are detected in the source file at the current stream position.
		 */
		virtual void consume(const StreamDesc& desc) = 0;
	};

	class LineCounter : public SourceFileConsumer
	{
	public:

		/**
		 * @throws ParseException: If syntax errors are detected in the source file at the current stream position.
		 */
		void consume(const StreamDesc& desc) override;


		int getLineCount() const;

	private:
		int mLineCount = 0;
	};

	class StreamPositionTracker : public SourceFileConsumer
	{
	public:

		/**
		 * Note: This function won't throw ParseException.
		 */
		void consume(const StreamDesc& desc) override;

		int getLine() const;
		int getColumn() const;

	private:
		int mLine = 1;
		int mColumn = 0;
	};

	class OffsetIncludeCalculator : public SourceFileConsumer
	{
	public:

		/**
		 * @throws ParseException: If syntax errors are detected in the source file at the current stream position.
		 */
		void consume(const StreamDesc& desc) override;

		int getOffset() const;
	private:
		int mLineCount = 0;
		std::string mLine;
		int mOffset = 0;
	};

	class IncludeCollector : public SourceFileConsumer
	{
	public:
		struct Include
		{
			std::string filePath;
			unsigned int lineBegin;
			unsigned int lineEnd;
		};

		/**
		 * @throws ParseException: If syntax errors are detected in the source file at the current stream position.
		 */
		void consume(const StreamDesc& desc) override;

		const std::vector<Include>& getIncludes() const;
	private:

		enum class State
		{
			DEFAULT,
			INCLUDE_DIRECTIVE,
			WHITESPACE,
			PATH,
			POST_INCLUDE
		};

		std::string mBuffer;
		std::vector<Include> mIncludes;
		State mState = State::DEFAULT;

		static bool isIgnorable(char c);

		static const char* getLineBegin(const StreamDesc& desc);
		static const char* getLineEnd(const StreamDesc& desc);

	};

	class FirstLineAfterVersionStatementSearcher : public SourceFileConsumer
	{
	public:
		
		/**
		 * @throws ParseException: If syntax errors are detected in the source file at the current stream position.
		 */
		void consume(const StreamDesc& desc) override;

		const size_t getResultPosition() const;

	private:

		enum class State
		{
			DEFAULT,
			VERSION_DIRECTIVE,
			FOUND,
		};

		std::string mBuffer;
		State mState = State::DEFAULT;
		size_t mFirstLineAfterVersionPosition = 0;
	};
}

std::ostream& operator<<(std::ostream& os, const nex::IncludeCollector::Include& include);

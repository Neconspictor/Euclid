#ifndef LOG_SINK_HPP
#define LOG_SINK_HPP
#include <platform/logging/LogMessage.hpp>
#include <memory>

namespace platform
{
	class LogSink {
	public:

		template <typename T>
		explicit LogSink(T impl);

		LogSink(const LogSink& sink);

		LogSink& operator =(LogSink sink);
		bool operator==(const LogSink& sink) const;
		void forward(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) const;
	private:
		struct Concept {
			virtual ~Concept() {}
			virtual Concept* clone() const = 0;
			virtual void operator()(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) = 0;

			virtual void forward(
				const std::string& prefix,
				const LogMessage::Meta& meta,
				const std::string& message
				) = 0;
		};

		template <typename T>
		struct Model : Concept {

			Model(T impl);

			virtual Concept* clone() const override;

			virtual void operator()(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) override;

			void forward(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) override;

			T mImpl;
		};

		std::unique_ptr<Concept> mWrapper;
	};

	LogSink makeConsoleSink();

	LogSink makeFileSink(const std::string& filename);
}

#include <platform/logging/LogSink.inl>
#endif
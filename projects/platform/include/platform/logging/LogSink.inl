#ifndef LOG_SINK_INL
#define LOG_SINK_INL

namespace platform
{
	template <typename T>
	LogSink::LogSink(T impl) :
		mWrapper(new Model<T>(std::move(impl)))
	{
	}

	template <typename T>
	LogSink::Model<T>::Model(T impl) :
		mImpl(std::move(impl))
	{
	}

	template <typename T>
	LogSink::Concept* LogSink::Model<T>::clone() const
	{
		return new Model<T>(mImpl);
	}

	template <typename T>
	void LogSink::Model<T>::operator()(const LogMessage::Meta& meta, const std::string& message)
	{
		mImpl(meta, message);
	}

	template <typename T>
	void LogSink::Model<T>::forward(
		const LogMessage::Meta& meta,
		const std::string& message
		) {
		mImpl(meta, message);
	}
}
#endif
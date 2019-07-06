#include <nex/util/ExceptionHandling.hpp>
#include <nex/common/Log.hpp>

void nex::ExceptionHandling::logExceptionWithStackTrace(nex::Logger & logger, const std::exception& e)
{
	const boost::stacktrace::stacktrace* st = boost::get_error_info<nex::traced>(e);

	if (st) {
		LOG(logger, nex::Fault) << "Stack trace:\n\n" << *st;
	}

	LOG(logger, nex::Fault) << "Exception: " << typeid(e).name() << ": " << e.what();
}
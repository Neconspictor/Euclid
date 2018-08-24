#pragma once

#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>

typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;

template <class E>
void throw_with_trace(const E& e) {
	throw boost::enable_error_info(e)
		<< traced(boost::stacktrace::stacktrace());
}
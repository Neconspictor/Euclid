/*
* Modified code from: https://github.com/IanBullard/event_taskmanager
*
* Copyright (c) 2014 GrandMaster (gijsber@gmail)
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#pragma optimize( "", on )

namespace nex
{
	template <typename T>
	LogEndpoint::LogEndpoint(T impl) :
		mWrapper(new WrapperImpl<T>(std::move(impl)))
	{
	}

	template <typename T>
	LogEndpoint::WrapperImpl<T>::WrapperImpl(T impl) :
		mImpl(std::move(impl))
	{
	}

	template <typename T>
	std::unique_ptr<LogEndpoint::Wrapper> LogEndpoint::WrapperImpl<T>::clone() const
	{
		return std::make_unique<WrapperImpl<T>>(mImpl);
	}

	/*template <typename T>
	void LogEndpoint::WrapperImpl<T>::operator()(const std::string& prefix, 
		const LogMessage::Meta& meta, const std::string& message)
	{
		mImpl(prefix, meta, message);
	}*/

	template <typename T>
	void LogEndpoint::WrapperImpl<T>::forward(
		const std::string& prefix,
		const LogMessage::Meta& meta,
		const std::string& message
		) {
		mImpl(prefix, meta, message);
	}
}
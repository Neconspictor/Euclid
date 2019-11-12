#pragma once

#include <fstream>
#include <string>

namespace nex
{
	class BinStream : public std::fstream
	{
	public:
		BinStream(size_t bufferSize = 1024);
		BinStream(const BinStream&) = delete;
		BinStream& operator=(const BinStream&) = delete;

		BinStream(BinStream&&) = delete;
		BinStream& operator=(BinStream&&) = delete;

		virtual ~BinStream() noexcept;

		void open(const char* filePath, std::ios_base::openmode mode);
		void open(const std::filesystem::path& path, std::ios_base::openmode mode);

		static void test();

	private:
		std::vector<char> mBuffer;
		std::filesystem::path mFile;
	};

	//class = std::enable_if<std::is_trivially_copyable<T>::value>::type* 
	template<typename T>
	nex::BinStream& operator>>(nex::BinStream& in, T& item)
	{
		static_assert(std::is_trivially_copyable<T>::value, "Type has to be trivially copyable!");
		in.read((char*)&item, sizeof(T));
		return in;
	}

	template<typename T>
	nex::BinStream& operator<<(nex::BinStream& out, const T& item)
	{
		static_assert(std::is_trivially_copyable<T>::value, "Type has to be trivially copyable!");
		out.write((const char*)&item, sizeof(T));
		return out;
	}

	nex::BinStream& operator>>(nex::BinStream& in, std::string& str);

	nex::BinStream& operator<<(nex::BinStream& out, const std::string& str);


	template<class T>
	nex::BinStream& operator<<(nex::BinStream& out, const std::unique_ptr<T>& elem) {
		out << *elem;
		return out;
	}

	template<class T>
	nex::BinStream& operator>>(nex::BinStream& in, std::unique_ptr<T>& elem) {
		elem = std::make_unique<T>();
		in >> *elem;

		return in;
	}

	template<class A, class B>
	nex::BinStream& operator<<(nex::BinStream& out, const std::pair<A, B>& pair) {
		out << pair.first;
		out << pair.second;

		return out;
	}

	template<class A, class B>
	nex::BinStream& operator>>(nex::BinStream& in, std::pair<A, B>& pair) {
		in >> pair.first;
		in >> pair.second;

		return in;
	}


	template<typename T>
	nex::BinStream& operator>>(nex::BinStream& in, std::vector<T>& vec)
	{
		size_t count = 0;
		in >> count;
		
		if (count > 0)
			vec.resize(count);

		for (size_t i = 0; i < count; ++i)
		{
			in >> vec[i];
		}

		return in;
	}

	template<typename T>
	nex::BinStream& operator<<(nex::BinStream& out, const std::vector<T>& vec)
	{
		const size_t count = vec.size();
		out << count;
		for (const auto& item : vec)
		{
			out << item;
		}

		return out;
	}


	template<template<typename, typename...> class Container, class Value, typename... Args,
		typename std::enable_if<!std::is_trivially_copyable<Container<Value, Args...>>::value>::type* = nullptr
	>
	nex::BinStream& operator>>(nex::BinStream& in, Container<Value, Args...>& container)
	{
		container.clear();

		size_t count = 0;
		in >> count;

		for (size_t i = 0; i < count; ++i)
		{
			using ValueType = std::remove_const_t<std::remove_reference_t<decltype(*container.begin())>>;
			ValueType value;
			in >> value;
			container.insert(container.end(), value);
		}

		return in;
	}

	template<template<typename, typename...> class Container, class Value, typename... Args,
		typename std::enable_if<!std::is_trivially_copyable<Container<Value, Args...>>::value>::type * = nullptr>
	nex::BinStream& operator<<(nex::BinStream& out, const Container<Value, Args...>& container)
	{
		const size_t count = std::distance(container.begin(), container.end());
		out << count;
		for (auto it = container.begin(); it != container.end(); ++it)
		{
			out << *it;
		}

		return out;
	}

	/**
	 * Specialization for char vectors (improved performance)
	 */
	nex::BinStream& operator>>(nex::BinStream& in, std::vector<char>& vec);

	/**
	 * Specialization for char vectors (improved performance)
	 */
	nex::BinStream& operator<<(nex::BinStream& out, const std::vector<char>& vec);
}

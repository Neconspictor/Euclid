#pragma once
#include <vector>
#include "nex/math/Constant.hpp"

namespace nex
{

	class BinStream;

	enum class LayoutType
	{
		UNSIGNED_INT, FIRST= UNSIGNED_INT,
		FLOAT,
		UNSIGNED_BYTE,
		UNSIGNED_SHORT, LAST = UNSIGNED_SHORT,
	};


	struct LayoutElement
	{
		LayoutType type;
		unsigned int count;
		bool normalized;

		static inline unsigned int getSizeOfType(LayoutType type);
	};

	class VertexLayout
	{
	private:
		std::vector<nex::LayoutElement> mElements;
		unsigned int mStride;

	public:
		VertexLayout() : mStride(0) {}

		template<typename T>
		inline void push(unsigned int count);

		template<>
		inline void push<Real>(unsigned int count);

		template<>
		inline void push<unsigned int>(unsigned int count);

		template<>
		inline void push<unsigned char>(unsigned int count);

		template<>
		inline void push<unsigned short>(unsigned int count);

		template<>
		inline void push<glm::vec3>(unsigned int count);

		template<>
		inline void push<glm::vec2>(unsigned int count);

		inline unsigned int getStride() const;
		inline const std::vector<LayoutElement>& getElements() const;

		void read(nex::BinStream& in);
		void write(nex::BinStream& out) const;
	};

	nex::BinStream& operator>>(nex::BinStream& in, VertexLayout& layout);

	nex::BinStream& operator<<(nex::BinStream& out, const VertexLayout& layout);

}

#include "VertexLayout.inl"
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


	struct VertexAttribute
	{
		LayoutType type;
		unsigned int count;
		bool normalized;
		/**
		 * Specifies whether the vertex element is used per instance; 
		 * If set to true, the vertex element is updated by instance draw call. 
		 * If set to false the vertex element will be updated per vertex shader call.
		 */
		bool instanced; 

		static inline unsigned int getSizeOfType(LayoutType type);
	};

	class VertexLayout
	{
	private:
		std::vector<nex::VertexAttribute> mElements;
		unsigned int mStride;

	public:
		VertexLayout() : mStride(0) {}

		template<typename T>
		inline void push(unsigned int count, bool instanced = false);

		template<>
		inline void push<float>(unsigned int count, bool instanced);

		template<>
		inline void push<unsigned int>(unsigned int count, bool instanced);

		template<>
		inline void push<unsigned char>(unsigned int count, bool instanced);

		template<>
		inline void push<unsigned short>(unsigned int count, bool instanced);

		template<>
		inline void push<glm::vec3>(unsigned int count, bool instanced);

		template<>
		inline void push<glm::vec2>(unsigned int count, bool instanced);

		inline unsigned int getStride() const;
		inline const std::vector<VertexAttribute>& getElements() const;

		void read(nex::BinStream& in);
		void write(nex::BinStream& out) const;
	};

	nex::BinStream& operator>>(nex::BinStream& in, VertexLayout& layout);

	nex::BinStream& operator<<(nex::BinStream& out, const VertexLayout& layout);

}

#include "VertexLayout.inl"
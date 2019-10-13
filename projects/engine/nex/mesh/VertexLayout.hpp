#pragma once
#include <vector>
#include "nex/math/Constant.hpp"

namespace nex
{

	class BinStream;
	class GpuBuffer;

	enum class LayoutPrimitive
	{
		UNSIGNED_INT, FIRST= UNSIGNED_INT,
		FLOAT,
		UNSIGNED_BYTE,
		UNSIGNED_SHORT, LAST = UNSIGNED_SHORT,
	};


	struct VertexAttribute
	{
		/**
		 * the data type of the attribute
		 */
		LayoutPrimitive type;

		/**
		 * Of how much primitives consists the attribute?
		 */
		unsigned int count;

		/**
		 * Should the primitives be normalized?
		 */
		bool normalized;
		/**
		 * Specifies whether the vertex element is used per instance; 
		 * If set to true, the vertex element is updated by instance draw call. 
		 * If set to false the vertex element will be updated per vertex shader call.
		 */
		bool instanced; 

		/**
		 * The buffer where the actual vertex attribute data is stored.
		 */
		GpuBuffer* buffer;



		static inline unsigned int getSizeOfType(LayoutPrimitive type);
	};

	class VertexLayout
	{
	private:
		std::vector<nex::VertexAttribute> mAttributes;
		unsigned int mStride;

	public:
		VertexLayout() : mStride(0) {}

		template<typename T>
		inline void push(unsigned int count, GpuBuffer* buffer, bool instanced = false);

		template<>
		inline void push<float>(unsigned int count, GpuBuffer* buffer, bool instanced);

		template<>
		inline void push<unsigned int>(unsigned int count, GpuBuffer* buffer, bool instanced);

		template<>
		inline void push<unsigned char>(unsigned int count, GpuBuffer* buffer, bool instanced);

		template<>
		inline void push<unsigned short>(unsigned int count, GpuBuffer* buffer, bool instanced);

		template<>
		inline void push<glm::vec3>(unsigned int count, GpuBuffer* buffer, bool instanced);

		template<>
		inline void push<glm::vec2>(unsigned int count, GpuBuffer* buffer, bool instanced);

		inline unsigned int getStride() const;
		inline const std::vector<VertexAttribute>& getAttributes() const;
		inline std::vector<VertexAttribute>& getAttributes();

		void read(nex::BinStream& in);
		void write(nex::BinStream& out) const;
	};

	nex::BinStream& operator>>(nex::BinStream& in, VertexLayout& layout);

	nex::BinStream& operator<<(nex::BinStream& out, const VertexLayout& layout);

}

#include "VertexLayout.inl"
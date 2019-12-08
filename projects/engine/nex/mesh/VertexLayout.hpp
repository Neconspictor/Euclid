#pragma once
#include <vector>
#include "nex/math/Constant.hpp"
#include <map>

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
		 * The vertex attribute index location (for the shader)
		 */
		unsigned location;

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
		 * Should the data be converted to a float data type?
		 */
		bool convertToFloat;


		static inline unsigned int getSizeOfType(LayoutPrimitive type);
	};

	struct BufferLayout {
		std::vector<nex::VertexAttribute> attributes;
		int stride = 0;
		int offset = 0;
	};


	class VertexLayout
	{
	public:

		using BufferLayoutMap = std::map<const nex::GpuBuffer*, nex::BufferLayout>;

		template<typename T>
		inline void push(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<float>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<unsigned int>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<unsigned char>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<unsigned short>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<glm::vec4>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<glm::vec3>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<glm::vec2>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<glm::uvec4>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<glm::uvec3>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		template<>
		inline void push<glm::uvec2>(unsigned int count, GpuBuffer* buffer, bool normalized, bool instanced, bool convertToFloat);

		inline const BufferLayoutMap& getBufferLayoutMap() const;
		inline BufferLayoutMap& getBufferLayoutMap();
		
		inline const BufferLayout* getLayout(const GpuBuffer* buffer) const;
		inline BufferLayout& getLayout(const GpuBuffer* buffer);

		void read(nex::BinStream& in);
		void write(nex::BinStream& out) const;

	private:
		BufferLayoutMap mMap;
		unsigned mLocationCounter = 0;
	};


	nex::BinStream& operator>>(nex::BinStream& in, BufferLayout& layout);

	nex::BinStream& operator<<(nex::BinStream& out, const BufferLayout& layout);


	nex::BinStream& operator>>(nex::BinStream& in, VertexLayout& layout);

	nex::BinStream& operator<<(nex::BinStream& out, const VertexLayout& layout);

}

#include "VertexLayout.inl"
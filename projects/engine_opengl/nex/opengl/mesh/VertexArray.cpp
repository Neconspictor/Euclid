#include "..\..\..\..\engine\nex\mesh\VertexArray.hpp"
#include <nex/mesh/VertexArray.hpp>
#include <nex/opengl/opengl.hpp>
#include "nex/opengl/RenderBackendGL.hpp"
#include "VertexArrayGL.hpp"
#include <nex/buffer/VertexBuffer.hpp>
#include <nex/mesh/VertexLayout.hpp>
#include <nex/opengl/buffer/GpuBufferGL.hpp>
#include <nex/util/ExceptionHandling.hpp>

namespace nex
{

	LayoutTypeGL translate(LayoutPrimitive type)
	{
		static LayoutTypeGL const table[]
		{
			UNSIGNED_INT,
			FLOAT,
			UNSIGNED_BYTE,
			UNSIGNED_SHORT,
		};

		static const unsigned size = (unsigned)LayoutPrimitive::LAST - (unsigned)LayoutPrimitive::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: LayoutType and LayoutTypeGL don't match!");

		return table[(unsigned)type];
	}

	bool isIntegerType(LayoutTypeGL type)
	{
		if (type == UNSIGNED_INT
			|| type == UNSIGNED_BYTE
			|| type == UNSIGNED_SHORT) {
			return true;
		}

		return false;
	}

	bool isFloatType(LayoutTypeGL type)
	{
		if (type == FLOAT) return true;
		return false;
	}

	bool isDoubleType(LayoutTypeGL type)
	{
		return false;
	}



	VertexArray::VertexArray() : mRendererID(GL_FALSE)
	{
	}

	VertexArray::VertexArray(VertexArray&& other) noexcept :
		mRendererID(other.mRendererID)
	{
		other.mRendererID = GL_FALSE;
	}

	VertexArray& VertexArray::operator=(VertexArray&& o) noexcept
	{
		if (this == &o) return *this;

		this->mRendererID = o.mRendererID;
		o.mRendererID = GL_FALSE;

		return *this;
	}

	VertexArray::~VertexArray()
	{
		if (mRendererID != GL_FALSE)
		{
			GLCall(glDeleteVertexArrays(1, &mRendererID));
			mRendererID = GL_FALSE;
		}
	}

	void VertexArray::bind() const
	{
		GLCall(glBindVertexArray(mRendererID));
	}

	VertexLayout& VertexArray::getLayout() {
		return mLayout;
	}
	const VertexLayout& VertexArray::getLayout() const {
		return mLayout;
	}

	void VertexArray::init()
	{
		if (mRendererID == GL_FALSE) 
			GLCall(glGenVertexArrays(1, &mRendererID));

		bind();

		for (const auto& it : mLayout.getBufferLayoutMap())
		{
			it.first->bind();
			assign(it.first, mLayout);
		}
	}

	void VertexArray::setLayout(const VertexLayout& layout) {
		mLayout = layout;
	}

	void VertexArray::unbind() const
	{
		GLCall(glBindVertexArray(0));
	}

	void VertexArray::unbindAny()
	{
		GLCall(glBindVertexArray(GL_FALSE));
	}

	void VertexArray::assign(const GpuBuffer* buffer, const VertexLayout& layout) {
		const auto* impl = buffer->getImpl();
		const auto& bufferLayout = *layout.getLayout(buffer);
		const auto& attributes = bufferLayout.attributes;
		const auto& stride = bufferLayout.stride;

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, impl->mRendererID));

		size_t offset = bufferLayout.offset;
		for (const auto& attribute : attributes)
		{
			const auto& location = attribute.location;
			GLCall(glEnableVertexAttribArray(location));


			auto glType = translate(attribute.type);

			if (isFloatType(glType) || attribute.convertToFloat) {
				GLCall(glVertexAttribPointer(location, attribute.count, translate(attribute.type),
					attribute.normalized, stride, (GLvoid*)offset));
			}
			else if (isIntegerType(glType)) {
				GLCall(glVertexAttribIPointer(location, attribute.count, translate(attribute.type),
					stride, (GLvoid*)offset));
			}
			else if (isDoubleType(glType)) {
				GLCall(glVertexAttribLPointer(location, attribute.count, translate(attribute.type),
					stride, (GLvoid*)offset));
			}
			else {
				throw_with_trace(std::runtime_error("VertexArray::assign: Not matched type: " + std::to_string(glType)));
			}

			

			GLCall(glVertexAttribDivisor(location, attribute.instanced ? 1 : 0));

			offset += attribute.count * VertexAttribute::getSizeOfType(attribute.type);
		}
	}
}
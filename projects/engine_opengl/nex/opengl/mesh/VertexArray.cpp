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

		//collect all used buffers
		std::set<GpuBuffer*> buffers;
		for (const auto& attribute : mLayout.getAttributes())
		{
			if (attribute.buffer != nullptr)
				buffers.insert(attribute.buffer);
		}

		//Now iterate over all buffers and connect attributes to buffers.

		for (auto* buffer : buffers) {
			assign(buffer, mLayout);
		}
	}

	void VertexArray::setLayout(const VertexLayout& layout) {
		mLayout = layout;
	}

	void VertexArray::unbind() const
	{
		GLCall(glBindVertexArray(0));
	}

	void VertexArray::assign(const GpuBuffer* buffer, const VertexLayout& layout) {
		const auto* impl = buffer->getImpl();
		const auto& attributes = layout.getAttributes();

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, impl->mRendererID));

		size_t offset = 0;
		for (unsigned int i = 0; i < attributes.size(); ++i)
		{
			const auto& attribute = attributes[i];

			// Only configure if attribute is assigned to current bound buffer
			if (attribute.buffer != buffer) continue;

			GLCall(glEnableVertexAttribArray(i));


			auto glType = translate(attribute.type);

			if (isFloatType(glType) || attribute.convertToFloat) {
				GLCall(glVertexAttribPointer(i, attribute.count, translate(attribute.type),
					attribute.normalized, layout.getStride(), (GLvoid*)offset));
			}
			else if (isIntegerType(glType)) {
				GLCall(glVertexAttribIPointer(i, attribute.count, translate(attribute.type),
					layout.getStride(), (GLvoid*)offset));
			}
			else if (isDoubleType(glType)) {
				GLCall(glVertexAttribLPointer(i, attribute.count, translate(attribute.type),
						layout.getStride(), (GLvoid*)offset));
			}
			else {
				throw_with_trace(std::runtime_error("VertexArray::assign: Not matched type: " + std::to_string(glType)));
			}

			

			GLCall(glVertexAttribDivisor(i, attribute.instanced ? 1 : 0));

			offset += attribute.count * VertexAttribute::getSizeOfType(attribute.type);
		}
	}
}
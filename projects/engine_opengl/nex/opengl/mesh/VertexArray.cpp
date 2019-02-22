#include <nex/mesh/VertexArray.hpp>
#include <nex/opengl/opengl.hpp>
#include "nex/opengl/RenderBackendGL.hpp"
#include "VertexArrayGL.hpp"

namespace nex
{

	LayoutTypeGL translate(LayoutType type)
	{
		static LayoutTypeGL const table[]
		{
			UNSIGNED_INT,
			FLOAT,
			UNSIGNED_BYTE,
			UNSIGNED_SHORT,
		};

		static const unsigned size = (unsigned)LayoutType::LAST - (unsigned)LayoutType::FIRST + 1;
		static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: LayoutType and LayoutTypeGL don't match!");

		return table[(unsigned)type];
	}



	VertexArray::VertexArray() : mRendererID(0)
	{
		GLCall(glGenVertexArrays(1, &mRendererID));
	}

	VertexArray::VertexArray(VertexArray&& other) noexcept :
		mRendererID(other.mRendererID), mBuffers(std::move(other.mBuffers))
	{
		other.mRendererID = GL_FALSE;
	}

	VertexArray& VertexArray::operator=(VertexArray&& o) noexcept
	{
		if (this == &o) return *this;

		this->mRendererID = o.mRendererID;
		o.mRendererID = GL_FALSE;

		this->mBuffers = std::move(o.mBuffers);

		return *this;
	}

	VertexArray::~VertexArray()
	{
		if (mRendererID != GL_FALSE)
		{
			GLCall(glDeleteVertexArrays(1, &mRendererID));
		}
	}

	void VertexArray::addBuffer(VertexBuffer buffer, const VertexLayout& layout)
	{
		bind();
		buffer.bind();

		mBuffers.emplace_back(std::move(buffer));

		const auto& elements = layout.getElements();

		size_t offset = 0;

		for (unsigned int i = 0; i < elements.size(); ++i)
		{
			const auto& elem = elements[i];
			
			GLCall(
				glVertexAttribPointer(i, elem.count, translate(elem.type),
					elem.normalized, layout.getStride(), (GLvoid*)offset)
			); //layout.getStride() TODO
			offset += elem.count * LayoutElement::getSizeOfType(elem.type);
			GLCall(glEnableVertexAttribArray(i));
		}

		//buffer.unbind();

		unbind();
	}

	void VertexArray::bind() const
	{
		GLCall(glBindVertexArray(mRendererID));
	}

	void VertexArray::unbind() const
	{
		GLCall(glBindVertexArray(0));
	}
}
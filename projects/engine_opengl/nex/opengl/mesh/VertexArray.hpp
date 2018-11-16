#pragma once
#include "VertexBuffer.hpp"
#include "VertexLayout.hpp"

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void addBuffer(const VertexBuffer& buffer, const VertexLayout& layout);

	void bind() const;
	void unbind() const;

private:
	unsigned int mRendererID;
};
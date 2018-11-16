#pragma once

class VertexBuffer
{
private:
	unsigned int mRendererID;
public:
	VertexBuffer(const void* data, size_t size);
	~VertexBuffer();

	void bind() const;
	void unbind() const;
};
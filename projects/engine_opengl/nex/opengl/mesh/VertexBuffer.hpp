#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, size_t size);
	

	VertexBuffer(VertexBuffer&& other) noexcept;
	VertexBuffer& operator=(VertexBuffer&& o) noexcept = default;

	VertexBuffer(const VertexBuffer& o) = delete;
	VertexBuffer& operator=(const VertexBuffer& o) = delete;

	~VertexBuffer();

	void bind() const;
	void unbind() const;

private:
	unsigned int mRendererID;
};
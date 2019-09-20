#pragma once

#include <nex/buffer/GpuBuffer.hpp>

namespace nex
{
	class VertexBuffer : public GpuBuffer
	{
	public:

		VertexBuffer(size_t size, const void* data = nullptr, UsageHint usage = UsageHint::STATIC_DRAW);
		VertexBuffer(UsageHint usage = UsageHint::STATIC_DRAW);


		VertexBuffer(VertexBuffer&& other) = default;
		VertexBuffer& operator=(VertexBuffer&& o) = default;

		VertexBuffer(const VertexBuffer& o) = delete;
		VertexBuffer& operator=(const VertexBuffer& o) = delete;

		virtual ~VertexBuffer();

		//Has to be implemented by render backend
		static void unbindAny();
	};
}
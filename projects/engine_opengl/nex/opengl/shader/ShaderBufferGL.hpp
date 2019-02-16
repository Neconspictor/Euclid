#pragma once
#include <glad/glad.h>
#include <nex/shader/ShaderBuffer.hpp>

namespace nex
{
	namespace ShaderBuffer
	{
		enum UsageHintGL {
			DYNAMIC_COPY = GL_DYNAMIC_COPY,
			DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
			DYNAMIC_READ = GL_DYNAMIC_READ,

			STATIC_COPY = GL_STATIC_COPY,
			STATIC_READ = GL_STATIC_READ,
			STATIC_DRAW = GL_STATIC_DRAW,

			STREAM_COPY = GL_STREAM_COPY,
			STREAM_DRAW = GL_STREAM_DRAW,
			STREAM_READ = GL_STREAM_READ,
		};

		enum AccessGL
		{
			READ_ONLY = GL_READ_ONLY,
			WRITE_ONLY = GL_WRITE_ONLY,
			READ_WRITE = GL_READ_WRITE,
		};
	};

	ShaderBuffer::UsageHintGL translate(nex::ShaderBuffer::UsageHint usageHint);
	ShaderBuffer::AccessGL translate(nex::ShaderBuffer::Access access);	
}
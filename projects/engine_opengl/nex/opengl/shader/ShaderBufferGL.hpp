#pragma once
#include <glad/glad.h>

namespace nex
{


	struct ShaderBufferGL
	{
		enum UsageHint {
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

		enum Access
		{
			READ_ONLY = GL_READ_ONLY,
			WRITE_ONLY = GL_WRITE_ONLY,
			READ_WRITE = GL_READ_WRITE,
		};
	};


	class ShaderStorageBufferGL
	{
	public:
		/**
		 * Creates a new shader buffer.
		 * @param binding : The binding location of the buffer in the shader.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		ShaderStorageBufferGL(unsigned int binding, size_t size, ShaderBufferGL::UsageHint hint);

		virtual ~ShaderStorageBufferGL();



		void bind();
		size_t getSize() const;
		ShaderBufferGL::UsageHint getUsageHint() const;

		/**
		 * Note: bind() has to be called before calling this function.
		 */
		void* map(ShaderBufferGL::Access usage);

		void unbind();
		void unmap();

		/**
		 * Note: bind() has to be called before calling this function.
		 */
		void update(const void* data, size_t size, size_t offset = 0);


	private:
		unsigned int mRendererID;
		unsigned int mBinding;
		size_t mSize;
		ShaderBufferGL::UsageHint mUsageHint;

		/**
		 * @param data : Used to initialize the buffer. Can be null for not initializing the buffer store.
		 * @param size : The size of the buffer store to be created
		 * @param hint : An hint how the store is going to be used.
		 *
		 * Note: bind() has to be called before calling this function.
		 */
		void createStore(void* data, size_t size, ShaderBufferGL::UsageHint hint);
	};



	class UniformBufferGL
	{
	public:

		/**
		 * Creates a new shader buffer.
		 * @param binding : The binding location of the buffer in the shader.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		UniformBufferGL(unsigned int binding, size_t size, ShaderBufferGL::UsageHint hint);

		virtual ~UniformBufferGL();

		

		void bind();
		size_t getSize() const;
		ShaderBufferGL::UsageHint getUsageHint() const;
		
		/**
		 * Note: bind() has to be called before calling this function.
		 */
		void* map(ShaderBufferGL::Access usage);
		
		void unbind();
		void unmap();
		
		/**
		 * Note: bind() has to be called before calling this function.
		 */
		void update(const void* data, size_t size, size_t offset = 0);


	private:
		unsigned int mRendererID;
		unsigned int mBinding;
		size_t mSize;
		ShaderBufferGL::UsageHint mUsageHint;

		/**
		 * @param data : Used to initialize the buffer. Can be null for not initializing the buffer store.
		 * @param size : The size of the buffer store to be created
		 * @param hint : An hint how the store is going to be used.
		 * 
		 * Note: bind() has to be called before calling this function.
		 */
		void createStore(void* data, size_t size, ShaderBufferGL::UsageHint hint);
	};
}
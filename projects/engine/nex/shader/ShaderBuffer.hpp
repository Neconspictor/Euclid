#pragma once

namespace nex
{
	namespace ShaderBuffer
	{
		enum class UsageHint {
			DYNAMIC_COPY, FIRST = DYNAMIC_COPY,
			DYNAMIC_DRAW,
			DYNAMIC_READ,

			STATIC_COPY,
			STATIC_READ,
			STATIC_DRAW,

			STREAM_COPY,
			STREAM_DRAW,
			STREAM_READ, LAST = STREAM_READ,
		};

		enum class Access
		{
			READ_ONLY, FIRST = READ_ONLY,
			WRITE_ONLY,
			READ_WRITE, LAST = READ_WRITE,
		};
	}


	/**
	 * Note: class has to be implemented by renderer backend!
	 */
	class ShaderStorageBuffer
	{
	public:
		/**
		 * Creates a new shader buffer.
		 * @param binding : The binding location of the buffer in the shader.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		ShaderStorageBuffer(unsigned int binding, size_t size, ShaderBuffer::UsageHint hint);

		~ShaderStorageBuffer();



		void bind();
		size_t getSize() const;
		ShaderBuffer::UsageHint getUsageHint() const;

		/**
		 * Note: bind() has to be called before calling this function.
		 */
		void* map(ShaderBuffer::Access usage);

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
		ShaderBuffer::UsageHint mUsageHint;

		/**
		 * @param data : Used to initialize the buffer. Can be null for not initializing the buffer store.
		 * @param size : The size of the buffer store to be created
		 * @param hint : An hint how the store is going to be used.
		 *
		 * Note: bind() has to be called before calling this function.
		 */
		void createStore(void* data, size_t size, ShaderBuffer::UsageHint hint);
	};



	/**
	 * Note: class has to be implemented by renderer backend!
	 */
	class UniformBuffer
	{
	public:

		/**
		 * Creates a new shader buffer.
		 * @param binding : The binding location of the buffer in the shader.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		UniformBuffer(unsigned int binding, size_t size, ShaderBuffer::UsageHint hint);

		~UniformBuffer();

		

		void bind();
		size_t getSize() const;
		ShaderBuffer::UsageHint getUsageHint() const;
		
		/**
		 * Note: bind() has to be called before calling this function.
		 */
		void* map(ShaderBuffer::Access usage);
		
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
		ShaderBuffer::UsageHint mUsageHint;

		/**
		 * @param data : Used to initialize the buffer. Can be null for not initializing the buffer store.
		 * @param size : The size of the buffer store to be created
		 * @param hint : An hint how the store is going to be used.
		 * 
		 * Note: bind() has to be called before calling this function.
		 */
		void createStore(void* data, size_t size, ShaderBuffer::UsageHint hint);
	};
}
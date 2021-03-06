#pragma once

namespace nex
{

	/**
	 * Note: This class is not intended to be used by user code, but as a base class for specialized buffer types.
	 * Therefore the constructor of this class is protected.
	 * Note: class has to be implemented by renderer backend!
	 */
	class GpuBuffer
	{
	public:

		class Impl;

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

		GpuBuffer(GpuBuffer&& other);
		GpuBuffer& operator=(GpuBuffer&& o);

		GpuBuffer(const GpuBuffer& o) = delete;
		GpuBuffer& operator=(const GpuBuffer& o) = delete;

		virtual ~GpuBuffer();

		/**
		 * Binds the buffer.
		 */
		void bind() const;
		
		/**
		 * Provides the api specific implementation.
		 */
		Impl* getImpl();

		/**
		 * Provides the api specific implementation.
		 */
		const Impl* getImpl() const;

		/**
		 * Provides the (byte) size of this buffer.
		 */
		size_t getSize() const;

		UsageHint getUsageHint() const;


		void* map(Access usage) const;

		/**
		 * Unbinds the buffer.
		 * Note: bind() has to be called before calling this function.
		 */
		void unbind() const;

		void unmap() const;

		/**
		 * @param data : Used to initialize the buffer. Can be null for not initializing the buffer store.
		 * @param size : The size of the buffer store to be created
		 * @param hint : An hint how the store is going to be used.
		 * @param allowOrphaning : Recreates the data store even if the old data store has the same size.
		 *
		 */
		void resize(size_t size, const void* data, UsageHint hint, bool allowOrphaning = true);


		static void syncWithGPU();

		/**
		 * Updates the content of the buffer.
		 * NOTE: The size of the new content mustn't exceed the size of the buffer. 
		 * A greater size will result to undefined behaviour!
		 * @param data : the new content
		 * @param size: the size of the new content.
		 * @param offset: start position for the update.
		 */
		void update(size_t size, const void* data, size_t offset = 0);

	protected:

		/**
		 * Creates a new, but uninitialized buffer.
		 * @param data: data for filling this buffer or nullptr (= no data)
		 * @param size: The size of the buffer. If data != nullptr, it also specifies the range of data for filling.
		 * @param internalBufferType: A render backend specific argument specifying the buffer type.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		GpuBuffer(void* internalBufferType, size_t size, const void* data, UsageHint usage);

		size_t mSize;
		UsageHint mUsageHint;
		Impl* mImpl;
	};

	class ShaderBuffer : public GpuBuffer
	{
	public:

		ShaderBuffer(ShaderBuffer&& other) = default;
		ShaderBuffer& operator=(ShaderBuffer&& o) = default;

		ShaderBuffer(const ShaderBuffer& o) = delete;
		ShaderBuffer& operator=(const ShaderBuffer& o) = delete;

		virtual ~ShaderBuffer();

		/**
		 * Binds the buffer to it's specified default binding point.
		 */
		void bindToTarget() const;

		/**
		 * Binds the buffer using a specified binding point.
		 */
		void bindToTarget(unsigned binding) const;

		unsigned getDefaultBinding() const;

	protected:

		/**
		 * Creates a new shader buffer.
		 * @param binding : The binding location of the buffer in the shader.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		ShaderBuffer(unsigned int binding, void* internalBufferType, size_t size, const void* data, UsageHint usage);


		unsigned int mBinding;
	};

}
#pragma once

#include <nex/buffer/GpuBuffer.hpp>

namespace nex
{
	/**
	 * Note: class has to be implemented by renderer backend!
	 */
	class ShaderStorageBuffer : public ShaderBuffer
	{
	public:
		/**
		 * Creates a new shader buffer.
		 * @param binding : The binding location of the buffer in the shader.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		ShaderStorageBuffer(unsigned int binding, size_t size, void* data, UsageHint usage);


		ShaderStorageBuffer(ShaderStorageBuffer&& other) = default;
		ShaderStorageBuffer& operator=(ShaderStorageBuffer&& o) = default;

		ShaderStorageBuffer(const ShaderStorageBuffer& o) = delete;
		ShaderStorageBuffer& operator=(const ShaderStorageBuffer& o) = delete;

		virtual ~ShaderStorageBuffer();
	};


	/**
	 * Note: class has to be implemented by renderer backend!
	 */
	class UniformBuffer : public ShaderBuffer
	{
	public:

		/**
		 * Creates a new shader buffer.
		 * @param binding : The binding location of the buffer in the shader.
		 * @param size : The size of the buffer. Must be a multiple of four.
		 */
		UniformBuffer(unsigned int binding, size_t size, void* data, UsageHint usage);

		UniformBuffer(UniformBuffer&& other) = default;
		UniformBuffer& operator=(UniformBuffer&& o) = default;

		UniformBuffer(const UniformBuffer& o) = delete;
		UniformBuffer& operator=(const UniformBuffer& o) = delete;

		virtual ~UniformBuffer();
	};
}
#pragma once
#include <glm/detail/type_vec4.hpp>

namespace nex
{
	enum class TextureUVTechnique;
	enum class CompareFunction;
	enum class TextureFilter;
	struct SamplerDesc;

	class Sampler
	{
	public:

		class Impl;

		// Class and subclasses shouldn't be movable/copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		// Inherited classes cannot be copied/moved as well
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;

		Sampler(const SamplerDesc& samplerState);

		~Sampler();

		// Has to be implemented by renderer backend
		void bind(unsigned textureBindingSlot);

		const SamplerDesc& getState() const;

		// Has to be implemented by renderer backend
		void setMinFilter(TextureFilter filter);

		// Has to be implemented by renderer backend
		void setMagFilter(TextureFilter filter);

		// Has to be implemented by renderer backend
		void setAnisotropy(float anisotropy);

		// Has to be implemented by renderer backend
		void useDepthComparison(bool use);

		// Has to be implemented by renderer backend
		void setCompareFunction(CompareFunction compareFunction);

		// Has to be implemented by renderer backend
		void setWrapS(TextureUVTechnique wrap);

		// Has to be implemented by renderer backend
		void setWrapT(TextureUVTechnique wrap);

		// Has to be implemented by renderer backend
		void setWrapR(TextureUVTechnique wrap);

		// Has to be implemented by renderer backend
		void setBorderColor(const glm::vec4& color);

		// Has to be implemented by renderer backend
		void setMinLOD(float lod);

		// Has to be implemented by renderer backend
		void setMaxLOD(float lod);

		// Has to be implemented by renderer backend
		void setLodBias(float bias);

		// Has to be implemented by renderer backend
		/**
		 * Unbinds any current bound Sampler from a specific texture binding point.
		 */
		static void unbind(unsigned textureBindingSlot);

	protected:
		//Used in order to avoid virtual function calls
		std::unique_ptr<Impl> mImpl;
	};
}
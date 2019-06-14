#pragma once
#include <glm/detail/type_vec4.hpp>
#include <nex/texture/TextureSamplerData.hpp>

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

		Sampler(const SamplerDesc& samplerState = {});
		
		// Class and subclasses shouldn't be copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = default;
		Sampler& operator=(const Sampler&) = delete;
		Sampler& operator=(Sampler&&) = default;

		

		~Sampler();

		// Has to be implemented by renderer backend
		void bind(unsigned textureBindingSlot) const;

		const SamplerDesc& getState() const;

		// Has to be implemented by renderer backend
		void setMinFilter(TextureFilter filter);

		// Has to be implemented by renderer backend
		void setMagFilter(TextureFilter filter);

		// Has to be implemented by renderer backend
		void setAnisotropy(Real anisotropy);

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
		void setMinLOD(Real lod);

		// Has to be implemented by renderer backend
		void setMaxLOD(Real lod);

		// Has to be implemented by renderer backend
		void setLodBias(Real bias);

		// Has to be implemented by renderer backend
		void setState(const SamplerDesc& desc);

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
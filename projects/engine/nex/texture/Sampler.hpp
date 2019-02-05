#pragma once
#include <glm/detail/type_vec4.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	struct SamplerState
	{
		glm::vec4 borderColor = { 0,0,0,0 };
		TextureFilter minFilter = TextureFilter::Near_Mipmap_Linear;
		TextureFilter magFilter = TextureFilter::Linear;
		TextureUVTechnique wrapS = TextureUVTechnique::Repeat;
		TextureUVTechnique wrapT = TextureUVTechnique::Repeat;
		TextureUVTechnique wrapR = TextureUVTechnique::Repeat;
		int minLOD = -1000;
		int maxLOD = 1000;
		int biasLOD = 0;
		//compareMode TODO
		//compareFunc TODO
		bool useDepthComparison = false; // Only used for depth-stencil maps
		DepthComparison compareFunction = DepthComparison::LESS_EQUAL;
		float anisotropy = 1.0f;
	};


	class Sampler
	{
	public:
		// Class and subclasses shouldn't be movable/copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		// Inherited classes cannot be copied/moved as well
		Sampler(const Texture&) = delete;
		Sampler& operator=(const Sampler&) = delete;

		virtual ~Sampler() = default;

		static Sampler* create(const SamplerState& samplerState);

		// Has to be implemented by renderer backend
		void bind(unsigned textureBindingSlot);

		const SamplerState& getState() const;

		// Has to be implemented by renderer backend
		void setMinFilter(TextureFilter filter);

		// Has to be implemented by renderer backend
		void setMagFilter(TextureFilter filter);

		// Has to be implemented by renderer backend
		void setAnisotropy(float anisotropy);

		// Has to be implemented by renderer backend
		void useDepthComparison(bool use);

		// Has to be implemented by renderer backend
		void setCompareFunction(DepthComparison compareFunction);

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
		void unbind(unsigned textureBindingSlot);

	protected:
		Sampler(const SamplerState& samplerState, void* impl) : mState(samplerState), mImpl(impl) {}
		SamplerState mState;

		//Used in order to avoid virtual function calls
		void* mImpl;
	};
}
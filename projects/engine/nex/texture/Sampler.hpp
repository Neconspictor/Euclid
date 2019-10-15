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


		const Impl* getImpl() const;
		Impl* getImpl();

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
		void setState(const SamplerDesc& desc);

		// Has to be implemented by renderer backend
		/**
		 * Unbinds any current bound Sampler from a specific texture binding point.
		 */
		static void unbind(unsigned textureBindingSlot);

		/**
		 * Provides the default image sampler.
		 * At the opposite to the rest of the static getters, this getter isn't const,
		 * so that it is possible to change sampling globally for all images (e.g. anisotropic filtering)
		 */
		static Sampler* getDefaultImage() {
			static auto sampler = createDefaultImageSampler();
			return &sampler;
		}

		static const Sampler* getPoint() {
			static auto sampler = createPointSampler();
			return &sampler;
		}

		static const Sampler* getLinear() {
			static auto sampler = createLinearSampler();
			return &sampler;
		}

		static const Sampler* getLinearMipMap() {
			static auto sampler = createLinearMipMapSampler();
			return &sampler;
		}

		static const Sampler* getNearMipMap() {
			static auto sampler = createNearMipMapSampler();
			return &sampler;
		}

	protected:
		//Used in order to avoid virtual function calls
		std::unique_ptr<Impl> mImpl;

	private: 
		static Sampler createPointSampler() {
			SamplerDesc desc;
			desc.minFilter = TextureFilter::NearestNeighbor;
			desc.magFilter = TextureFilter::NearestNeighbor;
			return Sampler(desc);
		}

		static Sampler createLinearSampler() {
			return Sampler();
		}

		static Sampler createLinearMipMapSampler() {
			SamplerDesc desc;
			desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
			return Sampler(desc);
		}

		static Sampler createNearMipMapSampler() {
			SamplerDesc desc;
			desc.minFilter = TextureFilter::Near_Mipmap_Near;
			desc.magFilter = TextureFilter::NearestNeighbor;
			return Sampler(desc);
		}

		static Sampler createDefaultImageSampler() {
			SamplerDesc desc;
			desc.minFilter = TextureFilter::Linear_Mipmap_Linear;
			desc.magFilter = TextureFilter::Linear;
			desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::Repeat;
			desc.maxAnisotropy = 16.0f;
			return Sampler(desc);
		}
	};
}
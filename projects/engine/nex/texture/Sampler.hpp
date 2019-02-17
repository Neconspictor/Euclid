#pragma once
#include <glm/detail/type_vec4.hpp>

namespace nex
{
	enum class CompareFunction
	{
		ALWAYS, FIRST = ALWAYS,
		EQUAL,
		GREATER,
		GREATER_EQUAL,
		LESS,
		LESS_EQUAL,
		NEVER,
		NOT_EQUAL, LAST = NOT_EQUAL
	};

	enum class TextureFilter
	{
		NearestNeighbor, FIRST = NearestNeighbor,
		Linear,
		Near_Mipmap_Near,     // trilinear filtering with double nearest neighbor filtering
		Near_Mipmap_Linear,   // trilinear filtering from nearest neighbor to bilinear filtering
		Linear_Mipmap_Near,   // trilinear filtering from bilinear to nearest neighbor filtering
		Linear_Mipmap_Linear, LAST = Linear_Mipmap_Linear,// trilinear filtering from bilinear to bilinear filtering
	};

	enum class TextureUVTechnique
	{
		ClampToBorder, FIRST = ClampToBorder,
		ClampToEdge,
		MirrorRepeat,
		MirrorClampToEdge,
		Repeat, LAST = Repeat,
	};

	struct SamplerDesc
	{
		glm::vec4 borderColor = { 0,0,0,0 };
		TextureFilter minFilter = TextureFilter::Near_Mipmap_Linear;
		TextureFilter magFilter = TextureFilter::Linear;
		TextureUVTechnique wrapS = TextureUVTechnique::Repeat;
		TextureUVTechnique wrapT = TextureUVTechnique::Repeat;
		TextureUVTechnique wrapR = TextureUVTechnique::Repeat;
		int minLOD = -1000;
		int maxLOD = 1000;
		float biasLOD = 0;
		bool useDepthComparison = false; // Only used for depth-stencil maps
		CompareFunction compareFunction = CompareFunction::LESS_EQUAL;
		float maxAnisotropy = 1.0f;
	};


	class Sampler
	{
	public:
		// Class and subclasses shouldn't be movable/copiable
		// Implicitly removes auto-generated move constructor/assignment operator
		// Inherited classes cannot be copied/moved as well
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;

		virtual ~Sampler() = default;

		static Sampler* create(const SamplerDesc& samplerState);

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
		void unbind(unsigned textureBindingSlot);

	protected:
		Sampler(const SamplerDesc& samplerState, void* impl) : mState(samplerState), mImpl(impl) {}
		SamplerDesc mState;

		//Used in order to avoid virtual function calls
		void* mImpl;
	};
}
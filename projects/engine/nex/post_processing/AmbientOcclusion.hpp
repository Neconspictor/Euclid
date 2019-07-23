#pragma once

#include <memory>
#include <ostream>

namespace nex {
	class PerspectiveCamera;


	class SceneNode;
	class TextureGL;
	class HBAO;
	class SSAO_Deferred;
	class Texture2D;
	class Camera;

	enum class AOTechnique
	{
		HBAO,
		SSAO
	};

	class AmbientOcclusionSelector
	{
	public:

		AmbientOcclusionSelector(unsigned width, unsigned height);

		~AmbientOcclusionSelector();

		nex::HBAO* getHBAO();
		SSAO_Deferred* getSSAO();

		bool isAmbientOcclusionActive() const;

		AOTechnique getActiveAOTechnique() const;

		void onSizeChange(unsigned width, unsigned height);

		Texture2D* renderAO(const Camera& camera, Texture2D* gDepth);

		void setAOTechniqueToUse(AOTechnique technique);
		void setUseAmbientOcclusion(bool useAO);

	private:

		std::unique_ptr<nex::HBAO> m_hbao;
		std::unique_ptr<SSAO_Deferred> m_ssao;
		bool m_useAO = true;
		AOTechnique m_usedAOTechnique = AOTechnique::HBAO;
	};

	std::ostream& operator<<(std::ostream& os, const nex::AOTechnique& aoTechnique);
}
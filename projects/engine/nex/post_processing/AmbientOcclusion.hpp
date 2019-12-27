#pragma once

#include <memory>
#include <ostream>

namespace nex {
	class PerspectiveCamera;


	class SceneNode;
	class TextureGL;
	class HBAO;
	class Texture2D;
	class Camera;

	enum class AOTechnique
	{
		HBAO
	};

	class AmbientOcclusionSelector
	{
	public:

		AmbientOcclusionSelector(unsigned width, unsigned height);

		~AmbientOcclusionSelector();

		nex::HBAO* getHBAO();

		bool isAmbientOcclusionActive() const;

		AOTechnique getActiveAOTechnique() const;

		Texture2D* getRenderResult();

		void onSizeChange(unsigned width, unsigned height);

		Texture2D* renderAO(const Camera& camera, Texture2D* gDepth);

		void setAOTechniqueToUse(AOTechnique technique);
		void setUseAmbientOcclusion(bool useAO);

	private:

		std::unique_ptr<nex::HBAO> mHbao;
		bool m_useAO = true;
		AOTechnique m_usedAOTechnique = AOTechnique::HBAO;
	};

	std::ostream& operator<<(std::ostream& os, const nex::AOTechnique& aoTechnique);
}
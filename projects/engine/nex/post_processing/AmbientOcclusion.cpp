#include <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/post_processing/HBAO.hpp>
#include "nex/texture/TextureManager.hpp"
#include "nex/camera/Camera.hpp"

namespace nex
{
	AmbientOcclusionSelector::AmbientOcclusionSelector(unsigned width, unsigned height) : mHbao(std::make_unique<HBAO>(width, height))
	{
	}

	AmbientOcclusionSelector::~AmbientOcclusionSelector() = default;

	nex::HBAO* AmbientOcclusionSelector::getHBAO()
	{
		return mHbao.get();
	}

	bool AmbientOcclusionSelector::isAmbientOcclusionActive() const
	{
		return m_useAO;
	}

	AOTechnique AmbientOcclusionSelector::getActiveAOTechnique() const
	{
		return m_usedAOTechnique;
	}

	Texture2D* AmbientOcclusionSelector::getRenderResult()
	{
		if (!isAmbientOcclusionActive())
			// Return a default white texture (means no ambient occlusion)
			return TextureManager::get()->getDefaultWhiteTexture();

		if (getActiveAOTechnique() == AOTechnique::HBAO)
			return mHbao->getBlurredResult();

		return nullptr;
	}

	void AmbientOcclusionSelector::onSizeChange(unsigned width, unsigned height)
	{
		mHbao->onSizeChange(width, height);
	}

	Texture2D* AmbientOcclusionSelector::renderAO(const Camera& camera, Texture2D* gDepth)
	{
		if (!isAmbientOcclusionActive())
			// Return a default white texture (means no ambient occlusion)
			return TextureManager::get()->getDefaultWhiteTexture();

		if (getActiveAOTechnique() == AOTechnique::HBAO)
		{
			nex::Projection projection;
			projection.farplane = camera.getFarDistance();
			projection.matrix = camera.getProjectionMatrix();
			projection.nearplane = camera.getNearDistance();

			auto* perspectiveCamera = dynamic_cast<const PerspectiveCamera*>(&camera);

			if (perspectiveCamera) {
				projection.fov = perspectiveCamera->getFovY();
				projection.orthoheight = 0;
				projection.perspective = true;
			}
			else {

				auto* orthoCamera = dynamic_cast<const OrthographicCamera*>(&camera);

				projection.orthoheight = orthoCamera->getHeight();
				projection.perspective = false;
			}

			getHBAO()->renderAO(gDepth, projection);

			return getHBAO()->getBlurredResult();
		}

		return nullptr;
	}

	void AmbientOcclusionSelector::setAOTechniqueToUse(AOTechnique technique)
	{
		m_usedAOTechnique = technique;
	}

	void AmbientOcclusionSelector::setUseAmbientOcclusion(bool useAO)
	{
		m_useAO = useAO;
	}
}

std::ostream& nex::operator<<(std::ostream& os, const nex::AOTechnique& aoTechnique)
{
	switch (aoTechnique)
	{
	case nex::AOTechnique::HBAO:
	{
		os << "HBAO";
		break;
	}

	default:
		nex::throw_with_trace(std::out_of_range("Not a registered aoTechnique: " + (int)aoTechnique));
	}

	return os;
}
#include <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/util/ExceptionHandling.hpp>

AmbientOcclusionSelector::AmbientOcclusionSelector(): m_usedAOTechnique(HBAO), m_hbao(nullptr), m_ssao(nullptr)
{
}

void AmbientOcclusionSelector::setHBAO(std::unique_ptr<hbao::HBAO> hbao)
{
	m_hbao = move(hbao);
}

void AmbientOcclusionSelector::setSSAO(std::unique_ptr<SSAO_Deferred> ssao)
{
	m_ssao = move(ssao);
}

std::ostream& operator<<(std::ostream& os, AmbientOcclusionSelector::AOTechnique aoTechnique)
{
	switch(aoTechnique)
	{
	case AmbientOcclusionSelector::HBAO:
		{
		os << "HBAO";
		break;
		}
	case AmbientOcclusionSelector::SSAO:
		{
		os << "SSAO";
		break;
		}
	default:
		throw_with_trace(std::out_of_range("Not a registered aoTechnique: " + (int)aoTechnique));
	}

	return os;
}

AmbientOcclusionSelector::~AmbientOcclusionSelector()
{
}

hbao::HBAO* AmbientOcclusionSelector::getHBAO()
{
	return m_hbao.get();
}

SSAO_Deferred* AmbientOcclusionSelector::getSSAO()
{
	return m_ssao.get();
}

bool AmbientOcclusionSelector::isAmbientOcclusionActive() const
{
	return m_useAO;
}

AmbientOcclusionSelector::AOTechnique AmbientOcclusionSelector::getActiveAOTechnique() const
{
	return m_usedAOTechnique;
}

void AmbientOcclusionSelector::setAOTechniqueToUse(AOTechnique technique)
{
	m_usedAOTechnique = technique;
}

void AmbientOcclusionSelector::setUseAmbientOcclusion(bool useAO)
{
	m_useAO = useAO;
}
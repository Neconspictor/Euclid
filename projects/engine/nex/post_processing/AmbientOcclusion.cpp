#include <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/util/ExceptionHandling.hpp>

namespace nex
{
	AmbientOcclusionSelector::AmbientOcclusionSelector() : m_hbao(nullptr), m_ssao(nullptr)
	{
	}

	void AmbientOcclusionSelector::setHBAO(std::unique_ptr<nex::HBAO> hbao)
	{
		m_hbao = move(hbao);
	}

	void AmbientOcclusionSelector::setSSAO(std::unique_ptr<SSAO_Deferred> ssao)
	{
		m_ssao = move(ssao);
	}

	AmbientOcclusionSelector::~AmbientOcclusionSelector()
	{
	}

	nex::HBAO* AmbientOcclusionSelector::getHBAO()
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
}

std::ostream& operator<<(std::ostream& os, nex::AmbientOcclusionSelector::AOTechnique aoTechnique)
{
	switch (aoTechnique)
	{
	case nex::AmbientOcclusionSelector::HBAO:
	{
		os << "HBAO";
		break;
	}
	case nex::AmbientOcclusionSelector::SSAO:
	{
		os << "SSAO";
		break;
	}
	default:
		nex::throw_with_trace(std::out_of_range("Not a registered aoTechnique: " + (int)aoTechnique));
	}

	return os;
}
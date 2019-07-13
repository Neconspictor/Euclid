#include <nex/pbr/Pbr.hpp>
#include <nex/light/Light.hpp>
#include "imgui/imgui.h"
#include "nex/gui/Util.hpp"
#include "nex/shadow/CascadedShadow.hpp"
#include "PbrDeferred.hpp"
#include "PbrForward.hpp"
#include <nex/shader/Pass.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <nex/pbr/PbrProbe.hpp>

nex::Pbr::Pbr(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe) :
	mAmbientLight(ambientLight), mCascadeShadow(cascadeShadow), mLight(dirLight), mProbe(probe)
{
	mCascadeShadow->addCascadeChangeCallback([&](CascadedShadow* cascade)-> void
	{
		reloadLightingShader(cascade);
	});
}

nex::Pbr::~Pbr() = default;

nex::AmbientLight* nex::Pbr::getAmbientLight()
{
	return mAmbientLight;
}

nex::CascadedShadow* nex::Pbr::getCascadedShadow()
{
	return mCascadeShadow;
}

void nex::Pbr::setCascadedShadow(CascadedShadow* shadow)
{
	mCascadeShadow = shadow;
}

nex::DirectionalLight* nex::Pbr::getDirLight()
{
	return mLight;
}

nex::PbrProbe* nex::Pbr::getProbe()
{
	return mProbe;
}

void nex::Pbr::setAmbientLight(AmbientLight* light)
{
	mAmbientLight = light;
}

void nex::Pbr::setDirLight(DirectionalLight* light)
{
	mLight = light;
}

void nex::Pbr::setProbe(PbrProbe* probe)
{
	mProbe = probe;
}

nex::PbrTechnique::PbrTechnique(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight,
	PbrProbe* probe) :
	Technique(nullptr),
	mDeferred(std::make_unique<PbrDeferred>(ambientLight, cascadeShadow, dirLight, probe)),
	mForward(std::make_unique<PbrForward>(ambientLight, cascadeShadow, dirLight, probe))
{
	useDeferred();
}

void nex::PbrTechnique::setProbe(PbrProbe* probe)
{
	mDeferred->setProbe(probe);
	mForward->setProbe(probe);
}

nex::PbrTechnique::~PbrTechnique() = default;

void nex::PbrTechnique::useDeferred()
{
	mDeferredUsed = true;
	setSelected(mDeferred->getGeometryPass());
}

void nex::PbrTechnique::useForward()
{
	mDeferredUsed = false;
	setSelected(mForward->getPass());
}

nex::PbrDeferred* nex::PbrTechnique::getDeferred()
{
	return mDeferred.get();
}

nex::PbrForward* nex::PbrTechnique::getForward()
{
	return mForward.get();
}

nex::Pbr* nex::PbrTechnique::getActive()
{
	if (mDeferredUsed) return mDeferred.get();
	return mForward.get();
}

nex::PbrGeometryPass * nex::PbrTechnique::getActiveGeometryPass()
{
	return (nex::PbrGeometryPass *)getSelected();
}

nex::Pbr_ConfigurationView::Pbr_ConfigurationView(PbrTechnique* pbr) : mPbr(pbr)
{
}

void nex::Pbr_ConfigurationView::drawSelf()
{
	ImGui::PushID(m_id.c_str());
	ImGui::LabelText("", "PBR:");


	auto* active = mPbr->getDeferred();

	auto* dirLight = active->getDirLight();

	glm::vec3 lightColor = dirLight->getColor();
	float dirLightPower = dirLight->getLightPower();


	if (ImGui::InputScalarN("Directional Light Color", ImGuiDataType_Float, &lightColor, 3))
	{
		lightColor = clamp(lightColor, glm::vec3(0), glm::vec3(1));
		dirLight->setColor(lightColor);
	}

	if (ImGui::DragFloat("Directional Light Power", &dirLightPower, 0.1f, 0.0f, 10.0f))
	{
		dirLight->setPower(dirLightPower);
	}

	drawLightSphericalDirection();


	float ambientLightPower = active->getAmbientLight()->getPower();

	if (ImGui::DragFloat("Amblient Light Power", &ambientLightPower, 0.1f, 0.0f, 10.0f))
	{
		active->getAmbientLight()->setPower(ambientLightPower);
	}


	nex::gui::Separator(2.0f);
	ImGui::PopID();
}

void nex::Pbr_ConfigurationView::drawLightSphericalDirection()
{
	auto* active = mPbr->getDeferred();
	auto* dirLight = active->getDirLight();
	glm::vec3 lightDirection = normalize(dirLight->getDirection());

	static SphericalCoordinate sphericalCoordinate = SphericalCoordinate::convert(-lightDirection);

	float temp[2] = { sphericalCoordinate.polar, sphericalCoordinate.azimuth };

	if (ImGui::DragFloat2("Light position (spherical coordinates)", temp, 0.05f))
	{
		sphericalCoordinate.polar = temp[0];
		sphericalCoordinate.azimuth = temp[1];
		lightDirection = -SphericalCoordinate::cartesian(sphericalCoordinate);
		lightDirection = clamp(lightDirection, glm::vec3(-1), glm::vec3(1));
		dirLight->setDirection(lightDirection);
	}
}
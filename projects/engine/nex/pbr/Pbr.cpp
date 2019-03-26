#include <nex/pbr/Pbr.hpp>
#include <nex/light/Light.hpp>
#include "imgui/imgui.h"
#include "nex/gui/Util.hpp"
#include "nex/shadow/CascadedShadow.hpp"

nex::Pbr::Pbr(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe) :
mAmbientLight(ambientLight), mCascadeShadow(cascadeShadow), mLight(dirLight), mProbe(probe)
{
	mCascadeShadow->addCascadeChangeCallback([&](const CascadedShadow& cascade)->void
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

nex::Pbr_ConfigurationView::Pbr_ConfigurationView(Pbr* pbr) : mPbr(pbr)
{
}

void nex::Pbr_ConfigurationView::drawSelf()
{
	ImGui::PushID(m_id.c_str());
	ImGui::LabelText("", "PBR:");


	auto* dirLight = mPbr->getDirLight();

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


	float ambientLightPower = mPbr->getAmbientLight()->getPower();

	if (ImGui::DragFloat("Amblient Light Power", &ambientLightPower, 0.1f, 0.0f, 10.0f))
	{
		mPbr->getAmbientLight()->setPower(ambientLightPower);
	}


	nex::gui::Separator(2.0f);
	ImGui::PopID();
}

void nex::Pbr_ConfigurationView::drawLightSphericalDirection()
{
	auto* dirLight = mPbr->getDirLight();
	glm::vec3 lightDirection = normalize(dirLight->getDirection());

	static SphericalCoordinate sphericalCoordinate = SphericalCoordinate::convert(-lightDirection);

	float temp[2] = { sphericalCoordinate.polar, sphericalCoordinate.azimuth };

	if (ImGui::DragFloat2("Light position (spherical coordinates)", temp, 0.05f))
	{
		//temp = clamp(temp, glm::vec2(-1.0f), glm::vec2(1.0f));
		//temp[0] = std::clamp<float>(temp[0], 0, M_PI);
		//temp[0] = std::clamp<float>(temp[0], -2 * M_PI, 2 * M_PI);
		//temp[1] = std::clamp<float>(temp[1], -2*M_PI, 2*M_PI);
		sphericalCoordinate.polar = temp[0];
		sphericalCoordinate.azimuth = temp[1];
		lightDirection = -SphericalCoordinate::cartesian(sphericalCoordinate);
		lightDirection = clamp(lightDirection, glm::vec3(-1), glm::vec3(1));
		dirLight->setDirection(lightDirection);
	}
}
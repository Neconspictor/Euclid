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
#include <nex/pbr/GlobalIllumination.hpp>

nex::Pbr::Pbr(GlobalIllumination* globalIllumination,
	CascadedShadow* cascadedShadow, DirLight* dirLight) :
	mCascadedShadow(cascadedShadow), mLight(dirLight), mGlobalIllumination(globalIllumination)
{
	setCascadedShadow(cascadedShadow);
}

nex::Pbr::~Pbr() = default;

nex::CascadedShadow* nex::Pbr::getCascadedShadow()
{
	return mCascadedShadow;
}

void nex::Pbr::setCascadedShadow(CascadedShadow* shadow)
{
	mCascadedShadow = shadow;

	if (mCascadedShadow) {
		mCascadedShadow->addCascadeChangeCallback([&](CascadedShadow* cascade)-> void
		{
			setCascadedShadow(cascade);
			reloadLightingShaders();
		});
	}
}

void nex::Pbr::setGI(GlobalIllumination* gi)
{
	mGlobalIllumination = gi;
}

nex::DirLight* nex::Pbr::getDirLight()
{
	return mLight;
}

nex::GlobalIllumination* nex::Pbr::getGlobalIllumination()
{
	return mGlobalIllumination;
}

void nex::Pbr::setDirLight(DirLight* light)
{
	mLight = light;
}

nex::PbrTechnique::PbrTechnique(
	GlobalIllumination* globalIllumination,
	CascadedShadow* cascadeShadow, DirLight* dirLight) :
	Technique(nullptr),
	mOverrideForward(nullptr),
	mOverrideDeferred(nullptr)
{

	auto deferredGeometryPass = std::make_unique<PbrDeferredGeometryPass>(Shader::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl" , 
		"pbr/pbr_deferred_geometry_pass_fs.glsl"));


	PbrDeferred::LightingPassFactory deferredFactory = [](CascadedShadow* c, GlobalIllumination* g) {
		return std::make_unique<PbrDeferredLightingPass>(
			"screen_space_vs.glsl",
			"pbr/pbr_deferred_lighting_pass_fs.glsl",
			g,
			c);
	};

	PbrForward::LightingPassFactory forwardFactory = [](CascadedShadow* c, GlobalIllumination* g) {
		return std::make_unique<PbrForwardPass>(
			"pbr/pbr_forward_vs.glsl", 
			"pbr/pbr_forward_fs.glsl",
			g,
			c);
	};

	mDeferred = std::make_unique<PbrDeferred>(std::move(deferredGeometryPass), 
		std::move(deferredFactory), 
		globalIllumination, 
		cascadeShadow, 
		dirLight);

	mForward = std::make_unique<PbrForward>(std::move(forwardFactory),
		globalIllumination,
		cascadeShadow,
		dirLight);

	useDeferred();
}

nex::PbrTechnique::~PbrTechnique() = default;

void nex::PbrTechnique::useDeferred()
{
	mDeferredUsed = true;
	auto* active = mOverrideDeferred ? mOverrideDeferred : mDeferred.get();
	setSelected(active->getGeometryPass());
}

void nex::PbrTechnique::useForward()
{
	mDeferredUsed = false;
	auto* active = mOverrideForward ? mOverrideForward : mForward.get();
	setSelected(active->getPass());
}

nex::PbrDeferred* nex::PbrTechnique::getDeferred()
{
	auto* active = mOverrideDeferred ? mOverrideDeferred : mDeferred.get();
	return active;
}

nex::PbrForward* nex::PbrTechnique::getForward()
{
	auto* active = mOverrideForward ? mOverrideForward : mForward.get();
	return active;
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

void nex::PbrTechnique::overrideForward(PbrForward * forward)
{
	mOverrideForward = forward;
	if (!mDeferredUsed) useForward();
}

void nex::PbrTechnique::overrideDeferred(PbrDeferred * deferred)
{
	mOverrideDeferred = deferred;
	if (mDeferredUsed) useDeferred();
}

void nex::PbrTechnique::setGI(GlobalIllumination* globalIllumination)
{
	mForward->setGI(globalIllumination);
	mDeferred->setGI(globalIllumination);
}

void nex::PbrTechnique::setShadow(CascadedShadow* cascadeShadow)
{
	mForward->setCascadedShadow(cascadeShadow);
	mDeferred->setCascadedShadow(cascadeShadow);
}

void nex::PbrTechnique::setDirLight(DirLight* dirLight)
{
	mForward->setDirLight(dirLight);
	mDeferred->setDirLight(dirLight);
}

void nex::PbrTechnique::updateShaders()
{
	mForward->reloadLightingShaders();
	mDeferred->reloadLightingShaders();
}

nex::Pbr_ConfigurationView::Pbr_ConfigurationView(PbrTechnique* pbr) : mPbr(pbr)
{
}

void nex::Pbr_ConfigurationView::drawSelf()
{
	ImGui::PushID(mId.c_str());
	ImGui::LabelText("", "PBR:");


	auto* active = mPbr->getDeferred();

	auto* dirLight = active->getDirLight();

	glm::vec3 lightColor = dirLight->color;
	float dirLightPower = dirLight->power;


	if (ImGui::InputScalarN("Directional Light Color", ImGuiDataType_Float, &lightColor, 3))
	{
		lightColor = clamp(lightColor, glm::vec3(0), glm::vec3(1));
		dirLight->color = lightColor;
		dirLight->_pad[0] = 1.0f;
	}

	if (ImGui::DragFloat("Directional Light Power", &dirLightPower, 0.1f, 0.0f, 10.0f))
	{
		dirLight->power = dirLightPower;
		dirLight->_pad[0] = 1.0f;
	}

	drawLightSphericalDirection();


	float ambientLightPower = active->getGlobalIllumination()->getAmbientPower();

	if (ImGui::DragFloat("Ambient Light Power", &ambientLightPower, 0.1f, 0.0f, 10.0f))
	{
		active->getGlobalIllumination()->setAmbientPower(ambientLightPower);
	}


	nex::gui::Separator(2.0f);
	ImGui::PopID();
}

void nex::Pbr_ConfigurationView::drawLightSphericalDirection()
{
	auto* active = mPbr->getDeferred();
	auto* dirLight = active->getDirLight();
	glm::vec3 lightDirection = dirLight->directionWorld;

	static SphericalCoordinate sphericalCoordinate = SphericalCoordinate::convert(lightDirection);
	glm::vec3 dirTest = SphericalCoordinate::cartesian(sphericalCoordinate);
	dirTest = normalize(dirTest);

	float temp[2] = { sphericalCoordinate.polar, sphericalCoordinate.azimuth };

	if (ImGui::DragFloat2("Light position (spherical coordinates)", temp, 0.005f))
	{
		sphericalCoordinate.polar = temp[0];
		sphericalCoordinate.azimuth = temp[1];
		lightDirection = SphericalCoordinate::cartesian(sphericalCoordinate);
		lightDirection = normalize(lightDirection);
		dirLight->directionWorld = lightDirection;
		dirLight->_pad[0] = 1.0f;
	}
}
#include <nex/pbr/Pbr.hpp>
#include <nex/light/Light.hpp>
#include "imgui/imgui.h"
#include "nex/gui/ImGUI_Extension.hpp"
#include "nex/shadow/CascadedShadow.hpp"
#include "PbrDeferred.hpp"
#include "PbrForward.hpp"
#include <nex/shader/Shader.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <nex/GI/Probe.hpp>
#include <nex/GI/GlobalIllumination.hpp>

nex::Pbr::Pbr(GlobalIllumination* globalIllumination,
	CascadedShadow* cascadedShadow, const DirLight* dirLight) :
	mCascadedShadow(nullptr), mLight(dirLight), mGlobalIllumination(globalIllumination)
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
		mCascadeChangedHandle = mCascadedShadow->addChangedCallback([&](CascadedShadow* cascade)-> void
		{
			reloadLightingShaders();
		});
	}
}

void nex::Pbr::setGI(GlobalIllumination* gi)
{
	mGlobalIllumination = gi;
}

nex::GlobalIllumination* nex::Pbr::getGlobalIllumination()
{
	return mGlobalIllumination;
}

void nex::Pbr::setDirLight(const DirLight* light)
{
	mLight = light;
}

nex::PbrTechnique::PbrTechnique(
	GlobalIllumination* globalIllumination,
	CascadedShadow* cascadeShadow, DirLight* dirLight) :
	mOverrideForward(nullptr),
	mOverrideDeferred(nullptr)
{

	auto deferredGeometryPass = std::make_unique<PbrDeferredGeometryShader>(ShaderProgram::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl" , 
		"pbr/pbr_deferred_geometry_pass_fs.glsl"));

	auto deferredGeometryBonesPass = std::make_unique<PbrDeferredGeometryBonesShader>(ShaderProgram::create(
		"pbr/pbr_deferred_geometry_pass_vs.glsl",
		"pbr/pbr_deferred_geometry_pass_fs.glsl",
		nullptr,
		nullptr,
		nullptr,
		{ "#define BONE_ANIMATION 1", "#define BONE_ANIMATION_TRAFOS_BINDING_POINT 1" }));


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
		std::move(deferredGeometryBonesPass),
		std::move(deferredFactory), 
		globalIllumination, 
		cascadeShadow, 
		dirLight);

	mForward = std::make_unique<PbrForward>(std::move(forwardFactory),
		globalIllumination,
		cascadeShadow,
		dirLight);
}

nex::PbrTechnique::~PbrTechnique() = default;

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

void nex::PbrTechnique::overrideForward(PbrForward * forward)
{
	mOverrideForward = forward;
}

void nex::PbrTechnique::overrideDeferred(PbrDeferred * deferred)
{
	mOverrideDeferred = deferred;
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

nex::Pbr_ConfigurationView::Pbr_ConfigurationView(PbrTechnique* pbr, DirLight* light) : mPbr(pbr), mLight(light)
{
}

void nex::Pbr_ConfigurationView::drawSelf()
{
	ImGui::PushID(mId.c_str());
	ImGui::LabelText("", "PBR:");


	auto* active = mPbr->getDeferred();

	glm::vec3 lightColor = mLight->color;
	float dirLightPower = mLight->power;


	if (ImGui::InputScalarN("Directional Light Color", ImGuiDataType_Float, &lightColor, 3))
	{
		lightColor = clamp(lightColor, glm::vec3(0), glm::vec3(1));
		mLight->color = lightColor;
		mLight->_pad[0] = 1.0f;
	}

	if (ImGui::DragFloat("Directional Light Power", &dirLightPower, 0.1f, 0.0f, 10.0f))
	{
		mLight->power = dirLightPower;
		mLight->_pad[0] = 1.0f;
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
	glm::vec3 lightDirection = mLight->directionWorld;

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
		mLight->directionWorld = lightDirection;
		mLight->_pad[0] = 1.0f;
	}
}
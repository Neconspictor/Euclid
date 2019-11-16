#include <nex/sky/AtmosphericScattering.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/mesh/MeshManager.hpp>
#include "nex/drawing/MeshDrawer.hpp"

nex::AtmosphericScattering::AtmosphericScattering() : Shader(
	ShaderProgram::create("atmospheric_scattering/atmosphericScattering_vs.glsl", "atmospheric_scattering/atmosphericScattering_fs.glsl"))
{

	mViewPortUniform = { mProgram->getUniformLocation("viewport"), UniformType::VEC2 };
	mInvProjUniform = { mProgram->getUniformLocation("inv_proj"), UniformType::MAT4 };
	mInvViewRotUniform = { mProgram->getUniformLocation("inv_view_rot"), UniformType::MAT3 };
	mLightDirUniform = { mProgram->getUniformLocation("lightdir"), UniformType::VEC3 };
	
	mRayleighBrightnessUniform = { mProgram->getUniformLocation("rayleigh_brightness"), UniformType::FLOAT };
	mMieBrightnessUniform = { mProgram->getUniformLocation("mie_brightness"), UniformType::FLOAT };
	mMieDistributionUniform = { mProgram->getUniformLocation("mie_distribution"), UniformType::FLOAT };
	mSpotBrightnessUniform = { mProgram->getUniformLocation("spot_brightness"), UniformType::FLOAT };
	
	mSurfaceHeightUniform = { mProgram->getUniformLocation("surface_height"), UniformType::FLOAT };
	mStepCountUniform = { mProgram->getUniformLocation("step_count"), UniformType::UINT };
	mIntensityUniform = { mProgram->getUniformLocation("intensity"), UniformType::FLOAT };
	
	mScatterStrengthUniform = { mProgram->getUniformLocation("scatter_strength"), UniformType::FLOAT };
	mRayleighCollectionPowerUniform = { mProgram->getUniformLocation("rayleigh_collection_power"), UniformType::FLOAT };
	mMieCollectionPowerUniform = { mProgram->getUniformLocation("mie_collection_power"), UniformType::FLOAT };

	mRayleighStrengthUniform = { mProgram->getUniformLocation("rayleigh_strength"), UniformType::FLOAT };
	mMieStrengthUniform = { mProgram->getUniformLocation("mie_strength"), UniformType::FLOAT };

	mInvViewProjUniform = { mProgram->getUniformLocation("invViewProj"), UniformType::MAT4 };
	mPrevViewProjUniform = { mProgram->getUniformLocation("prevViewProj"), UniformType::MAT4 };

}

void nex::AtmosphericScattering::renderSky()
{
	const auto& state = RenderState::getNoDepthTest();
	RenderBackend::get()->drawArray(state, Topology::TRIANGLE_STRIP, 0, 4);
	MeshDrawer::drawFullscreenTriangle(state, this);
}

void nex::AtmosphericScattering::setRayleigh(const Rayleigh& rayleigh)
{
	mProgram->setFloat(mRayleighBrightnessUniform.location, rayleigh.brightness);
	mProgram->setFloat(mRayleighCollectionPowerUniform.location, rayleigh.collectionPower);
	mProgram->setFloat(mRayleighStrengthUniform.location, rayleigh.strength);
}

void nex::AtmosphericScattering::setMie(const Mie& mie)
{
	mProgram->setFloat(mMieBrightnessUniform.location, mie.brightness);
	mProgram->setFloat(mMieCollectionPowerUniform.location, mie.collectionPower);
	mProgram->setFloat(mMieDistributionUniform.location, mie.distribution);
	mProgram->setFloat(mMieStrengthUniform.location, mie.strength);
}

void nex::AtmosphericScattering::setViewport(unsigned width, unsigned height)
{
	mProgram->setVec2(mViewPortUniform.location, glm::vec2(width, height));
}

void nex::AtmosphericScattering::setInverseProjection(const glm::mat4& mat)
{
	mProgram->setMat4(mInvProjUniform.location, mat);
}

void nex::AtmosphericScattering::setInverseViewRotation(const glm::mat3& mat)
{
	mProgram->setMat3(mInvViewRotUniform.location, mat);
}

void nex::AtmosphericScattering::setSpotBrightness(float brightness)
{
	mProgram->setFloat(mSpotBrightnessUniform.location, brightness);
}

void nex::AtmosphericScattering::setSurfaceHeight(float height)
{
	mProgram->setFloat(mSurfaceHeightUniform.location, height);
}

void nex::AtmosphericScattering::setStepCount(unsigned count)
{
	mProgram->setUInt(mStepCountUniform.location, count);
}

void nex::AtmosphericScattering::setLight(const Light& light)
{
	mProgram->setVec3(mLightDirUniform.location, light.direction);
	mProgram->setFloat(mIntensityUniform.location, light.intensity);
}

void nex::AtmosphericScattering::setScatterStrength(float strength)
{
	mProgram->setFloat(mScatterStrengthUniform.location, strength);
}

void nex::AtmosphericScattering::setPrevViewProj(const glm::mat4& mat)
{
	mProgram->setMat4(mPrevViewProjUniform.location, mat);
}

void nex::AtmosphericScattering::setInvViewProj(const glm::mat4& mat)
{
	mProgram->setMat4(mInvViewProjUniform.location, mat);
}

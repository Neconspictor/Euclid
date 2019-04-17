#include <nex/sky/AtmosphericScattering.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/drawing/StaticMeshDrawer.hpp"

nex::AtmosphericScattering::AtmosphericScattering() : Pass(
	Shader::create("atmospheric_scattering/atmosphericScattering_vs.glsl", "atmospheric_scattering/atmosphericScattering_fs.glsl"))
{

	mViewPortUniform = { mShader->getUniformLocation("viewport"), UniformType::VEC2 };
	mInvProjUniform = { mShader->getUniformLocation("inv_proj"), UniformType::MAT4 };
	mInvViewRotUniform = { mShader->getUniformLocation("inv_view_rot"), UniformType::MAT3 };
	mLightDirUniform = { mShader->getUniformLocation("lightdir"), UniformType::VEC3 };
	
	mRayleighBrightnessUniform = { mShader->getUniformLocation("rayleigh_brightness"), UniformType::FLOAT };
	mMieBrightnessUniform = { mShader->getUniformLocation("mie_brightness"), UniformType::FLOAT };
	mMieDistributionUniform = { mShader->getUniformLocation("mie_distribution"), UniformType::FLOAT };
	mSpotBrightnessUniform = { mShader->getUniformLocation("spot_brightness"), UniformType::FLOAT };
	
	mSurfaceHeightUniform = { mShader->getUniformLocation("surface_height"), UniformType::FLOAT };
	mStepCountUniform = { mShader->getUniformLocation("step_count"), UniformType::UINT };
	mIntensityUniform = { mShader->getUniformLocation("intensity"), UniformType::FLOAT };
	
	mScatterStrengthUniform = { mShader->getUniformLocation("scatter_strength"), UniformType::FLOAT };
	mRayleighCollectionPowerUniform = { mShader->getUniformLocation("rayleigh_collection_power"), UniformType::FLOAT };
	mMieCollectionPowerUniform = { mShader->getUniformLocation("mie_collection_power"), UniformType::FLOAT };

	mRayleighStrengthUniform = { mShader->getUniformLocation("rayleigh_strength"), UniformType::FLOAT };
	mMieStrengthUniform = { mShader->getUniformLocation("mie_strength"), UniformType::FLOAT };
}

void nex::AtmosphericScattering::renderSky()
{
	RenderState state = RenderState::createNoDepthTest();
	RenderBackend::get()->drawArray(state, Topology::TRIANGLE_STRIP, 0, 4);
	StaticMeshDrawer::drawFullscreenTriangle(state, this);
}

void nex::AtmosphericScattering::setRayleigh(const Rayleigh& rayleigh)
{
	mShader->setFloat(mRayleighBrightnessUniform.location, rayleigh.brightness);
	mShader->setFloat(mRayleighCollectionPowerUniform.location, rayleigh.collectionPower);
	mShader->setFloat(mRayleighStrengthUniform.location, rayleigh.strength);
}

void nex::AtmosphericScattering::setMie(const Mie& mie)
{
	mShader->setFloat(mMieBrightnessUniform.location, mie.brightness);
	mShader->setFloat(mMieCollectionPowerUniform.location, mie.collectionPower);
	mShader->setFloat(mMieDistributionUniform.location, mie.distribution);
	mShader->setFloat(mMieStrengthUniform.location, mie.strength);
}

void nex::AtmosphericScattering::setViewport(unsigned width, unsigned height)
{
	mShader->setVec2(mViewPortUniform.location, glm::vec2(width, height));
}

void nex::AtmosphericScattering::setInverseProjection(const glm::mat4& mat)
{
	mShader->setMat4(mInvProjUniform.location, mat);
}

void nex::AtmosphericScattering::setInverseViewRotation(const glm::mat3& mat)
{
	mShader->setMat3(mInvViewRotUniform.location, mat);
}

void nex::AtmosphericScattering::setSpotBrightness(float brightness)
{
	mShader->setFloat(mSpotBrightnessUniform.location, brightness);
}

void nex::AtmosphericScattering::setSurfaceHeight(float height)
{
	mShader->setFloat(mSurfaceHeightUniform.location, height);
}

void nex::AtmosphericScattering::setStepCount(unsigned count)
{
	mShader->setUInt(mStepCountUniform.location, count);
}

void nex::AtmosphericScattering::setLight(const Light& light)
{
	mShader->setVec3(mLightDirUniform.location, light.direction);
	mShader->setFloat(mIntensityUniform.location, light.intensity);
}

void nex::AtmosphericScattering::setScatterStrength(float strength)
{
	mShader->setFloat(mScatterStrengthUniform.location, strength);
}
#pragma once
#include <glm/glm.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/mesh/VertexArray.hpp>

namespace nex
{
	class AtmosphericScattering : public Pass
	{
	public:

		struct Rayleigh
		{
			Real brightness;
			Real collectionPower;
			Real strength;
		};

		struct Mie
		{
			Real brightness;
			Real distribution;
			Real collectionPower;
			Real strength;
		};

		struct Light
		{
			// a normalized direction vector
			glm::vec3 direction;
			Real intensity;
		};

		AtmosphericScattering();

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void renderSky();

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setRayleigh(const Rayleigh& rayleigh);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setMie(const Mie& mie);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setViewport(unsigned width, unsigned height);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setInverseProjection(const glm::mat4& mat);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setInverseViewRotation(const glm::mat3& mat);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setSpotBrightness(Real brightness);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 * @param height : in range [0.15, 1.0]
		 */
		void setSurfaceHeight(Real height);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setStepCount(unsigned count);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setLight(const Light& light);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setScatterStrength(Real strength);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setPrevViewProj(const glm::mat4& mat);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 */
		void setInvViewProj(const glm::mat4& mat);

	private:
		/*glm::vec2 mViewPort;
		glm::mat4 mInvProj;
		glm::mat4 mInvViewRot;
		Rayleigh mRayleigh;
		Mie mMie;
		Light mLight;
		Real mSpotBrightness;
		Real mSurfaceHeight;
		unsigned mStepCount;
		Real mScatterStrength;*/

		Uniform mViewPortUniform;
		Uniform mInvProjUniform;
		Uniform mInvViewRotUniform;
		Uniform mLightDirUniform;
		Uniform mRayleighBrightnessUniform;
		Uniform mMieBrightnessUniform;
		Uniform mMieDistributionUniform;
		Uniform mSpotBrightnessUniform;
		Uniform mSurfaceHeightUniform;
		Uniform mStepCountUniform;
		Uniform mIntensityUniform;
		Uniform mScatterStrengthUniform;
		Uniform mRayleighCollectionPowerUniform;
		Uniform mMieCollectionPowerUniform;
		Uniform mRayleighStrengthUniform;
		Uniform mMieStrengthUniform;

		Uniform mInvViewProjUniform;
		Uniform mPrevViewProjUniform;
	};
}
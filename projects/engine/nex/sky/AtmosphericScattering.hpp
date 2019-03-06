#pragma once
#include <glm/glm.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/mesh/VertexArray.hpp>

namespace nex
{
	class AtmosphericScattering : public Shader
	{
	public:

		struct Rayleigh
		{
			float brightness;
			float collectionPower;
			float strength;
		};

		struct Mie
		{
			float brightness;
			float distribution;
			float collectionPower;
			float strength;
		};

		struct Light
		{
			// a normalized direction vector
			glm::vec3 direction;
			float intensity;
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
		void setSpotBrightness(float brightness);

		/**
		 * NOTE: This shader has to be bound for this function to work correctly!
		 * @param height : in range [0.15, 1.0]
		 */
		void setSurfaceHeight(float height);

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
		void setScatterStrength(float strength);
		 
	private:
		/*glm::vec2 mViewPort;
		glm::mat4 mInvProj;
		glm::mat4 mInvViewRot;
		Rayleigh mRayleigh;
		Mie mMie;
		Light mLight;
		float mSpotBrightness;
		float mSurfaceHeight;
		unsigned mStepCount;
		float mScatterStrength;*/

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

		VertexArray mFullscreenTriangleStrip;
	};
}
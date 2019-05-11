#pragma once
#include <nex/util/Complex.hpp>
#include "nex/shader/Pass.hpp"
#include "nex/gui/Drawable.hpp"

namespace nex
{
	class Mesh;
	class Camera;

	class Ocean
	{
	public:

		/**
		 * A vertex structure containing useful data when generating and updating the water surface.
		 */
		struct VertexCompute
		{
			glm::vec3 originalPosition;
			Complex height0;
			Complex height0NegativeWaveConjugate;
		};

		/**
		 * A vertex structure containing data for rendering the water
		 */
		struct VertexRender
		{
			glm::vec3 position;
			glm::vec3 normal;
		};

		/**
		 * Creates a new Ocean object that is tileable. This means, that the first row and the first column are mirrored, so that
		 * The last row and last column match the first row resp. column.
		 *
		 * @param pointCount : Amount of points. Has to be >= 2. Note that the last row and last column will be a replicate of the first row resp. column for tiling reasons.
		 * Thus there will be (pointCount.x * pointCount.y) unique points. For performance reasons pointCount.x and pointCount.y should be a power of 2.
		 * @param maxWaveLength : The maximum extension of a wave in the x-z plane
		 * @param dimension : The dimension of a tile (in object space)
		 * @param spectrumScale : Scales the used philip spectrum for wave generation. Has to be > 0.
		 * @param windDirection : The direction of wind. Its length has to be > 0
		 * @param windSpeed : The speed of wind.
		 * @param periodTime : The time of one period (in seconds). After that, the simulation gets repeated. Has to be > 0.
		 */
		Ocean(const glm::uvec2& pointCount,
			const glm::vec2& maxWaveLength,
			const glm::vec2& dimension,
			float spectrumScale,
			const glm::vec2& windDirection,
			float windSpeed,
			float periodTime);

		~Ocean();

		/**
		 * Computes the height of a location on the (x,z) plane at a specific time.
		 */
		float computeHeight(const glm::vec2& locationXZ, float time) const;

		float dispersion(const glm::vec2& wave) const;

		void draw(Camera* camera, const glm::vec3& lightDir);

		Complex heightTildeZero(const glm::vec2& wave) const;
		Complex heightTilde(const glm::vec2& wave, float time) const;
		float philipsSpectrum(const glm::vec2& wave) const;

		bool* getWireframeState();

		/**
		 * Simulates ocean state at time t.
		 * @param t : time. Has to be > 0
		 */
		void simulate(float t);

	private:


		class SimpleShadedPass : public Pass
		{
		public:
			SimpleShadedPass();

			void setUniforms(Camera* camera, const glm::mat4& trafo, const glm::vec3& lightDir);

			Uniform transform;
			Uniform lightUniform;
			Uniform normalMatrixUniform;
		};


		/**
		 * Amount of unique points in the x-z plane.
		 */
		glm::uvec2 mUniquePointCount;

		/**
		 * The totoal amount of points in the x-z plane.
		 * Note: This matches mUniquePointCount + (1,1).
		 */
		glm::uvec2 mTildePointCount;

		/**
		 * max extension of a wave in the x-z plane.
		 */
		glm::vec2 mWaveLength;

		/**
		 * Dimension of the ocean tile (in object space)
		 */
		glm::vec2 mDimension;


		/**
		 * A scaling factor for the Philip spectrum
		 */
		float mSpectrumScale;

		/**
		 * wind direction on the x-z plane
		 */
		glm::vec2 mWindDirection;

		/**
		 * The speed of wind
		 */
		float mWindSpeed;

		/**
		 * The length of the simulation period.
		 */
		float mPeriodTime;

		std::vector<VertexCompute> mVerticesCompute;
		std::vector<VertexRender> mVerticesRender;
		std::vector<unsigned> mIndices;
		std::unique_ptr<Mesh> mMesh;
		std::unique_ptr<SimpleShadedPass> mSimpleShadedPass;
		bool mWireframe;


		static constexpr float GRAVITY = 9.81f;

		static float generateGaussianRand();
	};


	namespace gui
	{
		class OceanConfig : public nex::gui::Drawable
		{
		public:
			OceanConfig(Ocean* ocean);

		protected:
			void drawSelf() override;

			Ocean* mOcean;
		};
	}
}

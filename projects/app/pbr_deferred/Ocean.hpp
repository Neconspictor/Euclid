#pragma once
#include <nex/util/Complex.hpp>
#include "nex/shader/Pass.hpp"
#include "nex/gui/Drawable.hpp"
#include "nex/util/Math.hpp"

namespace nex
{
	class Mesh;
	class Camera;
	class Texture2D;

	class Iterator2D
	{
	public:

		enum class PrimitiveMode
		{
			ROWS,
			COLUMNS
		};

		/**
		 * @param vec : The vector used for iterating
		 * @param mode : The mode of iteration
		 * @param primitiveIndex : Specifies the primitive index to use for iteration (e.g. the 3rd column).
		 * @param elementNumber : The number of elements that form a primitive.
		 *
		 * @throws std::out_of_range : If the specified primitive cannot be mapped by the vec argument.
		 */
		Iterator2D(std::vector<nex::Complex>& vec,
			const PrimitiveMode mode,
			const size_t primitiveIndex,
			const size_t elementNumber);

		/**
		 * Provides access to the ith element of the current mode primitive.
		 * @throws std::out_of_range : if the index is negative or equal or greater than the primitive mode
		 * element number.
		 */
		nex::Complex& operator[](const size_t index);
		nex::Complex& operator[](const size_t index) const;

	private:
		std::vector<nex::Complex>* mVec;
		PrimitiveMode mMode;
		size_t mPrimitiveIndex;
		size_t mCount;

		size_t getVectorIndex(const size_t primitiveIndex) const;
		bool isInRange(const size_t vectorIndex) const;
	};


	class OceanFFT
	{
	public:
		OceanFFT(unsigned N);

		unsigned reverse(unsigned i) const;
		nex::Complex twiddle(unsigned x, unsigned N);

		void fft(nex::Complex* input, nex::Complex* output, int stride, int offset, bool vertical);

		void fft(const nex::Iterator2D&  input, nex::Iterator2D&  output, bool vertical);

		void fftInPlace(std::vector<nex::Complex>& x, bool inverse);

	private:

		unsigned N;
		unsigned which;
		unsigned log_2_N;
		static constexpr float pi2 = 2 * nex::util::PI;
		std::vector<unsigned> reversed;
		std::vector<std::vector<nex::Complex>> T;
		std::vector<nex::Complex> c [2];
	};

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
		 * Holds all simulation result data for a point on the x-z plane.
		 */
		struct ResultData
		{
			Complex height; // The resulting height of the point
			glm::vec2 displacement; // A displacement vector on the x-z plane for choppy waves
			glm::vec3 normal; // The normal vector of the point
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
		ResultData simulatePoint(const glm::vec2& locationXZ, float time) const;

		float dispersion(const glm::vec2& wave) const;

		void draw(Camera* camera, const glm::vec3& lightDir);

		Complex heightZero(const glm::vec2& wave) const;
		Complex height(int x, int z, float time) const;
		float philipsSpectrum(const glm::vec2& wave) const;

		bool* getWireframeState();

		/**
		 * Simulates ocean state at time t.
		 * @param t : time. Has to be > 0
		 */
		void simulate(float t);

		void simulateFFT(float t);

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

		class HeightZeroComputePass : public ComputePass
		{
		public:
			HeightZeroComputePass(const glm::uvec2& uniquePointCount, const glm::vec2& waveLength, const glm::vec2& windDirection,
				float spectrumScale, float windSpeed);


			void compute();
			Texture2D* getResult();

		private:

			Uniform mWindSpeedUniform;
			Uniform mWindDirectionUniform;
			Uniform mSpectrumScaleUniform;
			Uniform mUniquePointCountUniform;
			Uniform mWaveLengthUniform;
			UniformTex mResultTexture;
			UniformTex mRandTextureUniform;
			std::unique_ptr<Texture2D> mHeightZero;
			std::unique_ptr<Texture2D> mRandNormalDistributed;
			glm::uvec2 mUniquePointCount;
			glm::vec2 mWaveLength;
			glm::vec2 mWindDirection;
			float mSpectrumScale;
			float mWindSpeed;
		};


		class HeightComputePass : public ComputePass
		{
		public:
			HeightComputePass(const glm::uvec2& uniquePointCount, const glm::vec2& waveLength, float periodTime);


			void compute(float time, Texture2D* heightZero);

			Texture2D* getHeight();
			Texture2D* getSlopeX();
			Texture2D* getSlopeZ();
			Texture2D* getDx();
			Texture2D* getDz();

		private:

			Uniform mPeriodTimeUniform;
			Uniform mUniquePointCountUniform;
			glm::uvec2 mUniquePointCount;
			Uniform mWaveLengthUniform;
			UniformTex mHeightZeroTextureUniform;
			Sampler mHeightZeroSampler;
			UniformTex mResultHeightTextureUniform;
			UniformTex mResultSlopeXTextureUniform;
			UniformTex mResultSlopeZTextureUniform;
			UniformTex mResultDxTextureUniform;
			UniformTex mResultDzTextureUniform;
			std::unique_ptr<Texture2D> mHeight;
			std::unique_ptr<Texture2D> mHeightSlopeX;
			std::unique_ptr<Texture2D> mHeightSlopeZ;
			std::unique_ptr<Texture2D> mHeightDx;
			std::unique_ptr<Texture2D> mHeightDz;

			Uniform mTimeUniform;
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
		std::unique_ptr<HeightZeroComputePass> mHeightZeroComputePass;
		std::unique_ptr<HeightComputePass> mHeightComputePass;
		bool mWireframe;


		unsigned N;
		std::vector<nex::Complex> h_tilde, // for fast fourier transform
			h_tilde_slopex, h_tilde_slopez,
			h_tilde_dx, h_tilde_dz;
		OceanFFT fft;


		static constexpr float GRAVITY = 9.81f;

		static float generateGaussianRand();

		void simulateFFT(std::vector<ResultData>& out, float t, unsigned startIndex, unsigned N, int stride);
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

#pragma once
#include <nex/math/Complex.hpp>
#include "nex/shader/Pass.hpp"
#include "nex/gui/Drawable.hpp"
#include "nex/math/Constant.hpp"

namespace nex
{
	class Mesh;
	class Camera;
	class Texture;
	class Texture2D;
	class CascadedShadow;
	class GlobalIllumination;
	class PSSR;

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

	class Ocean
	{
	public:

		virtual ~Ocean();

		static float generateGaussianRand();

		bool* getWireframeState();

		Complex heightZero(const glm::vec2& wave) const;
		float philipsSpectrum(const glm::vec2& wave) const;

		virtual void simulate(float t) = 0;

		virtual void updateAnimationTime(float t);

		float getTileSize() const;
		glm::mat4 getModelMatrix() const;

		float getWaterHeight() const;
		void setWaterHeight(float height);

		const glm::vec3& getPosition() const;
		void setPosition(const glm::vec3& position);

	protected:

		/**
		 * Creates a new Ocean object that is tileable. This means, that the first row and the first column are mirrored, so that
		 * The last row and last column match the first row resp. column.
		 *
		 * @param N : Amount of unique points and waves. Must be a power of 2 and must be greater 0.
		 * @param maxWaveLength : The maximum extension of a wave in the x-z plane
		 * @param dimension : The dimension of a tile (in object space)
		 * @param spectrumScale : Scales the used philip spectrum for wave generation. Has to be > 0.
		 * @param windDirection : The direction of wind. Its length has to be > 0
		 * @param windSpeed : The speed of wind.
		 * @param periodTime : The time of one period (in seconds). After that, the simulation gets repeated. Has to be > 0.
		 *
		 * @throws std::invalid_argument : if N is not a power of 2 or is zero; periodTime <= 0; spectrumScale <= 0; windDirection == 0;
		 */
		Ocean(unsigned N,
			unsigned maxWaveLength,
			float dimension,
			float waterHeight,
			float spectrumScale,
			const glm::vec2& windDirection,
			float windSpeed,
			float periodTime);

		/**
		 * Amount of unique points and waves in one axis direction (x resp. z axis).
		 */
		unsigned mN;

		/**
		 * The totoal amount of points in one axis direction (x resp. z axis).
		 * Note: This matches mN + 1.
		 */
		unsigned mPointCount;

		/**
		 * max extension of a wave in the x-z plane.
		 */
		unsigned mWaveLength;

		/**
		 * Dimension of the ocean tile (in object space)
		 */
		float mDimension;


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

		bool mWireframe;

		float mAnimationTime;

		float mWaterHeight;

		glm::vec3 mPosition;


		static constexpr float GRAVITY = 9.81f;
	};

	class OceanCpu : public Ocean
	{
	public:

		virtual ~OceanCpu();

		float dispersion(const glm::vec2& wave) const;

		void draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightDir);

		Complex height(int x, int z, float time) const;

	protected:

		OceanCpu(unsigned N,
			unsigned maxWaveLength,
			float dimension,
			float waterHeight,
			float spectrumScale,
			const glm::vec2& windDirection,
			float windSpeed,
			float periodTime);

		void generateMesh();

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

		class SimpleShadedPass : public Pass
		{
		public:
			SimpleShadedPass();

			void setUniforms(const glm::mat4& projection, const glm::mat4& view, 
				const glm::mat4& trafo, const glm::vec3& lightDir);

			Uniform transform;
			Uniform lightUniform;
			Uniform normalMatrixUniform;
		};

		std::vector<VertexCompute> mVerticesCompute;
		std::vector<VertexRender> mVerticesRender;
		std::vector<unsigned> mIndices;
		std::unique_ptr<Mesh> mMesh;
		std::unique_ptr<SimpleShadedPass> mSimpleShadedPass;
	};

	class OceanCpuDFT : public OceanCpu
	{
	public:
		OceanCpuDFT(unsigned N,
			unsigned maxWaveLength,
			float dimension,
			float waterHeight,
			float spectrumScale,
			const glm::vec2& windDirection,
			float windSpeed,
			float periodTime);

		virtual ~OceanCpuDFT();

		/**
		 * Simulates ocean state at time t.
		 * @param t : time. Has to be > 0
		 */
		void simulate(float t) override;

	protected:
		/**
		 * Computes the height of a location on the (x,z) plane at a specific time.
		 */
		ResultData simulatePoint(const glm::vec2& locationXZ, float time) const;
	};



	class OceanCpuFFT : public OceanCpu
	{
	public:
		OceanCpuFFT(unsigned N,
			unsigned maxWaveLength,
			float dimension,
			float waterHeight,
			float spectrumScale,
			const glm::vec2& windDirection,
			float windSpeed,
			float periodTime);

		virtual ~OceanCpuFFT();

		void simulate(float t) override;

	private:

		unsigned reverse(unsigned i) const;
		nex::Complex twiddle(unsigned x, unsigned N);

		void fft(nex::Complex* input, nex::Complex* output, int stride, int offset, bool vertical);

		void fft(const nex::Iterator2D&  input, nex::Iterator2D&  output, bool vertical);

		void fftInPlace(std::vector<nex::Complex>& x, bool inverse);

		unsigned mCurrent;
		unsigned mLogN;
		static constexpr float pi2 = static_cast<float>(2 * nex::PI);
		std::vector<unsigned> mReversed;
		std::vector<std::vector<nex::Complex>> mTwiddle;
		std::vector<nex::Complex> mTemp[2];

		std::vector<nex::Complex> mHeights, // for fast fourier transform
			mSlopeX, mSlopeZ,
			mHeightDx, mHeightDz;
	};


	/**
	 * A GPU implementation of the ocean
	 */
	class OceanGPU : public Ocean
	{
	public:
		void testHeightGeneration();
		OceanGPU(unsigned N, 
			unsigned maxWaveLength, 
			float dimension,
			float waterHeight,
			float spectrumScale, 
			const glm::vec2& windDirection, 
			float windSpeed, 
			float periodTime);

		virtual ~OceanGPU();

		/**
		 * @param waterMinDepth : a 1D texture in format R32I and colorspace RED_INTEGER; 
		 *                     has to have the same width as depth and stencil texture
		 * @param waterMaxDepth : a 1D texture in format R32I and colorspace RED_INTEGER;
		 *                     has to have the same width as depth and stencil texture
		 */
		void computeWaterDepths(Texture* waterMinDepth, Texture* waterMaxDepth, 
			Texture* depth, Texture* stencil, const glm::mat4& inverseViewProjMatrix);

		/**
		 * Draws the ocean.
		 */
		void draw(const glm::mat4& projection, 
			const glm::mat4& view, 
			const glm::mat4& inverseViewProjMatrix,
			const glm::vec3& lightDir, 
			CascadedShadow* cascadedShadow,
			Texture* color, 
			Texture* luminance, 
			Texture* depth,
			Texture* irradiance,
			GlobalIllumination* gi,
			const glm::vec3& cameraPosition,
			const glm::vec3& cameraDir);

		void drawUnderWaterView(
			Texture* color, 
			Texture* waterMinDepth,
			Texture* waterMaxDepth,
			Texture* depth,
			Texture* waterStencil,
			const glm::mat4& inverseViewProjMatrix,
			const glm::vec3& cameraPos);

		/**
		 * Simulates ocean state at time t.
		 * @param t : time. Has to be > 0
		 */
		void simulate(float t) override;

		Texture* getHeightMap();
		Texture* getDX();
		Texture* getDZ();

		/**
		 * Resizes render targets to match screen resolution
		 */
		void resize(unsigned width, unsigned height);

	private:

		void computeButterflyTexture(bool debug = false);

		void generateMesh();


		/**
		 * A vertex structure containing data for rendering the water
		 */
		struct Vertex
		{
			glm::vec3 position;
			glm::vec2 texCoords;
		};


		class UnderWaterView : public Pass
		{
		public:
			UnderWaterView();

			void setColorMap(Texture* texture);
			void setInverseViewProjMatrix_GPass(const glm::mat4& mat);
			void setInverseModelMatrix_Ocean(const glm::mat4& mat);
			void setOceanTileSize(float tileSize);
			void setDepthMap(Texture* texture);
			void setOceanHeightMap(Texture* texture);
			void setOceanDX(Texture* texture);
			void setOceanDZ(Texture* texture);
			void setOceanMinHeightMap(Texture* texture);
			void setOceanMaxHeightMap(Texture* texture);
			void setStencilMap(Texture* texture);
			void setCameraPosition(const glm::vec3& pos);

		private:
			UniformTex mColorMap;
			UniformTex mOceanHeightMap;
			UniformTex mDepthMap;
			UniformTex mOceanDX;
			UniformTex mOceanDZ;
			UniformTex mOceanMinHeightMap;
			UniformTex mOceanMaxHeightMap;
			UniformTex mStencilMap;

			Uniform mInverseViewProjMatrix_GPass;
			Uniform mInverseModelMatrix_Ocean;
			Uniform mOceanTileSize;
			Uniform mCameraPosition;
		};


		class WaterDepthClearPass : public ComputePass 
		{
		public:
			WaterDepthClearPass();

			void setWaterMinDepthOut(Texture* waterMinDepth);
			void setWaterMaxDepthOut(Texture* waterMaxDepth);

		private:
			UniformTex mWaterMinDepth;
			UniformTex mWaterMaxDepth;
		};

		class WaterDepthPass : public ComputePass
		{
		public:
			WaterDepthPass();

			void setDepth(Texture* depth);
			void setStencil(Texture* stencil);
			void setWaterMinDepthOut(Texture* waterMinDepth);
			void setWaterMaxDepthOut(Texture* waterMaxDepth);
			void setInverseViewProjMatrix(const glm::mat4& mat);

		private:
			UniformTex mWaterMinDepth;
			UniformTex mWaterMaxDepth;
			UniformTex mDepth;
			UniformTex mStencil;
			Uniform mInverseViewProjMatrix;
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

		class ButterflyComputePass : public ComputePass
		{
		public:
			/**
			 * @param N : The size of the DFT. Must be a power of 2.
			 *
			 * @throws std::invalid_argument : if N is not a power of 2
			 */
			ButterflyComputePass(unsigned N);


			void compute();

			Texture2D* getButterfly();

		private:

			Uniform mNUniform;
			unsigned mN;
			UniformTex mButterflyUniform;
			std::unique_ptr<Texture2D> mButterfly;
		};

		class IfftPass : public ComputePass
		{
		public:
			/**
			 * A compute pass for transforming frequency domain textures into time domain.
			 */
			IfftPass(int N);


			void useButterfly(Texture2D* butterfly);

			/**
			 * @param : the stage of the iFFT: has to be greater/equal 0 and  smaller log2(N)
			 */
			void setStage(int stage);

			/**
			 * Note: shader has to be bound!
			 * @param vertical: Specifies whether the iFFT should operate on columns of the input texture.
			 * If vertical is false, the iFFT operates on rows.
			 */
			void setVertical(bool vertical);

			/**
			 * Note: shader has to be bound and uniforms N and vertical has to be set!
			 * Computes a 1D FFT in the currently set direction (vertical or horizontal). Note, that the content of input will be modified.
			 */
			void computeAllStages(Texture2D* texture);

		private:

			/**
			 * Note: Shader has to be bound!
			 * input texture has to have format rgba32f
			 */
			void setButterfly(Texture2D* butterfly);


			/**
			 * Note: Shader has to be bound!
			 * input texture has to have format rg32f
			 */
			void setInput(Texture2D* input);

			/**
			 * Note: Shader has to be bound!
			 * output texture has to have format rg32f
			 */
			void setOutput(Texture2D* output);

			Uniform mNUniform;
			int mN;
			int mLog2N;
			Uniform mStageUniform;
			Uniform mVerticalUniform;
			UniformTex mInputUniform;
			UniformTex mButterflyUniform;
			UniformTex mOutputUniform;

			std::unique_ptr<ComputePass> mBlit;
			UniformTex mBlitSourceUniform;
			UniformTex mBlitDestUniform;

			std::unique_ptr<Texture2D> mPingPong;

			Texture2D* mButterfly;
		};

		class NormalizePermutatePass : public ComputePass
		{
		public:
			/**
			 * A compute pass for transforming frequency domain textures into time domain.
			 */
			NormalizePermutatePass(int N);

			void compute(Texture2D* height, Texture2D* slopeX, Texture2D* slopeZ, Texture2D* dX, Texture2D* dZ);


		private:

			Uniform mNUniform;
			int mN;
			UniformTex mHeightUniform;
			UniformTex mSlopeXUniform;
			UniformTex mSlopeZUniform;
			UniformTex mdXUniform;
			UniformTex mdZUniform;
		};

		class WaterShading : public Pass
		{
		public:
			WaterShading();

			void setUniforms(const glm::mat4& projection, 
				const glm::mat4& view, 
				const glm::mat4& trafo, 
				const glm::mat4& inverseViewProjMatrix,
				const glm::vec3& lightDir, 
				nex::CascadedShadow* cascadedShadow,
				Texture2D* height,
				Texture2D* slopeX, Texture2D* slopeZ, Texture2D* dX, Texture2D* dZ,
				Texture* color, 
				Texture* luminance,
				Texture* depth,
				Texture* irradiance,
				Texture* foam,
				Texture* projHash,
				nex::GlobalIllumination* gi,
				const glm::vec3& cameraPosition,
				const glm::vec2& windDir,
				float time,
				float tileSize,
				float waterLevel);

			Uniform transform;
			Uniform modelMatrixUniform;
			Uniform modelViewUniform;
			Uniform lightUniform;
			Uniform normalMatrixUniform;
			Uniform windDirection;
			Uniform animationTime;
			Uniform mTileSize;
			Uniform mInverseViewProjMatrix;
			UniformTex heightUniform;
			UniformTex slopeXUniform;
			UniformTex slopeZUniform;
			UniformTex dXUniform;
			UniformTex dZUniform;
			UniformTex colorUniform;
			UniformTex luminanceUniform;
			UniformTex depthUniform;
			UniformTex cascadedDepthMap;
			UniformTex mIrradiance;
			UniformTex mVoxelTexture;
			UniformTex mFoamTexture;
			UniformTex mProjHash;
			Uniform mCameraPosition;
			Uniform mWaterLevel;

			Sampler sampler;
		};


		std::unique_ptr<HeightZeroComputePass> mHeightZeroComputePass;
		std::unique_ptr<HeightComputePass> mHeightComputePass;
		std::unique_ptr<ButterflyComputePass> mButterflyComputePass;
		std::unique_ptr<IfftPass> mIfftComputePass;
		std::unique_ptr<NormalizePermutatePass> mNormalizePermutatePass;

		std::vector<Vertex> mVertices;
		std::vector<unsigned> mIndices;
		std::unique_ptr<Mesh> mMesh;
		std::unique_ptr<WaterShading> mSimpleShadedPass;
		std::unique_ptr<WaterDepthClearPass> mWaterDepthClearPass;
		std::unique_ptr<WaterDepthPass> mWaterDepthPass;
		std::unique_ptr<UnderWaterView> mUnderWaterView;
		Texture* mFoamTexture;
		std::unique_ptr<PSSR> mPSSR;
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
#pragma once

#include <nex/texture/Sprite.hpp>
#include <nex/gui/Drawable.hpp>
#include <nex/shader/Shader.hpp>
#include <glm/glm.hpp>
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Attachment.hpp>

namespace nex {
	class HbaoConfigurationView;
	class Sampler;

#define UBO_SCENE     0



	struct Projection {
		float nearplane;
		float farplane;
		float fov;
		float orthoheight;
		bool  perspective;
		glm::mat4  matrix;
	};

	struct SceneData {
		glm::mat4  viewProjMatrix;
		glm::mat4  viewMatrix;
		glm::mat4  viewMatrixIT;

		glm::uvec2 viewport;
		glm::uvec2 _pad;
	};

	struct HBAOData {
		float   RadiusToScreen;        // radius
		float   R2;     // radius * radius
		float   NegInvR2;     // -1/radius
		float   NDotVBias;

		glm::vec2    InvFullResolution;
		glm::vec2    InvQuarterResolution;

		float   AOMultiplier;
		float   PowExponent;
		glm::vec2    _pad0;

		glm::vec4    projInfo;
		glm::vec2    projScale;
		int     projOrtho;
		int     _pad1;

		static constexpr unsigned int AO_RANDOMTEX_SIZE = 4;

		glm::vec4    float2Offsets[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
		glm::vec4    jitters[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
	};


	class BilateralBlurShader : public Shader {
	public:

		explicit BilateralBlurShader();
		virtual ~BilateralBlurShader() = default;

		void draw(RenderTarget2D* temp, 
			RenderTarget2D* result,
			const Texture* viewspaceZ,
			const Texture* aoUnblurred,
			float width,
			float height,
			float sharpness);

	private:
		Uniform mSharpness;
		Uniform mInvResolutionDirection;
		UniformTex mViewSpaceZ;
		UniformTex mAoTexture;
 	};

	class DepthToViewSpaceZShader : public Shader {
	public:

		DepthToViewSpaceZShader();
		virtual ~DepthToViewSpaceZShader() = default;

		void draw(const Projection& projection, const Texture* depth);

		const Projection& getLastProjectionData() const;

	private:
		std::unique_ptr<Sampler> mSampler;
		Projection mProjection;
	};

	class DisplayAoShader : public Shader {
	public:

		DisplayAoShader();
		virtual ~DisplayAoShader() = default;

		void draw(const Texture* ao);
	};

	class HbaoShader : public Shader {
	public:

		HbaoShader();
		virtual ~HbaoShader() = default;

		void setHbaoUBO(UniformBuffer* hbao_ubo);
		void setLinearDepth(Texture* linearDepth);
		void setRamdomView(Texture* randomView);

	private:
		Sampler mPointSampler2;
	};


	class HbaoDeinterleavedShader : public Shader {
	public:

		HbaoDeinterleavedShader();
		virtual ~HbaoDeinterleavedShader() = default;

		void setHbaoUBO(UniformBuffer* hbao_ubo);
		void setLinearDepth(Texture* linearDepth);
		void setViewNormals(Texture* normals);
		void setImageOutput(Texture* imgOutput);

	private:

		UniformTex mLinearDepth;
		UniformTex mViewNormals;
		UniformTex mImgOutput;
	};

	class ViewNormalShader : public Shader {
	public:

		ViewNormalShader();
		virtual ~ViewNormalShader();

		void setProjInfo(const glm::vec4& projInfo);
		void setProjOrtho(bool projOrtho);
		void setInvFullResolution(const glm::vec2& invFullResolution);
		void setLinearDepth(Texture* linearDepth);

	private:
		
		Uniform mProjInfo;
		Uniform mProjOrtho;
		Uniform mInvFullResolution;
		UniformTex mLinearDepth;
	};

	class DeinterleaveShader : public Shader {
	public:

		DeinterleaveShader();
		virtual ~DeinterleaveShader();

		void setInfo(const glm::vec4& info);
		void setLinearDepth(Texture* linearDepth);

	private:

		Uniform mInfo;
		UniformTex mLinearDepth;
	};

	class ReinterleaveShader : public Shader {
	public:

		ReinterleaveShader();
		virtual ~ReinterleaveShader();

		void setTextureResultArray(Texture* resultArray);

	private:
		UniformTex mResultArray;
	};

	class HbaoBlur {
	public:

		HbaoBlur();
		virtual ~HbaoBlur();

		void bindPreset(int id);

		Shader* getPreset(int id);

		void setSharpness(float sharpness);
		void setInvResolutionDirection(const glm::vec2& invResolustion);
		void setSource(Texture* source);

	private:

		std::unique_ptr<Shader> mBlurPreset[2];
		int mActivePreset;
		Uniform mSharpness[2];
		Uniform mInvResolutionDirection[2];
		UniformTex mSource;

	};


	/**
	 * An Horizon based ambient occlusion (HBAO) implementation for deferred rendering.
	 * This implementation doesn't support MSAA for performance reasons.
	 */
	class HBAO {
	public:

		HBAO(unsigned int windowWidth,
			unsigned int windowHeight);

		Texture2D* getAO_Result();
		Texture2D* getBlurredResult();
		Texture2D* getViewSpaceNormals();
		Texture2D* getViewSpaceZ();
		const Projection& getViewSpaceZProjectionInfo() const;

		Texture* getViewSpaceZ4thView(int index);
		Texture* getAoResultView4th(int index);

		void onSizeChange(unsigned int newWidth, unsigned int newHeight);

		void renderAO(const Texture* depth, const Projection& projection, bool blur);
		void renderCacheAwareAO(const Texture* depth, const Projection& projection, bool blur);

		void displayAOTexture(const Texture* ao);

		float getBlurSharpness() const;
		void setBlurSharpness(float sharpness);

		static const int  HBAO_RANDOM_SIZE = HBAOData::AO_RANDOMTEX_SIZE;
		static const int  HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE * HBAO_RANDOM_SIZE;
		static const int HBAO_NUM_DIRECTIONS = 8; // keep in sync with shader implementation!
		static const int NUM_MRT = 8; // number of simultaneous framebuffer bindings in cache aware ao calculation

	protected:

		static float randomFloat(float a, float b);
		static float lerp(float a, float b, float f);

		void drawLinearDepth(const Texture* depthTexture, const Projection & projection);
		void initRenderTargets(unsigned int width, unsigned int height);
		void prepareHbaoData(const Projection& projection, int width, int height);

	protected:
		friend HbaoConfigurationView;

		float mBlurSharpness;
		float mMeters2ViewSpace;
		float mRadius;
		float mIntensity;
		float mBias;

		unsigned int mWindowWidth;
		unsigned int mWindowHeight;

		Sprite screenSprite;

	protected:

		std::unique_ptr<BilateralBlurShader> mBilateralBlur;
		std::unique_ptr<DepthToViewSpaceZShader> mDepthToViewSpaceZ;
		std::unique_ptr<DisplayAoShader> mAoDisplayShader;
		std::unique_ptr<HbaoShader> mHbaoShader;

		std::unique_ptr<RenderTarget2D> mViewSpaceZRT;
		std::unique_ptr<RenderTarget2D> mAoResultRT;
		std::unique_ptr<RenderTarget2D> mTempRT;
		std::unique_ptr<RenderTarget2D> mAoBlurredResultRT;
		


		//Drawer* m_modelDrawer;

		std::unique_ptr<Texture2DArray> mHbaoRandomTexture;
		std::unique_ptr<Texture> mHbaoRandomview;
		glm::vec4 mHbaoRandom[HBAO_RANDOM_ELEMENTS];
		UniformBuffer mHbaoUbo;

		//cache aware stuff
		std::unique_ptr<HbaoDeinterleavedShader> mHbaoDeinterleavedShader;
		std::unique_ptr<ViewNormalShader> mViewNormalShader;
		std::unique_ptr<DeinterleaveShader> mDeinterleaveShader;
		std::unique_ptr<ReinterleaveShader> mReinterleaveShader;
		std::unique_ptr<HbaoBlur> mHbaoBlur;
		std::unique_ptr<Texture2DArray> mDepthArray4th;
		std::unique_ptr<Texture2DArray> mHbaoResultArray4th;
		std::shared_ptr<Texture> mDepthView4th[HBAO_RANDOM_ELEMENTS];
		std::shared_ptr<Texture> mHbaoResultView4th[HBAO_RANDOM_ELEMENTS];
		std::unique_ptr<RenderTarget> mDeinterleaveRT;
		std::vector<RenderAttachment> mDeinterleaveAttachment[HBAO_RANDOM_ELEMENTS / NUM_MRT];
		std::unique_ptr<RenderTarget> mCacheAwareAoRT;
		std::unique_ptr<RenderTarget2D> mViewSpaceNormalsRT;
		

		HBAOData   m_hbaoDataSource;
	};


	class HbaoConfigurationView : public nex::gui::Drawable {
	public:
		HbaoConfigurationView(HBAO* hbao);

	protected:
		void drawSelf() override;

	private:
		HBAO * mHbao;
	};
}
#pragma once

#include <nex/texture/Sprite.hpp>
#include <nex/gui/Drawable.hpp>
#include <nex/shader/Pass.hpp>
#include <glm/glm.hpp>
#include <nex/shader/ShaderBuffer.hpp>
#include <nex/mesh/VertexArray.hpp>
#include <nex/texture/RenderTarget.hpp>

namespace nex {
	class HBAO_ConfigurationView;
	class Sampler;

#define UBO_SCENE     0



	static const unsigned int AO_RANDOMTEX_SIZE = 4;

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

		glm::vec4    float2Offsets[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
		glm::vec4    jitters[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
	};


	class BilateralBlur : public Pass {
	public:

		explicit BilateralBlur();
		virtual ~BilateralBlur() = default;

		void setLinearDepth(Texture* linearDepth);
		void setSharpness(float sharpness);
		void setSourceTexture(Texture* source, unsigned int textureWidth, unsigned int textureHeight);

		void draw(RenderTarget2D* temp, RenderTarget2D* result);

	private:
		Texture* m_linearDepth;
		float m_sharpness;
		Texture* m_source;
		unsigned int m_textureHeight;
		unsigned int m_textureWidth;
 	};

	class DepthLinearizer : public Pass {
	public:

		DepthLinearizer();
		virtual ~DepthLinearizer() = default;

		void draw();
		void setInputTexture(Texture* input);
		void setProjection(const Projection* projection);

	private:
		Texture* m_input;
		const Projection* m_projection;
		std::unique_ptr<Sampler> mSampler;
	};

	class DisplayTex : public Pass {
	public:

		DisplayTex();
		virtual ~DisplayTex() = default;

		void draw();
		void setInputTexture(Texture* input);

	private:
		Texture* m_input;
	};

	class HBAO_Shader : public Pass {
	public:

		HBAO_Shader();
		virtual ~HBAO_Shader() = default;

		void draw();
		void setHbaoData(const HBAOData& hbao);
		void setHbaoUBO(UniformBuffer* hbao_ubo);
		void setLinearDepth(Texture* linearDepth);
		void setRamdomView(Texture* randomView);

	private:
		HBAOData m_hbao_data;
		Texture* m_hbao_randomview;
		UniformBuffer* m_hbao_ubo;
		Texture* m_linearDepth;
		Sampler mPointSampler2;
	};


	/**
	 * An Horizon based ambient occlusion (HBAO) implementation for deferred rendering.
	 * This implementation doesn't support MSAA for performance reasons.
	 */
	class HBAO {
	public:

		HBAO(unsigned int windowWidth,
			unsigned int windowHeight);

		//virtual ~HBAO_GL();

		Texture2D* getAO_Result();
		Texture2D* getBlurredResult();

		void onSizeChange(unsigned int newWidth, unsigned int newHeight);

		void renderAO(Texture* depth, const Projection& projection, bool blur);

		void displayAOTexture(Texture* texture);

		float getBlurSharpness() const;
		void setBlurSharpness(float sharpness);

	protected:

		static float randomFloat(float a, float b);
		static float lerp(float a, float b, float f);

		void drawLinearDepth(Texture* depthTexture, const Projection & projection);
		void initRenderTargets(unsigned int width, unsigned int height);
		void prepareHbaoData(const Projection& projection, int width, int height);

	protected:
		std::unique_ptr<BilateralBlur> m_bilateralBlur;
		std::unique_ptr<DepthLinearizer> m_depthLinearizer;
		std::unique_ptr<DisplayTex> m_aoDisplay;
		std::unique_ptr<HBAO_Shader> m_hbaoShader;

		std::unique_ptr<RenderTarget2D> m_depthLinearRT;
		std::unique_ptr<RenderTarget2D> m_aoResultRT;
		std::unique_ptr<RenderTarget2D> m_tempRT;
		std::unique_ptr<RenderTarget2D> m_aoBlurredResultRT;
		


		//StaticMeshDrawer* m_modelDrawer;

		std::unique_ptr<Texture2DArray> m_hbao_random;
		std::unique_ptr<Texture> m_hbao_randomview;
		UniformBuffer m_hbao_ubo;


		HBAOData   m_hbaoDataSource;
		VertexArray m_fullscreenTriangleVAO;


	protected:
		friend HBAO_ConfigurationView;

		float m_blur_sharpness;
		float m_meters2viewspace;
		float m_radius;
		float m_intensity;
		float m_bias;

		unsigned int windowWidth;
		unsigned int windowHeight;

		Sprite screenSprite;

		static const int  HBAO_RANDOM_SIZE = AO_RANDOMTEX_SIZE;
		static const int  HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE * HBAO_RANDOM_SIZE;
		static const int HBAO_NUM_DIRECTIONS = 8; // keep in sync with shader implementation!
	};


	class HBAO_ConfigurationView : public nex::gui::Drawable {
	public:
		HBAO_ConfigurationView(HBAO* hbao);

	protected:
		void drawSelf() override;

	private:
		HBAO * m_hbao;
		Drawable* m_parent;
		float m_test;
	};
}
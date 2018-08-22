#ifndef HBAO_GL_HPP
#define HBAO_GL_HPP

#include <nex/post_processing/HBAO.hpp>

class TextureGL;
class BaseRenderTargetGL;
class ShaderGL;
class ModelDrawerGL;

namespace hbao {
	class BilateralBlur : public ShaderGL {
	public:

		explicit BilateralBlur();
		virtual ~BilateralBlur() = default;

		void setLinearDepth(TextureGL* linearDepth);
		void setSharpness(float sharpness);
		void setSourceTexture(TextureGL* source, unsigned int textureWidth, unsigned int textureHeight);

		virtual void draw(const Mesh& mesh) override;
		void draw(OneTextureRenderTarget* temp, BaseRenderTargetGL* result);

	private:
		TextureGL* m_linearDepth;
		float m_sharpness;
		TextureGL* m_source;
		unsigned int m_textureHeight;
		unsigned int m_textureWidth;
 	};

	class DepthLinearizer : public ShaderGL {
	public:

		DepthLinearizer();
		virtual ~DepthLinearizer() = default;

		virtual void draw(const Mesh& mesh) override;
		void draw();
		void setInputTexture(TextureGL* input);
		void setProjection(const Projection* projection);

	private:
		TextureGL* m_input;
		const Projection* m_projection;
	};

	class DisplayTex : public ShaderGL {
	public:

		DisplayTex();
		virtual ~DisplayTex() = default;

		virtual void draw(const Mesh& mesh) override;
		void draw();
		void setInputTexture(TextureGL* input);

	private:
		TextureGL* m_input;
	};

	class HBAO_Shader : public ShaderGL {
	public:

		HBAO_Shader();
		virtual ~HBAO_Shader() = default;

		virtual void draw(const Mesh& mesh) override;
		void draw();
		void setHbaoData(HBAOData hbao);
		void setHbaoUBO(GLuint hbao_ubo);
		void setLinearDepth(TextureGL* linearDepth);
		void setRamdomView(TextureGL* randomView);

	private:
		HBAOData m_hbao_data;
		TextureGL* m_hbao_randomview;
		GLuint m_hbao_ubo;
		TextureGL* m_linearDepth;
	};

	class HBAO_GL : public HBAO {
	public:

		HBAO_GL(unsigned int windowWidth,
			unsigned int windowHeight, ModelDrawerGL* modelDrawer);

		virtual ~HBAO_GL();

		virtual Texture* getAO_Result() override;
		virtual Texture* getBlurredResult() override;

		virtual void onSizeChange(unsigned int newWidth, unsigned int newHeight) override;

		virtual void renderAO(Texture* depth, const Projection& projection, bool blur) override;

		virtual void displayAOTexture() override;
		virtual void displayTexture(Texture* texture) override;

	protected:

		void drawLinearDepth(TextureGL* depthTexture, const Projection & projection);
		void initRenderTargets(unsigned int width, unsigned int height);
		void prepareHbaoData(const Projection& projection, int width, int height);

	protected:
		std::unique_ptr<hbao::BilateralBlur> m_bilateralBlur;
		std::unique_ptr<hbao::DepthLinearizer> m_depthLinearizer;
		std::unique_ptr<hbao::DisplayTex> m_aoDisplay;
		std::unique_ptr<hbao::HBAO_Shader> m_hbaoShader;

		std::unique_ptr<OneTextureRenderTarget> m_depthLinearRT;
		std::unique_ptr<OneTextureRenderTarget> m_aoResultRT;
		std::unique_ptr<OneTextureRenderTarget> m_tempRT;
		std::unique_ptr<OneTextureRenderTarget> m_aoBlurredResultRT = nullptr;
		


		ModelDrawerGL* m_modelDrawer;

		TextureGL m_hbao_random;
		TextureGL m_hbao_randomview;
		GLuint m_hbao_ubo{};

		HBAOData   m_hbaoDataSource;
		GLuint m_fullscreenTriangleVAO;
	};
}

#endif //HBAO_GL_HPP
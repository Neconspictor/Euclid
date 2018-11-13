#ifndef HBAO_GL_HPP
#define HBAO_GL_HPP

#include <nex/opengl/texture/Sprite.hpp>
#include <nex/gui/Drawable.hpp>
#include <glm/glm.hpp>

class TextureGL;
class BaseRenderTargetGL;
class ShaderGL;
class ModelDrawerGL;

/*-----------------------------------------------------------------------
Copyright (c) 2014, NVIDIA. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Neither the name of its contributors may be used to endorse
or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

namespace hbao {
#define UBO_SCENE     0

	class HBAO_ConfigurationView;

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


	class BilateralBlur : public ShaderGL {
	public:

		explicit BilateralBlur();
		virtual ~BilateralBlur() = default;

		void setLinearDepth(TextureGL* linearDepth);
		void setSharpness(float sharpness);
		void setSourceTexture(TextureGL* source, unsigned int textureWidth, unsigned int textureHeight);

		void draw(const MeshGL& mesh) override;
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

		void draw(const MeshGL& mesh) override;
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

		void draw(const MeshGL& mesh) override;
		void draw();
		void setInputTexture(TextureGL* input);

	private:
		TextureGL* m_input;
	};

	class HBAO_Shader : public ShaderGL {
	public:

		HBAO_Shader();
		virtual ~HBAO_Shader() = default;

		void draw(const MeshGL& mesh) override;
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


	/**
	 * An Horizon based ambient occlusion (HBAO) implementation for deferred rendering.
	 * This implementation doesn't support MSAA for performance reasons.
	 */
	class HBAO_GL {
	public:

		HBAO_GL(unsigned int windowWidth,
			unsigned int windowHeight, ModelDrawerGL* modelDrawer);

		virtual ~HBAO_GL();

		TextureGL* getAO_Result();
		TextureGL* getBlurredResult();

		void onSizeChange(unsigned int newWidth, unsigned int newHeight);

		void renderAO(TextureGL* depth, const Projection& projection, bool blur);

		void displayAOTexture(TextureGL* texture);

		float getBlurSharpness() const;
		void setBlurSharpness(float sharpness);

	protected:

		static float randomFloat(float a, float b);
		static float lerp(float a, float b, float f);

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


	class HBAO_ConfigurationView : public nex::engine::gui::Drawable {
	public:
		HBAO_ConfigurationView(HBAO_GL* hbao);

	protected:
		void drawSelf() override;

	private:
		HBAO_GL * m_hbao;
		Drawable* m_parent;
		float m_test;
	};
}

#endif //HBAO_GL_HPP
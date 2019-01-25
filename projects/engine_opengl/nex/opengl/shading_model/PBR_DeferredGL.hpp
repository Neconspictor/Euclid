#ifndef PBR_DEFERRED_GL_HPP
#define PBR_DEFERRED_GL_HPP

#include <nex/texture/Texture.hpp>
#include <nex/gui/Drawable.hpp>
#include <nex/opengl/shading_model/PBR.hpp>
#include <nex/opengl/shadowing/CascadedShadowGL.hpp>

namespace nex
{

	class PBR_GBuffer : public RenderTarget
	{
	public:

		// Has to be implemented by renderer backend
		PBR_GBuffer(unsigned width, unsigned height);

		Texture* getAlbedo() const;
		Texture* getAoMetalRoughness() const;
		Texture* getNormal() const;
		Texture* getDepth() const;

	private:
		RenderAttachment albedo;
		RenderAttachment aoMetalRoughness;
		RenderAttachment normal;
		RenderAttachment depth;
	};



	class PBR_DeferredGL : public PBR {

	public:
		PBR_DeferredGL(RendererOpenGL* renderer, Texture* backgroundHDR);

		void drawGeometryScene(SceneNode * scene,
			const glm::mat4& view,
			const glm::mat4& projection);

		void drawLighting(SceneNode * scene,
			PBR_GBuffer* gBuffer,
			Texture* ssaoMap,
			const DirectionalLight& light,
			const glm::mat4& viewFromGPass,
			const glm::mat4& projFromGPass,
			const glm::mat4& worldToLight,
			CascadedShadowGL::CascadeData* cascadeData,
			Texture* cascadedDepthMap);

		void drawSky(const glm::mat4& projection,
			const glm::mat4& view);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

	private:
		Sprite screenSprite;
	};

	class PBR_Deferred_ConfigurationView : public nex::gui::Drawable {
	public:
		PBR_Deferred_ConfigurationView(PBR_DeferredGL* pbr);

	protected:
		void drawSelf() override;

	private:
		PBR_DeferredGL * m_pbr;
	};
}

#endif
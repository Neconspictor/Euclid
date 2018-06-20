#pragma once

#include <texture/TextureManager.hpp>
#include <shader/ShaderManager.hpp>
#include <platform/Renderer.hpp>
#include <model/ModelManager.hpp>
#include <drawing/ModelDrawer.hpp>
#include <antialiasing/SMAA.hpp>
#include <post_processing/blur/GaussianBlur.hpp>
#include <shading_model/ShadingModelFactory.hpp>
#include <post_processing/SSAO.hpp>
#include <post_processing/HBAO.hpp>

enum class CullingMode
{
	Front, Back
};

enum RenderComponent {
	Color = 1 << 0,
	Depth = 1 << 1,
	Stencil = 1 << 2
};

class EffectLibrary {
public:
	virtual GaussianBlur* getGaussianBlur() = 0;
};


/**
 * A 3D renderer is a renderer specific for 3D content. 
 * A 3D renderer renders so called models (visual content)
 * and shades them with the use of shaders. A 3D renderer
 * also understands the concept of textures. As shaders 
 * and textures are strong coupled with the implementation
 * of a renderer, the Renderer3D interface provides methods
 * for creating and storing shaders and textures via specialised
 * manager objects.
 */
class Renderer3D : public Renderer
{
public:

	virtual void blitRenderTargets(BaseRenderTarget* src , BaseRenderTarget* dest, const Dimension& dim, int renderComponents) = 0;

	virtual void clearRenderTarget(BaseRenderTarget* renderTarget, int renderComponents) = 0;

	virtual CubeDepthMap* createCubeDepthMap(int width, int height) = 0;

	virtual DepthMap* createDepthMap(int width, int height) = 0;

	virtual CubeRenderTarget* createCubeRenderTarget(int width, int height, const TextureData& data = {false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32}) = 0;

	virtual RenderTarget* create2DRenderTarget(int width, int height, const TextureData& data = { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 }, int samples = 1) = 0;

	virtual RenderTarget* createRenderTarget(int samples = 1) = 0;

	virtual std::unique_ptr<SSAO_Deferred> createDeferredSSAO() = 0;

	virtual std::unique_ptr<hbao::HBAO> createHBAO() = 0;

	virtual RenderTarget* createVarianceShadowMap(int width, int height) = 0;
	
	virtual void cullFaces(CullingMode mode = CullingMode::Back) = 0;

	virtual void destroyCubeRenderTarget(CubeRenderTarget* target) = 0;

	virtual void destroyRenderTarget(RenderTarget* target) = 0;

	virtual void enableAlphaBlending(bool enable) = 0;

	virtual void enableBackfaceDrawing(bool enable) = 0;
	
	/**
	 * Enables / Disables depth mask writing. 
	 * models drawn with disabled depth mask will always overwrite
	 * the existing fragments/pixels.
	 */
	virtual void enableDepthWriting(bool enable) = 0;

	virtual BaseRenderTarget* getDefaultRenderTarget() = 0;

	virtual EffectLibrary* getEffectLibrary() = 0;

	virtual ShadingModelFactory& getShadingModelFactory() = 0;

	/**
	 * Provides access to a shader manager that creates and stores shaders
	 * compatible to this renderer
	 */
	virtual ShaderManager* getShaderManager() = 0;

	/**
	* Provides a texture manager, that creates and stores textures in a format
	* this renderer is able to handle.
	*/
	virtual TextureManager* getTextureManager() = 0;

	/**
	 * Provides a facility class for drawing models.
	 */
	virtual ModelDrawer* getModelDrawer() = 0;

	/** 
	* Provides access to a mesh manager, that creates and stores 3d meshes.
	 */
	virtual ModelManager* getModelManager() = 0;

	virtual SMAA* getSMAA() = 0;

	/**
	 * Renders an equirectangular texture (2D) to a cubemap and returns the result;
	 */
	virtual CubeRenderTarget* renderCubeMap(int width, int height, Texture* equirectangularMap) = 0;

	virtual void setBackgroundColor(glm::vec3 color) = 0;

	/**
	 * Sets the number of samples used for msaa
	 */
	virtual void setMSAASamples(unsigned int samples) = 0;

	virtual void useCubeDepthMap(CubeDepthMap* cubeDepthMap) = 0;

	virtual void useCubeRenderTarget(CubeRenderTarget* target, CubeMap::Side side, unsigned int mipLevel = 0) = 0;

	/**
	* All draw calls are performed on a offscreen texture.
	* The output of all draw calls won't be visible after swapping the window's screen buffer
	*/
	virtual void useBaseRenderTarget(BaseRenderTarget* target) = 0;

	/**
	 * Draws directly to the screen buffer -> 
	 */
	virtual void useScreenTarget() = 0;

	/**
	* All draw calls are performed on a variance shadow map texture.
	* Only the depth value and it's square are written (no color information).
	*/
	virtual void useVarianceShadowMap(RenderTarget* map) = 0;
};
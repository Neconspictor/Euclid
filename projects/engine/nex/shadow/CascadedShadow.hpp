#pragma once
#include <nex/shader/Shader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/gui/Drawable.hpp"

namespace nex
{
	/**
 * Abstract class for Cascaded shadow implementations.
 */
	class CascadedShadow
	{
	public:

		struct CascadeData {
			glm::mat4 inverseViewMatrix;
			std::vector<glm::mat4> lightViewProjectionMatrices;
			std::vector <glm::vec4> scaleFactors; // only x component is used
			std::vector <glm::vec4> cascadedFarPlanes; // far plane splits in (positive z-axis) view space; only x component is used
			unsigned numCascades; // IMPORTANT: Keep in sync with shader implementation(s)

			std::vector<char> shaderBuffer; // used for shader data transfer

			static unsigned calcCascadeDataByteSize(unsigned numCascades);
		};

		struct PCFFilter
		{
			unsigned sampleCountX;
			unsigned sampleCountY;
			bool useLerpFiltering;

			bool operator==(const PCFFilter& o);
		};

		CascadedShadow(unsigned int cascadeWidth, unsigned int cascadeHeight, unsigned numCascades, const PCFFilter& pcf, bool antiFlickerOn = true);

		/**
		 * Allows rendering to the i-th cascade.
		 */
		void begin(int cascadeIndex);

		/**
		 * Finishes rendering to the i-th shadow cascade.
		 * Should be called after rendering to the cascade
		 * Has to be called AFTER CascadedShadow::begin(int)
		 */
		void end();

		nex::Texture* getDepthTextureArray();

		/**
		 * Resizes the cascades
		 */
		void resize(unsigned int cascadeWidth, unsigned int cascadeHeight);

		void addCascadeChangeCallback(std::function<void(CascadedShadow*)> callback);
		void informCascadeChanges();

		/**
		 * Renders a mesh with a given model matrix to the active cascade
		 */
		//void render(SubMesh* mesh, const glm::mat4* modelMatrix);

		/**
		 * Updates the cascades. Has to be called once per frame and before actual renering to the cascades happens.
		 */
		void frameUpdate(Camera* camera, const glm::vec3& lightDirection);

		bool getAntiFlickering() const;

		CascadeData* getCascadeData();

		Shader* getDepthPassShader();

		unsigned getHeight() const;

		const PCFFilter& getPCF() const;

		unsigned getWidth() const;

		const glm::mat4& getWorldToShadowSpace() const;
		const glm::mat4& getShadowView() const;

		void setAntiFlickering(bool enable);

		void setPCF(const PCFFilter& filter, bool informOberservers = true);


	protected:

		void resizeCascadeData(unsigned numCascades);

		void updateTextureArray();

		void updateCascadeData();


		struct BoundingSphere
		{
			glm::vec3 center;
			float radius;
		};

		struct GlobalShadow
		{
			glm::mat4 worldToShadowSpace;
			glm::mat4 shadowView;
			float radius;
		};

		void frameUpdateNew(Camera* camera, const glm::vec3& lightDirection);
		void frameUpdateOld(Camera* camera, const glm::vec3& lightDirection);


		GlobalShadow calcShadowSpaceMatrix(Camera* camera, const glm::vec3& lightDirection);
		void calcSplitSchemes(Camera* camera);
		void calcSplitDistances(Camera* camera);
		BoundingSphere extractFrustumBoundSphere(Camera* camera, float nearSplitDistance, float farSplitDistance);
		void extractFrustumPoints(Camera* camera, float nearSplitDistance, float farSplitDistance, glm::vec3 (&frustumCorners)[8]);
		bool cascadeNeedsUpdate(const glm::mat4& shadowView, int cascadeIdx, const glm::vec3& newCenter,
			const glm::vec3& oldCenter, float cascadeBoundRadius, glm::vec3* offset);

	protected:

		class DepthPassShader : public Shader
		{
		public:
			DepthPassShader();
			void onModelMatrixUpdate(const glm::mat4& modelMatrix) override;
		};


		DepthPassShader mDepthPassShader;
		RenderTarget mRenderTarget;

	protected:
		unsigned int mCascadeWidth;
		unsigned int mCascadeHeight;

		float mShadowMapSize;
		CascadeData mCascadeData;
		bool mAntiFlickerOn;
		std::vector<float> mSplitDistances;
		std::vector<glm::vec3> mCascadeBoundCenters;
		GlobalShadow mGlobal;
		PCFFilter mPCF;
		std::list<std::function<void(CascadedShadow*)>> mCallbacks;
	};

	class CascadedShadow_ConfigurationView : public nex::gui::Drawable {
	public:
		CascadedShadow_ConfigurationView(CascadedShadow* model);

	protected:
		void drawCascadeDimensionConfig();
		void drawPCFConfig();
		void drawSelf() override;

	private:
		CascadedShadow * mModel;
	};
}

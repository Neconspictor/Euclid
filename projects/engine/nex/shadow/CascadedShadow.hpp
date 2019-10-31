#pragma once
#include <nex/texture/Texture.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/gui/Drawable.hpp"
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/gui/TextureView.hpp>

namespace nex
{
	class SceneNearFarComputePass;


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

		class CascadeDataPass : public ComputePass
		{
		public:

			struct Input
			{
				glm::mat4 viewMatrix;
				glm::mat4 projectionMatrix;
				glm::vec4 lightDirection; // w-component isn't used
				glm::vec4 nearFarPlane; // x and y component hold the near and far plane of the camera
				glm::vec4 shadowMapSize; // only x component is used
				glm::vec4 cameraPostionWS; // w component isn't used
				glm::vec4 cameraLook; // w component isn't used

				glm::vec4 _pad0; 
				glm::vec4 _pad1;
				glm::vec4 _pad2;
			};

			struct DistanceInput
			{
				glm::vec4 minMax; // x and y component hold the min and max (positive) viewspace z value; the other components are not used
			};


			CascadeDataPass(unsigned numCascades);

			ShaderStorageBuffer* getSharedOutput();

			/**
			 * NOTE: This shader has to be bound!
			 */
			void useDistanceInputBuffer(ShaderStorageBuffer* buffer);


			/**
			 * NOTE: This shader has to be bound!
			 */
			void setUseAntiFlickering(bool use);
			void update(const Input& input);
			void resetPrivateData();

			// public access for easier access
			std::unique_ptr<ShaderStorageBuffer> mInputBuffer;
			//std::unique_ptr<ShaderStorageBuffer> mDistanceInputBuffer;
			std::unique_ptr<ShaderStorageBuffer> mSharedOutput;
			std::unique_ptr<ShaderStorageBuffer> mPrivateOutput;
			Uniform mUseAntiFlickering;
			unsigned mNumCascades;
		};

		CascadedShadow(unsigned int cascadeWidth, unsigned int cascadeHeight, unsigned numCascades, const PCFFilter& pcf, float biasMultiplier, bool antiFlickerOn = true);


		~CascadedShadow();

		std::vector<std::string> generateCsmDefines() const;

		/**
		 * Allows rendering to the i-th cascade.
		 */
		void begin(int cascadeIndex);

		void enable(bool enable, bool informObservers = true);

		/**
		 * Finishes rendering to the i-th shadow cascade.
		 * Should be called after rendering to the cascade
		 * Has to be called AFTER CascadedShadow::begin(int)
		 */
		void end();

		nex::Texture* getDepthTextureArray();
		const nex::Texture* getDepthTextureArray() const;

		/**
		 * Resizes the cascades
		 */
		void resize(unsigned int cascadeWidth, unsigned int cascadeHeight);
		void resizeCascadeData(unsigned numCascades, bool informObservers = true);

		void addCascadeChangeCallback(std::function<void(CascadedShadow*)> callback);
		void informCascadeChanges();

		bool isEnabled() const;

		/**
		 * Renders a mesh with a given model matrix to the active cascade
		 */
		//void render(SubMesh* mesh, const glm::mat4* modelMatrix);

		/**
		 * Updates the cascades. Has to be called once per frame and before actual renering to the cascades happens.
		 */
		void frameUpdate(const Camera& camera, const glm::vec3& lightDirection, Texture2D* depth);

		bool getAntiFlickering() const;

		float getBiasMultiplier() const;

		const CascadeData& getCascadeData() const;

		TransformPass* getDepthPass();

		unsigned getHeight() const;

		const PCFFilter& getPCF() const;

		float getShadowStrength()const;

		unsigned getWidth() const;

		const glm::mat4& getWorldToShadowSpace() const;
		const glm::mat4& getShadowView() const;

		void frameReset();

		void setAntiFlickering(bool enable);

		void setBiasMultiplier(float bias, bool informObservers = true);

		void setPCF(const PCFFilter& filter, bool informOberservers = true);

		/**
		 * @param strength : a float in the range [0,1]
		 */
		void setShadowStrength(float strength);
		ShaderStorageBuffer* getCascadeBuffer();
		const ShaderStorageBuffer* getCascadeBuffer() const;

		void useTightNearFarPlane(bool use);


	protected:

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

		class DepthPass : public TransformPass
		{
		public:
			DepthPass(unsigned numCascades);

			void setCascadeIndex(unsigned index);
			void setCascadeShaderBuffer(ShaderStorageBuffer* buffer);
			void updateConstants(const Camera& camera);

		private:
			unsigned mNumCascades;
		};

		GlobalShadow calcShadowSpaceMatrix(const Camera& camera, const glm::vec3& lightDirection);
		
		void calcSplitSchemes(const glm::vec2& minMaxPositiveZ);
		
		void calcSplitDistances(float range, const glm::vec2& minMaxPositiveZ);
		
		bool cascadeNeedsUpdate(const glm::mat4& shadowView, int cascadeIdx, const glm::vec3& newCenter,
			const glm::vec3& oldCenter, float cascadeBoundRadius, glm::vec3* offset);

		BoundingSphere extractFrustumBoundSphere(const Camera& camera, float nearSplitDistance, float farSplitDistance);

		void updateCascadeData();

		void updateTextureArray();

		void frameUpdateTightNearFarPlane(const Camera& camera, const glm::vec3& lightDirection, nex::ShaderStorageBuffer* minMaxOutputBuffer);
		void frameUpdateNoTightNearFarPlane(const Camera& camera, const glm::vec3& lightDirection, const glm::vec2& minMaxPositiveZ);
		



		std::unique_ptr<DepthPass> mDepthPass;
		std::unique_ptr<CascadeDataPass> mDataComputePass;
		std::unique_ptr<SceneNearFarComputePass> mSceneNearFarComputeShader;
		RenderTarget mRenderTarget;

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
		bool mEnabled;
		float mBiasMultiplier;
		float mShadowStrength;
		bool mUseTightNearFarPlane;
	};

	class CascadedShadow_ConfigurationView : public nex::gui::Drawable {
	public:
		CascadedShadow_ConfigurationView(CascadedShadow* model);

	protected:
		void drawShadowStrengthConfig();
		void drawCascadeNumConfig();
		void drawCascadeBiasConfig();
		void drawCascadeDimensionConfig();
		void drawPCFConfig();
		void drawSelf() override;

	private:
		CascadedShadow * mModel;
		nex::gui::TextureView mCascadeView;
	};
}

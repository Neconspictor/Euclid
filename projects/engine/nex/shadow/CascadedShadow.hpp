#pragma once
#include <nex/texture/Texture.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/texture/RenderTarget.hpp>
#include "nex/gui/Drawable.hpp"
#include <nex/buffer/ShaderBuffer.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/gui/TextureView.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/shadow/ShadowCommon.hpp>
#include <nex/util/CallbackContainer.hpp>
#include "interface/shadow/cascade_common.h"

namespace nex
{
	class SceneNearFarComputePass;


	class CascadedShadow
	{
	public:

		using ChangedCallback = CallbackCollection<void(CascadedShadow*)>;

		class CascadeDataPass : public ComputeShader
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

		CascadedShadow(unsigned int cascadeWidth, 
			unsigned int cascadeHeight, 
			unsigned numCascades, 
			const PCFFilter& pcf, 
			float biasMultiplier, 
			bool antiFlickerOn = true,
			float shadowStrength = 0.0f);


		~CascadedShadow();

		std::vector<std::string> generateCsmDefines() const;

		/**
		 * Binds necessary resources (shader, rendertarget, buffers) needed for rendering.
		 */
		void bind(const RenderContext& constants);

		/**
		 * Allows rendering to the i-th cascade.
		 * Note: CascadeShadow has to be bound!
		 */
		void begin(int cascadeIndex);

		void enable(bool enable, bool informObservers = true);

		nex::Texture* getDepthTextureArray();
		const nex::Texture* getDepthTextureArray() const;

		/**
		 * Resizes the cascades
		 */
		void resize(unsigned int cascadeWidth, unsigned int cascadeHeight);
		void resizeCascadeData(unsigned numCascades, bool informObservers = true);

		ChangedCallback::Handle addChangedCallback(const ChangedCallback::Callback& callback);
		void removeChangedCallback(const ChangedCallback::Handle& handle);
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

		bool getUseLogarithmicSplits() const;

		float getBiasMultiplier() const;

		const nex::CascadeData& getCascadeData() const;
		unsigned getNumCascades() const;

		TransformShader* getDepthPass();

		unsigned getHeight() const;

		const PCFFilter& getPCF() const;

		float getShadowStrength()const;

		unsigned getWidth() const;

		const glm::mat4& getWorldToShadowSpace() const;
		const glm::mat4& getShadowView() const;

		void frameReset();

		void render(const nex::RenderCommandQueue::Buffer& shadowCommands,
			const RenderContext& constants);

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

		void useLogarithmicSplits(bool use);


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

		class DepthPass : public TransformShader
		{
		public:
			DepthPass(unsigned numCascades, bool useBones);

			void setCascadeIndexRaw(unsigned index);
			void setCascadeIndex(unsigned index);
			void updateConstants(const RenderContext& constants) override;

		private:
			unsigned mNumCascades;
			unsigned mCascadeIndex;
		};

		GlobalShadow calcShadowSpaceMatrix(const Camera& camera, const glm::vec3& lightDirection);
		
		void calcSplitSchemes(const glm::vec2& minMaxPositiveZ);
		
		void calcSplitDistances(float range, const glm::vec2& minMaxPositiveZ);
		
		bool cascadeNeedsUpdate(const glm::mat4& shadowView, int cascadeIdx, const glm::vec3& newCenter,
			const glm::vec3& oldCenter, float cascadeBoundRadius, glm::vec3* offset);

		BoundingSphere extractFrustumBoundSphere(const Camera& camera, float nearSplitDistance, float farSplitDistance);

		void updateTextureArray();

		void frameUpdateTightNearFarPlane(const Camera& camera, const glm::vec3& lightDirection, nex::ShaderStorageBuffer* minMaxOutputBuffer);
		void frameUpdateNoTightNearFarPlane(const Camera& camera, const glm::vec3& lightDirection, const glm::vec2& minMaxPositiveZ);
		



		std::unique_ptr<DepthPass> mDepthPass;
		std::unique_ptr<DepthPass> mDepthPassBones;
		std::unique_ptr<CascadeDataPass> mDataComputePass;
		std::unique_ptr<SceneNearFarComputePass> mSceneNearFarComputeShader;
		RenderTarget mRenderTarget;

		unsigned int mCascadeWidth;
		unsigned int mCascadeHeight;

		float mShadowMapSize;
		CascadeData mCascadeData;
		unsigned mNumCascades = 4;
		bool mAntiFlickerOn;
		std::vector<float> mSplitDistances;
		std::vector<glm::vec3> mCascadeBoundCenters;
		GlobalShadow mGlobal;
		PCFFilter mPCF;
		ChangedCallback mCallbacks;
		bool mEnabled;
		float mBiasMultiplier;
		float mShadowStrength;
		bool mUseTightNearFarPlane;
		bool mUseLogarithmicSplits;
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

		unsigned mNumCascades;
		std::unique_ptr<nex::gui::ApplyButton> mNumConfigApplyButton;

		float mBias;
		std::unique_ptr<nex::gui::ApplyButton> mBiasApplyButton;

		glm::uvec2 mCascadeDimension;
		std::unique_ptr<nex::gui::ApplyButton> mCascadeDimensioApplyButton;

		PCFFilter mPcf;
		std::unique_ptr<nex::gui::ApplyButton> mPcfApplyButton;
	};
}

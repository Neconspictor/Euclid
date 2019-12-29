#pragma once
#include <nex/renderer/RenderCommand.hpp>
#include <vector>
#include <unordered_set>
#include <nex/math/Sphere.hpp>


namespace nex
{

	class Camera;
	struct Frustum;
	class Shader;

	class RenderCommandQueue
	{
	public:

		using BufferCollection = std::vector<std::vector<RenderCommand>*>;
		using ConstBufferCollection = std::vector<const std::vector<RenderCommand>*>;
		using Buffer = std::vector<RenderCommand>;

		enum class CullingMethod {
			FRUSTUM,
			FRUSTUM_SPHERE
		};

		enum BufferType {
			Deferrable = 1 << 0,
			BeforeTransparent = 1 << 1,
			Forward =  1 << 2,
			Probe = 1 << 3,
			Transparent = 1 << 4,
			Shadow = 1 << 5,
		};

		RenderCommandQueue(Camera* camera = nullptr);

		void clear();


		ConstBufferCollection getCommands(int types) const;

		/**
		 * Provides render commands that are not deferrable 
		 * and should be rendered before transparent commands 
		 * are rendered.
		 */
		Buffer& getBeforeTransparentCommands();
		const Buffer& getBeforeTransparentCommands() const;

		/**
		 * Provides pbr render commands that can be rendered in a deferred way.
		 */
		Buffer& getDeferrablePbrCommands();
		const Buffer& getDeferrablePbrCommands() const;

		/**
		 * Render commands that must be rendered in a forward way.
		 */
		Buffer& getForwardCommands();
		const Buffer& getForwardCommands() const;

		Buffer& getProbeCommands();
		const Buffer& getProbeCommands() const;

		Buffer& getShadowCommands();
		const Buffer& getShadowCommands() const;

		std::multimap<unsigned, nex::RenderCommand>& getToolCommands();
		const std::multimap<unsigned, nex::RenderCommand>& getToolCommands() const;

		Buffer& getTransparentCommands();
		const Buffer& getTransparentCommands() const;

		std::unordered_set<nex::Shader*>& getShaders();
		const std::unordered_set<nex::Shader*>& getShaders() const;

		void push(const RenderCommand& command, bool cull = false);

		void useCameraCulling(Camera* camera);
		void useSphereCulling(const glm::vec3& position, float radius);

		void sort();


	private:

		bool isInRange(bool doCulling, const RenderCommand& command) const;
		bool boxInFrustum(const nex::Frustum& frustum, const nex::AABB& box) const;

		static bool defaultCompare(const RenderCommand& a, const RenderCommand& b);
		bool transparentCompare(const RenderCommand& a, const RenderCommand& b);

		const glm::vec3& getCullPosition() const;

		Buffer mBeforeTransparentCommands;
		Buffer mDeferredPbrCommands;
		Buffer mForwardCommands;
		Buffer mShadowCommands;
		std::multimap<unsigned, RenderCommand> mToolCommands;
		Buffer mTransparentCommands;
		Buffer mProbeCommands;
		std::unordered_set<nex::Shader*> mShaders;
		Camera* mCamera;
		CullingMethod mCullingMethod;
		nex::Sphere mSphereCuller;
	};
}

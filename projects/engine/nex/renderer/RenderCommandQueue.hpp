#pragma once
#include <nex/renderer/RenderCommand.hpp>
#include <vector>
#include <unordered_set>
#include <nex/math/Sphere.hpp>


namespace nex
{

	class Camera;
	struct Frustum;
	class Technique;

	class RenderCommandQueue
	{
	public:

		using BufferCollection = std::vector<std::vector<RenderCommand>*>;
		using Buffer = std::vector<RenderCommand>;

		enum class CullingMethod {
			FRUSTUM,
			FRUSTUM_SPHERE
		};

		enum BufferType {
			Deferrable = 1 << 0,
			Forward =  1 << 1,
			Probe = 1 << 2,
			Transparent = 1 << 3,
			Shadow = 1 << 4,
		};

		RenderCommandQueue(Camera* camera = nullptr);

		void clear();


		BufferCollection getCommands(int types);

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

		std::unordered_set<nex::Technique*>& getTechniques();
		const std::unordered_set<nex::Technique*>& getTechniques() const;

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

		Buffer mDeferredPbrCommands;
		Buffer mForwardCommands;
		Buffer mShadowCommands;
		std::multimap<unsigned, RenderCommand> mToolCommands;
		Buffer mTransparentCommands;
		Buffer mProbeCommands;
		std::unordered_set<nex::Technique*> mTechniques;
		Camera* mCamera;
		CullingMethod mCullingMethod;
		nex::Sphere mSphereCuller;
	};
}

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

		using BufferCollection = std::vector<const std::vector<RenderCommand>*>;
		using Buffer = std::vector<RenderCommand>;

		enum class CullingMethod {
			FRUSTUM,
			FRUSTUM_SPHERE
		};

		enum BufferType {
			Deferrable = 1 << 0,
			Forward =  1 << 1,
			Transparent = 1 << 2,
			Shadow = 1 << 3,
		};

		RenderCommandQueue(Camera* camera = nullptr);

		void clear();


		BufferCollection getCommands(int types) const;

		/**
		 * Provides pbr render commands that can be rendered in a deferred way.
		 */
		const Buffer& getDeferrablePbrCommands() const;

		/**
		 * Render commands that must be rendered in a forward way.
		 */
		const Buffer& getForwardCommands() const;
		const std::multimap<unsigned, nex::RenderCommand>& getToolCommands() const;
		const Buffer& getTransparentCommands() const;
		const Buffer& getShadowCommands() const;
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

		Buffer mPbrCommands;
		Buffer mForwardCommands;
		Buffer mShadowCommands;
		std::multimap<unsigned, RenderCommand> mToolCommands;
		Buffer mTransparentCommands;
		std::unordered_set<nex::Technique*> mTechniques;
		Camera* mCamera;
		CullingMethod mCullingMethod;
		nex::Sphere mSphereCuller;
	};
}

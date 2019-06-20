#pragma once
#include <nex/renderer/RenderCommand.hpp>
#include <vector>
#include <unordered_set>


namespace nex
{

	class Camera;
	struct Frustum;
	class Technique;

	class RenderCommandQueue
	{
	public:

		RenderCommandQueue(Camera* camera = nullptr);

		void clear();

		/**
		 * Provides pbr render commands that can be rendered in a deferred way.
		 */
		const std::vector<RenderCommand>& getDeferrablePbrCommands() const;

		/**
		 * Render commands that must be rendered in a forward way.
		 */
		const std::vector<RenderCommand>& getForwardCommands() const;
		const std::vector<RenderCommand>& getTransparentCommands() const;
		const std::vector<RenderCommand>& getShadowCommands() const;
		const std::unordered_set<nex::Technique*>& getTechniques() const;

		void push(const RenderCommand& command, bool cull = false);

		/**
		 * Sets the camera used for frustum culling
		 */
		void setCamera(Camera* camera);

		void sort();


	private:

		bool isOutsideFrustum(const RenderCommand& command) const;
		static bool boxInFrustum(const nex::Frustum& frustum, const nex::AABB& box);


		static bool defaultCompare(const RenderCommand& a, const RenderCommand& b);
		bool transparentCompare(const RenderCommand& a, const RenderCommand& b);

		std::vector<RenderCommand> mPbrCommands;
		std::vector<RenderCommand> mForwardCommands;
		std::vector<RenderCommand> mShadowCommands;
		std::vector<RenderCommand> mTransparentCommands;
		std::unordered_set<nex::Technique*> mTechniques;
		Camera* mCamera;
	};
}

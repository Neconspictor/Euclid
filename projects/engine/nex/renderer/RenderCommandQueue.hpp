#pragma once
#include <nex/renderer/RenderCommand.hpp>
#include <vector>


namespace nex
{

	class Camera;
	struct Frustum;

	class RenderCommandQueue
	{
	public:

		RenderCommandQueue(Camera* camera = nullptr);

		void clear();

		const std::vector<RenderCommand>& getDeferredCommands() const;
		const std::vector<RenderCommand>& getForwardCommands() const;
		const std::vector<RenderCommand>& getShadowCommands() const;

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

		std::vector<RenderCommand> mDeferredCommands;
		std::vector<RenderCommand> mForwardCommands;
		std::vector<RenderCommand> mShadowCommands;
		Camera* mCamera;
	};
}

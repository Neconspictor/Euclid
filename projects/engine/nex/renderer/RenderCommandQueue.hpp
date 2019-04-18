#pragma once
#include <nex/renderer/RenderCommand.hpp>
#include <vector>


namespace nex
{

	class Camera;

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

		std::vector<RenderCommand> mDeferredCommands;
		std::vector<RenderCommand> mForwardCommands;
		std::vector<RenderCommand> mShadowCommands;
		Camera* mCamera;
	};
}
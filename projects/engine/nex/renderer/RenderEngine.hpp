#pragma once
#include <memory>
#include <nex/common/ConcurrentQueue.hpp>

namespace nex
{
	class RenderEngine
	{
	public:

		using CommandQueue = nex::ConcurrentQueue<std::function<void()>>;

		RenderEngine();

		RenderEngine(const RenderEngine&) = delete;
		RenderEngine(RenderEngine&&) = default;

		RenderEngine& operator=(const RenderEngine&) = delete;
		RenderEngine& operator=(RenderEngine&&) = default;

		virtual ~RenderEngine();

		static CommandQueue* getCommandQueue();

	protected:
	};
}
#include <nex/renderer/RenderEngine.hpp>

nex::RenderEngine::RenderEngine() : mCommandQueue(std::make_shared<CommandQueue>())
{
}

nex::RenderEngine::~RenderEngine() = default;

std::shared_ptr<nex::RenderEngine::CommandQueue> nex::RenderEngine::getCommandQueue() const
	{
		return mCommandQueue;
	}
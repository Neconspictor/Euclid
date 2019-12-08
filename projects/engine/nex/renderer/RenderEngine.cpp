#include <nex/renderer/RenderEngine.hpp>

nex::RenderEngine::RenderEngine()
{
}

nex::RenderEngine::~RenderEngine() = default;

nex::RenderEngine::CommandQueue* nex::RenderEngine::getCommandQueue()
	{
		static CommandQueue commandQueue;
		return &commandQueue;
	}
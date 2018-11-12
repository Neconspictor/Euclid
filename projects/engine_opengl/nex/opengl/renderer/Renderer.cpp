#include <nex/renderer/Renderer.hpp>

Renderer::Renderer(Backend backend) : m_renderBackend(std::move(backend)) {
	
}
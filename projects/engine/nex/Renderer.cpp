#include <nex//Renderer.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>

using namespace std;

Renderer::Renderer() : 
	logClient(nex::getLogServer()), width(0), height(0), xPos(0), yPos(0)
{
	logClient.setPrefix("[Renderer]");
}

Renderer::Viewport Renderer::getViewport() const
{
	return { xPos, yPos, width, height };
}

void Renderer::setViewPort(int x, int y, int width, int height)
{
	xPos = x;
	yPos = y;
	this->width = width;
	this->height = height;
}

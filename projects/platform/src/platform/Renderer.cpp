#include <platform//Renderer.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;

Renderer::Renderer() : 
	logClient(platform::getLogServer()), width(0), height(0), xPos(0), yPos(0)
{
	logClient.setPrefix("[Renderer]");
}

void Renderer::setViewPort(int x, int y, int width, int height)
{
	xPos = x;
	yPos = y;
	this->width = width;
	this->height = height;
	init();
}
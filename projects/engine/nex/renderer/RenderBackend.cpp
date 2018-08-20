#include <nex/renderer/RenderBackend.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>

using namespace std;

RenderBackend::RenderBackend() : 
	logClient(nex::getLogServer()), width(0), height(0), xPos(0), yPos(0)
{
	logClient.setPrefix("[RenderBackend]");
}

Viewport RenderBackend::getViewport() const
{
	return { xPos, yPos, width, height };
}

void RenderBackend::setViewPort(int x, int y, int width, int height)
{
	xPos = x;
	yPos = y;
	this->width = width;
	this->height = height;
}
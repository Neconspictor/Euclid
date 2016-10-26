#ifndef ABSTRACT_RENDERER_FACTORY
#define ABSTRACT_RENDERER_FACTORY

#include "Renderer.hpp"
#include "Platform.hpp"

class AbstractRendererFactory
{
public:
	virtual ~AbstractRendererFactory(){}

	virtual Renderer* createRenderer(Platform const& platform) = 0;

	/**
	 * Creates a new renderer for a given platform and connects it to a window specified by the handle.
	 * All drawing actions performed by this renderer will be displayed by this window.
	 */
	virtual Renderer* createRenderer(Platform const& platform, int handle) = 0;
};

#endif
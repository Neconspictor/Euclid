#pragma once

#include <platform/extern/imgui/imgui.h>

/**
 * An interface for concrete render implementations of ImGUI 
 * https://github.com/ocornut/imgui
 */
class ImGUI_Impl
{
public:
	virtual ~ImGUI_Impl() = default;

	virtual void newFrame() = 0;
	
	virtual void renderDrawData(ImDrawData* draw_data) = 0;
	
	virtual void shutdown() = 0; // Todo do shutdown in destructor?
};
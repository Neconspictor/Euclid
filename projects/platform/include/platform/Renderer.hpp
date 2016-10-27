#ifndef RENDERER_HPP
#define RENDERER_HPP

class Platform;
//class Entity;
#include <ostream>

enum RendererType : char
{
	OPENGL,
	DIRECTX
};

inline std::ostream& operator<< (std::ostream & os, RendererType type)
{
	switch (type)
	{
	case OPENGL: return os << "OpenGL";
	case DIRECTX: return os << "DirectX";
		// omit default case to trigger compiler warning for missing cases
	};
	return os << static_cast<std::uint16_t>(type);
}


class Renderer
{
public:

	virtual ~Renderer()
	{
	}

	virtual void init() = 0;
	virtual void beginScene() = 0;
	virtual void endScene() = 0;
	//virtual void addEntityToScene(Entity* entity) = 0;
	virtual void present() = 0;
	virtual void release() = 0;
	virtual RendererType getType() = 0;

	void Renderer::setShaderBaseFolder(std::string baseFolder);

};
#endif
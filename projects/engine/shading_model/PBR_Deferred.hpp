#ifndef PBR_DEFERRED_HPP
#define PBR_DEFERRED_HPP

#include <texture/Texture.hpp>
#include <renderer/Renderer3D.hpp>
#include <model/Vob.hpp>
#include<shader/PBRShader.hpp>
#include <scene/SceneNode.hpp>
#include <light/Light.hpp>
#include <sprite/Sprite.hpp>
#include <shading_model/PBR.hpp>

class PBR_Deferred : public PBR {

public:
	PBR_Deferred();
  virtual ~PBR_Deferred();

protected:
};

#endif
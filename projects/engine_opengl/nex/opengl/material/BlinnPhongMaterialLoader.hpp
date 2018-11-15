#pragma once

#include <nex/opengl/material/AbstractMaterialLoader.hpp>


class BlinnPhongMaterialLoader : public AbstractMaterialLoader {

public:

	BlinnPhongMaterialLoader(TextureManagerGL* textureManager);

	virtual ~BlinnPhongMaterialLoader();

	std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

private:

	nex::Logger m_logger;

};
#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/logging/LoggingClient.hpp>


class BlinnPhongMaterialLoader : public AbstractMaterialLoader {

public:

	BlinnPhongMaterialLoader(TextureManager* textureManager);

	virtual ~BlinnPhongMaterialLoader();

	virtual std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

private:

	nex::LoggingClient logClient;

};
#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/logging/LoggingClient.hpp>


class PbrMaterialLoader : public AbstractMaterialLoader {

public:

	PbrMaterialLoader(TextureManager* textureManager);

	virtual ~PbrMaterialLoader();

	virtual std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

private:

	nex::LoggingClient logClient;

};
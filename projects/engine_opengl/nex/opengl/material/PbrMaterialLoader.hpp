#pragma once

#include <nex/opengl/material/AbstractMaterialLoader.hpp>
#include <nex/logging/LoggingClient.hpp>


class PbrMaterialLoader : public AbstractMaterialLoader {

public:

	PbrMaterialLoader(TextureManagerGL* textureManager);

	virtual ~PbrMaterialLoader();

	std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

private:

	nex::LoggingClient logClient;

};
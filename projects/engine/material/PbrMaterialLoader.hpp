#pragma once

#include <material/AbstractMaterialLoader.hpp>
#include <platform/logging/LoggingClient.hpp>


class PbrMaterialLoader : public AbstractMaterialLoader {

public:

	PbrMaterialLoader(TextureManager* textureManager);

	virtual ~PbrMaterialLoader();

	virtual std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

private:

	platform::LoggingClient logClient;

};
#pragma once

#include <material/AbstractMaterialLoader.hpp>
#include <platform/logging/LoggingClient.hpp>


class BlinnPhongMaterialLoader : public AbstractMaterialLoader {

public:

	BlinnPhongMaterialLoader(TextureManager* textureManager);

	virtual ~BlinnPhongMaterialLoader();

	virtual std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

private:

	platform::LoggingClient logClient;

};
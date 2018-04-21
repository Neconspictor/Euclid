#ifndef BLINN_PHONG_MATERIAL_LOADER_HPP
#define BLINN_PHONG_MATERIAL_LOADER_HPP

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

#endif ABSTRACT_MATERIAL_LOADER_HPP
#pragma once

#include <model/opengl/BaseAssimpModelLoader.hpp>

class BlinnPhongMaterialLoader : public AssimpModelLoader {

public:

	BlinnPhongMaterialLoader();

	virtual ~BlinnPhongMaterialLoader();

protected:
	virtual std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

private:

	platform::LoggingClient logClient;

};
#pragma once

#include <nex/opengl/material/AbstractMaterialLoader.hpp>
#include <nex/common/Log.hpp>


namespace nex
{
	class BlinnPhongMaterialLoader : public nex::AbstractMaterialLoader {

	public:

		BlinnPhongMaterialLoader(TextureManagerGL* textureManager);

		virtual ~BlinnPhongMaterialLoader();

		std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const override;

	private:

		nex::Logger m_logger;

	};
}
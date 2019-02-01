#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/common/Log.hpp>


namespace nex
{
	class BlinnPhongMaterialLoader : public nex::AbstractMaterialLoader {

	public:

		BlinnPhongMaterialLoader(TextureManager* textureManager);

		virtual ~BlinnPhongMaterialLoader() = default;

		std::vector<std::unique_ptr<Material>> loadShadingMaterial(const aiScene* scene) const override;

	private:

		nex::Logger m_logger;

	};
}
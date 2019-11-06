#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <filesystem>

namespace nex
{
	/**
	 * A 3d scene imported from 3d application exchange file formats (e.g. *.obj, *.3ds, *.fbx, etc...).
	 */
	class ImportScene
	{
	public:
		ImportScene() = default;
		~ImportScene() = default;

		ImportScene(ImportScene&&) = default;
		ImportScene& operator=(ImportScene&&) = default;

		ImportScene(const ImportScene&) = delete;
		ImportScene& operator=(const ImportScene&) = delete;

		/**
		 * Reads a 3d scene from file and processes it with default applied assimp flags.
		 * @throws std::runtime_error : If the scene couldn't be loaded.
		 */
		static ImportScene read(const std::filesystem::path& file);

		const std::filesystem::path& getFilePath()const;
		const aiScene* getAssimpScene()const;

	private:
		Assimp::Importer mImporter;
		const aiScene* mAssimpScene;
		std::filesystem::path mFile;
	};
}
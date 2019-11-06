#include <nex/import/ImportScene.hpp>
#include <assimp/postprocess.h>
#include <nex/common/Log.hpp>
#include <nex/util/ExceptionHandling.hpp>


nex::ImportScene nex::ImportScene::read(const std::filesystem::path& file) {

	ImportScene importScene;

	const aiScene* scene = importScene.mImporter.ReadFile(file.generic_string(),
		aiProcess_Triangulate
		//| aiProcess_FlipUVs
		| aiProcess_GenSmoothNormals
		| aiProcess_CalcTangentSpace
		| aiProcessPreset_TargetRealtime_MaxQuality);


	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		nex::Logger logger("ImportScene");
		LOG(logger, nex::Error) << "read: " << importScene.mImporter.GetErrorString();
		std::stringstream ss;
		ss << "read: Couldn't load assimp scene: " << file;
		throw_with_trace(std::runtime_error(ss.str()));
	}

	importScene.mFile = file;

	return importScene;
}

const std::filesystem::path& nex::ImportScene::getFilePath() const
{
	return mFile;
}

const aiScene* nex::ImportScene::getAssimpScene() const
{
	return mAssimpScene;
}
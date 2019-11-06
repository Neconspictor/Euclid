#pragma once

#include <nex/anim/Rig.hpp>
#include <nex/exception/ResourceLoadException.hpp>

struct aiScene;
struct aiNode;
struct aiString;

namespace nex
{
	class ImportScene;

	/**
	 * Loads an animation rig from an ImportScene.
	 */
	class RigLoader {
	public:
		/**
		 * @throws nex::ResourceLoadException : if the scene contains a malformed rig.
		 */
		Rig load(const ImportScene& scene);

	private:

		const aiNode* findByName(const aiScene* scene, const aiString& name) const;

		/**
		 * @throws nex::ResourceLoadException : if the scene contains a malformed node hierarchy.
		 */
		const aiNode* getRootBone(const aiScene* scene, const std::vector<const aiBone*>& bones) const;
		
		/**
		 * @throws nex::ResourceLoadException : if the scene contains bones with the same name
		 */
		std::vector<const aiBone*> getBones(const aiScene* scene) const;

		bool isBoneNode(const aiNode* node, const std::vector<const aiBone*>& bones) const;

		Bone create(const aiBone* bone);
	};
}
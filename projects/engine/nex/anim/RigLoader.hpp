#pragma once

#include <nex/anim/Rig.hpp>
#include <nex/exception/ResourceLoadException.hpp>

struct aiScene;
struct aiNode;
struct aiBone;
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
		std::unique_ptr<nex::Rig> load(const ImportScene& scene, const std::string& id);

	private:

		const aiNode* findByName(const aiScene* scene, const aiString& name) const;

		/**
		 * @throws nex::ResourceLoadException : if the scene contains a malformed node hierarchy.
		 */
		const aiNode* getRootBone(const aiScene* scene, const std::vector<const aiNode*>& bones) const;
		
		/**
		 * @throws nex::ResourceLoadException : if the scene contains bones with the same name
		 */
		std::vector<const aiNode*> getBones(const ImportScene& scene) const;
		std::vector<const aiBone*> getBonesWithAssignedVertices(const ImportScene& scene) const;

		const aiBone* getBone(const aiNode* node, const std::vector<const aiBone*>& bones) const;

		bool isBoneWithAssignedVertices(const aiNode* node, const std::vector<const aiBone*>& bones) const;

		std::unique_ptr<nex::BoneData> create(const aiNode* boneNode, const aiBone* bone, const glm::mat4& invRootNodeTrafo) const;

		/**
		 * Invokes a function for the whole node hierarchy.
		 *  - The first argument of the function has to be a const aiNode*.
		 *  - The invoked function has to return a bool indicating whether iteration should be continued.
		 */
		template<class Func, typename... Args>
		static void for_each(const aiNode* root, Func& func, Args&&... args) {

			std::queue<const aiNode*> nodes;
			nodes.push(root);
			while (!nodes.empty()) {
				auto* node = nodes.front();
				nodes.pop();

				if (!std::invoke(func, node, std::forward<Args>(args)...)) {
					break;
				}

				// push children
				for (int i = 0; i < node->mNumChildren; ++i) {
					nodes.push(node->mChildren[i]);
				}
			}
		}
	};
}
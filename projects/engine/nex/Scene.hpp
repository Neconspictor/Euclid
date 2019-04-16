#pragma once
#include <set>
#include <nex/mesh/Vob.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <vector>
#include <memory>

namespace nex
{
	class Mesh;
	class Material;

	template<class Iterator>
	struct IteratorRange
	{
		Iterator begin;
		Iterator end;
	};


	class SceneNode
	{
	public:

		using Children = IteratorRange<std::set<SceneNode*>::const_iterator>;

		SceneNode();

		void addChild(SceneNode* node);
		void clear();
		Children getChildren() const;
		Mesh* getMesh() const;
		Material* getMaterial() const;

		const glm::mat4& getLocalTrafo() const;
		const glm::mat4& getWorldTrafo() const;
		void removeChild(SceneNode* node);
		void setMesh(Mesh* mesh);
		void setMaterial(Material* material);
		void setParent(SceneNode* parent);
		void setLocalTrafo(const glm::mat4& trafo);
		void setWorldTrafo(const glm::mat4& trafo);


		void updateChildrenWorldTrafos();
		void updateWorldTrafoHierarchy();
		void setPositionLocal(glm::vec3 position);

	private:

		void updateWorldTrafo();

		std::set<SceneNode*> mChildren;
		Mesh* mMesh;
		Material* mMaterial;
		SceneNode* mParent;
		glm::mat4 mLocalTrafo;
		glm::mat4 mWorldTrafo;
	};

	/**
	 * A scene manages the creation and lifetime of scene nodes.
	 */
	class Scene
	{
	public:

		/**
		 * Creates a new scene object.
		 */
		Scene();

		/**
		 * Creates a new node. the node is valid until the function clear is called or the scene object is destroyed.
		 */
		SceneNode* createNode(SceneNode* parent = nullptr);

		/**
		 * Provides the root node.
		 */
		SceneNode* getRoot() const;

		/**
		 * Deletes all nodes except the root node.
		 */
		void clear();

	private:
		std::unique_ptr<SceneNode> mRoot;
		std::vector<std::unique_ptr<SceneNode>> mNodes;
	};
}
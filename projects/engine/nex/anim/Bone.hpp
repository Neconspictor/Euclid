#pragma once

#include <string>
#include <glm/glm.hpp>

namespace nex
{
	/**
	 * Represents a bone of an animation rig.
	 */
	class Bone {
		
		bool operator==(const Bone& bone);

		/**
		 * Provides the name of this bone.
		 */
		const std::string& getName()const;

		/**
		 * Sets the name of this bone.
		 */
		void setName(const std::string& name);

		/**
		 * Provides the transformation matrix that goes from the Bone Space
		 * to the space of the parent bone. If this bone has no parent, it provides 
		 * a transformation matrix to (mesh) local space.
		 */
		const glm::mat4& getBoneToParentTrafo()const;
		
		/**
		 * Sets the bone to parent space trafo.
		 */
		void setBoneToParentTrafo(const glm::mat4& mat);

		/**
		 * Checks if this bone is a root bone.
		 * A root bone is a bone with no parent.
		 */
		bool isRoot() const;

		/**
		 * Provides the parent bone.
		 */
		const Bone* getParent() const;
		Bone* getParent();

		/**
		 * Provides the children bones.
		 */
		const std::vector<Bone>& getChildren() const;
		std::vector<Bone>& getChildren();

	private:
		std::string mName;
		glm::mat4 mBoneToParentTrafo;
		Bone* mParent = nullptr;
		std::vector<Bone> mChildren;
	};
}
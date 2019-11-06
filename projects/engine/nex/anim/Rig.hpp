#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace nex
{
	/**
	 * Represents a bone of an animation rig.
	 * A bone inside a rig is uniquely identified by its name.
	 */
	class Bone {

	public:
		using BoneVec = std::vector<std::unique_ptr<Bone>>;

		/**
		 * Creates a new Bone object.
		 * @throws std::invalid_argument : if name is an empty string.
		 */
		Bone(const std::string& name);

		Bone(const Bone&) = default;
		Bone& operator=(const Bone&) = default;
		
		Bone(Bone&&) = default;
		Bone& operator=(Bone&&) = default;

		~Bone() = default;
		
		bool operator==(const Bone& bone);

		/**
		 * Adds a child bone.
		 * @throws std::invalid_argument : if the bone or any child bone has the same name as the bone to be added.
		 */
		void addChild(std::unique_ptr<Bone> bone);

		/**
		 * Provides a bone from the hierarchy identified by its unique name.
		 */
		const Bone* getByName(const std::string& name) const;
		Bone* getByName(const std::string& name);

		/**
		 * Provides a bone from the hierarchy identified by its name SID.
		 */
		const Bone* getBySID(unsigned sid) const;
		Bone* getBySID(unsigned sid);

		/**
		 * Provides the SID of the bone.
		 */
		unsigned getSID() const;

		/**
		 * Provides the name of this bone.
		 */
		const std::string& getName() const;

		/**
		 * Checks if the bone has the specified name.
		 */
		bool hasName(const std::string& name)  const;

		/**
		 * Checks if the specified SID matches the SID of the bone's name.
		 */
		bool hasSID(unsigned sid) const;

		/**
		 * Sets the name of this bone.
		 * @throws std::invalid_argument : if name is an empty string.
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

		void setParent(Bone* parent);

		/**
		 * Provides the children bones.
		 */
		const BoneVec& getChildren() const;
		BoneVec& getChildren();

	private:
		std::string mName;
		unsigned mNameSID = 0;
		glm::mat4 mBoneToParentTrafo;
		Bone* mParent = nullptr;
		BoneVec mChildren;
	};


	/**
	 * An (animation) rig manages a hierarchy of bones.
	 * Each bone in the hierarchy has a unique name. 
	 */
	class Rig 
	{
	public:

		Rig(std::unique_ptr<Bone> root);

		/**
		 * Provides the root bone of the hierarchy.
		 */
		const Bone* getRoot() const;
		Bone* getRoot();

		/**
		 * Provides a bone from the hierarchy identified by its unique name.
		 */
		const Bone* getByName(const std::string& name) const;
		Bone * getByName(const std::string& name);

		/**
		 * Provides a bone from the hierarchy identified by its name SID.
		 */
		const Bone* getBySID(unsigned sid) const;
		Bone* getBySID(unsigned sid);

		/**
		 * Adds a bone to the hierarchy. 
		 * @throws std::invalid_argument :  - if any bone in the hierarchy has the same name as the bone to be added.
		 *									- if the parent bone isn't present in the hierarchy.
		 */
		void addBone(std::unique_ptr<Bone> bone, unsigned parentSID);
		void addBone(std::unique_ptr<Bone> bone, const std::string& parentName);

	private:

		std::unique_ptr<Bone> mRoot;
	};
}
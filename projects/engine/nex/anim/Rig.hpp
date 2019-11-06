#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace nex
{
	class Rig;

	/**
	 * Represents a bone weight for a vertex of a mesh
	 */
	/*struct Weight {

		// The index of the vertex
		unsigned index;

		// The weight; Specifies how much the vertex gets influenced by the Bone.
		float weight;
	};*/

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
		 * Provides the bind-pose trafo. The bind-pose trafo transforms a from mesh space to bind-pose space.
		 */
		const glm::mat4& getBindPoseTrafo()const;
		
		/**
		 * Sets the bind-pose trafo. The bind-pose trafo transforms a from mesh space to bind-pose space.
		 */
		void setBindPoseTrafo(const glm::mat4& mat);

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

		/**
		 * Provides the bone id of this bone.
		 * The bone id is an optimization for rendering is used as an index
		 * into an array of bone transformation matrices. This allows fast access to 
		 * the bone's accumulated transformations matrix.
		 */
		unsigned getBoneID() const;

		//TODO move to mesh class
		//const std::vector<Weight>& getWeights() const;
		//std::vector<Weight>& getWeights();

	private:
		friend Rig;
			/**
		 * Sets the bone id of this bone.
		 * NOTE: This function should only be called by the Rig class.
		 */
			void setBoneID(unsigned id);

	private:
		std::string mName;
		unsigned mNameSID = 0;
		unsigned mBoneID;
		glm::mat4 mBindPoseTrafo;
		Bone* mParent = nullptr;
		BoneVec mChildren;

		//std::vector<Weight> mWeights;
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
		 * Provides the number of bones this rig has.
		 */
		unsigned getBoneCount() const;

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

		/**
		 * Optimizes Rig for rendering (e.g. bone id assignment).
		 */
		void optimize();

		void setRoot(std::unique_ptr<Bone> bone);

	private:

		void recalculateBoneCount();

		std::unique_ptr<Bone> mRoot;
		unsigned mBoneCount;
	};
}
#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace nex
{
	class RigData;
	class BoneData;

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
	class Bone 
	{
	public:

		/**
		 * Defines the maximum number of (direct) children a bone can have.
		 * A maximum is needed, since shaders need POD.
		 */
		static constexpr unsigned short MAX_CHILDREN_SIZE = 10;

		/**
		 * Creates a new Bone object from a BoneData object.
		 * NOTE: the bone argument has to be optimized (BoneData::isOptimized())!
		 * @throws nex::ResourceLoadException : if "bone" isn't optimized or if "bone" is malformed 
		 *										(e.g. more childrens than MAX_CHILDREN_SIZE). 
		 */
		Bone(const BoneData& bone);

		/**
		 * Creates an uninitialized and not usable Bone object.
		 * NOTE: DO NOT USE. This constructor is only used for technical reasons (e.g. vector resizing etc.) 
		 */
		Bone() = default;

		/**
		 * Provides the bind-pose transformation matrix for this bone.
		 */
		const glm::mat4& getBindPoseTrafo() const;

		/**
		 * Provides the number of (direct) bone children.
		 */
		unsigned short getChildrenCount() const;

		/**
		 * Provides the array indices of the bone children.
		 * NOTE: Query getChildrenCount() to retrieve the number of children.
		 */
		const std::array<short, MAX_CHILDREN_SIZE>& getChildrenIDs() const;

		/**
		 * Provides the bone id of this bone.
		 * The bone id is an optimization for rendering is used as an index
		 * into an array of bone transformation matrices. This allows fast access to
		 * the bone's accumulated transformations matrix.
		 */
		short getID() const;

		//unsigned getNameSID() const;

		/**
		 * Provides the bone array index of the parent bone.
		 * If this bone hasn't got a parent, the result will be negative.
		 */
		short getParentID() const;
	
	private:
		//unsigned mNameSID;
		short mBoneID;
		short mParentID;
		glm::mat4 mBindPoseTrafo;
		std::array<short, MAX_CHILDREN_SIZE> mChildrenIDs;
		unsigned short mChildrenCount;
	};

	/**
	* An (animation) rig manages a hierarchy of bones.
	* Each bone in the hierarchy has a unique name.
	* Used for constructing a Rig object.
	*/
	class Rig
	{
	public:
		
		/**
		 * Creates a new Rig object from a RigData object.
		 * @throws nex::ResourceLoadException : if the RigData is malformed (e.g. not optimized).
		 */
		Rig(const RigData& data);

		/**
		 * Provides the array of bones.
		 */
		const std::vector<Bone>& getBones() const;

		/**
		 * Provides the SIDs of the bones.
		 */
		const std::vector<unsigned> getSIDs() const;

		/**
		 * Searches a bone by its name.
		 * Time complexity: O(N)
		 */
		const Bone* getByName(const std::string& name) const;

		/**
		 * Searches a bone by its SID.
		 * Time complexity: O(N)
		 */
		const Bone* getBySID(unsigned sid) const;

	private:
		std::vector<Bone> mBones;
		std::vector<unsigned> mSIDs;
	};


	/**
	 * Used for constructing a Rig object.
	 */
	class RigData 
	{
	public:

		RigData();

		/**
		 * Provides the number of bones this rig has.
		 */
		unsigned getBoneCount() const;

		/**
		 * Provides the root bone of the hierarchy.
		 */
		const BoneData* getRoot() const;
		BoneData* getRoot();

		/**
		 * Provides a bone from the hierarchy identified by its unique name.
		 */
		const BoneData* getByName(const std::string& name) const;
		BoneData* getByName(const std::string& name);

		/**
		 * Provides a bone from the hierarchy identified by its name SID.
		 */
		const BoneData* getBySID(unsigned sid) const;
		BoneData* getBySID(unsigned sid);

		/**
		 * Adds a bone to the hierarchy. 
		 * @throws std::invalid_argument :  - if any bone in the hierarchy has the same name as the bone to be added.
		 *									- if the parent bone isn't present in the hierarchy.
		 */
		void addBone(std::unique_ptr<BoneData> bone, unsigned parentSID);
		void addBone(std::unique_ptr<BoneData> bone, const std::string& parentName);

		/**
		 * Checks if this RigData is optimized.
		 */
		bool isOptimized() const;

		/**
		 * Optimizes RigData for rendering (e.g. bone id assignment).
		 */
		void optimize();

		/**
		 * Sets the root for thsi RigData.
		 */
		void setRoot(std::unique_ptr<BoneData> bone);

	private:

		/**
		 * Recalculates the total number of bones in the hierarchy.
		 */
		void recalculateBoneCount();

		std::unique_ptr<BoneData> mRoot;
		unsigned mBoneCount;
		bool mOptimized;
	};



	/**
	 * Used for constructing Bone objects.
	 */
	class BoneData {

	public:
		using BoneVec = std::vector<std::unique_ptr<BoneData>>;

		/**
		 * Creates a new Bone object.
		 * @throws std::invalid_argument : if name is an empty string.
		 */
		BoneData(const std::string& name = "");

		BoneData(const BoneData&) = default;
		BoneData& operator=(const BoneData&) = default;

		BoneData(BoneData&&) = default;
		BoneData& operator=(BoneData&&) = default;

		~BoneData() = default;

		bool operator==(const BoneData& bone);

		/**
		 * Adds a child bone.
		 * @throws std::invalid_argument : if the bone or any child bone has the same name as the bone to be added.
		 */
		void addChild(std::unique_ptr<BoneData> bone);


		/**
		 * Invokes a function for the whole bone hierarchy (bone, bone children, children of bone children...)
		 *  - The first argument of the function has to be a Bone pointer.
		 *  - The invoked function has to return a bool indicating whether iteration should be continued.
		 */
		template<class Func, typename... Args>
		void for_each(Func& func, Args&&... args) {

			std::queue<BoneData*> bones;
			bones.push(this);
			while (!bones.empty()) {
				auto* bone = bones.front();
				bones.pop();

				if (!std::invoke(func, bone, std::forward<Args>(args)...)) {
					break;
				}

				for (auto& child : bone->getChildren()) {
					bones.push(child.get());
				}
			}
		}

		/**
		 * const version of the for_each function.
		 * TODO merge duplicate function.
		 */
		template<class Func, typename... Args>
		void for_each(Func& func, Args&&... args) const {

			std::queue<const BoneData*> bones;
			bones.push(this);
			while (!bones.empty()) {
				auto* bone = bones.front();
				bones.pop();

				if (!std::invoke(func, bone, std::forward<Args>(args)...)) {
					break;
				}

				for (auto& child : bone->getChildren()) {
					bones.push(child.get());
				}
			}
		}

		/**
		 * Provides the bind-pose trafo. The bind-pose trafo transforms a from mesh space to bind-pose space.
		 */
		const glm::mat4& getBindPoseTrafo()const;

		/**
		 * Provides a bone from the hierarchy identified by its unique name.
		 */
		const BoneData* getByName(const std::string& name) const;
		BoneData* getByName(const std::string& name);

		/**
		 * Provides a bone from the hierarchy identified by its name SID.
		 */
		const BoneData* getBySID(unsigned sid) const;
		BoneData* getBySID(unsigned sid);

		/**
		 * Provides the SID of the bone.
		 */
		unsigned getSID() const;

		/**
		 * Checks if the bone has the specified name.
		 */
		bool hasName(const std::string& name)  const;

		/**
		 * Checks if the specified SID matches the SID of the bone's name.
		 */
		bool hasSID(unsigned sid) const;

		/**
		 * Checks, if the BoneData is optimized and thus ready for initializing a Bone object.
		 */
		bool isOptimized() const;

		/**
		 * Checks if this bone is a root bone.
		 * A root bone is a bone with no parent.
		 */
		bool isRoot() const;

		/**
		 * Provides the bone id of this bone.
		 * The bone id is an optimization for rendering is used as an index
		 * into an array of bone transformation matrices. This allows fast access to
		 * the bone's accumulated transformations matrix.
		 */
		short getBoneID() const;

		/**
		 * Provides the children bones.
		 */
		const BoneVec& getChildren() const;
		BoneVec& getChildren();

		/**
		 * Provides the parent bone.
		 */
		const BoneData* getParent() const;
		BoneData* getParent();


		//TODO move to mesh class
		//const std::vector<Weight>& getWeights() const;
		//std::vector<Weight>& getWeights();

		/**
		 * Sets the bind-pose trafo. The bind-pose trafo transforms a from mesh space to bind-pose space.
		 */
		void setBindPoseTrafo(const glm::mat4& mat);

		/**
		 * Sets the name of this bone.
		 * @throws std::invalid_argument : if name is an empty string.
		 */
		void setName(const std::string& name);

		/**
		 * Sets the parent of the bone. Can be nullptr to indicate that the bone has no parent.
		 */
		void setParent(BoneData* parent);

	private:
		friend RigData;

		/**
		 * Initializes acceleration data structures
		 * NOTE: This function should only be called by the RigData class.
		 */
		void optimize(short id);

	private:
		unsigned mNameSID = 0;
		short mBoneID;
		glm::mat4 mBindPoseTrafo;
		BoneData* mParent = nullptr;
		BoneVec mChildren;
		bool mOptimized;

		//std::vector<Weight> mWeights;
	};
}
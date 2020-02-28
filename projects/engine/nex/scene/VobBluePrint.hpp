#pragma once

#include <nex/scene/Vob.hpp>


namespace nex
{
	class Mesh;
	class MeshGroup;
	class MeshBatch;
	class Rig;
	class BoneAnimation;

	class VobBluePrint {
	public:

		/**
		 * Creates a new blue-print.
		 * @throws std::invalid_argument : if the unique_ptr of the bluePrint vob is empty. 
		 */
		VobBluePrint(std::unique_ptr<Vob> bluePrint);

		/**
		 * Creates a vob hierarchy using the blue-print. The resulting vob will be connected to this blue-print.
		 */
		std::unique_ptr<Vob> createBluePrint() const;


		/**
		 * Creates a channel id generator needed for initializing a keyframe animation that should be usable by vobs of this blue-print.
		 */
		std::unique_ptr<KeyFrameAnimation::ChannelIDGenerator> createGenerator() const;

		/**
		 * Adds a vector of keyframe animations pairs to this blue-print.
		 * Note: it won't be checked if the keyframe animations are applicable to vob of this blue-print! 
		 */
		void addKeyFrameAnimations(std::vector<std::unique_ptr<KeyFrameAnimation>> anis);
		//@throws std::invalid_argument : If one of the keyframe animations is not applicable to vobs of this blue - print.

		/**
		 * Provides for each vob of the blue-print vob hierarchy an inverse parent-to-world trafo.
		 */
		const std::vector<glm::mat4>& getInverseParentToWorldTrafos() const;
		
		/**
		 * Provides all registered keyframe animations. The Sid key is the SID of the keyframe animations name.
		 */
		const std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>>& getKeyFrameAnimations() const;

		/**
		 * Provides a lexicographical (by name) sorted vector of keframe animations 
		 */
		const std::vector<KeyFrameAnimation*>& getKeyFrameAnimationsSorted() const;

		/**
		 * Provides the mapping of vob node name SIDs to matric array indices.
		 */
		const std::unordered_map<nex::Sid, unsigned>& getMapping() const;

		/**
		 * The maximum number of channels for keyframe animations.
		 */
		unsigned getMaxChannelCount() const;

		/**
		 * Provides the name SID of the blue print root vob.
		 */
		nex::Sid getBluePrintRootNameSID() const;

		/**
		 * Returns the matrix array index for a keyframe animation matrix array for a vob which is connected to this blue-print.
		 * @throws std::invalid_argument : If the vob isn't connected to this blue-print.
		 */
		unsigned mapToMatrixArrayIndex(const nex::Vob& vob) const;

		/**
		 * Returns the matrix array index for a keyframe animation matrix array for a vob node represented by its node name SID.
		 * @throws std::invalid_argument : If the sid isn't mapped to an array index.
		 */
		unsigned mapToMatrixArrayIndex(nex::Sid sid) const;

	private:
		std::unique_ptr<Vob> mVob;
		std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>> mKeyFrameAnis;
		std::vector<KeyFrameAnimation*> mKeyFrameAnisSorted;
		nex::Sid mBluePrintRootNameSID;
		std::vector<glm::mat4> mInverseParentToWorldTrafos;

		// Maps array index of a matrix array to vob name SIDs of the blue print vob.
		// This mapping is needed for keyframe animations, which need such a mapping since they operate on a channel to matrix index mapping.
		// Vobs using this blue print can use this mapping for mapping their own vob hierarchy to the matrix array calculated by the keyframe animation object.
		// Note: Each child vob created by the blue print has a name SID to a (child) vob of the blue-print vob. This way it is guaranteed to have a consistent 
		// mapping even if the vob's name is changed afterwards.
		//std::unordered_map<unsigned, nex::Sid> mMatrixIndexToBluePrintChildVobNameSID;
		std::unordered_map<nex::Sid, unsigned> mBluePrintChildVobNameSIDToMatrixIndex;

		int fillMap(const nex::Vob& vob, int currentIndex);
		void createSortedAnis();
		void calcInverseParentToWorldTrafos(std::vector<glm::mat4>& trafos, Vob* root);
	};
}
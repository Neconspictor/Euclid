#pragma once

#include <nex/anim/KeyFrameAnimation.hpp>

namespace nex
{
	class Rig;
	class BoneAnimationData;

	class BoneAnimation : public KeyFrameAnimation {

	public:		

		BoneAnimation(const BoneAnimationData& data);

		virtual ~BoneAnimation() = default;

		/**
		 * Propagates matrix transformations from parent bone to children bones.
		 * Note: input and output is expected to be in bone space.
		 */
		void applyParentHierarchyTrafos(std::vector<glm::mat4>& vec) const;


		/**
		 * Provides the rig this bone animation belongs to.
		 */
		const Rig* getRig() const;

		/**
		 * Provides the id of the rig this bone animation belongs to.
		 */
		const std::string& getRigID() const;

		/**
		 * Provides the sid of the rig this bone animation belongs to.
		 */
		unsigned getRigSID() const;

		void write(nex::BinStream& out) const override;
		void load(nex::BinStream& in) override;

		/**
		 * Creates an unintialized animation, that is not usable.
		 * NOTE: Use at own risk!
		 */
		static BoneAnimation createUnintialized();

	protected:

		BoneAnimation() = default;

		std::string mRigID;
		unsigned mRigSID;
	};

	class BoneAnimationData : public KeyFrameAnimationData
	{
	public:

		virtual ~BoneAnimationData() = default;

		using Sid = unsigned;

		/**
		 * Sets the rig this animation references to.
		 */
		void setRig(const Rig* rig);


		const Rig* mRig = nullptr;
	};

	nex::BinStream& operator>>(nex::BinStream& in, BoneAnimation& ani);
	nex::BinStream& operator<<(nex::BinStream& out, const BoneAnimation& ani);
}
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

		VobBluePrint(std::unique_ptr<Vob> bluePrint);

		std::unique_ptr<Vob> createBluePrint() const;


		void addKeyFrameAnimations(std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>> anis);
		const std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>>& getKeyFrameAnimations() const;

	private:
		std::unique_ptr<Vob> mVobHierarchy;
		std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>> mKeyFrameAnis;
	};
}
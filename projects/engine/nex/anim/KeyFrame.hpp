#pragma once

#include <glm/glm.hpp>

namespace nex 
{

	class Bone;

	struct KeyFrame
	{
		struct Comparator {
			bool operator()(const KeyFrame& a, const KeyFrame& b) const 
			{
				if (a.time == b.time) {
					return a.boneSID < b.boneSID;
				}

				return a.time < b.time;
			}
		};

		// The SID of the bone the key frame references to.
		unsigned boneSID;

		// The point in time in the animation the key frame should be active.
		double time;
	};

	struct PositionKeyFrame : public KeyFrame
	{
		// The position the bone should have for this key frame.
		glm::vec3 position;
	};

	struct RotationKeyFrame : public KeyFrame
	{
		// The rotation the bone should have for this key frame.
		glm::quat rotation;
	};

	struct ScaleKeyFrame : public KeyFrame
	{
		// The scale the bone should have for this key frame.
		glm::vec3 scale;
	};


	struct OptimizedKeyFrame
	{
		Bone* bone;
		double time;
	};

	struct OptPositionKeyFrame : public OptimizedKeyFrame
	{
		// The position the bone should have for this key frame.
		glm::vec3 position;
	};

	struct OptRotationKeyFrame : public OptimizedKeyFrame
	{
		// The rotation the bone should have for this key frame.
		glm::quat rotation;
	};

	struct OptScaleKeyFrame : public OptimizedKeyFrame
	{
		// The scale the bone should have for this key frame.
		glm::vec3 scale;
	};
}
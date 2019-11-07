#pragma once

#include <glm/glm.hpp>

namespace nex 
{
	template<class ID>
	struct KeyFrame
	{
		struct Comparator {
			bool operator()(const KeyFrame& a, const KeyFrame& b) const 
			{
				if (a.time == b.time) {
					return a.id < b.id;
				}

				return a.time < b.time;
			}
		};

		ID id;

		// The point in time in the animation the key frame should be active.
		double time;
	};

	template<class ID>
	struct PositionKeyFrame : public KeyFrame<ID>
	{
		// The position the bone should have for this key frame.
		glm::vec3 position;
	};

	template<class ID>
	struct RotationKeyFrame : public KeyFrame<ID>
	{
		// The rotation the bone should have for this key frame.
		glm::quat rotation;
	};

	template<class ID>
	struct ScaleKeyFrame : public KeyFrame<ID>
	{
		// The scale the bone should have for this key frame.
		glm::vec3 scale;
	};
}
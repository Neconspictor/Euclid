#pragma once

#include <glm/glm.hpp>

namespace nex
{
	template<class ID>
	struct KeyFrame
	{
		struct Comparator {

			/**
			 * Sort by id, than by time
			 */
			bool operator()(const KeyFrame& a, const KeyFrame& b) const
			{
				if (a.id == b.id) {
					return a.time < b.time;
				}

				return a.id < b.id;
			}
		};

		ID id;

		// The point in time in the animation the key frame should be active.
		float time;
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


	template<class T>
	struct MinMaxData {
		float minTime = FLT_MAX;
		float maxTime = -FLT_MAX;
		unsigned minKeyID;
		unsigned maxKeyID;
		T minData;
		T maxData;
	};

	struct MinMaxKeyFrame {
		MinMaxData<glm::vec3> positions;
		MinMaxData<glm::quat> rotations;
		MinMaxData<glm::vec3> scales;
	};
}
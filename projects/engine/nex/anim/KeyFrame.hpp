#pragma once

#include <glm/glm.hpp>

namespace nex
{
	using Sid = unsigned;
	using ChannelID = Sid;

	template<class DataType, class ID>
	struct KeyFrame
	{
		struct Comparator {

			/**
			 * Sort by id, than by frame
			 */
			bool operator()(const KeyFrame& a, const KeyFrame& b) const
			{
				if (a.id == b.id) {
					return a.frame < b.frame;
				}

				return a.id < b.id;
			}
		};

		// The id of the entity this keyframe is assigned to (e.g. bone or vob)
		ID id;

		// The frame number in the animation this key frame should be active.
		int frame;

		DataType data;


		static inline DataType mix(const DataType& a, const DataType& b, float ratio);
	};

	template<class DataType, class ID>
	inline DataType KeyFrame<DataType, ID>::mix(const DataType& a, const DataType& b, float ratio) {
		static_assert(false, "Not implemented yet!");
	}

	inline glm::vec3 KeyFrame<glm::vec3, ChannelID>::mix(const glm::vec3& a, const glm::vec3& b, float ratio) {
		return glm::mix(a, b, ratio);
	}

	inline glm::quat KeyFrame<glm::quat, ChannelID>::mix(const glm::quat& a, const glm::quat& b, float ratio) {
		return glm::slerp(a, b, ratio);
	}


	struct CompoundKeyFrame {
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
	};


	template<class T>
	struct MixData {
		T minData;
		T maxData;
		float ratio;
	};
}
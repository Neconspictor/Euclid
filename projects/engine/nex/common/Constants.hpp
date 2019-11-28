#pragma once

namespace nex
{
	class Camera;
	struct DirLight;

	struct Constants
	{
		const Camera* camera;
		unsigned windowWidth;
		unsigned windowHeight;
		float time;
		float frameTime;
		const DirLight* sun;
	};
}
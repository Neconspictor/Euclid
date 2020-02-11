#pragma once

#include <memory>
#include <nex/scene/Scene.hpp>
#include <nex/GI/Probe.hpp>

namespace nex
{
	class Scene;
	class Vob;
	class ProbeVob;


	enum class ProbeSelectionAlgorithm 
	{
		NEAREST = 0,
		FOUR_NEAREST_INTERPOLATION,
	};

	class ProbeSelector
	{
	public:

		struct Selection {
			Probe* probes[4] = { nullptr, nullptr , nullptr , nullptr };
			float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			unsigned count = 0;
		};

		using Selector = Selection(const Vob * vob, const Scene & scene, Probe::Type type);

		static Selector* getSelector(ProbeSelectionAlgorithm alg);

		static Selection selectNearest(const Vob* vob, const Scene& scene, Probe::Type type);

		static Selection selectFourNearest(const Vob* vob, const Scene& scene, Probe::Type type);

		static void assignProbes(const Scene&  scene, Selector* selector, Probe::Type type);

	private:

		static void interpolateDiffuseProbes(const Selection& selection, glm::vec4* result);
	};
}
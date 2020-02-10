#pragma once

#include <memory>
#include <nex/scene/Scene.hpp>
#include <nex/GI/Probe.hpp>

namespace nex
{
	class Scene;
	class Vob;
	class ProbeVob;

	class ProbeSelector
	{
	public:

		using Selector = ProbeVob* (const Vob * vob, const Scene & scene, Probe::Type type);

		static ProbeVob* selectNearest(const Vob* vob, const Scene& scene, Probe::Type type);

		static void assignProbes(const Scene&  scene, Selector* selector, Probe::Type type);
	};
}
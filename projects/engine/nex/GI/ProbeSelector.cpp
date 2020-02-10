#include <nex/GI/ProbeSelector.hpp>

nex::ProbeVob* nex::ProbeSelector::selectNearest(const Vob* vob, const Scene& scene, Probe::Type type)
{
	auto lock = scene.acquireLock();

	const auto& vobPos = vob->getPositionLocalToWorld();
	float selectedDistanceSquared = std::numeric_limits<float>::infinity();
	ProbeVob* nearest = nullptr;

	for (auto* probeVob : scene.getActiveProbeVobsUnsafe()) {

		auto* probe = probeVob->getProbe();
		if (probe->getType() != type) continue;

		auto diff = vobPos - probeVob->getPositionLocalToWorld();

		auto currentDistSquared = glm::dot(diff, diff);
		if (currentDistSquared < selectedDistanceSquared) {
			selectedDistanceSquared = currentDistSquared;
			nearest = probeVob;
		}
	}

	return nearest;
}

void nex::ProbeSelector::assignProbes(const Scene& scene, Selector* selector, Probe::Type type)
{
	auto lock = scene.acquireLock();
	for (auto* vob : scene.getActiveVobsUnsafe()) {

		if (!vob->usesPerObjectMaterialData()) continue;
		auto& perObjectMaterialData = vob->getPerObjectMaterialData();
		if (!perObjectMaterialData.probesUsed) continue;

		if (const auto* selectedProbe = selector(vob, scene, type)) {
			const auto* probe = selectedProbe->getProbe();
			const auto& index = probe->getArrayIndex();

			switch (type) {
			case Probe::Type::Irradiance:
				perObjectMaterialData.diffuseReflectionProbeID = index;
				break;
			case Probe::Type::Reflection:
				perObjectMaterialData.specularReflectionProbeID = index;
				break;
			}
		}
	}
}
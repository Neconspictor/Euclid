#include <nex/GI/ProbeSelector.hpp>

nex::ProbeSelector::Selector* nex::ProbeSelector::getSelector(ProbeSelectionAlgorithm alg)
{
	switch (alg)
	{
	case nex::ProbeSelectionAlgorithm::NEAREST:
		return selectNearest;
		break;
	case nex::ProbeSelectionAlgorithm::FOUR_NEAREST_INTERPOLATION:
		return selectFourNearest;
		break;
	default:
		nex::throw_with_trace(std::runtime_error("unknown algorithm: " + (int)alg));
		break;
	}
	return nullptr;
}

nex::ProbeSelector::Selection nex::ProbeSelector::selectNearest(const Vob* vob, const Scene& scene, Probe::Type type)
{
	auto lock = scene.acquireLock();
	Selection selection;

	const auto& vobPos = vob->getPositionLocalToWorld();
	float selectedDistanceSquared = std::numeric_limits<float>::infinity();

	for (auto* probeVob : scene.getActiveProbeVobsUnsafe()) {

		auto* probe = probeVob->getProbe();
		if (probe->getType() != type) continue;

		auto diff = vobPos - probeVob->getPositionLocalToWorld();

		auto currentDistSquared = glm::dot(diff, diff);
		if (currentDistSquared < selectedDistanceSquared) {
			selectedDistanceSquared = currentDistSquared;
			selection.probes[0] = probe;
		}
	}

	if (selection.probes[0]) {
		selection.count = 1;
		selection.weights[0] = 1.0f;
		selection.weights[1] = 0.0f;
		selection.weights[2] = 0.0f;
		selection.weights[3] = 0.0f;

		selection.probes[1] = nullptr;
		selection.probes[2] = nullptr;
		selection.probes[3] = nullptr;
	}

	return selection;
}

nex::ProbeSelector::Selection nex::ProbeSelector::selectFourNearest(const Vob* vob, const Scene& scene, Probe::Type type)
{
	constexpr unsigned k = 4;

	auto lock = scene.acquireLock();
	Selection selection;

	if (vob->getName() == "cerberus") {
		bool test = false;
	}

	const auto& vobPos = vob->getPositionLocalToWorld();
	auto probeVobs = scene.getActiveProbeVobsUnsafe();

	// TODO: very slow -> use spatial acceleration data structure (tetrahedilization) 

	// delete not needed probe vobs
	auto it = std::remove_if(probeVobs.begin(), probeVobs.end(), [=](const ProbeVob* a) {
		return a->getProbe()->getType() != type;
	});
	probeVobs.erase(it, probeVobs.end());

	// sort by distance
	std::sort(probeVobs.begin(), probeVobs.end(), [=](const ProbeVob* a, const ProbeVob* b) {

		auto diffA = vobPos - a->getPositionLocalToWorld();
		auto diffB = vobPos - b->getPositionLocalToWorld();

		return dot(diffA, diffA) < dot(diffB, diffB);
	});

	//collect probes and distances
	for (auto* probeVob : probeVobs) {

		auto* probe = probeVob->getProbe();
		auto diff = vobPos - probeVob->getPositionLocalToWorld();
		auto currentDist = glm::length2(diff);
		selection.probes[selection.count] = probe;

		// store distance for later processing
		selection.weights[selection.count] = currentDist;
		++selection.count;

		if (selection.count == k) break;
	}

	// calc weights from distances
	// TODO: handle zero distances
	auto weight0 = selection.weights[0];

	if (weight0 < 0.00001f) {
		selection.weights[0] = 1.0f;
		selection.weights[1] = 0.0f;
		selection.weights[2] = 0.0f;
		selection.weights[3] = 0.0f;
	}
	else {
		for (int i = 0; i < selection.count; ++i) {
			
			selection.weights[i] = 1.0f / (selection.weights[i] / weight0);
		}
	}


	// weight0 is always 1.0f
	// we ensure that the inv weight sum calc is always valid.
	float weightSum = 1.0; 
	for (int i = 1; i < selection.count; ++i) {
		weightSum += selection.weights[i];
	}

	float invWeightSum = 1.0 / weightSum;
	// normalize weights; sum of weights is than 1.0
	if (nex::isValid(invWeightSum)) {
		
		for (int i = 0; i < selection.count; ++i) {
			selection.weights[i] *= invWeightSum;
		}
	}
	else {
		selection.weights[0] = 1.0f;
		selection.weights[1] = 0.0f;
		selection.weights[2] = 0.0f;
		selection.weights[3] = 0.0f;
	}


	weightSum = selection.weights[0];
	for (int i = 1; i < selection.count; ++i) {
		weightSum += selection.weights[i];
	}

	selection.weights[0] += 1.0f - weightSum;

	return selection;
}

void nex::ProbeSelector::assignProbes(const Scene& scene, Selector* selector, Probe::Type type)
{
	auto lock = scene.acquireLock();
	for (auto* vob : scene.getActiveVobsUnsafe()) {

		if (!vob->usesPerObjectMaterialData()) continue;
		auto& perObjectMaterialData = vob->getPerObjectMaterialData();
		if (!perObjectMaterialData.probesUsed) continue;

		const auto selection = selector(vob, scene, type);
		if (!selection.count) continue;

		const auto* probe = selection.probes[0];
		const auto& index = probe->getArrayIndex();

		switch (type) {
		case Probe::Type::Irradiance:
			perObjectMaterialData.diffuseReflectionProbeID = index;
			assert(sizeof(perObjectMaterialData.diffuseSHCoefficients) == Probe::SPHERHICAL_HARMONICS_WIDTH * sizeof(glm::vec4));
			interpolateDiffuseProbes(selection, perObjectMaterialData.diffuseSHCoefficients);

			break;
		case Probe::Type::Reflection:
			perObjectMaterialData.specularReflectionProbeID = index;

			perObjectMaterialData.specularReflectionWeights = (glm::vec4&)selection.weights;
			for (int i = 0; i < 4; ++i) {

				perObjectMaterialData.specularReflectionIds[i] = 0;
				if (i < selection.count) {
					perObjectMaterialData.specularReflectionIds[i] = selection.probes[i]->getArrayIndex();
				}
			}

			break;
		}
	}
}

void nex::ProbeSelector::interpolateDiffuseProbes(const Selection& selection, glm::vec4* result)
{
	for (int i = 0; i < Probe::SPHERHICAL_HARMONICS_WIDTH; ++i) {
		result[i] = glm::vec4(0.0);
	}

	for (int i = 0; i < selection.count; ++i) {
		auto* probe = selection.probes[i];
		const auto& coefficients = probe->getSHCoefficients();
		const auto& weight = selection.weights[i];

		for (int j = 0; j < Probe::SPHERHICAL_HARMONICS_WIDTH; ++j) {
			result[j] += weight * coefficients[j];
		}
	}
}
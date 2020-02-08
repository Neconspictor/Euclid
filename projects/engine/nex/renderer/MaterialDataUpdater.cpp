#include <nex/renderer/MaterialDataUpdater.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/scene/Vob.hpp>

void nex::MaterialUpdater::updateMaterialData(Scene* scene, ShaderBuffer* materialBuffer)
{
	auto lock = scene->acquireLock();
	const auto& vobs = scene->getActiveVobsUnsafe();
	const unsigned size = std::min<unsigned>(vobs.size(), 1024);

	std::vector<PerObjectMaterialData> data(size);

	for (unsigned id = 0; id < size; ++id) {
		auto* vob = vobs[id];
		vob->setPerObjectMaterialDataID(id);
		data[id] = vob->getPerObjectMaterialData();
	}
}
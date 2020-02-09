#include <nex/renderer/MaterialDataUpdater.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/common/Log.hpp>

void nex::MaterialDataUpdater::updateMaterialData(Scene* scene, ShaderBuffer* materialBuffer)
{
	auto lock = scene->acquireLock();
	const auto& vobs = scene->getActiveVobsUnsafe();
	const unsigned size = std::min<unsigned>(vobs.size(), MAX_PER_OBJECT_MATERIAL_DATA);


#ifndef EUCLID_ALL_OPTIMIZATIONS
	if (vobs.size() > MAX_PER_OBJECT_MATERIAL_DATA) {
		LOG(Logger("nex::MaterialDataUpdater::updateMaterialData"), Warning) << "Not all objects can be mapped to a material data object!";
	}
#endif

	static PerObjectMaterialData data[MAX_PER_OBJECT_MATERIAL_DATA];

	for (unsigned dataID = 0, vobID = 0; 
		dataID < size && vobID < vobs.size(); 
		++vobID) 
	{
		auto* vob = vobs[vobID];

		if (!vob->usesPerObjectMaterialData()) continue;

		vob->setPerObjectMaterialDataID(dataID);
		data[dataID] = vob->getPerObjectMaterialData();
		++dataID;
	}

	materialBuffer->resize(sizeof(PerObjectMaterialData) * MAX_PER_OBJECT_MATERIAL_DATA, 
		data,
		nex::GpuBuffer::UsageHint::STREAM_DRAW);
}
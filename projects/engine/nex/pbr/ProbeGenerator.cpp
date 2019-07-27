#include <nex/pbr/ProbeGenerator.hpp>

nex::ProbeGenerator::ProbeGenerator(nex::Scene* scene) : mScene(scene)
{
}

void nex::ProbeGenerator::setScene(nex::Scene* scene)
{
	mScene = scene;
}

void nex::ProbeGenerator::show(bool visible)
{
	//TODO
}

std::unique_ptr<nex::PbrProbe> nex::ProbeGenerator::generate() const
{
	//TODO
	return nullptr;
}
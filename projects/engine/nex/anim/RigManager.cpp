#include <nex/anim/RigManager.hpp>
#include <nex/util/ExceptionHandling.hpp>

void nex::RigManager::add(std::unique_ptr<Rig> rig)
{
	auto id = rig->getID();
	auto result = mRigs.insert(std::pair<unsigned, std::unique_ptr<Rig>>(id, std::move(rig)));
	if (!result.second) {
		throw_with_trace(std::invalid_argument("nex::RigManager::add : There exists already a rig with id "
			+ std::to_string(id)));
	}
}

const nex::Rig* nex::RigManager::getByID(unsigned id) const
{
	auto it = mRigs.find(id);

	if (it != mRigs.end())
		return it->second.get();

	return nullptr;
}

nex::RigManager* nex::RigManager::get()
{
	static RigManager instance;
	return &instance;
}
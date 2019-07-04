#include <nex/resource/Resource.hpp>

nex::Resource::Resource() 
{
}

nex::Resource::~Resource() = default;

void nex::Resource::finalize() {};

bool nex::Resource::isLoaded() const
{
	return mPromise.get_future().is_ready();
}

void nex::Resource::setIsLoadedStatus(FutureType future)
{
	mPromise = PromiseType(future.get_state());
}

void nex::Resource::setIsLoaded(bool useThisPointer, ResourceType resource)
{
	if (useThisPointer)
		resource = this;

		mPromise.set_value(resource);
}

nex::Resource::FutureType nex::Resource::getIsLoadedStatus() const
{
	return mPromise.get_future();
}
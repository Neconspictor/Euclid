#include <nex/resource/Resource.hpp>

nex::Resource::Resource() 
{
}

bool nex::Resource::isLoaded() const
{
	return mPromise.get_future().is_ready();
}

void nex::Resource::setIsLoadedStatus(nex::Future<void> future)
{
	mPromise = Promise<void>(future.get_state());
}

void nex::Resource::setIsLoaded()
{
	mPromise.set();
}

nex::Future<void> nex::Resource::getIsLoadedStatus() const
{
	return mPromise.get_future();
}
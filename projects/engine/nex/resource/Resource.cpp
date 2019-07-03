#include <nex/resource/Resource.hpp>

bool nex::Resource::isLoaded() const
{
	return mFuture.is_ready();
}

void nex::Resource::setIsLoadedStatus(nex::Future<void> future)
{
	mFuture = std::move(future);
}

const nex::Future<void>& nex::Resource::getIsLoadedStatus() const
{
	return mFuture;
}
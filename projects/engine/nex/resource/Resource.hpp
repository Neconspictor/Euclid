#pragma once
#include <atomic>
#include <nex/common/Future.hpp>

namespace nex {
	class Resource {
	public:

		/**
		 * Checks if the resource has finished loading (non-blocking). 
		 */
		bool isLoaded() const;

		/**
		 * Sets the future used to query the 'is loaded' status of this resource. 
		 */
		void setIsLoadedStatus(Future<void> future);

		/**
		 * Returns a future that specifies the 'is loaded' status of the resource.
		 */
		const Future<void>& getIsLoadedStatus() const;

	protected:
		Future<void> mFuture;
	};
}
#pragma once
#include <atomic>
#include <nex/common/Future.hpp>

namespace nex {
	class Resource {
	public:

		Resource();

		/**
		 * Checks if the resource has finished loading (non-blocking). 
		 */
		bool isLoaded() const;

		/**
		 * Sets the future used to query the 'is loaded' status of this resource. 
		 */
		void setIsLoadedStatus(Future<void> future);

		/**
		 * Sets the 'is loaded' status to true.
		 */
		void setIsLoaded();

		/**
		 * Returns a future that specifies the 'is loaded' status of the resource.
		 */
		Future<void> getIsLoadedStatus() const;

	protected:
		Promise<void> mPromise;
	};
}
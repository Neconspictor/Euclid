#pragma once
#include <atomic>
#include <nex/common/Future.hpp>

namespace nex {
	class Resource {
	public:

		using ResourceType = Resource* ;
		using FutureType = Future<ResourceType>;
		using PromiseType = Promise<ResourceType>;

		Resource();

		// A resource mustn't be copyable and movable in order to guarantee asynchronous resource loading.
		Resource(Resource&& o) noexcept = delete;
		Resource& operator=(Resource&& o) noexcept = delete;
		Resource(const Resource& o) noexcept = delete;
		Resource& operator=(const Resource& o) noexcept = delete;

		virtual ~Resource();

		/**
		 * Some resources need finalization as for some render backends (e.g. opengl) 
		 * some internal functions and resources are only valid for the current active context.
		 * As a worker thread might use a different render context (if any at all), a resource might
		 * not be fully initialized. Therefore this function is intended to be used to 'polish' the resource.
		 */
		virtual void finalize();

		/**
		 * Checks if the resource has finished loading (non-blocking). 
		 */
		bool isLoaded() const;

		/**
		 * Sets the future used to query the 'is loaded' status of this resource. 
		 */
		void setIsLoadedStatus(FutureType future);

		/**
		 * Sets the 'is loaded' status to true.
		 */
		void setIsLoaded(bool useThisPointer = true, ResourceType resource = nullptr);

		/**
		 * Returns a future that specifies the 'is loaded' status of the resource.
		 */
		FutureType getIsLoadedStatus() const;

	protected:
		PromiseType mPromise;
		bool mFinalized = false;
	};

	template<class T>
	class ResourceWrapper : public Resource {
	public:

		ResourceWrapper(T* data) : Resource(), mData(data)
		{
		}

		T* get() {
			return mData;
		}

		const T* get()const {
			return mData;
		}

	private:
		T* mData;

	};
}
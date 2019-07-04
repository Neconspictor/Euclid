/**
 * Custom versions of std::future, std::promise and std::packaged_task, that allow non-blocking polling of the status of a future.
 * c++20 is supposed to extend the future class in this class. So this classes will prospectively be replaced.
 */
#pragma once

#include <atomic>
#include <mutex>

namespace nex
{
	template<class T> class _Future;
	template<class T> class _PromiseBase;
	template<class Ret, class... ArgsTypes> class _PackagedTask;


	/**
	 * Typedefs for easy swichting to std::future, std::promise and std::packaged_task
	 */
	template<class T> using Future = _Future<T>;
	template<class T> using Promise = _PromiseBase<T>;
	template<class Ret, class... ArgsTypes> using PackagedTask = _PackagedTask<Ret, ArgsTypes...>;

	template<class _Fret>
	struct _P_ArgType
	{	// type for functions returning T
		using type = _Fret;
	};

	template<class _Fret>
	struct _P_ArgType<_Fret&>
	{	// type for functions returning reference to T
		using type = _Fret * ;
	};

	template<>
	struct _P_ArgType<void>
	{	// type for functions returning void
		using type = int;
	};

	template<class T>
	class _FutureSharedState {
	public:

		_FutureSharedState() {

		}


		_FutureSharedState(const _FutureSharedState&) = delete;
		_FutureSharedState& operator=(const _FutureSharedState&) = delete;
		virtual ~_FutureSharedState() = default;

		T& get() {
			std::unique_lock<std::mutex> lock(mMutex);
			mCondition.wait(lock, [=] {return mFinished.load(); });
			return mElem;
		}

		const T& get() const {
			std::unique_lock<std::mutex> lock(mMutex);
			mCondition.wait(lock, [=] {return mFinished.load(); });
			return mElem;
		}

		bool is_ready() const
		{
			return mFinished;
		}

		void set_value(const T& elem)
		{
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mElem = elem;
				mFinished = true;
			}

			mCondition.notify_all();
		}

		void set_value(T&& elem)
		{
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mElem = std::forward<T>(elem);
				mFinished = true;
			}

			mCondition.notify_all();
		}

		void retain() {
			++mRefs;
		}

		void release() {
			if (--mRefs == 0) {
				// Note, that this class (and subclasses) are not intended to be used by end user.
				delete this;
			}
		}

	private:
		friend Future<T>;
		friend Promise<T>;

		T mElem;
		std::condition_variable mCondition;
		std::mutex mMutex;
		std::atomic<bool> mFinished = false;
		std::atomic<unsigned long> mRefs = 1;
	};

	template<class T>
	class _FutureStateManager {
	public:

		_FutureStateManager()
			: mState(nullptr)
		{	// construct with no associated asynchronous state object
		}

		_FutureStateManager(_FutureSharedState<T> * newState)
			: mState(newState)
		{	// construct with a new state
		}

		_FutureStateManager(const _FutureStateManager& other)
			: mState(nullptr)
		{	// construct from other
			copyFrom(other);
		}

		_FutureStateManager(_FutureStateManager&& other)
			: mState(nullptr)
		{	// construct from rvalue other
			moveFrom(other);
		}

		~_FutureStateManager() noexcept
		{	// destroy
			if (mState)
				mState->release();
		}

		_FutureStateManager& operator=(const _FutureStateManager& _Other)
		{	// assign from _Other
			copyFrom(_Other);
			return (*this);
		}

		_FutureStateManager& operator=(_FutureStateManager&& _Other)
		{	// assign from rvalue _Other
			moveFrom(_Other);
			return (*this);
		}

		void copyFrom(const _FutureStateManager& other)
		{	// copy stored associated asynchronous state object from other
			if (this != std::addressof(other))
			{	// different, copy
				if (mState)
					mState->release();
				if (other.mState == nullptr)
					mState = nullptr;
				else
				{	// do the copy
					other.mState->retain();
					mState = other.mState;
				}
			}
		}

		void moveFrom(_FutureStateManager& other)
		{	// move stored associated asynchronous state object from other
			if (this != _STD addressof(other))
			{	// different, move
				if (mState)
					mState->release();
				mState = other.mState;
				other.mState = nullptr;
			}
		}

		bool is_ready() const
		{
			return mState->is_ready();
		}

		void set_value(const T& value) {
			if (valid())
				mState->set_value(value);
		}

		void set_value(T&& value) {
			if (valid())
				mState->set_value(std::forward<T>(value));
		}

		[[nodiscard]] bool valid() const noexcept
		{	// return status
			return (mState != nullptr && !(mState->is_ready()));
		}

		_FutureSharedState<T>* get_state() {
			return mState;
		}

	protected:
		T& get_intern() {

			return mState->get();
		}

		const T& get_intern() const {

			return mState->get();
		}

		_FutureSharedState<T>* mState;
	};

	struct Nil{};

	template<class T>
	class _Future : public _FutureStateManager<T> {
		using MyBase = _FutureStateManager<T>;
	public:

		_Future() noexcept
		{	// construct
		}

		_Future(const _Future& other) noexcept : MyBase(other) {}

		_Future(_Future&& other) noexcept
			: MyBase(std::move(other))
		{	// construct from rvalue future object
		}

		_Future& operator=(_Future&& right) noexcept
		{	// assign from rvalue future object
			MyBase::operator=(std::move(right));
			return (*this);
		}

		_Future& operator=(const _Future& right) noexcept
		{	// assign from rvalue future object
			MyBase::operator=(right);
			return (*this);
		}

		_Future(const _FutureStateManager& state, Nil nil)
			: MyBase(state)
		{	// construct from associated asynchronous state object
		}

		~_Future() noexcept
		{	// destroy
		}

		T get() const
		{
			return get_intern();
		}

	};

	template<>
	class _Future<void> : public _FutureStateManager<int> {
		using MyBase = _FutureStateManager<int>;
	public:

		_Future() noexcept
		{	// construct
		}

		_Future(const _Future& other) noexcept : MyBase(other) {}

		_Future(_Future&& other) noexcept
			: MyBase(std::move(other))
		{	// construct from rvalue future object
		}

		_Future& operator=(_Future&& right) noexcept
		{	// assign from rvalue future object
			MyBase::operator=(std::move(right));
			return (*this);
		}

		_Future& operator=(const _Future& right) noexcept
		{	// assign from rvalue future object
			MyBase::operator=(right);
			return (*this);
		}

		_Future(const _FutureStateManager& state, Nil nil)
			: MyBase(state)
		{	// construct from associated asynchronous state object
		}

		~_Future() noexcept
		{	// destroy
		}

		void get() const
		{
			get_intern();
		}
	};



	template<class T>
	class _PromiseBase {
	public:

		using InternType = typename _P_ArgType<T>::type;

		_PromiseBase() : mManager(new _FutureSharedState<T>())
		{
		}

		_PromiseBase(_FutureSharedState<T>* sharedState) : mManager(sharedState)
		{
		}

		_Future<T> get_future() const
		{
			_Future<T> future(mManager, {});
			return future;
		}

		_FutureStateManager<T>& get_state_manager() {
			return mManager;
		}

		const _FutureStateManager<T>& get_state_manager() const {
			return mManager;
		}

		void set_value(T elem)
		{
			mManager.set_value(std::move(elem));
		}

		_FutureSharedState<T>* get_state() {
			return mManager.get_state();
		}

	private:
		_FutureStateManager<T> mManager;
	};

	template<>
	class _PromiseBase<void> {
	public:

		using InternType = typename _P_ArgType<void>::type;

		_PromiseBase() : mManager(new _FutureSharedState<InternType>())
		{
		}

		_PromiseBase(_FutureSharedState<InternType>* sharedState) : mManager(sharedState)
		{
		}

		_Future<void> get_future() const
		{
			_Future<void> future(mManager, {});
			return future;
		}

		_FutureStateManager<InternType>& get_state_manager() {
			return mManager;
		}

		const _FutureStateManager<InternType>& get_state_manager() const {
			return mManager;
		}

		void set()
		{
			mManager.set_value(1);
		}

		_FutureSharedState<InternType>* get_state() {
			return mManager.get_state();
		}

	private:
		_FutureStateManager<InternType> mManager;
	};


	template<class> class _PackagedTaskState;

	template<class Func, class... ArgsType>
	class _PackagedTaskState<Func(ArgsType...)> : 
	public _FutureSharedState<
		// Return type of Func(ArgsType...)
		Func
	>
	{
	public:

		template<class FuncType2>
		_PackagedTaskState(const FuncType2& func) : mTask(func)

		{
			// construct from rvalue function object
		}

		template<class FuncType2>
		_PackagedTaskState(FuncType2&& func) : mTask(std::forward<FuncType2>(func))

		{
			// construct from rvalue function object
		}

		virtual ~_PackagedTaskState() = default;

		void call(ArgsType... args) {
			set_value(mTask(std::forward<ArgsType>(args)...));
		}

	private:
		std::function<Func(ArgsType...)> mTask;
	};

	template<class Func, class... ArgsType>
	class _PackagedTaskState<Func&(ArgsType...)> :
		public _FutureSharedState<
		// Return type of Func(ArgsType...)
		Func*
		>
	{
	public:

		template<class FuncType2>
		explicit _PackagedTaskState(const FuncType2& func) : mTask(func)

		{
			// construct from rvalue function object
		}

		template<class FuncType2>
		explicit _PackagedTaskState(FuncType2&& func) : mTask(std::forward<FuncType2>(func))

		{
			// construct from rvalue function object
		}

		virtual ~_PackagedTaskState() = default;

		void call(ArgsType... args) {
			set_value(std::addressof(mTask(std::forward<ArgsType>(args)...)));
		}

	private:
		std::function<Func(ArgsType...)> mTask;
	};


	template<class... ArgsType>
	class _PackagedTaskState<void(ArgsType...)> :
		public _FutureSharedState<int>
	{
	public:

		template<class FuncType2>
		explicit _PackagedTaskState(const FuncType2& func) : mTask(func)

		{
			// construct from rvalue function object
		}

		template<class FuncType2>
		explicit _PackagedTaskState(FuncType2&& func) : mTask(std::forward<FuncType2>(func))

		{
			// construct from rvalue function object
		}

		virtual ~_PackagedTaskState() = default;

		void call(ArgsType... args) {
			mTask(std::forward<ArgsType>(args)...);
			set_value(1);
		}

	private:
		std::function<void(ArgsType...)> mTask;
	};


	template<class _Ty>
	using remove_reference_t = typename std::remove_reference<_Ty>::type;

	template<class _Ty>
	using remove_cv_t = typename std::remove_cv<_Ty>::type;

	template<bool _Test,
		class _Ty = void>
		using enable_if_t = typename std::enable_if<_Test, _Ty>::type;

	template<class _Ty,
		class _Uty>
		_INLINE_VAR constexpr bool is_same_v = std::is_same<_Ty, _Uty>::value;


	template <class Ret, class... ArgsType>
	class _PackagedTask<Ret(ArgsType...)>
	{
	public:
		//using ReturnType = typename _P_ArgType<Ret>::type;// decltype(std::declval<Func>()(std::declval<ArgsType>()...));
		using PromiseType = _PromiseBase<Ret>;
		using PackagedType = _PackagedTaskState<Ret(ArgsType...)>;

		_PackagedTask() noexcept
			: mPromise(nullptr)
		{	// construct
		}


		template<class RetType2,
			class = enable_if_t<!is_same_v<remove_cv_t<remove_reference_t<RetType2>>, _PackagedTask>>>
		explicit _PackagedTask(RetType2&& func) : mPromise(new PackagedType(std::forward<RetType2>(func)))
		{
			// construct from rvalue function object
		}

		_PackagedTask(_PackagedTask&& other) noexcept
			: mPromise(std::move(other.mPromise))
		{	// construct from rvalue _PackagedTask object
		}

		_PackagedTask& operator=(_PackagedTask&& other) noexcept
		{	// assign from rvalue _PackagedTask object
			mPromise = std::move(other.mPromise);
			return (*this);
		}

		void operator()(ArgsType... args) {
			auto* packagedTask = static_cast<PackagedType*>(mPromise.get_state());
			packagedTask->call(std::forward<ArgsType>(args)...);
		}

		Future<Ret> get_future() const {

			Future<Ret> local(mPromise.get_state_manager(), {});

			return local;
		}

	private:
		PromiseType mPromise;

	};

	void FutureTest();
}
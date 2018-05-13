#ifndef BASE_ATOMIC_INCLUDE
#define BASE_ATOMIC_INCLUDE

#include "base/config.hpp"

#ifdef M_PLATFORM_WIN
#ifndef M_WIN32_LEAN_AND_MEAN  
#define WIN32_LEAN_AND_MEAN // 在所有Windows.h包含的地方加上它
#endif
#include <Windows.h>
#endif

M_BASE_NAMESPACE_BEGIN

template<typename T>
struct atomic;

template<typename T>
struct atomic
{
	static inline T get(T* pv){
#ifndef M_PLATFORM_WIN
		return __sync_val_compare_and_swap(pv, 0, 0);
#else
		return InterlockedCompareExchange(pv,0,0);
#endif
	}

	static inline void add(T* pv,T x){
#ifndef M_PLATFORM_WIN
		__sync_add_and_fetch(pv,x); 
#else
		InterlockedExchangeAdd(pv,x);
#endif
	}

	static inline T getAndAdd(T* pv,T x){
#ifndef M_PLATFORM_WIN
		return __sync_fetch_and_add(pv, x);
#else
		return InterlockedExchangeAdd(pv,x);
#endif
	}

	static inline T addAndGet(T* pv,T x){
#ifndef M_PLATFORM_WIN
		return __sync_add_and_fetch(pv,x);
#else
		return InterlockedExchangeAdd(pv,x)+x;
#endif
	}

	static inline void sub(T* pv,T x){
#ifndef M_PLATFORM_WIN
		__sync_sub_and_fetch(pv,x);
#else
		InterlockedExchangeAdd(pv,-x);
#endif
	}

	static inline T getAndSub(T* pv,T x){
#ifndef M_PLATFORM_WIN
		return __sync_fetch_and_sub(pv,x);
#else
		return InterlockedExchangeAdd(pv,-x);
#endif
	}

	static inline T subAndGet(T* pv,T x){
#ifndef M_PLATFORM_WIN
		return __sync_sub_and_fetch(pv,x);
#else
		return InterlockedExchangeAdd(pv,-x)-x;
#endif
	}

	static inline T getAndSet(T* pv,T x){
		// 先取后设置
#ifndef M_PLATFORM_WIN
		return __sync_lock_test_and_set(pv,x);
#else
		return InterlockedExchange(pv,x);
#endif
	}
};

#ifdef M_PLATFORM_WIN
template<>
struct atomic<base::s_int64_t>
{
	static inline base::s_int64_t get(base::s_int64_t* pv){
		return InterlockedCompareExchange64(pv,0,0);
	}

	static inline void add(base::s_int64_t* pv, base::s_int64_t x){
		InterlockedExchangeAdd64(pv,x);
	}

	static inline __int64 getAndAdd(base::s_int64_t* pv, base::s_int64_t x){
		return InterlockedExchangeAdd64(pv,x);
	}

	static inline __int64 addAndGet(base::s_int64_t* pv, base::s_int64_t x){
		return InterlockedExchangeAdd64(pv,x)+x;
	}

	static inline void sub(base::s_int64_t* pv, base::s_int64_t x){
		InterlockedExchangeAdd64(pv,-x);
	}

	static inline __int64 getAndSub(base::s_int64_t* pv, base::s_int64_t x){
		return InterlockedExchangeAdd64(pv,-x);
	}

	static inline __int64 subAndGet(base::s_int64_t* pv, base::s_int64_t x){
		return InterlockedExchangeAdd64(pv,-x)-x;
	}

	static inline __int64 getAndSet(base::s_int64_t* pv, base::s_int64_t x){
		return InterlockedExchange64(pv,x);
	}
};
#endif

template<typename T>
class atomicinteger
{
private:
	T mValue;

public:
	atomicinteger():mValue(0){}

	inline T get()
	{
		return atomic<T>::get(&mValue);		
	}

	inline void add(T x)
	{
		atomic<T>::add(&mValue,x);
	}

	inline T getAndAdd(T x)
	{
		return atomic<T>::getAndAdd(&mValue,x);
	}

	inline T addAndGet(T x)
	{
		return atomic<T>::addAndGet(&mValue,x);
	}

	inline void sub(T x)
	{
		atomic<T>::sub(&mValue,x);
	}

	inline T getAndSub(T x)
	{
		return atomic<T>::getAndSub(&mValue,x);
	}

	inline T subAndGet(T x)
	{
		return atomic<T>::subAndGet(&mValue,x);
	}

	inline T getAndSet(T x)
	{
		return atomic<T>::getAndSet(&mValue,x);
	}
};

#ifdef M_PLATFORM_WIN
typedef atomicinteger<long> atomicint32;
#else
typedef atomicinteger<base::s_int32_t> atomicint32;
#endif
typedef atomicinteger<base::s_int64_t> atomicint64;

M_BASE_NAMESPACE_END
#endif  // ATOMIC_H

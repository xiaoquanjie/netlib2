#ifndef M_BASE_MUTEXLOCK_INCLUDE
#define M_BASE_MUTEXLOCK_INCLUDE

#include "base/config.hpp"
M_BASE_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#ifndef M_WIN32_LEAN_AND_MEAN  
#define WIN32_LEAN_AND_MEAN // 在所有Windows.h包含的地方加上它
#endif
#include <windows.h>
class MutexLock
{
public:
	MutexLock(){
		InitializeCriticalSection(&_cs);
	}
	~MutexLock(){
		DeleteCriticalSection(&_cs);
	}
	void lock(){
		EnterCriticalSection(&_cs);
	}
	void unlock(){
		LeaveCriticalSection(&_cs);
	}
	CRITICAL_SECTION& mutex() {
		return _cs;
	}
private:
	MutexLock(const MutexLock&);
	MutexLock& operator=(const MutexLock&);

	CRITICAL_SECTION _cs;
};
#endif

#ifndef M_PLATFORM_WIN
#include <pthread.h>
class MutexLock
{
public:
	MutexLock(){
		assert(pthread_mutex_init(&_mutex, 0) == 0);
	}
	~MutexLock(){
		pthread_mutex_destroy(&_mutex);
	}
	void lock(){
		assert(pthread_mutex_lock(&_mutex) == 0);
	}
	void unlock(){
		assert(pthread_mutex_unlock(&_mutex) == 0);
	}
	pthread_mutex_t& mutex() {
		return _mutex;
	}
private:
	MutexLock(const MutexLock&);
	MutexLock& operator=(const MutexLock&);

	pthread_mutex_t _mutex;
};
#endif

class ScopedLock
{
public:
	ScopedLock(MutexLock& mutex) :_mutex(mutex){
		_mutex.lock();
	}
	~ScopedLock(){
		_mutex.unlock();
	}
private:
	ScopedLock(const ScopedLock&);
	ScopedLock& operator=(const ScopedLock&);

	MutexLock& _mutex;
};

M_BASE_NAMESPACE_END
#endif
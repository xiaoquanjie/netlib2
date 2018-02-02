#ifndef M_BASE_CONDITION_INCLUDE
#define M_BASE_CONDITION_INCLUDE

#include "base/config.hpp"
#include "base/mutexlock.hpp"
M_BASE_NAMESPACE_BEGIN

class  Condition {
public:
	Condition(MutexLock& mutex);

	~Condition();

	void wait();

	// returns true if time out, false otherwise.
	bool wait(int second);

	void notify();

	void notifyall();

private:
	MutexLock& _mutex;
#ifdef M_PLATFORM_WIN
	CONDITION_VARIABLE _cond;
#else
	pthread_cond_t _cond;
#endif
};

#ifdef M_PLATFORM_WIN
inline Condition::Condition(MutexLock& mutex) 
	:_mutex(mutex){
	InitializeConditionVariable(&_cond);
}

inline Condition::~Condition() {
}

inline void Condition::wait() {
	SleepConditionVariableCS(&_cond, &(_mutex.mutex()), INFINITE);
}

inline bool Condition::wait(int second) {
	DWORD milseds = second * 1000;
	BOOL  ret = SleepConditionVariableCS(&_cond, &(_mutex.mutex()), 
		milseds);
	return (ret == 0 
		&& GetLastError() == ERROR_TIMEOUT);
}

inline void Condition::notify() {
	WakeConditionVariable(&_cond);
}

inline void Condition::notifyall() {
	WakeAllConditionVariable(&_cond);
}
#else
inline Condition::Condition(MutexLock& mutex)
	:_mutex(mutex) {
	pthread_cond_init(&_cond, NULL);
}

inline Condition::~Condition() {
	pthread_cond_destroy(&_cond);
}

inline void Condition::wait() {
	pthread_cond_wait(&_cond, &(_mutex.mutex()));
}

inline bool Condition::wait(int second) {
	struct timespec abstime;
	clock_gettime(CLOCK_REALTIME, &abstime);
	abstime.tv_sec += second;
	return ETIMEDOUT == pthread_cond_timedwait(&_cond, &(_mutex.mutex()), &abstime);
}

inline void Condition::notify() {
	pthread_cond_signal(&_cond);
}

inline void Condition::notifyall() {
	pthread_cond_broadcast(&_cond);
}
#endif
M_BASE_NAMESPACE_END
#endif
#ifndef M_BASE_THREAD_INCLUDE
#define M_BASE_THREAD_INCLUDE

#include "base/config.hpp"
#include <string>
#include <assert.h>

#ifndef M_PLATFORM_WIN
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>
#include <tr1/memory>
#include <stdio.h>

#ifndef gettid
#define gettid() syscall(SYS_gettid)
#endif

#else
#include <memory>
#include <Windows.h>
#include <process.h>
#endif

M_BASE_NAMESPACE_BEGIN

class IRunnable
{
public:
	virtual void run(void* param) = 0;
	virtual ~IRunnable() {}
};

#ifndef M_PLATFORM_WIN
namespace detail
{
	static unsigned int tid() {
		static __thread unsigned int t_thread_id;
		if (__builtin_expect(t_thread_id == 0, 0)) {
			t_thread_id = gettid();
		}
		return t_thread_id;
	}

	static const char* tid_str() {
		static __thread char t_thread_id_str[32];
		static __thread int t_id_str_flag;
		if (__builtin_expect(t_id_str_flag == 0, 0)) {
			t_id_str_flag = 1;
			snprintf(t_thread_id_str, sizeof(t_thread_id_str), "%5d ", tid());
		}
		return t_thread_id_str;
	}
}
#endif

#ifndef M_THREAD_DATA_ID
#ifndef M_PLATFORM_WIN
#define M_THREAD_DATA_ID unsigned int* thrid_;
#define M_THREAD_DATA_ID_STR std::string* thrid_str_;
#else
#define M_THREAD_DATA_ID
#define M_THREAD_DATA_ID_STR
#endif
#endif

#ifndef M_PLATFORM_WIN
#define M_GETHTREADID(x) *x->thrid_= detail::tid();
#define M_GETHTREADID_STR(x) *x->thrid_str_ = detail::tid_str();
#else
#define M_GETHTREADID(x)
#define M_GETHTREADID_STR(x)
#endif

#ifndef M_PLATFORM_WIN
#define M_BINDTHREADID(x,id) x->thrid_ = &id;
#define M_BINDTHREADID_STR(x,idstr) x->thrid_str_ = &idstr;
#else
#define M_BINDTHREADID(x,id)
#define M_BINDTHREADID_STR(x,idstr)
#endif

#ifndef M_THREAD_HANDLE_TYPE
#ifdef M_PLATFORM_WIN
#define M_THREAD_HANDLE_TYPE HANDLE
#else
#define M_THREAD_HANDLE_TYPE pthread_t
#endif
#endif

#ifndef M_CLOSE_THREAD_HANDLE
#ifdef M_PLATFORM_WIN
#define M_CLOSE_THREAD_HANDLE CloseHandle
#else
#define M_CLOSE_THREAD_HANDLE pthread_detach
#endif
#endif

#ifndef M_CUR_THREADID
#ifdef M_PLATFORM_WIN
#define M_CUR_THREADID GetCurrentThreadId()
#else
#define M_CUR_THREADID detail::tid()
#endif
#endif

#ifndef M_MIL_SLEEP
#ifdef M_PLATFORM_WIN
#define M_MIL_SLEEP(milsec) Sleep(milsec)
#else
#define M_MIL_SLEEP(milsec) usleep(milsec * 1000)
#endif
#endif

#ifndef M_THREAD_JOIN
#ifdef M_PLATFORM_WIN
#define M_THREAD_JOIN(handler) WaitForSingleObject(handler, INFINITE)
#else
#define M_THREAD_JOIN(handler) pthread_join(handler, 0)
#endif
#endif

#ifndef M_THREAD_STATIC_RUN
#ifdef M_PLATFORM_WIN
#define M_THREAD_STATIC_RUN  static unsigned int __stdcall run
#else
#define M_THREAD_STATIC_RUN static void* run
#endif
#endif

#ifndef M_PLATFORM_WIN
#define CHECK_THREAD(t,data) if (0!=t) delete data;
#else
#define CHECK_THREAD(t,data) if (INVALID_HANDLE_VALUE==t) delete data;
#endif

namespace thread_
{
	template<typename func_type, typename cls_type>
	struct threaddata{
		func_type func_;
		cls_type* cls_;
		void* param_;
		M_THREAD_DATA_ID;
		M_THREAD_DATA_ID_STR;
	};

	template<typename func_type>
	struct threaddata<func_type, void>{
		func_type func_;
		void* cls_;
		void* param_;
		M_THREAD_DATA_ID;
		M_THREAD_DATA_ID_STR;
	};

	template<>
	struct threaddata<IRunnable, void>{
		std::tr1::shared_ptr<IRunnable> ptr_obj_;
		void* param_;
		M_THREAD_DATA_ID;
		M_THREAD_DATA_ID_STR;
	};

	template<typename func_type, typename cls_type>
	struct thread_runnable{
		M_THREAD_STATIC_RUN(void* param){
			threaddata<func_type, cls_type>* threaddata_ = (threaddata<func_type, cls_type>*)(param);
			M_GETHTREADID(threaddata_);
			M_GETHTREADID_STR(threaddata_);
			((threaddata_->cls_)->*(threaddata_->func_))(threaddata_->param_);
			delete threaddata_;
			return 0;
		}
	};

	template<typename func_type>
	struct thread_runnable<func_type, void>{
		M_THREAD_STATIC_RUN(void* param){
			threaddata<func_type, void>* threaddata_ = (threaddata<func_type, void>*)(param);
			M_GETHTREADID(threaddata_);
			M_GETHTREADID_STR(threaddata_);
			threaddata_->func_(threaddata_->param_);
			delete threaddata_;
			return 0;
		}
	};

	template<>
	struct thread_runnable<IRunnable, void>{
		M_THREAD_STATIC_RUN(void* param){
			threaddata<IRunnable, void>* threaddata_ = (threaddata<IRunnable, void>*)(param);
			M_GETHTREADID(threaddata_);
			M_GETHTREADID_STR(threaddata_);
			threaddata_->ptr_obj_->run(threaddata_->param_);
			delete threaddata_;
			return 0;
		}
	};
}

class thread
{
public:
	template<typename func_type>
	thread(func_type func, void* param) 
		:_thrid(0){
		thread_::threaddata<func_type, void>* threaddata_ = new thread_::threaddata<func_type, void>;
		threaddata_->cls_ = (void*)0;
		threaddata_->func_ = func;
		threaddata_->param_ = param;
		M_BINDTHREADID(threaddata_, _thrid);
		M_BINDTHREADID_STR(threaddata_, _thrid_str);

#ifndef M_PLATFORM_WIN
		int ret = pthread_create(&_handler, 0, &thread_::thread_runnable<func_type, void>::run, threaddata_);
		CHECK_THREAD(ret, threaddata_);
#else
		_handler = (HANDLE)_beginthreadex(0, 0, &thread_::thread_runnable<func_type, void>::run, threaddata_, 0, &_thrid);
		_thrid_str = std::to_string(_thrid);
		CHECK_THREAD(_handler, threaddata_);
#endif
	}

	template<typename func_type, typename cls_type>
	thread(func_type func, cls_type* cls, void* param) 
		:_thrid(0){
		thread_::threaddata<func_type, cls_type>* threaddata_ = new thread_::threaddata<func_type, cls_type>;
		threaddata_->cls_ = cls;
		threaddata_->func_ = func;
		threaddata_->param_ = param;
		M_BINDTHREADID(threaddata_, _thrid);
		M_BINDTHREADID_STR(threaddata_, _thrid_str);

#ifndef M_PLATFORM_WIN
		int ret = pthread_create(&_handler, 0, &thread_::thread_runnable<func_type, cls_type>::run, threaddata_);
		CHECK_THREAD(ret, threaddata_);
#else
		_handler = (HANDLE)_beginthreadex(0, 0, &thread_::thread_runnable<func_type, cls_type>::run, threaddata_, 0, &_thrid);
		_thrid_str = std::to_string(_thrid);
		CHECK_THREAD(_handler, threaddata_);
#endif
	}

	thread(std::tr1::shared_ptr<IRunnable> ptrObj, void* param)
		:_thrid(0){
		thread_::threaddata<IRunnable, void>* threaddata_ = new thread_::threaddata<IRunnable, void>;
		threaddata_->ptr_obj_ = ptrObj;
		threaddata_->param_ = param;
		M_BINDTHREADID(threaddata_, _thrid);
		M_BINDTHREADID_STR(threaddata_, _thrid_str);

#ifndef M_PLATFORM_WIN
		int ret = pthread_create(&_handler, 0, &thread_::thread_runnable<IRunnable, void>::run, threaddata_);
		CHECK_THREAD(ret, threaddata_);
#else
		_handler = (HANDLE)_beginthreadex(0, 0, &thread_::thread_runnable<IRunnable, void>::run, threaddata_, 0, &_thrid);
		_thrid_str = std::to_string(_thrid);
		CHECK_THREAD(_handler, threaddata_);
#endif
	}

	~thread()
	{
#ifdef M_PLATFORM_WIN
		CloseHandle(_handler);
#endif
	}

	void join(){
		M_THREAD_JOIN(_handler);
	}

	void detach(){
		M_CLOSE_THREAD_HANDLE(_handler);
	}

	unsigned int tid()const{
		return _thrid;
	}

	const std::string tid_str()const {
		return _thrid_str;
	}

	static unsigned int ctid(){
		return M_CUR_THREADID;
	}

#ifdef M_PLATFORM_WIN
	static std::string ctid_str() {
		unsigned int id = ctid();
		return std::to_string(id);
	}
#else
	static const char* ctid_str() {
		return detail::tid_str();
	}
#endif

	static void sleep(unsigned int milsec){
		M_MIL_SLEEP(milsec);
	}

private:
	unsigned int _thrid;
	std::string  _thrid_str;
	M_THREAD_HANDLE_TYPE _handler;
};

M_BASE_NAMESPACE_END
#endif
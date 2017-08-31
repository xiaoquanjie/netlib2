#ifndef THREAD_HPP
#define THREAD_HPP

#include "socket/config.hpp"
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


class IRunnable
{
public:
	virtual void run(void* param)=0;
	virtual ~IRunnable(){}
};

#ifndef M_PLATFORM_WIN
namespace detail
{
	extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	void   cacheTid();
	int	   tid();
	const  char* tidString();
}
#endif

#ifndef DEF_THREADID
#ifndef M_PLATFORM_WIN
#define DEF_THREADID unsigned int* thrid_;
#else
#define DEF_THREADID
#endif
#endif

#ifndef M_PLATFORM_WIN
#define DEF_GETHTREADID(x) *x->thrid_= detail::tid();
#else
#define DEF_GETHTREADID(x)
#endif

#ifndef M_PLATFORM_WIN
#define DEF_BINDTHREADID(x,id) x->thrid_ = &id;
#else
#define DEF_BINDTHREADID(x,id)
#endif

namespace thread_
{
	template<typename func_type,typename cls_type>
	struct threaddata
	{
		func_type func_;
		cls_type* cls_;
		void*	  param_;
		DEF_THREADID;
	};

	template<typename func_type>
	struct threaddata<func_type,void>
	{
		func_type func_;
		void*	  cls_;
		void*	  param_;
		DEF_THREADID;
	};

	template<>
	struct threaddata<IRunnable,void>
	{
		std::tr1::shared_ptr<IRunnable> ptr_obj_;
		void*						    param_;
		DEF_THREADID;
	};

	template<typename func_type,typename cls_type>
	struct thread_runnable
	{
#ifndef M_PLATFORM_WIN
		static void* run(void* param)
#else
		static unsigned int __stdcall run(void* param)
#endif
		{
			threaddata<func_type,cls_type>* threaddata_ = (threaddata<func_type,cls_type>*)(param);
			DEF_GETHTREADID(threaddata_);
			((threaddata_->cls_)->*(threaddata_->func_))(threaddata_->param_);
			delete threaddata_;
			return 0;
		}
	};

	template<typename func_type>
	struct thread_runnable<func_type,void>
	{
#ifndef M_PLATFORM_WIN
		static void* run(void* param)
#else
		static unsigned int __stdcall run(void* param)
#endif
		{
			threaddata<func_type,void>* threaddata_ = (threaddata<func_type,void>*)(param);
			DEF_GETHTREADID(threaddata_);
			threaddata_->func_(threaddata_->param_);
			delete threaddata_;
			return 0;
		}
	};

	template<>
	struct thread_runnable<IRunnable,void>
	{
#ifndef M_PLATFORM_WIN
		static void* run(void* param)
#else
		static unsigned int __stdcall run(void* param)
#endif
		{
			threaddata<IRunnable,void>* threaddata_ = (threaddata<IRunnable,void>*)(param);
			DEF_GETHTREADID(threaddata_);
			threaddata_->ptr_obj_->run(threaddata_->param_);
			delete threaddata_;
			return 0;
		}
	};
}

#ifndef M_PLATFORM_WIN
#define CHECK_THREAD(t,data) if (0!=t) delete data;
#else
#define CHECK_THREAD(t,data) if (INVALID_HANDLE_VALUE==t) delete data;
#endif

class thread
{
public:
	template<typename func_type>
	thread(func_type func,void* param):tid_(0)
	{
		thread_::threaddata<func_type,void>* threaddata_ = new thread_::threaddata<func_type,void>;
		threaddata_->cls_  = (void*)0;
		threaddata_->func_ = func;
		threaddata_->param_= param;
		DEF_BINDTHREADID(threaddata_,tid_);

#ifndef M_PLATFORM_WIN
		int ret = pthread_create(&thr_,0,&thread_::thread_runnable<func_type,void>::run,threaddata_);
		CHECK_THREAD(ret,threaddata_);
#else
		thr_ = (HANDLE)_beginthreadex(0,0,&thread_::thread_runnable<func_type,void>::run,threaddata_,0,&tid_);
		CHECK_THREAD(thr_,threaddata_);
#endif
	}

	template<typename func_type,typename cls_type>
	thread(func_type func,cls_type* cls,void* param):tid_(0)
	{
		thread_::threaddata<func_type,cls_type>* threaddata_ = new thread_::threaddata<func_type,cls_type>;
		threaddata_->cls_  = cls;
		threaddata_->func_ = func;
		threaddata_->param_= param;
		DEF_BINDTHREADID(threaddata_,tid_);

#ifndef M_PLATFORM_WIN
		int ret = pthread_create(&thr_,0,&thread_::thread_runnable<func_type,cls_type>::run,threaddata_);
		CHECK_THREAD(ret,threaddata_);
#else
		thr_ = (HANDLE)_beginthreadex(0,0,&thread_::thread_runnable<func_type,cls_type>::run,threaddata_,0,&tid_);
		CHECK_THREAD(thr_,threaddata_);
#endif
	}

	thread(std::tr1::shared_ptr<IRunnable> ptrObj,void* param):tid_(0)
	{
		thread_::threaddata<IRunnable,void>* threaddata_ = new thread_::threaddata<IRunnable,void>;
		threaddata_->ptr_obj_ = ptrObj;
		threaddata_->param_	  = param;
		DEF_BINDTHREADID(threaddata_,tid_);

#ifndef M_PLATFORM_WIN
		int ret = pthread_create(&thr_,0,&thread_::thread_runnable<IRunnable,void>::run,threaddata_);
		CHECK_THREAD(ret,threaddata_);
#else
		thr_ = (HANDLE)_beginthreadex(0,0,&thread_::thread_runnable<IRunnable,void>::run,threaddata_,0,&tid_);
		CHECK_THREAD(thr_,threaddata_);
#endif
	}

	~thread()
	{
#ifdef M_PLATFORM_WIN
		CloseHandle(thr_);
#endif
	}

	void join()
	{
#ifndef M_PLATFORM_WIN
		pthread_join(thr_,0);
#else
		WaitForSingleObject(thr_,INFINITE);
#endif
	}

	void detach()
	{
#ifndef M_PLATFORM_WIN
		pthread_detach(thr_);
#else
		CloseHandle(thr_);
#endif
	}

	unsigned int tid()const
	{
		return tid_;
	}

	static unsigned int ctid()
	{
#ifndef M_PLATFORM_WIN
		return detail::tid();
#else
		return ::GetCurrentThreadId();
#endif
	}

	static void sleep(unsigned int milsec)
	{
#ifndef M_PLATFORM_WIN
		usleep(milsec*1000);
#else
		::Sleep(milsec);
#endif
	}

private:
#ifndef M_PLATFORM_WIN
	pthread_t thr_;
#else
	HANDLE	  thr_;
#endif
	unsigned int tid_;
};


#endif
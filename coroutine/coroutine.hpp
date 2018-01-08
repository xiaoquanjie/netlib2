#ifndef M_COROUTINE_COROUTINE_INCLUDE
#define M_COROUTINE_COROUTINE_INCLUDE

#include "coroutine/config.hpp"
#include "base/slist.hpp"
#include "base/svector.hpp"
#include "base/tls.hpp"
#include "base/circular_queue.hpp"
#include "base/mutexlock.hpp"
#include <map>
#include "base/thread.hpp"
M_COROUTINE_NAMESPACE_BEGIN

typedef void(*_coroutine_func_)(void*ud);
#define COROUTINE_READY   (1)
#define COROUTINE_RUNNING (2)
#define COROUTINE_SUSPEND (3)
#define COROUTINE_DEAD	  (4)

#define DEFAULT_COROUTINE (1024)

#ifdef M_PLATFORM_WIN
struct _coroutine_ {
	int _id;
	int _status;
	LPVOID _ctx;
	void* _data;
	_coroutine_func_ _function;
};
typedef std::map<int, _coroutine_*> CoroutineMap;
struct _schedule_ {
	_schedule_() {
		_cap = 0;
		_nco = 0;
		_curco = 0;
		_ctx = 0;
		_co = 0;
	}
	int _cap;
	int _nco;
	LPVOID _ctx;
	_coroutine_** _co;
	_coroutine_* _curco;
	base::svector<int> _freeid;
};
#else
#define M_COROUTINE_STACK_SIZE  4*1024*1024
struct _coroutine_ {
	int			_id;
	int			_status;
	char*		_stack;
	int			_size;
	int			_cap;
	ucontext_t	_ctx;
	void*		_data;
	_coroutine_func_ _function;
};
typedef std::map<int, _coroutine_*> CoroutineMap;
struct _schedule_ {
	_schedule_() {
		_cap = 0;
		_nco = 0;
		_curco = 0;
		_co = 0;
		_pri_stack = false;
	}
	int _cap;
	int _nco;
	bool _pri_stack;
	ucontext_t _ctx;
	_coroutine_** _co;
	_coroutine_* _curco;
	base::svector<int> _freeid;
	char _stack[M_COROUTINE_STACK_SIZE];
};
#endif

#define _tlsdata_ base::tlsdata
#define gschedule _tlsdata_<_schedule_>::data()
#define gpristacksize _tlsdata_<unsigned int>::data()

template<typename T>
class basecoroutine {
public:
	// init environment
	static bool initEnv(unsigned int stack_size = 128 * 1204, bool pri_stack = false);
	// create one new coroutine
	static int create(_coroutine_func_ routine, void* data);
	// close
	static void close();
	// resume
	static void resume(int co_id);
	// yield
	static void yield();
	// current coroutine id
	static unsigned int curid() {
		_schedule_& schedule = gschedule;
		if (schedule._curco) {
			return schedule._curco->_id;
		}
		return -1;
	}
	static void destroy(int co_id);

private:
	static _coroutine_* _alloc_co_(_coroutine_func_ routine, void* data) {
		_schedule_& schedule = gschedule;
		_coroutine_* co = (_coroutine_*)malloc(sizeof(_coroutine_));
		co->_function = routine;
		co->_data = data;
		co->_status = COROUTINE_READY;
		if (!schedule._freeid.empty()) {
			int id = schedule._freeid.back();
			schedule._freeid.pop_back();
			schedule._co[id] = co;
			co->_id = id;
		}
		else {
			if (schedule._nco >= schedule._cap) {
				schedule._co = (_coroutine_**)realloc(schedule._co, schedule._cap * 2 * sizeof(_coroutine_*));
				memset(schedule._co + schedule._cap, 0, schedule._cap * sizeof(_coroutine_*));
				schedule._co[schedule._cap] = co;
				schedule._cap *= 2;
				co->_id = schedule._nco++;
			}
			else {
				int id = schedule._nco++;
				schedule._co[id] = co;
				co->_id = id;
			}
		}
		return co;
	}
};

// private stack is invalid for windows fiber
class Coroutine : public basecoroutine<Coroutine> {
public:
	// init environment
	static bool initEnv(unsigned int stack_size = 128 * 1204, bool pri_stack = false) {
		return basecoroutine::initEnv(stack_size, pri_stack);
	}
	// create one new coroutine
	static int create(_coroutine_func_ routine, void* data) {
		return basecoroutine::create(routine, data);
	}
	// close
	static void close() {
		return basecoroutine::close();
	}
	// resume
	static void resume(int co_id) {
		return basecoroutine::resume(co_id);
	}
	// yield
	static void yield() {
		return basecoroutine::yield();
	}
	static unsigned int curid() {
		return basecoroutine::curid();
	}
	static void destroy(int co_id) {
		basecoroutine::destroy(co_id);
	}
};

struct co_task {
	void* p;
	void(*func)(void*);
};

struct co_task_wrapper {
	int co_id;
	co_task** task;
};

typedef base::slist<co_task*> tasklist;
typedef base::svector<co_task*> taskvector;
typedef base::slist<co_task_wrapper*> taskwrapperlist;
typedef base::svector<co_task_wrapper*> taskwrappervector;
typedef base::circular_queue<int> id_circular_queue;
typedef base::slist<int> intlist;

#define gfreetaskvec _tlsdata_<taskvector,0>::data()
#define gworktasklist _tlsdata_<tasklist,0>::data()
#define gfreecovec	_tlsdata_<taskwrappervector,0>::data()
#define gallcolist _tlsdata_<taskwrapperlist,0>::data()
#define gresumetaskque _tlsdata_<id_circular_queue,0>::data()

template<typename T>
struct BaseCoroutineTask {
protected:
	static std::map<int, intlist*> _queue_map;
	static base::MutexLock _mutex;
};

template<typename T>
std::map<int, intlist*> BaseCoroutineTask<T>::_queue_map;

template<typename T>
base::MutexLock BaseCoroutineTask<T>::_mutex;

class CoroutineTask : public BaseCoroutineTask<CoroutineTask>{
public:
	static bool doResume() {
		if (Coroutine::curid() == -1) {
			id_circular_queue& idqueue = gresumetaskque;
			if (!idqueue.empty()) {
				int id = -1;
				idqueue.pop_front(id);
				Coroutine::resume(id);
				return true;
			}
		}
		return false;
	}

	static void doResume(int id) {
		if (Coroutine::curid() == -1) {
			Coroutine::resume(id);
		}
	}

	static void doThrResume() {
		if (Coroutine::curid() == -1) {
			unsigned int thrid = base::thread::ctid();
			intlist tmp;
			_mutex.lock();
			base::ScopedLock scoped(_mutex);
			std::map<int, intlist*>::iterator iter = _queue_map.find(thrid);
			if (iter != _queue_map.end())
				tmp.join(*iter->second);
			_mutex.unlock();
			int co_id;
			while (!tmp.empty()) {
				 co_id = tmp.front();
				 tmp.pop_front();
				 Coroutine::resume(co_id);
			}
		}
	}

	static void addResume(int id) {
		id_circular_queue& idqueue = gresumetaskque;
		idqueue.push_back(id);
	}

	static void addResume(int thrid, int co_id) {
		_mutex.lock();
		std::map<int, intlist*>::iterator iter = _queue_map.find(thrid);
		if (iter != _queue_map.end()) {
			iter->second->push_back(co_id);
		}
		else {
			intlist* pslist = new intlist;
			pslist->push_back(co_id);
			_queue_map[thrid] = pslist;
		}
		_mutex.unlock();
	}

	static bool doTask() {
		if (Coroutine::curid() == -1) {
			co_task* task = _get_task();
			if (task) {
				co_task_wrapper* wrapper = _get_co_task_wrapper();
				wrapper->task = &task;
				Coroutine::resume(wrapper->co_id);
				return true;
			}
		}
		return false;
	}

	static void doTask(void(*func)(void*), void*p) {
		if (Coroutine::curid() == -1) {
			co_task* task = 0;
			taskvector& tl = gfreetaskvec;
			if (!tl.empty()) {
				task = tl.back();
				tl.pop_back();
			}
			else {
				task = (co_task*)malloc(sizeof(co_task));
			}
			task->func = func;
			task->p = p;
			co_task_wrapper* wrapper = _get_co_task_wrapper();
			wrapper->task = &task;
			Coroutine::resume(wrapper->co_id);
		}
	}

	static void addTask(void(*func)(void*), void*p) {
		co_task* task = 0;
		taskvector& tl = gfreetaskvec;
		if (!tl.empty()) {
			task = tl.back();
			tl.pop_back();
		}
		else {
			task = (co_task*)malloc(sizeof(co_task));
		}
		task->func = func;
		task->p = p;
		gworktasklist.push_back(task);
	}

	static void clrTask() {
		if (Coroutine::curid() == -1) {
			taskvector& tl = gfreetaskvec;
			while (tl.size()) {
				free(tl.back());
				tl.pop_back();
			}
			tasklist& wtl = gworktasklist;
			while (wtl.size()) {
				free(wtl.front());
				wtl.pop_front();
			}
			taskwrapperlist& wtw = gallcolist;
			while (wtw.size()) {
				co_task_wrapper* wrapper = wtw.front();
				Coroutine::destroy(wrapper->co_id);
				free(wrapper);
				wtw.pop_front();
			}
			id_circular_queue& idqueue = gresumetaskque;
			idqueue.clear();

			unsigned int thrid = base::thread::ctid();
			base::ScopedLock scoped(_mutex);
			std::map<int, intlist*>::iterator iter = _queue_map.find(thrid);
			if (iter != _queue_map.end())
				iter->second->clear();
		}
	}

private:

	static co_task* _get_task() {
		tasklist& tl = gworktasklist;
		if (tl.empty()) {
			return 0;
		}
		else {
			co_task* task = tl.front();
			tl.pop_front();
			return task;
		}
	}

	static co_task_wrapper* _get_co_task_wrapper() {
		co_task_wrapper* wrapper = 0;
		taskwrappervector& tw = gfreecovec;
		if (!tw.empty()) {
			wrapper = tw.back();
			tw.pop_back();
		}
		else {
			wrapper = (co_task_wrapper*)malloc(sizeof(co_task_wrapper));
			wrapper->co_id = Coroutine::create(_co_task_func_, wrapper);
			gallcolist.push_back(wrapper);
		}
		wrapper->task = 0;
		return wrapper;
	}

	static void _co_task_func_(void* p) {
		co_task_wrapper* wrapper = (co_task_wrapper*)p;
		taskvector& tl = gfreetaskvec;
		taskwrappervector& tw = gfreecovec;
		co_task* task = 0;
		while (wrapper->task) {
			task = *(wrapper->task);
			if (task) {
				task->func(task->p);
				tl.push_back(task);
			}
			tw.push_back(wrapper);
			Coroutine::yield();
		}
		free(wrapper);
	}
};

M_COROUTINE_NAMESPACE_END
#endif // end for M_COROUTINE_COROUTINE_INCLUDE

#include "coroutine/co_linux_impl.hpp"
#include "coroutine/co_win_impl.hpp"
#ifndef M_COROUTINE_COWIN_IMPL_INCLUDE
#define M_COROUTINE_COWIN_IMPL_INCLUDE

#ifdef M_PLATFORM_WIN
M_COROUTINE_NAMESPACE_BEGIN

template<int N>
void __stdcall pub_coroutine(LPVOID p);

template<typename T>
bool basecoroutine<T>::initEnv(unsigned int stack_size, bool pri_stack) {
	_schedule_& schedule = gschedule;
	if (!schedule._ctx) {
		gpristacksize = stack_size;
		LPVOID ctx = ::ConvertThreadToFiberEx(0, FIBER_FLAG_FLOAT_SWITCH);
		if (!ctx) {
			DWORD error = ::GetLastError();
			return false;
		}
		else {
			schedule._ctx = ctx;
			schedule._curco = 0;
			schedule._cap = DEFAULT_COROUTINE;
			schedule._co = (_coroutine_**)malloc(sizeof(_coroutine_*)*schedule._cap);
			memset(schedule._co, 0, sizeof(_coroutine_*)*schedule._cap);
		}
	}
	return true;
}

template<typename T>
int basecoroutine<T>::create(_coroutine_func_ routine, void* data) {
	LPVOID ctx = ::CreateFiberEx(gpristacksize, 0, FIBER_FLAG_FLOAT_SWITCH, pub_coroutine<0>, 0);
	if (ctx) {
		_coroutine_* co = _alloc_co_(routine, data);
		co->_ctx = ctx;
		return co->_id;
	}
	else {
		DWORD error = ::GetLastError();
		return -1;
	}
}

template<typename T>
void basecoroutine<T>::close() {
	_schedule_& schedule = gschedule;
	if (!schedule._curco) {
		for (int i = 0; i < schedule._cap; ++i) {
			_coroutine_* co = schedule._co[i];
			if (co) {
				::DeleteFiber(co->_ctx);
				free(co);
			}
		}
		schedule._freeid.clear();
		schedule._nco = 0;
		schedule._cap = 0;
		schedule._ctx = 0;
		schedule._curco = 0;
		free(schedule._co);
		schedule._co = 0;
		::ConvertFiberToThread();
	}
}

template<typename T>
void basecoroutine<T>::resume(int co_id) {
	_schedule_& schedule = gschedule;
	if (schedule._curco) {
		return;
	}
	if (co_id < 0 || co_id >= schedule._cap) {
		return;
	}
	_coroutine_* co = schedule._co[co_id];
	if (co) {
		switch (co->_status) {
		case COROUTINE_READY:
		case COROUTINE_SUSPEND:
			co->_status = COROUTINE_RUNNING;
			schedule._curco = co;
			::SwitchToFiber(co->_ctx);
			if (co->_status == COROUTINE_DEAD) {
				::DeleteFiber(co->_ctx);
				free(co);
				schedule._co[co_id] = 0;
				schedule._freeid.push_back(co_id);
			}
			schedule._curco = 0;
			break;
		}
	}
}

template<typename T>
void basecoroutine<T>::yield() {
	_schedule_& schedule = gschedule;
	if (schedule._curco) {
		schedule._curco->_status = COROUTINE_SUSPEND;
		::SwitchToFiber(schedule._ctx);
	}
}

template<typename T>
void basecoroutine<T>::destroy(int co_id) {
	_schedule_& schedule = gschedule;
	if (!schedule._curco || schedule._curco->_id != co_id) {
		if (co_id >= 0 && co_id < schedule._cap) {
			_coroutine_* co = schedule._co[co_id];
			if (co) {
				::DeleteFiber(co->_ctx);
				free(co);
				schedule._co[co_id] = 0;
				schedule._freeid.push_back(co_id);
			}
		}
	}
}

template<int N>
void __stdcall pub_coroutine(LPVOID p) {
	_schedule_& schedule = gschedule;
	if (schedule._curco) {
		(schedule._curco->_function)(schedule._curco->_data);
		schedule._curco->_status = COROUTINE_DEAD;
		::SwitchToFiber(schedule._ctx);
	}
}

M_COROUTINE_NAMESPACE_END
#endif
#endif
#ifndef M_COROUTINE_COLINUX_IMPL_INCLUDE
#define M_COROUTINE_COLINUX_IMPL_INCLUDE

#ifndef M_PLATFORM_WIN
M_COROUTINE_NAMESPACE_BEGIN

template<int N>
void pub_coroutine();

template<int N>
void save_stack(_coroutine_* c, char* top);

template<typename T>
bool basecoroutine<T>::initEnv(unsigned int stack_size, bool pri_stack) {
	_schedule_& schedule = gschedule;
	if (!schedule._co) {
		schedule._pri_stack = pri_stack;
		gpristacksize = stack_size;
		schedule._curco = 0;
		schedule._cap = DEFAULT_COROUTINE;
		schedule._co = (_coroutine_**)malloc(sizeof(_coroutine_*)*schedule._cap);
		memset(schedule._co, 0, sizeof(_coroutine_*)*schedule._cap);
	}
}

template<typename T>
int basecoroutine<T>::create(_coroutine_func_ routine, void* data) {
	_schedule_& schedule = gschedule;
	if (schedule._co) {
		_coroutine_* co = _alloc_co_(routine, data);
		assert(getcontext(&co->_ctx) == 0);
		co->_ctx.uc_link = &schedule._ctx;
		co->_size = 0;
		if (schedule._pri_stack) {
			co->_cap = gpristacksize;
			co->_stack = (char*)malloc(co->_cap);
			co->_ctx.uc_stack.ss_sp = co->_stack;
			co->_ctx.uc_stack.ss_size = co->_cap;
		}
		else {
			co->_stack = 0;
			co->_cap = 0;
			co->_ctx.uc_stack.ss_sp = schedule._stack;
			co->_ctx.uc_stack.ss_size = M_COROUTINE_STACK_SIZE;
		}
		makecontext(&co->_ctx, pub_coroutine<0>, 0);
		return co->_id;
	}
	return -1;
}

template<typename T>
void basecoroutine<T>::close() {
	_schedule_& schedule = gschedule;
	if (!schedule._curco) {
		for (int i = 0; i < schedule._cap; ++i) {
			_coroutine_* co = schedule._co[i];
			if (co) {
				free(co->_stack);
				free(co);
			}
		}
	}
	schedule._freeid.clear();
	schedule._nco = 0;
	schedule._cap = 0;
	schedule._curco = 0;
	free(schedule._co);
	schedule._co = 0;
	schedule._pri_stack = false;
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
		case COROUTINE_SUSPEND:
			if (!schedule._pri_stack)
				memcpy(schedule._stack + M_COROUTINE_STACK_SIZE - co->_size, co->_stack, co->_size);
		case COROUTINE_READY:
			co->_status = COROUTINE_RUNNING;
			schedule._curco = co;
			swapcontext(&schedule._ctx, &co->_ctx);
			if (co->_status == COROUTINE_DEAD) {
				free(co->_stack);
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
		_coroutine_* co = schedule._curco;
		if (!schedule._pri_stack)
			save_stack<0>(co, schedule._stack + M_COROUTINE_STACK_SIZE);
		co->_status = COROUTINE_SUSPEND;
		swapcontext(&co->_ctx, &schedule._ctx);
	}
}

template<typename T>
void basecoroutine<T>::destroy(int co_id) {
	_schedule_& schedule = gschedule;
	if (!schedule._curco || schedule._curco->_id != co_id) {
		if (co_id >= 0 && co_id < schedule._cap) {
			_coroutine_* co = schedule._co[co_id];
			if (co) {
				free(co->_stack);
				free(co);
				schedule._co[co_id] = 0;
				schedule._freeid.push_back(co_id);
			}
		}
	}
}

template<int N>
void pub_coroutine() {
	_schedule_& schedule = gschedule;
	if (schedule._curco) {
		(schedule._curco->_function)(schedule._curco->_data);
		schedule._curco->_status = COROUTINE_DEAD;
		swapcontext(&schedule._curco->_ctx, &schedule._ctx);
	}
}

template<int N>
void save_stack(_coroutine_* c, char* top) {
	char dummy = 0;
	assert(top - &dummy <= M_COROUTINE_STACK_SIZE);
	c->_size = top - &dummy;
	if (c->_cap < c->_size) {
		free(c->_stack);
		c->_cap = c->_size;
		c->_stack = (char*)malloc(c->_cap);
	}
	memcpy(c->_stack, &dummy, c->_size);
}

M_COROUTINE_NAMESPACE_END
#endif
#endif
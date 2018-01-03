#ifndef M_BASE_CIRCULAR_QUEUE_INCLUDE
#define M_BASE_CIRCULAR_QUEUE_INCLUDE

#include "base/config.hpp"
M_BASE_NAMESPACE_BEGIN

template<typename T>
class circular_queue {
protected:
	size_t _cap;
	size_t _head;
	size_t _tail;
	T* _element;

public:
	circular_queue(size_t cap) {
		_head = 0;
		_tail = 0;
		_cap = cap + 1;
		_element = (T*)malloc(sizeof(T)*_cap);
	}

	circular_queue() {
		_head = 0;
		_tail = 0;
		_cap = 1024 + 1;
		_element = (T*)malloc(sizeof(T)*_cap);
	}

	~circular_queue() {
		clear();
		free(_element);
	}

	void clear() {
		while (true) {
			if (_head == _tail)
				break;
			T* p = &_element[_head];
			p->~T();
			_head = (_head + 1) % _cap;
		}
	}

	size_t capacity()const {
		return _cap - 1;
	}

	bool full()const {
		return (size() == (_cap - 1));
	}

	bool empty()const {
		return (_head == _tail);
	}

	size_t size()const {
		return (_cap - _head + _tail) % _cap;
	}

	T& front() {
		return _element[_head];
	}

	void pop_front() {
		if (_head != _tail) {
			_element[_head].~T();
			_head = (_head + 1) % _cap;
		}
	}

	bool pop_front(T& t) {
		if (_head != _tail) {
			t = _element[_head];
			_element[_head].~T();
			_head = (_head + 1) % _cap;
			return true;
		}
		else {
			return false;
		}
	}

	T& back() {
		return _element[(_cap + _tail - 1) % _cap];
	}

	void pop_back() {
		if (_head != _tail) {
			_tail = (_cap + _tail - 1) % _cap;
			_element[_tail].~T();
		}
	}

	bool pop_back(T& t) {
		if (_head != _tail) {
			_tail = (_cap + _tail - 1) % _cap;
			t = _element[_tail];
			_element[_tail].~T();
			true;
		}
		else {
			return false;
		}
	}

	bool push_back(const T& t) {
		if (full()) {
			return false;
		}
		else {
			_element[_tail] = t;
			_tail = (_tail + 1) % _cap;
			return true;
		}
	}
};

M_BASE_NAMESPACE_END
#endif
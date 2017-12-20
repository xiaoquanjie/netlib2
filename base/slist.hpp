#ifndef M_BASE_SLIST_INCLUDE
#define M_BASE_SLIST_INCLUDE

#include "base/config.hpp"
M_BASE_NAMESPACE_BEGIN

template<class T>
class slist
{
protected:
	struct Node{
		T val;
		Node *next;
	};
public:
	inline slist();

	inline ~slist();
	
	inline void push_back(const T& t);

	inline T front();

	inline void pop_front();

	inline void clear();
	
	inline int size()const;

	inline bool empty()const;

	void swap(slist<T>& other) {
		if (this == &other)
			return;

		int c = other._count;
		other._count = this->_count;
		this->_count = c;
		Node* h = other._head;
		other._head = this->_head;
		this->_head = h;
		h = other._tail;
		other._tail = this->_tail;
		this->_tail = h;
	}

	inline void join(slist<T>& other);

private:
	int   _count;
	Node* _head;
	Node* _tail;
};

template<class T>
inline slist<T>::slist(){
	_tail = _head = NULL;
	_count = 0;
}

template<class T>
inline slist<T>::~slist(){
	Node *p, *pnext;
	for (p = _head; p != NULL; p = pnext){
		pnext = p->next;
		delete p;
	}
	_count = 0;
	_tail = _head = 0;
}

template<class T>
inline void slist<T>::push_back(const T& t) {
	Node* pnode = new Node;
	pnode->val = t;
	pnode->next = 0;
	if (_tail) {
		_tail->next = pnode;
		_tail = pnode;
	}
	else
		_tail = pnode;
	if (!_head)
		_head = _tail;
	++_count;
}

template<class T>
inline T slist<T>::front() {
	return _head->val;
}

template<class T>
inline void slist<T>::pop_front() {
	if (_head) {
		Node* pnode = _head->next;
		delete _head;
		if (_head == _tail)
			_tail = pnode;
		_head = pnode;
		_count--;
	}
}

template<class T>
inline int slist<T>::size()const{
	return _count;
}

template<class T>
inline bool slist<T>::empty()const {
	return (_count == 0);
}

template<class T>
inline void slist<T>::clear()
{
	Node *cur = _head;
	while (cur != NULL)
	{
		Node* next = cur->next;
		free(cur);
		cur = next;
	}
	_head = NULL;
	_count = 0;
}

template<class T>
inline void slist<T>::join(slist<T>& other) {
	if (this == &other)
		return;
	if (_tail) {
		_tail->next = other._head;
		_tail = other._tail;
	}
	else {
		_head = other._head;
		_tail = other._tail;
	}
	other._head = other._tail = 0;
	this->_count += other._count;
	other._count = 0;
}

M_BASE_NAMESPACE_END
#endif
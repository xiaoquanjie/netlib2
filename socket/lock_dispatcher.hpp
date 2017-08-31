/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/8/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_LOCK_DISPATCHER_INCLUDE
#define M_LOCK_DISPATCHER_INCLUDE

M_SOCKET_NAMESPACE_BEGIN
#define M_LOCK_LIST_SIZE 64

template<typename LockType=MutexLock>
class LockDispatcher
{
public:
	M_SOCKET_DECL LockDispatcher();

	M_SOCKET_DECL ~LockDispatcher();

	M_SOCKET_DECL LockType& GetLock(s_uint32_t idx);

private:
	LockDispatcher(const LockDispatcher&);
	LockDispatcher& operator=(const LockDispatcher&);

	LockType _locks[M_LOCK_LIST_SIZE];
};

template<typename LockType>
LockDispatcher<LockType>::LockDispatcher()
{

}

template<typename LockType>
LockDispatcher<LockType>::~LockDispatcher()
{

}

template<typename LockType>
LockType& LockDispatcher<LockType>::GetLock(s_uint32_t idx)
{
	return _locks[idx%M_LOCK_LIST_SIZE];
}

M_SOCKET_NAMESPACE_END
#endif
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

#ifndef M_EPOLL_ACCESS_INCLUDE
#define M_EPOLL_ACCESS_INCLUDE

#include "coroutine/coroutine.hpp"
#include "socket/linux_epoll.hpp"
#ifndef M_PLATFORM_WIN

// ipv4 or ipv6 bit(0)
#define M_Impl_S_V4(impl)\
	M_CLR_BIT(impl._core->_state,0)
#define M_Impl_S_V6(impl)\
	M_SET_BIT(impl._core->_state,0)
#define M_Impl_G_V(impl)\
	M_GET_BIT(impl._core->_state,0)

// bind bit(1)
#define M_Impl_S_Bind(impl)\
	M_SET_BIT(impl._core->_state,1)
#define M_Impl_S_Unbind(impl)\
	M_CLR_BIT(impl._core->_state,1)
#define M_Impl_G_Bind(impl)\
	M_GET_BIT(impl._core->_state,1)

// block mode bit(2)
#define M_Impl_S_Block(impl)\
	M_SET_BIT(impl._core->_state,2)
#define M_Impl_G_Block(impl)\
	M_GET_BIT(impl._core->_state,2)
#define M_Impl_S_Nonblock(impl)\
	M_CLR_BIT(impl._core->_state,2)
#define M_Impl_G_Nonblock(impl)\
	(!M_GET_BIT(impl._core->_state,2))

// type bit(3~4)
#define M_Impl_S_Type(impl,type)\
	M_SET_MBIT(impl._core->_state,type,3,4)
#define M_Impl_G_Type(impl)\
	M_GET_MBIT(impl._core->_state,3,4)

// accept flag bit(5)
#define M_Impl_G_Accept_Flag(impl)\
	M_GET_BIT(impl._core->_state,5)
#define M_Impl_S_Accept_Flag(impl)\
	M_SET_BIT(impl._core->_state,5)
#define M_Impl_C_Accept_Flag(impl)\
	M_CLR_BIT(impl._core->_state,5)

// connect flag bit(6)
#define M_Impl_G_Connect_Flag(impl)\
	M_GET_BIT(impl._core->_state,6)
#define M_Impl_S_Connect_Flag(impl)\
	M_SET_BIT(impl._core->_state,6)
#define M_Impl_C_Connect_Flag(impl)\
	M_CLR_BIT(impl._core->_state,6)

// write flag bit(7)
#define M_Impl_G_Write_Flag(impl)\
	M_GET_BIT(impl._core->_state,7)
#define M_Impl_S_Write_Flag(impl)\
	M_SET_BIT(impl._core->_state,7)
#define M_Impl_C_Write_Flag(impl)\
	M_CLR_BIT(impl._core->_state,7)

// read flag bit(8)
#define M_Impl_G_Read_Flag(impl)\
	M_GET_BIT(impl._core->_state,8)
#define M_Impl_S_Read_Flag(impl)\
	M_SET_BIT(impl._core->_state,8)
#define M_Impl_C_Read_Flag(impl)\
	M_CLR_BIT(impl._core->_state,8)

// close flag bit(9)
#define M_Impl_G_Close_Flag(impl)\
	M_GET_BIT(impl._core->_state,9)
#define M_Impl_S_Close_Flag(impl)\
	M_SET_BIT(impl._core->_state,9)
#define M_Impl_C_Close_Flag(impl)\
	M_CLR_BIT(impl._core->_state,9)

#define M_Impl_Fd(impl)\
	impl._core->_fd
#define M_Impl_State(impl)\
	impl._core->_state
#define M_Impl_Epoll(impl)\
	impl._core->_epoll
#define M_Impl_Op(impl)\
	impl._core->_op
#define M_Impl_Aop(impl)\
	impl._core->_op._aop._oper
#define M_Impl_Cop(impl)\
	impl._core->_op._cop._oper
#define M_Impl_Rop(impl)\
	impl._core->_op._rop._oper
#define M_Impl_Wop(impl)\
	impl._core->_op._wop._oper

#define M_Impl_Mutex(impl)\
	(impl._core->_mutex)

#define M_Swap_Handler(type,handler1,handler2)\
	handler1.swap(const_cast<type>(handler2));

using namespace coroutine;

M_SOCKET_NAMESPACE_BEGIN

inline void EpollService::Access::ConstructImpl(EpollService& service, SocketImpl& impl, s_uint16_t type) {
	impl.Init();
	M_Impl_S_V4(impl);
	M_Impl_S_Unbind(impl);
	M_Impl_S_Block(impl);
	M_Impl_S_Type(impl, (s_uint8_t)type);
}

inline void EpollService::Access::DestroyImpl(EpollService& service, SocketImpl& impl) {
	ConstructImpl(service, impl, E_NULL_SOCKET_TYPE);
}

inline bool EpollService::Access::IsOpen(EpollService& service, SocketImpl& impl, SocketError& error) {
	return (M_Impl_Fd(impl) != M_INVALID_SOCKET);
}

template<typename GettableOptionType>
inline void EpollService::Access::GetOption(EpollService& service, SocketImpl& impl,
	GettableOptionType& opt, SocketError& error) {
	socklen_t len = opt.Size();
	s_int32_t ret = g_getsockopt(M_Impl_Fd(impl), opt.Level(), opt.Name(), (char*)opt.Data(), &len);
	M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
}

template<typename SettableOptionType>
inline void EpollService::Access::SetOption(EpollService& service, SocketImpl& impl,
	const SettableOptionType& opt, SocketError& error) {
	s_int32_t ret = g_setsockopt(M_Impl_Fd(impl), opt.Level(), opt.Name(), (char*)opt.Data(), opt.Size());
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename EndPoint>
inline EndPoint EpollService::Access::RemoteEndPoint(EndPoint, EpollService& service,
	const SocketImpl& impl, SocketError& error) {
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetRemoteEndPoint(M_Impl_Fd(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template<typename EndPoint>
inline EndPoint EpollService::Access::LocalEndPoint(EndPoint, EpollService& service,
	const SocketImpl& impl, SocketError& error) {
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetLocalEndPoint(M_Impl_Fd(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

inline void EpollService::Access::Cancel(EpollService& service, SocketImpl& impl, SocketError& error) {
	if (M_Impl_G_Bind(impl)) {
		CtlEpoll(service, impl, 0, M_EPOLL_CTL_DEL, 0, error);
	}
}

inline void EpollService::Access::CtlEpoll(EpollService& service, SocketImpl& impl
	, EpollService::OperationSet* opset, s_int32_t flag, s_int32_t events, SocketError& error) 
{
	if (M_EPOLL_CTL_ADD == flag && M_Impl_Epoll(impl) == -1) {
		ScopedLock scoped_l(service._mutex);
		if (!service._implvector.empty()) {
			service._implidx++;
			s_uint32_t size = (s_uint32_t)service._implvector.size();
			IoServiceImpl& serviceimpl = *(service._implvector[service._implidx%size]);
			M_Impl_Epoll(impl) = serviceimpl._handler;
			serviceimpl._fdcnt++;
		}
	}
	if (M_Impl_Epoll(impl) == -1) {
		error = SocketError(M_ERR_NOT_SERVICE);
		return;
	}
	epoll_event_t event;
	event.events = events;
	event.data.ptr = opset;
	s_int32_t ret = g_epoll_ctl(M_Impl_Epoll(impl), flag, M_Impl_Fd(impl), &event);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

inline void EpollService::Access::Run(EpollService& service, SocketError& error) {
	IoServiceImpl* psimpl = _CreateIoImpl(service, error);
	if (psimpl) {
		_DoRun(service, *psimpl, false, error);
		_ReleaseIoImpl(service, psimpl);
	}
}

inline void EpollService::Access::CoRun(EpollService& service, SocketError& error) {
	IoServiceImpl* psimpl = _CreateIoImpl(service, error);
	if (psimpl) {
		Coroutine::initEnv();
		_DoRun(service, *psimpl, true, error);
		_ReleaseIoImpl(service, psimpl);
		CoroutineTask::clrTask();
		Coroutine::close();
	}
}

inline void EpollService::Access::Stop(EpollService& service, SocketError& error) {
	ScopedLock scoped_l(service._mutex);
	for (EpollService::IoServiceImplVector::iterator iter = service._implvector.begin();
		iter != service._implvector.end(); ++iter) {
		EpollService::OperationSet* opset = new EpollService::OperationSet;
		opset->_type = E_FINISH_OP;
		socket_t fd = g_socket(M_AF_INET, M_SOCK_STREAM, M_IPPROTO_IP);
		epoll_event_t event;
		event.events = M_EPOLLERR | M_EPOLLIN | M_EPOLLOUT;
		event.data.ptr = opset;
		s_int32_t ret = g_epoll_ctl((*iter)->_handler, M_EPOLL_CTL_ADD, fd, &event);
		if (ret != 0) {
			delete opset;
			g_closesocket(fd);
			M_DEFAULT_SOCKET_ERROR2(error);
		}
	}
}

inline bool EpollService::Access::Stopped(const EpollService& service) {
	ScopedLock scoped_lock(service._mutex);
	return (service._implvector.empty() ? true : false);
}

//////////////////////////////////////////////////////////////////////////////////////////

inline EpollService::IoServiceImpl* 
EpollService::Access::_CreateIoImpl(EpollService& service, SocketError& error)
{
	IoServiceImpl& simpl = service.GetServiceImpl();
	if (simpl._handler == 0) {
		error = SocketError(M_ERR_IOCP_INVALID);
		return 0;
	}
	simpl._service = &service;
	service._mutex.lock();
	service._implmap[simpl._handler] = &simpl;
	service._implvector.push_back(&simpl);
	++service._implcnt;
	service._mutex.unlock();
	return &simpl;
}

inline void EpollService::Access::_ReleaseIoImpl(
	EpollService& service, IoServiceImpl* simpl)
{
	simpl->_mutex.lock();
	while (simpl->_closereqs.size()) {
		SocketClose* close = simpl->_closereqs.front();
		simpl->_closereqs.pop_front();
		delete close;
	}
	while (simpl->_closereqs2.size()) {
		SocketClose* close = simpl->_closereqs2.front();
		simpl->_closereqs2.pop_front();
		delete close;
	}
	while (simpl->_taskvec.size()) {
		free(simpl->_taskvec.back());
		simpl->_taskvec.pop_back();
	}
	simpl->Close();
	simpl->_mutex.unlock();

	service._mutex.lock();
	service._implvector.erase(std::find(service._implvector.begin(),
		service._implvector.end(), simpl));
	service._implcnt--;
	service._mutex.unlock();
}

inline void EpollService::Access::_DoRun(EpollService& service, IoServiceImpl& simpl, bool isco,
	SocketLib::SocketError& error) 
{
	base::slist<SocketClose*> closes1;
	base::slist<SocketClose*> closes2;
	const int max_events = 128;
	epoll_event_t events[max_events];
	for (;;) {
		_DoClose(&simpl, closes1, closes2);
		g_setlasterr(0);
		g_bzero(&events, sizeof(events));
		s_int32_t ret = g_epoll_wait(simpl._handler, events, max_events, 500);
		if (ret == 0) {
			1; // time out
			continue;
		}
		if (ret < 0 && M_ERR_LAST != M_EINTR) {
			M_DEFAULT_SOCKET_ERROR2(error);
			break;;
		}
		bool brk = false;
		for (s_int32_t idx = 0; idx < ret; ++idx) {
			EpollService::OperationSet* opset = (EpollService::OperationSet*)events[idx].data.ptr;
			if (opset->_type & E_FINISH_OP) {
				delete opset;
				brk = true;
			}
			else
				_ExecOp(isco, simpl, opset, &events[idx]);
		}
		if (brk)
			break;
	}
}

inline void EpollService::Access::_ExecOp(bool isco, IoServiceImpl& serviceimpl, 
	EpollService::OperationSet* opset, epoll_event_t* event)
{
	if (!isco) {
		_DoExecOp(&serviceimpl, opset, event);
	}
	else {
		CoEventTask* task = 0;
		if (serviceimpl._taskvec.size()) {
			task = serviceimpl._taskvec.back();
			serviceimpl._taskvec.pop_back();
		}
		else {
			task = (CoEventTask*)malloc(sizeof(CoEventTask));
		}
		task->simpl = &serviceimpl;
		task->opset = opset;
		task->event = event;
		CoroutineTask::doTask(_DoExecCoOp, task);
		if (serviceimpl._taskvec.size() < 1024)
			serviceimpl._taskvec.push_back(task);
		else
			free(task);
	}
}

inline void EpollService::Access::_DoExecCoOp(void* param) {
	CoEventTask* task = (CoEventTask*)param;
	_DoExecOp(task->simpl, task->opset, task->event);
}

inline void EpollService::Access::_DoExecOp(IoServiceImpl* serviceimpl, 
	EpollService::OperationSet* opset, epoll_event_t* event)
{
	if ((opset->_type & E_ACCEPT_OP)) {
		opset->_aop._oper->Complete(*serviceimpl, event);
		return;
	}
	if (opset->_type & E_CONNECT_OP) {
		opset->_cop._oper->Complete(*serviceimpl, event);
		return;
	}

	bool flag = false;
	if (opset->_type & E_READ_OP &&
		(event->events&M_EPOLLIN || event->events&M_EPOLLERR || event->events&M_EPOLLHUP)) {
		flag = true;
		opset->_rop._oper->Complete(*serviceimpl, event);
	}
	if (opset->_type & E_WRITE_OP &&
		(event->events&M_EPOLLOUT || event->events&M_EPOLLERR || event->events&M_EPOLLHUP)) {
		flag = true;
		opset->_wop._oper->Complete(*serviceimpl, event);
		return;
	}
	if (!flag) {
	}
}

inline EpollService::IoServiceImpl* EpollService::Access::_GetIoServiceImpl(
	EpollService& service, SocketImpl& impl) 
{
	if (M_Impl_Epoll(impl) == -1)
		return 0;
	EpollService::IoServiceImplMap::iterator iter = service._implmap.find(M_Impl_Epoll(impl));
	if (iter == service._implmap.end())
		return 0;
	return iter->second;
}

inline void EpollService::Access::_DoClose(IoServiceImpl* simpl
	, base::slist<SocketClose*>&closereqs, base::slist<SocketClose*>&closereqs2)
{
	if (simpl->_closereqs.size()) {
		simpl->_mutex.lock();
		closereqs.swap(simpl->_closereqs);
		simpl->_mutex.unlock();
	}

	while (closereqs.size()) {
		SocketClose* cls = closereqs.front();
		MutexLock& mlock = M_Impl_Mutex(cls->_impl);
		mlock.lock();
		g_closesocket(M_Impl_Fd(cls->_impl));
		M_Impl_Fd(cls->_impl) = M_INVALID_SOCKET;
		M_Impl_State(cls->_impl) = 0;
		mlock.unlock();
		if (cls->_handler)
			cls->_handler();
		cls->Clear();

		closereqs2.push_back(cls);
		closereqs.pop_front();
	}

	if (closereqs2.size()) {
		simpl->_mutex.lock();
		simpl->_closereqs2.join(closereqs2);
		simpl->_mutex.unlock();
	}
	assert(closereqs.size() == 0);
	assert(closereqs2.size() == 0);
}

inline s_uint32_t EpollService::Access::GetServiceCount(const EpollService& service) {
	ScopedLock scoped(service._mutex);
	return service._implvector.size();
}

inline void EpollService::Access::Close(EpollService& service, SocketImpl& impl, SocketError& error) {
	Close(service, impl, 0, error);
}

inline void EpollService::Access::Close(EpollService& service, SocketImpl& impl, function_t<void()> handler,
	SocketError& error)
{
	MutexLock& mlock = M_Impl_Mutex(impl);
	mlock.lock();
	if (M_Impl_Fd(impl) != M_INVALID_SOCKET && !M_Impl_G_Close_Flag(impl)) {
		service._mutex.lock();
		IoServiceImpl* simpl = _GetIoServiceImpl(service, impl);
		if (simpl)
			--simpl->_fdcnt;
		service._mutex.unlock();

		if (M_Impl_G_Block(impl)) {
			if (g_closesocket(M_Impl_Fd(impl)) == M_SOCKET_ERROR)
				M_DEFAULT_SOCKET_ERROR2(error);
			M_Impl_Fd(impl) = M_INVALID_SOCKET;
			M_Impl_State(impl) = 0;
		}
		else {
			M_Impl_S_Close_Flag(impl);
		}
		mlock.unlock();

		if (simpl) {
			ScopedLock scoped_l(simpl->_mutex);
			SocketClose* close = 0;
			if (simpl->_closereqs2.size()) {
				close = simpl->_closereqs2.front();
				simpl->_closereqs2.pop_front();
			}
			else
				close = new SocketClose;

			close->_handler = handler;
			close->_impl = impl;
			simpl->_closereqs.push_back(close);
		}
		return;
	}
	mlock.unlock();
}

template<typename ProtocolType>
inline void EpollService::Access::Open(EpollService& service, SocketImpl& impl, ProtocolType pt, SocketError& error) {
	if (M_Impl_Fd(impl) != M_INVALID_SOCKET) {
		error = SocketError(M_ERR_SOCKET_OPEN);
		return;
	}
	if (pt.Family() == M_AF_INET) {
		M_Impl_S_V4(impl);
	}
	else {
		M_Impl_S_V6(impl);
	}

	M_Impl_Fd(impl) = g_socket(pt.Family(), pt.Type(), pt.Protocol());
	if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}
}

template<typename EndPoint>
inline void EpollService::Access::Bind(EpollService& service, SocketImpl& impl, const EndPoint& ep, SocketError& error) {
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_bind(M_Impl_Fd(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

inline void EpollService::Access::Listen(EpollService& service, SocketImpl& impl, s_int32_t flag, SocketError& error) {
	s_int32_t ret = g_listen(M_Impl_Fd(impl), flag);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

inline void EpollService::Access::Shutdown(EpollService& service, SocketImpl& impl, EShutdownType what, SocketError& error) {
	if (0 != g_shutdown(M_Impl_Fd(impl), what))
		M_DEFAULT_SOCKET_ERROR2(error);
}

inline void EpollService::Access::Accept(EpollService& service, SocketImpl& impl, SocketImpl& peer, SocketError& error)
{
	MutexLock& mlock = M_Impl_Mutex(impl);
	do
	{
		mlock.lock();
		if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_Impl_G_Accept_Flag(impl)) {
			error = SocketError(M_ERR_POSTED_ACCEPT);
			break;
		}
		if (M_Impl_G_Nonblock(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_Impl_S_Accept_Flag(impl);
		mlock.unlock();

		for (;;) {
			M_Impl_Fd(peer) = g_accept(M_Impl_Fd(impl), 0, 0);
			if (M_Impl_Fd(peer) != M_INVALID_SOCKET) {
				break;
			}
			if (M_ERR_LAST == M_ECONNRESET) {
				continue;
			}
			M_DEFAULT_SOCKET_ERROR2(error);
			break;
		}

		mlock.lock();
		M_Impl_C_Accept_Flag(impl);
		mlock.unlock();
		return;
	} while (false);
	mlock.unlock();
}

inline void EpollService::Access::AsyncAccept(EpollService& service, SocketImpl& accept_impl, SocketImpl& cli_impl
	, const M_COMMON_HANDLER_TYPE(IocpService2)& handler, SocketError& error)
{
	ScopedLock scoped_l(M_Impl_Mutex(accept_impl));
	if (M_Impl_Fd(accept_impl) == M_INVALID_SOCKET) {
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_Impl_G_Accept_Flag(accept_impl)) {
		error = SocketError(M_ERR_POSTED_ACCEPT);
		return;
	}
	if (!M_Impl_G_Nonblock(accept_impl)) {
		if (!detail::Util::SetNonBlock(M_Impl_Fd(accept_impl))) {
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_Impl_S_Nonblock(accept_impl)
	}

	M_Impl_S_Accept_Flag(accept_impl);
	M_Impl_Op(accept_impl)._type = E_ACCEPT_OP;

	AptOpType* aop = dynamic_cast<AptOpType*>(M_Impl_Aop(accept_impl));
	aop->_acpt_impl = accept_impl;
	aop->_cli_impl = cli_impl;
	M_Swap_Handler(M_COMMON_HANDLER_TYPE(IocpService2)&, aop->_handler, handler);

	if (!M_Impl_G_Bind(accept_impl)) {
		M_Impl_S_Bind(accept_impl);
		CtlEpoll(service, accept_impl, &M_Impl_Op(accept_impl), M_EPOLL_CTL_ADD, M_EPOLLIN, error);
	}
	if (error) {
		M_Impl_C_Accept_Flag(accept_impl);
		M_Impl_S_Unbind(accept_impl);
		aop->Clear();
	}
}

template<typename EndPoint>
inline void EpollService::Access::Connect(EpollService& service, SocketImpl& impl, const EndPoint& ep, 
	SocketError& error, s_uint32_t timeo_sec)
{
	MutexLock& mlock = M_Impl_Mutex(impl);
	do
	{
		mlock.lock();
		if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_Impl_G_Connect_Flag(impl)) {
			error = SocketError(M_ERR_POSTED_CONNECT);
			break;
		}
		if (!ep.Valid()) {
			error = SocketError(M_ERR_ENDPOINT_INVALID);
			break;
		}
		if (timeo_sec != -1) {
			if (M_Impl_G_Block(impl)) {
				detail::Util::SetNonBlock(M_Impl_Fd(impl));
			}
		}
		else {
			if (M_Impl_G_Nonblock(impl)) {
				error = SocketError(M_ERR_IS_NONBLOCK);
				break;
			}
		}
		M_Impl_S_Connect_Flag(impl);
		mlock.unlock();

		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		s_int32_t ret = g_connect(M_Impl_Fd(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
		if (ret == -1 && timeo_sec != -1) {
			if (M_ERR_LAST == M_EINPROGRESS) {
				ret = Select(impl, false, timeo_sec, error);
				if (ret != 0)
					Close(service, impl, error);
			}
		}

		if (ret != 0 && !error)
			M_DEFAULT_SOCKET_ERROR(ret != 0, error);

		mlock.lock();
		detail::Util::SetBlock(M_Impl_Fd(impl));
		M_Impl_C_Connect_Flag(impl);
		mlock.unlock();
		return;
	} while (false);
	mlock.unlock();
}

template<typename EndPoint>
inline void EpollService::Access::AsyncConnect(EpollService& service, SocketImpl& impl, const EndPoint& ep
	, const M_COMMON_HANDLER_TYPE(EpollService)& handler, SocketError& error)
{
	ScopedLock scoped_l(M_Impl_Mutex(impl));
	if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_Impl_G_Connect_Flag(impl)) {
		error = SocketError(M_ERR_POSTED_CONNECT);
		return;
	}
	if (!ep.Valid()) {
		error = SocketError(M_ERR_ENDPOINT_INVALID);
		return;
	}
	if (!M_Impl_G_Nonblock(impl)) {
		if (!detail::Util::SetNonBlock(M_Impl_Fd(impl))) {
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_Impl_S_Nonblock(impl)
	}

	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_connect(M_Impl_Fd(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	if (ret == -1 && M_ERR_LAST != M_EINPROGRESS) {
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}

	M_Impl_S_Connect_Flag(impl);
	M_Impl_Op(impl)._type = E_CONNECT_OP;
	CoOpType* cop = dynamic_cast<CoOpType*>(M_Impl_Cop(impl));
	cop->_impl = impl;
	M_Swap_Handler(M_COMMON_HANDLER_TYPE(EpollService)&, cop->_handler, handler);

	if (!M_Impl_G_Bind(impl)) {
		M_Impl_S_Bind(impl);
		CtlEpoll(service, impl, &M_Impl_Op(impl), M_EPOLL_CTL_ADD, M_EPOLLOUT | M_EPOLLONESHOT, error);
	}
	if (error) {
		M_Impl_C_Connect_Flag(impl);
		M_Impl_S_Unbind(impl);
		cop->Clear();
	}
}

inline s_int32_t EpollService::Access::RecvSome(EpollService& service, SocketImpl& impl,
	s_byte_t* data, s_uint32_t size, SocketError& error)
{
	MutexLock& mlock = M_Impl_Mutex(impl);
	do
	{
		mlock.lock();
		if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_Impl_G_Read_Flag(impl)) {
			error = SocketError(M_ERR_POSTED_READ);
			break;
		}
		if (M_Impl_G_Nonblock(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_Impl_S_Read_Flag(impl);
		mlock.unlock();

		s_int32_t ret = g_recv(M_Impl_Fd(impl), data, size, 0);
		mlock.lock();
		M_Impl_C_Read_Flag(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return ret;
	} while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

inline void EpollService::Access::AsyncRecvSome(EpollService& service, SocketImpl& impl, s_byte_t* data, s_uint32_t size
	, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error)
{
	RdOpType* rop = dynamic_cast<RdOpType*>(M_Impl_Rop(impl));
	rop->_wsabuf.buf = data;
	rop->_wsabuf.len = size;
	rop->_impl = impl;
	M_Swap_Handler(M_RW_HANDLER_TYPE(EpollService)&, rop->_handler, hander);
	do
	{
		ScopedLock scoped_l(M_Impl_Mutex(impl));
		if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_Impl_G_Read_Flag(impl)) {
			error = SocketError(M_ERR_POSTED_READ);
			break;
		}
		if (!M_Impl_G_Nonblock(impl)) {
			if (!detail::Util::SetNonBlock(M_Impl_Fd(impl))) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_Impl_S_Nonblock(impl)
		}

		M_Impl_S_Read_Flag(impl);
		M_Impl_Op(impl)._type = E_READ_OP;

		int ctl = M_EPOLL_CTL_MOD;
		if (!M_Impl_G_Bind(impl)) {
			M_Impl_S_Bind(impl);
			ctl = M_EPOLL_CTL_ADD;
		}
		int flag = M_EPOLLIN;
		if (M_Impl_G_Write_Flag(impl)) {
			flag |= M_EPOLLOUT;
			M_Impl_Op(impl)._type |= E_WRITE_OP;
		}

		CtlEpoll(service, impl, &M_Impl_Op(impl), ctl, flag, error);
		if (error) {
			M_Impl_C_Read_Flag(impl);
			M_Impl_S_Unbind(impl);
			rop->Clear();
		}
		return;
	} while (false);
	rop->Clear();
}

inline s_int32_t EpollService::Access::SendSome(EpollService& service, SocketImpl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	MutexLock& mlock = M_Impl_Mutex(impl);
	do
	{
		mlock.lock();
		if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_Impl_G_Write_Flag(impl)) {
			error = SocketError(M_ERR_POSTED_WRITE);
			break;
		}
		if (M_Impl_G_Nonblock(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_Impl_S_Write_Flag(impl);
		mlock.unlock();

		s_int32_t ret = g_send(M_Impl_Fd(impl), data, size, MSG_NOSIGNAL);
		mlock.lock();
		M_Impl_C_Write_Flag(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return ret;
	} while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

inline void EpollService::Access::AsyncSendSome(EpollService& service, SocketImpl& impl, const s_byte_t* data, s_uint32_t size
	, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error)
{
	WrOpType* wop = dynamic_cast<WrOpType*>(M_Impl_Wop(impl));
	wop->_wsabuf.buf = const_cast<s_byte_t*>(data);
	wop->_wsabuf.len = size;
	wop->_impl = impl;
	M_Swap_Handler(M_RW_HANDLER_TYPE(EpollService)&, wop->_handler, hander);
	do
	{
		ScopedLock scoped_l(M_Impl_Mutex(impl));
		if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_Impl_G_Write_Flag(impl)) {
			error = SocketError(M_ERR_POSTED_WRITE);
			break;
		}
		if (!M_Impl_G_Nonblock(impl)) {
			if (!detail::Util::SetNonBlock(M_Impl_Fd(impl))) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_Impl_S_Nonblock(impl)
		}

		M_Impl_S_Write_Flag(impl);
		M_Impl_Op(impl)._type = E_WRITE_OP;

		int ctl = M_EPOLL_CTL_MOD;
		if (!M_Impl_G_Bind(impl)) {
			M_Impl_S_Bind(impl);
			ctl = M_EPOLL_CTL_ADD;
		}
		int flag = M_EPOLLOUT;
		if (M_Impl_G_Read_Flag(impl)) {
			flag |= M_EPOLLIN;
			M_Impl_Op(impl)._type |= E_READ_OP;
		}

		CtlEpoll(service, impl, &M_Impl_Op(impl), ctl, flag, error);
		if (error) {
			M_Impl_C_Write_Flag(impl);
			M_Impl_S_Unbind(impl);
			wop->Clear();
		}
		return;
	} while (false);
	wop->Clear();
}

inline s_int32_t EpollService::Access::Select(SocketImpl& impl, bool rd_or_wr, s_uint32_t timeo_sec, SocketError& error) {
	// -1 == time out,0 == ok,other == error
	socket_t fd = M_Impl_Fd(impl);
	timeval tm;
	tm.tv_sec = timeo_sec;
	tm.tv_usec = 0;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);
	int ret = 0;
	if (rd_or_wr) {
		ret = select(fd, &set, NULL, NULL, &tm);
	}
	else {
		ret = select(fd, NULL, &set, NULL, &tm);
	}
	if (ret == 0) {
		error = SocketError(M_ERR_TIMEOUT);
		return -1;
	}
	else if (ret == -1) {
		M_DEFAULT_SOCKET_ERROR2(error);
		return -2;
	}

	int code_len = sizeof(int);
	int code = 0;
	g_getsockopt(fd, M_SOL_SOCKET, M_SO_ERROR, (char*)&code, (socklen_t *)&code_len);
	if (code == 0) {
		return 0;
	}
	else {
		error = SocketError(code);
		return -2;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

namespace iodetail {
	inline Operation::Operation() :_oper(0) {
	}

	inline Operation::~Operation() {
		delete _oper;
		_oper = 0;
	}

	inline bool AcceptOperation::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event) {
		MutexLock& mlock = M_Impl_Mutex(this->_acpt_impl);
		mlock.lock();
		if (M_Impl_Fd(this->_acpt_impl) != M_INVALID_SOCKET)
			M_Impl_C_Accept_Flag(this->_acpt_impl);
		mlock.unlock();

		SocketError error;
		socket_t fd = g_accept(M_Impl_Fd(this->_acpt_impl), 0, 0);
		if (fd < 0) {
			Opts::SoError error_opt;
			EpollService::Access::GetOption(*serviceimpl.GetService(), this->_acpt_impl, error_opt, error);
			error = error ? SocketError(M_ERR_ASYNC_ACCEPT_FAIL) : SocketError(error_opt.Value());
		}
		else {
			M_Impl_Fd(this->_cli_impl) = fd;
			if (M_Impl_G_V(this->_acpt_impl)) {
				M_Impl_S_V4(this->_cli_impl);
			}
			else {
				M_Impl_S_V6(this->_cli_impl);
			}
		}

		function_t<void(SocketError)> handler;
		handler.swap(this->_handler);
		this->Clear();
		handler(error);
		return true;
	}

	inline void AcceptOperation::Clear() {
		this->_handler = 0;
		this->_acpt_impl = this->_cli_impl = SocketImpl();
	}

	inline bool ConnectOperation::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event) {
		bool notify = false;
		MutexLock& mlock = M_Impl_Mutex(this->_impl);
		mlock.lock();
		if (M_Impl_Fd(this->_impl) != M_INVALID_SOCKET) {
			M_Impl_C_Connect_Flag(this->_impl);
			notify = true;
		}
		EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
		opset->_type &= ~(E_CONNECT_OP);
		mlock.unlock();

		SocketError error;
		if (event->events & M_EPOLLERR) {
			Opts::SoError error_opt;
			EpollService::Access::GetOption(*serviceimpl.GetService(), this->_impl, error_opt, error);
			error = error ? SocketError(M_ERR_ASYNC_CONNECT_FAIL) : SocketError(error_opt.Value());
		}

		function_t <void(SocketError)> handler;
		handler.swap(this->_handler);
		this->Clear();
		if (notify)
			handler(error);
		return true;
	}

	inline void ConnectOperation::Clear() {
		this->_handler = 0;
		this->_impl = SocketImpl();
	}

	inline bool WriteOperation::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event) {
		bool notify = false;
		SocketError error;
		MutexLock& mlock = M_Impl_Mutex(this->_impl);
		mlock.lock();
		EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
		opset->_type &= ~(E_WRITE_OP);
		if (M_Impl_Fd(this->_impl) != M_INVALID_SOCKET) {
			notify = true;
			M_Impl_C_Write_Flag(this->_impl);
			s_int32_t flag = 0;
			if (opset->_type & E_READ_OP)
				flag = M_EPOLLIN;
		 	EpollService::Access::CtlEpoll(*serviceimpl.GetService(), this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);
		}
		mlock.unlock();

		s_int32_t ret = g_send(M_Impl_Fd(this->_impl), this->_wsabuf.buf
			, (s_uint32_t)this->_wsabuf.len, MSG_NOSIGNAL);
		if (ret <= 0) {
			Opts::SoError error_opt;
			EpollService::Access::GetOption(*serviceimpl.GetService(), this->_impl, error_opt, error);
			error = error ? SocketError(M_ERR_ASYNC_WRITE_FAIL) : SocketError(error_opt.Value());
		}

		function_t <void(s_uint32_t, SocketError)> handler;
		handler.swap(this->_handler);
		this->Clear();
		if (notify)
			handler(ret < 0 ? 0 : ret, error);
		return true;
	}

	inline void WriteOperation::Clear() {
		this->_impl = SocketImpl();
		this->_handler = 0;
		this->_wsabuf.buf = 0;
		this->_wsabuf.len = 0;
	}

	inline bool ReadOperation::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event) {
		SocketError error;
		bool notify = false;
		MutexLock& mlock = M_Impl_Mutex(this->_impl);
		mlock.lock();
		if (M_Impl_Fd(this->_impl) != M_INVALID_SOCKET) {
			notify = true;
			M_Impl_C_Read_Flag(this->_impl);
			EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
			opset->_type &= ~(E_READ_OP);
			s_int32_t flag = 0;
			if (opset->_type & E_WRITE_OP)
				flag = M_EPOLLOUT;
			EpollService::Access::CtlEpoll(*serviceimpl.GetService(), this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);
		}
		mlock.unlock();

		s_int32_t ret = g_recv(M_Impl_Fd(this->_impl), this->_wsabuf.buf
			, (s_uint32_t)this->_wsabuf.len, 0);
		if (ret <= 0)
		{
			Opts::SoError error_opt;
			EpollService::Access::GetOption(*serviceimpl.GetService(), this->_impl, error_opt, error);
			error = (error) ? SocketError(M_ERR_ASYNC_READ_FAIL) : SocketError(error_opt.Value());
		}

		function_t <void(s_uint32_t, SocketError)> handler;
		handler.swap(this->_handler);
		this->Clear();
		if (notify)
			handler(ret < 0 ? 0 : ret, error);
		return true;
	}

	inline void ReadOperation::Clear() {
		this->_impl = SocketImpl();
		this->_handler = 0;
		this->_wsabuf.buf = 0;
		this->_wsabuf.len = 0;
	}

	inline void SocketImpl::Init() {
		if (!_core) {
			_core.reset(new core);
			_core->_epoll = -1;
			_core->_fd = M_INVALID_SOCKET;
			_core->_state = 0;
			_core->_op._type = 0;
			_core->_op._aop._oper = new AptOpType;
			_core->_op._cop._oper = new CoOpType;
			_core->_op._wop._oper = new WrOpType;
			_core->_op._rop._oper = new RdOpType;
		}
	}
}

M_SOCKET_NAMESPACE_END
#endif
#endif
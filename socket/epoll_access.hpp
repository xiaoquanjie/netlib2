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

#include "socket/config.hpp"
#ifndef M_PLATFORM_WIN

// ipv4 or ipv6 bit(0)
#define M_IMPL_S_V4(impl)\
	M_CLR_BIT(impl._core->_state,0)
#define M_IMPL_S_V6(impl)\
	M_SET_BIT(impl._core->_state,0)
#define M_IMPL_G_V(impl)\
	M_GET_BIT(impl._core->_state,0)

// bind bit(1)
#define M_IMPL_S_BIND(impl)\
	M_SET_BIT(impl._core->_state,1)
#define M_IMPL_S_UNBIND(impl)\
	M_CLR_BIT(impl._core->_state,1)
#define M_IMPL_G_BIND(impl)\
	M_GET_BIT(impl._core->_state,1)

// block mode bit(2)
#define M_IMPL_S_BLOCK(impl)\
	M_SET_BIT(impl._core->_state,2)
#define M_IMPL_G_BLOCK(impl)\
	M_GET_BIT(impl._core->_state,2)
#define M_IMPL_S_NONBLOCK(impl)\
	M_CLR_BIT(impl._core->_state,2)
#define M_IMPL_G_NONBLOCK(impl)\
	(!M_GET_BIT(impl._core->_state,2))

// type bit(3~4)
#define M_IMPL_S_TYPE(impl,type)\
	M_SET_MBIT(impl._core->_state,type,3,4)
#define M_IMPL_G_TYPE(impl)\
	M_GET_MBIT(impl._core->_state,3,4)

// accept flag bit(5)
#define M_IMPL_G_ACCEPT_FLAG(impl)\
	M_GET_BIT(impl._core->_state,5)
#define M_IMPL_S_ACCEPT_FLAG(impl)\
	M_SET_BIT(impl._core->_state,5)
#define M_IMPL_C_ACCEPT_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,5)

// connect flag bit(6)
#define M_IMPL_G_CONNECT_FLAG(impl)\
	M_GET_BIT(impl._core->_state,6)
#define M_IMPL_S_CONNECT_FLAG(impl)\
	M_SET_BIT(impl._core->_state,6)
#define M_IMPL_C_CONNECT_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,6)

// write flag bit(7)
#define M_IMPL_G_WRITE_FLAG(impl)\
	M_GET_BIT(impl._core->_state,7)
#define M_IMPL_S_WRITE_FLAG(impl)\
	M_SET_BIT(impl._core->_state,7)
#define M_IMPL_C_WRITE_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,7)

// read flag bit(8)
#define M_IMPL_G_READ_FLAG(impl)\
	M_GET_BIT(impl._core->_state,8)
#define M_IMPL_S_READ_FLAG(impl)\
	M_SET_BIT(impl._core->_state,8)
#define M_IMPL_C_READ_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,8)

// close flag bit(9)
#define M_IMPL_G_CLOSE_FLAG(impl)\
	M_GET_BIT(impl._core->_state,9)
#define M_IMPL_S_CLOSE_FLAG(impl)\
	M_SET_BIT(impl._core->_state,9)
#define M_IMPL_C_CLOSE_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,9)

#define M_IMPL_FD(impl)\
	impl._core->_fd
#define M_IMPL_STATE(impl)\
	impl._core->_state
#define M_IMPL_EPOLL(impl)\
	impl._core->_epoll
#define M_IMPL_OP(impl)\
	impl._core->_op
#define M_IMPL_AOP(impl)\
	impl._core->_op._aop._oper
#define M_IMPL_COP(impl)\
	impl._core->_op._cop._oper
#define M_IMPL_ROP(impl)\
	impl._core->_op._rop._oper
#define M_IMPL_WOP(impl)\
	impl._core->_op._wop._oper

#define M_IMPL_INIT(impl)\
	impl.Init<0>()
#define M_IMPL_MUTEX(impl)\
	(impl._core->_mutex)

#define M_SWAP_HANDLER(type,handler1,handler2)\
	handler1.swap(const_cast<type>(handler2));

M_SOCKET_DECL void EpollService::Access::ConstructImpl(EpollService& service, Impl& impl, s_uint16_t type){
	M_IMPL_INIT(impl);
	M_IMPL_S_V4(impl);
	M_IMPL_S_UNBIND(impl);
	M_IMPL_S_BLOCK(impl);
	M_IMPL_S_TYPE(impl, (s_uint8_t)type);
}

M_SOCKET_DECL void EpollService::Access::DestroyImpl(EpollService& service, Impl& impl){
	ConstructImpl(service, impl, E_NULL_SOCKET_TYPE);
}

M_SOCKET_DECL bool EpollService::Access::IsOpen(EpollService& service, Impl& impl, SocketError& error){
	return (M_IMPL_FD(impl) != M_INVALID_SOCKET);
}

template<typename GettableOptionType>
M_SOCKET_DECL void EpollService::Access::GetOption(EpollService& service, Impl& impl, GettableOptionType& opt, SocketError& error){
	socklen_t len = opt.Size();
	s_int32_t ret = g_getsockopt(M_IMPL_FD(impl), opt.Level(), opt.Name(), (char*)opt.Data(), &len);
	M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
}

template<typename SettableOptionType>
M_SOCKET_DECL void EpollService::Access::SetOption(EpollService& service, Impl& impl, const SettableOptionType& opt, SocketError& error){
	s_int32_t ret = g_setsockopt(M_IMPL_FD(impl), opt.Level(), opt.Name(), (char*)opt.Data(), opt.Size());
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint EpollService::Access::RemoteEndPoint(EndPoint, EpollService& service, const Impl& impl, SocketError& error){
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetRemoteEndPoint(M_IMPL_FD(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint EpollService::Access::LocalEndPoint(EndPoint, EpollService& service, const Impl& impl, SocketError& error){
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetLocalEndPoint(M_IMPL_FD(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

M_SOCKET_DECL void EpollService::Access::Cancel(EpollService& service, Impl& impl, SocketError& error){
	if (M_IMPL_G_BIND(impl)) {
		CtlEpoll(service, impl, 0, M_EPOLL_CTL_DEL, 0, error);
	}
}

M_SOCKET_DECL void EpollService::Access::CreateEpoll(IoServiceImpl& impl, SocketError& error){
	if (impl._handler != -1){
		error = SocketError(M_ERR_EPOLL_EXIST);
		return;
	}
	impl._handler = g_epoll_create(1);
	M_DEFAULT_SOCKET_ERROR(impl._handler < 0, error);
}

M_SOCKET_DECL void EpollService::Access::DestroyEpoll(IoServiceImpl& impl){
	if (impl._handler != -1){
		g_closesocket(impl._handler);
		impl._handler = -1;
		impl._fdcnt = 0;
	}
}

M_SOCKET_DECL void EpollService::Access::CtlEpoll(EpollService& service, Impl& impl
	, EpollService::OperationSet* opset, s_int32_t flag, s_int32_t events, SocketError& error){
	if (M_EPOLL_CTL_ADD == flag && M_IMPL_EPOLL(impl) == -1){
		ScopedLock scoped_l(service._mutex);
		if (!service._implvector.empty()) {
			service._implidx++;
			s_uint32_t size = (s_uint32_t)service._implvector.size();
			IoServiceImpl& serviceimpl = *(service._implvector[service._implidx%size]);
			M_IMPL_EPOLL(impl) = serviceimpl._handler;
			serviceimpl._fdcnt++;
		}
	}
	if (M_IMPL_EPOLL(impl) == -1){
		error = SocketError(M_ERR_NOT_SERVICE);
		return;
	}
	epoll_event_t event;
	event.events = events;
	event.data.ptr = opset;
	s_int32_t ret = g_epoll_ctl(M_IMPL_EPOLL(impl), flag, M_IMPL_FD(impl), &event);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void EpollService::Access::ExecOp(IoServiceImpl& serviceimpl
	, EpollService::OperationSet* opset, epoll_event_t* event){
	if ((opset->_type & E_ACCEPT_OP)) {
		opset->_aop._oper->Complete(serviceimpl, event);
		return;
	}
	if (opset->_type & E_CONNECT_OP) {
		opset->_cop._oper->Complete(serviceimpl, event);
		return;
	}

	bool flag = false;
	if (opset->_type & E_READ_OP &&
		(event->events&M_EPOLLIN || event->events&M_EPOLLERR || event->events&M_EPOLLHUP)){
		flag = true;
		opset->_rop._oper->Complete(serviceimpl, event);
	}
	if (opset->_type & E_WRITE_OP &&
		(event->events&M_EPOLLOUT || event->events&M_EPOLLERR || event->events&M_EPOLLHUP)){
		flag = true;
		opset->_wop._oper->Complete(serviceimpl, event);
		return;
	}
	if (!flag)
	{
		//M_DEBUG_PRINT("type: " << opset->_type);
		//assert(0);
	}
}

M_SOCKET_DECL void EpollService::Access::Run(EpollService& service, SocketError& error)
{
	IoServiceImpl* simpl = new IoServiceImpl(service);
	CreateEpoll(*simpl, error);
	if (error) {
		delete simpl;
		return;
	}

	service._mutex.lock();
	service._implmap[simpl->_handler] = simpl;
	service._implvector.push_back(simpl);
	++service._implcnt;
	service._mutex.unlock();

	const int max_events = 128;
	epoll_event_t events[max_events];
	base::slist<ImplCloseReq*> closereqs, closereqs2;
	for (;;){
		_DoClose(simpl, closereqs, closereqs2);
		g_setlasterr(0);
		g_bzero(&events, sizeof(events));
		s_int32_t ret = g_epoll_wait(simpl->_handler, events, max_events, -1);
		if (ret == 0){
			1; // time out
			continue;
		}
		if (ret < 0 && M_ERR_LAST != M_EINTR){
			M_DEFAULT_SOCKET_ERROR2(error);
			break;;
		}
		bool brk = false;
		for (s_int32_t idx = 0; idx < ret; ++idx){
			EpollService::OperationSet* opset = (EpollService::OperationSet*)events[idx].data.ptr;
			if (opset->_type & E_FINISH_OP){
				delete opset;
				brk = true;
			}
			else
				ExecOp(*simpl, opset, &events[idx]);
		}
		if (brk)
			break;
	}

	simpl->_mutex.lock();
	while (simpl->_closereqs.size()) {
		ImplCloseReq* req = simpl->_closereqs.front();
		simpl->_closereqs.pop_front();
		delete req;
	}
	while (simpl->_closereqs2.size()) {
		ImplCloseReq* req = simpl->_closereqs2.front();
		simpl->_closereqs2.pop_front();
		delete req;
	}
	simpl->_mutex.unlock();

	service._mutex.lock();
	service._implvector.erase(std::find(service._implvector.begin(), service._implvector.end(), simpl));
	service._implcnt--;
	service._mutex.unlock();
}

M_SOCKET_DECL void EpollService::Access::Stop(EpollService& service, SocketError& error){
	ScopedLock scoped_l(service._mutex);
	for (EpollService::IoServiceImplVector::iterator iter=service._implvector.begin();
		iter!=service._implvector.end(); ++iter){
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

M_SOCKET_DECL bool EpollService::Access::Stopped(const EpollService& service){
	ScopedLock scoped_lock(service._mutex);
	return (service._implvector.empty() ? true : false);
}

M_SOCKET_DECL EpollService::IoServiceImpl* EpollService::Access::_GetIoServiceImpl(EpollService& service, Impl& impl){
	if (M_IMPL_EPOLL(impl) == -1)
		return 0;
	
	EpollService::IoServiceImplMap::iterator iter = service._implmap.find(M_IMPL_EPOLL(impl));
	if (iter == service._implmap.end())
		return 0;
	return iter->second;
}

M_SOCKET_DECL void EpollService::Access::_DoClose(IoServiceImpl* simpl
	, base::slist<ImplCloseReq*>&closereqs, base::slist<ImplCloseReq*>&closereqs2) {
	if (simpl->_closereqs.size()) {
		simpl->_mutex.lock();
		closereqs.swap(simpl->_closereqs);
		simpl->_mutex.unlock();
	}

	while (closereqs.size()) {
		ImplCloseReq* req = closereqs.front();
		MutexLock& mlock = M_IMPL_MUTEX(req->_impl);
		mlock.lock();
		g_closesocket(M_IMPL_FD(req->_impl));
		M_IMPL_FD(req->_impl) = M_INVALID_SOCKET;
		M_IMPL_STATE(req->_impl) = 0;
		mlock.unlock();
		if (req->_handler)
			req->_handler();
		req->Clear();

		closereqs2.push_back(req);
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

M_SOCKET_DECL s_uint32_t EpollService::Access::GetServiceCount(const EpollService& service){
	ScopedLock scoped(service._mutex);
	return service._implvector.size();
}

M_SOCKET_DECL void EpollService::Access::Close(EpollService& service, Impl& impl, SocketError& error){
	Close(service, impl, 0, error);

	if (M_IMPL_FD(impl) != M_INVALID_SOCKET)
	{
		service._mutex.lock();
		EpollService::IoServiceImpl* serviceimpl = _GetIoServiceImpl(service, impl);
		if (serviceimpl)
		{
			__sync_sub_and_fetch(&serviceimpl->_fdcnt, 1);
			//serviceimpl->_closeimplvector.push_back(impl);
		}
		service._mutex.unlock();

		if (M_IMPL_FD(impl) != M_INVALID_SOCKET)
		{
			if (M_IMPL_G_BIND(impl))
			{
				SocketError error2;
				CtlEpoll(service, impl, 0, M_EPOLL_CTL_DEL, 0, error2);
			}
			if (g_closesocket(M_IMPL_FD(impl)) == M_SOCKET_ERROR)
				M_DEFAULT_SOCKET_ERROR2(error);

			M_IMPL_FD(impl) = M_INVALID_SOCKET;
			M_IMPL_STATE(impl) = 0;
		}
	}
}

M_SOCKET_DECL void EpollService::Access::Close(EpollService& service, Impl& impl, function_t<void()> handler, SocketError& error) {
	MutexLock& mlock = M_IMPL_MUTEX(impl);
	mlock.lock();
	if (M_IMPL_FD(impl) != M_INVALID_SOCKET && !M_IMPL_G_CLOSE_FLAG(impl)) {
		service._mutex.lock();
		IoServiceImpl* simpl = _GetIoServiceImpl(service, impl);
		if (simpl)
			--simpl->_fdcnt;
		service._mutex.unlock();

		if (M_IMPL_G_BLOCK(impl)) {
			if (g_closesocket(M_IMPL_FD(impl)) == M_SOCKET_ERROR)
				M_DEFAULT_SOCKET_ERROR2(error);
			M_IMPL_FD(impl) = M_INVALID_SOCKET;
			M_IMPL_STATE(impl) = 0;
		}
		else {
			M_IMPL_S_CLOSE_FLAG(impl);
		}
		mlock.unlock();

		if (simpl) {
			ScopedLock scoped_l(simpl->_mutex);
			ImplCloseReq* req = 0;
			if (simpl->_closereqs2.size()) {
				req = simpl->_closereqs2.front();
				simpl->_closereqs2.pop_front();
			}
			else
				req = new ImplCloseReq;

			req->_handler = handler;
			req->_impl = impl;
			simpl->_closereqs.push_back(req);
		}
		return;
	}
	mlock.unlock();
}

template<typename ProtocolType>
M_SOCKET_DECL void EpollService::Access::Open(EpollService& service, Impl& impl, ProtocolType pt, SocketError& error){
	if (M_IMPL_FD(impl) != M_INVALID_SOCKET){
		error = SocketError(M_ERR_SOCKET_OPEN);
		return;
	}
	if (pt.Family() == M_AF_INET) {
		M_IMPL_S_V4(impl);
	}
	else {
		M_IMPL_S_V6(impl);
	}

	M_IMPL_FD(impl) = g_socket(pt.Family(), pt.Type(), pt.Protocol());
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::Bind(EpollService& service, Impl& impl, const EndPoint& ep, SocketError& error){
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_bind(M_IMPL_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void EpollService::Access::Listen(EpollService& service, Impl& impl, s_int32_t flag, SocketError& error){
	s_int32_t ret = g_listen(M_IMPL_FD(impl), flag);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void EpollService::Access::Shutdown(EpollService& service, Impl& impl, EShutdownType what, SocketError& error){
	if (0 != g_shutdown(M_IMPL_FD(impl), what))
		M_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void EpollService::Access::Accept(EpollService& service, EpollService::Impl& impl, EpollService::Impl& peer, SocketError& error)
{
	MutexLock& mlock = M_IMPL_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL_G_ACCEPT_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_ACCEPT);
			break;
		}
		if (M_IMPL_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL_S_ACCEPT_FLAG(impl);
		mlock.unlock();

		for (;;){
			M_IMPL_FD(peer) = g_accept(M_IMPL_FD(impl), 0, 0);
			if (M_IMPL_FD(peer) != M_INVALID_SOCKET) {
				break;
			}
			if (M_ERR_LAST == M_ECONNRESET) {
				continue;
			}
			M_DEFAULT_SOCKET_ERROR2(error);
			break;
		}

		mlock.lock();
		M_IMPL_C_ACCEPT_FLAG(impl);
		mlock.unlock();
		return;
	} 
	while (false);
	mlock.unlock();
}

M_SOCKET_DECL void EpollService::Access::AsyncAccept(EpollService& service, Impl& accept_impl, Impl& cli_impl
	, const M_COMMON_HANDLER_TYPE(IocpService2)& handler, SocketError& error)
{
	ScopedLock scoped_l(M_IMPL_MUTEX(accept_impl));
	if (M_IMPL_FD(accept_impl) == M_INVALID_SOCKET) {
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_ACCEPT_FLAG(accept_impl)) {
		error = SocketError(M_ERR_POSTED_ACCEPT);
		return;
	}
	if (!M_IMPL_G_NONBLOCK(accept_impl)) {
		if (!detail::Util::SetNonBlock(M_IMPL_FD(accept_impl))) {
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_NONBLOCK(accept_impl)
	}

	M_IMPL_S_ACCEPT_FLAG(accept_impl);
	M_IMPL_OP(accept_impl)._type = E_ACCEPT_OP;

	AcceptOperation2* aop = dynamic_cast<AcceptOperation2*>(M_IMPL_AOP(accept_impl));
	aop->_acpt_impl = accept_impl;
	aop->_cli_impl = cli_impl;
	M_SWAP_HANDLER(M_COMMON_HANDLER_TYPE(IocpService2)&, aop->_handler, handler);

	if (!M_IMPL_G_BIND(accept_impl)) {
		M_IMPL_S_BIND(accept_impl);
		CtlEpoll(service, accept_impl, &M_IMPL_OP(accept_impl), M_EPOLL_CTL_ADD, M_EPOLLIN, error);
	}
	if (error) {
		M_IMPL_C_ACCEPT_FLAG(accept_impl);
		M_IMPL_S_UNBIND(accept_impl);
		aop->Clear();
	}
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::Connect(EpollService& service, EpollService::Impl& impl, const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec)
{
	MutexLock& mlock = M_IMPL_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL_G_CONNECT_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_CONNECT);
			break;
		}
		if (!ep.Valid()){
			error = SocketError(M_ERR_ENDPOINT_INVALID);
			break;
		}
		if (timeo_sec != -1) {
			if (M_IMPL_G_BLOCK(impl)) {
				detail::Util::SetNonBlock(M_IMPL_FD(impl));
			}
		}
		else {
			if (M_IMPL_G_NONBLOCK(impl)) {
				error = SocketError(M_ERR_IS_NONBLOCK);
				break;
			}
		}
		M_IMPL_S_CONNECT_FLAG(impl);
		mlock.unlock();

		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		s_int32_t ret = g_connect(M_IMPL_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
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
		detail::Util::SetBlock(M_IMPL_FD(impl));
		M_IMPL_C_CONNECT_FLAG(impl);
		mlock.unlock();
		return;
	} 
	while (false);
	mlock.unlock();
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::AsyncConnect(EpollService& service, Impl& impl, const EndPoint& ep
	, const M_COMMON_HANDLER_TYPE(EpollService)& handler, SocketError& error)
{
	ScopedLock scoped_l(M_IMPL_MUTEX(impl));
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_CONNECT_FLAG(impl)){
		error = SocketError(M_ERR_POSTED_CONNECT);
		return;
	}
	if (!ep.Valid()) {
		error = SocketError(M_ERR_ENDPOINT_INVALID);
		return;
	}
	if (!M_IMPL_G_NONBLOCK(impl)){
		if (!detail::Util::SetNonBlock(M_IMPL_FD(impl))){
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_NONBLOCK(impl)
	}

	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_connect(M_IMPL_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	if (ret == -1 && M_ERR_LAST != M_EINPROGRESS){
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}

	M_IMPL_S_CONNECT_FLAG(impl);
	M_IMPL_OP(impl)._type = E_CONNECT_OP;
	ConnectOperation2* cop = dynamic_cast<ConnectOperation2*>(M_IMPL_COP(impl));
	cop->_impl = impl;
	M_SWAP_HANDLER(M_COMMON_HANDLER_TYPE(EpollService)&, cop->_handler, handler);

	if (!M_IMPL_G_BIND(impl)){
		M_IMPL_S_BIND(impl);
		CtlEpoll(service, impl, &M_IMPL_OP(impl), M_EPOLL_CTL_ADD, M_EPOLLOUT | M_EPOLLONESHOT, error);
	}
	if (error){
		M_IMPL_C_CONNECT_FLAG(impl);
		M_IMPL_S_UNBIND(impl);
		cop->Clear();
	}
}

M_SOCKET_DECL s_int32_t EpollService::Access::RecvSome(EpollService& service, EpollService::Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	MutexLock& mlock = M_IMPL_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL_G_READ_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_READ);
			break;
		}
		if (M_IMPL_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL_S_READ_FLAG(impl);
		mlock.unlock();

		s_int32_t ret = g_recv(M_IMPL_FD(impl), data, size, 0);
		mlock.lock();
		M_IMPL_C_READ_FLAG(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return ret;
	} 
	while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

M_SOCKET_DECL void EpollService::Access::AsyncRecvSome(EpollService& service, Impl& impl, s_byte_t* data, s_uint32_t size
	, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error)
{
	ReadOperation2* rop = dynamic_cast<ReadOperation2*>(M_IMPL_ROP(impl));
	rop->_wsabuf.buf = data;
	rop->_wsabuf.len = size;
	rop->_impl = impl;
	M_SWAP_HANDLER(M_RW_HANDLER_TYPE(EpollService)&, rop->_handler, hander);
	do 
	{
		ScopedLock scoped_l(M_IMPL_MUTEX(impl));
		if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL_G_READ_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_READ);
			break;
		}
		if (!M_IMPL_G_NONBLOCK(impl)){
			if (!detail::Util::SetNonBlock(M_IMPL_FD(impl))){
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_IMPL_S_NONBLOCK(impl)
		}

		M_IMPL_S_READ_FLAG(impl);
		M_IMPL_OP(impl)._type = E_READ_OP;

		int ctl = M_EPOLL_CTL_MOD;
		if (!M_IMPL_G_BIND(impl)){
			M_IMPL_S_BIND(impl);
			ctl = M_EPOLL_CTL_ADD;
		}
		int flag = M_EPOLLIN;
		if (M_IMPL_G_WRITE_FLAG(impl)){
			flag |= M_EPOLLOUT;
			M_IMPL_OP(impl)._type |= E_WRITE_OP;
		}

		CtlEpoll(service, impl, &M_IMPL_OP(impl), ctl, flag, error);
		if (error){
			M_IMPL_C_READ_FLAG(impl);
			M_IMPL_S_UNBIND(impl);
			rop->Clear();
		}
		return;
	} 
	while (false);
	rop->Clear();
}

M_SOCKET_DECL s_int32_t EpollService::Access::SendSome(EpollService& service, EpollService::Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	MutexLock& mlock = M_IMPL_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL_G_WRITE_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_WRITE);
			break;
		}
		if (M_IMPL_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL_S_WRITE_FLAG(impl);
		mlock.unlock();

		s_int32_t ret = g_send(M_IMPL_FD(impl), data, size, MSG_NOSIGNAL);
		mlock.lock();
		M_IMPL_C_WRITE_FLAG(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return ret;
	} 
	while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

M_SOCKET_DECL void EpollService::Access::AsyncSendSome(EpollService& service, Impl& impl, const s_byte_t* data, s_uint32_t size
	, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error)
{
	WriteOperation2* wop = dynamic_cast<WriteOperation2*>(M_IMPL_WOP(impl));
	wop->_wsabuf.buf = const_cast<s_byte_t*>(data);
	wop->_wsabuf.len = size;
	wop->_impl = impl;
	M_SWAP_HANDLER(M_RW_HANDLER_TYPE(EpollService)&, wop->_handler, hander);
	do 
	{
		ScopedLock scoped_l(M_IMPL_MUTEX(impl));
		if (M_IMPL_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL_G_WRITE_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_WRITE);
			break;
		}
		if (!M_IMPL_G_NONBLOCK(impl)){
			if (!detail::Util::SetNonBlock(M_IMPL_FD(impl))){
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_IMPL_S_NONBLOCK(impl)
		}

		M_IMPL_S_WRITE_FLAG(impl);
		M_IMPL_OP(impl)._type = E_WRITE_OP;

		int ctl = M_EPOLL_CTL_MOD;
		if (!M_IMPL_G_BIND(impl)){
			M_IMPL_S_BIND(impl);
			ctl = M_EPOLL_CTL_ADD;
		}
		int flag = M_EPOLLOUT;
		if (M_IMPL_G_READ_FLAG(impl)){
			flag |= M_EPOLLIN;
			M_IMPL_OP(impl)._type |= E_READ_OP;
		}

		CtlEpoll(service, impl, &M_IMPL_OP(impl), ctl, flag, error);
		if (error){
			M_IMPL_C_WRITE_FLAG(impl);
			M_IMPL_S_UNBIND(impl);
			wop->Clear();
		}
		return;
	} 
	while (false);
	wop->Clear();
}

M_SOCKET_DECL s_int32_t EpollService::Access::Select(Impl& impl, bool rd_or_wr, s_uint32_t timeo_sec, SocketError& error) {
	// -1 == time out,0 == ok,other == error
	socket_t fd = M_IMPL_FD(impl);
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

M_SOCKET_DECL bool EpollService::AcceptOperation2::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	MutexLock& mlock = M_IMPL_MUTEX(this->_acpt_impl);
	mlock.lock();
	if (M_IMPL_FD(this->_acpt_impl) != M_INVALID_SOCKET)
		M_IMPL_C_ACCEPT_FLAG(this->_acpt_impl);
	mlock.unlock();

	SocketError error;
	socket_t fd = g_accept(M_IMPL_FD(this->_acpt_impl), 0, 0);
	if (fd < 0) {
		Opts::SoError error_opt;
		EpollService::Access::GetOption(serviceimpl.GetService(), this->_acpt_impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_ACCEPT_FAIL) : SocketError(error_opt.Value());
	}
	else {
		M_IMPL_FD(this->_cli_impl) = fd;
		if (M_IMPL_G_V(this->_acpt_impl)) {
			M_IMPL_S_V4(this->_cli_impl);
		}
		else {
			M_IMPL_S_V6(this->_cli_impl);
		}
	}

	function_t<void(SocketError)> handler;
	handler.swap(this->_handler);
	this->Clear();
	handler(error);
	return true;
}

M_SOCKET_DECL void EpollService::AcceptOperation2::Clear(){
	this->_handler = 0;
	this->_acpt_impl = this->_cli_impl = Impl();
}

M_SOCKET_DECL bool EpollService::ConnectOperation2::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event){
	bool notify = false;
	MutexLock& mlock = M_IMPL_MUTEX(this->_impl);
	mlock.lock();
	if (M_IMPL_FD(this->_impl) != M_INVALID_SOCKET) {
		M_IMPL_C_CONNECT_FLAG(this->_impl);
		notify = true;
	}
	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	opset->_type &= ~(E_CONNECT_OP);
	mlock.unlock();

	SocketError error;
	if (event->events & M_EPOLLERR){
		Opts::SoError error_opt;
		EpollService::Access::GetOption(serviceimpl.GetService(), this->_impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_CONNECT_FAIL) : SocketError(error_opt.Value());
	}

	function_t <void(SocketError)> handler;
	handler.swap(this->_handler);
	this->Clear();
	if (notify)
		handler(error);
	return true;
}

M_SOCKET_DECL void EpollService::ConnectOperation2::Clear(){
	this->_handler = 0;
	this->_impl = Impl();
}

M_SOCKET_DECL bool EpollService::WriteOperation2::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event){
	bool notify = false;
	SocketError error;
	MutexLock& mlock = M_IMPL_MUTEX(this->_impl);
	mlock.lock();
	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	opset->_type &= ~(E_WRITE_OP);
	if (M_IMPL_FD(this->_impl) != M_INVALID_SOCKET) {
		notify = true;
		M_IMPL_C_WRITE_FLAG(this->_impl);
		s_int32_t flag = 0;
		if (opset->_type & E_READ_OP)
			flag = M_EPOLLIN;
		Access::CtlEpoll(serviceimpl.GetService(), this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);
	}
	mlock.unlock();

	s_int32_t ret = g_send(M_IMPL_FD(this->_impl), this->_wsabuf.buf
		, (s_uint32_t)this->_wsabuf.len, MSG_NOSIGNAL);
	if (ret <= 0){
		Opts::SoError error_opt;
		Access::GetOption(serviceimpl.GetService(), this->_impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_WRITE_FAIL) : SocketError(error_opt.Value());
	}

	function_t <void(s_uint32_t, SocketError)> handler;
	handler.swap(this->_handler);
	this->Clear();
	if (notify)
		handler(ret < 0 ? 0 : ret, error);
	return true;
}

M_SOCKET_DECL void EpollService::WriteOperation2::Clear(){
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

M_SOCKET_DECL bool EpollService::ReadOperation2::Complete(IoServiceImpl& serviceimpl, epoll_event_t* event){
	SocketError error;
	bool notify = false;
	MutexLock& mlock = M_IMPL_MUTEX(this->_impl);
	mlock.lock();
	if (M_IMPL_FD(this->_impl) != M_INVALID_SOCKET) {
		notify = true;
		M_IMPL_C_READ_FLAG(this->_impl);
		EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
		opset->_type &= ~(E_READ_OP);
		s_int32_t flag = 0;
		if (opset->_type & E_WRITE_OP)
			flag = M_EPOLLOUT;
		Access::CtlEpoll(serviceimpl.GetService(), this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);
	}
	mlock.unlock();

	s_int32_t ret = g_recv(M_IMPL_FD(this->_impl), this->_wsabuf.buf
		, (s_uint32_t)this->_wsabuf.len, 0);
	if (ret <= 0)
	{
		Opts::SoError error_opt;
		Access::GetOption(serviceimpl.GetService(), this->_impl, error_opt, error);
		error = (error) ? SocketError(M_ERR_ASYNC_READ_FAIL) : SocketError(error_opt.Value());
	}

	function_t <void(s_uint32_t, SocketError)> handler;
	handler.swap(this->_handler);
	this->Clear();
	if (notify)
		handler(ret < 0 ? 0 : ret, error);
	return true;
}

M_SOCKET_DECL void EpollService::ReadOperation2::Clear(){
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}


#endif
#endif
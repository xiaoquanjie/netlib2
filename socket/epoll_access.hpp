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

#include "config.hpp"
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

#define M_IMPL_INIT(impl)\
	impl.Init()
#define M_IMPL_MUTEX(impl)\
	(impl._core->_mutex)

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

M_SOCKET_DECL void EpollService::Access::ExecOp(IoServiceImpl& serviceimpl, EpollService::OperationSet* opset, epoll_event_t* event)
{
	/*if (opset->_type & E_ACCEPT_OP)
	{
		opset->_aop.Complete(serviceimpl, event);
		return;
	}
	if (opset->_type & E_CONNECT_OP)
	{
		opset->_cop->Complete(serviceimpl, event);
		return;
	}
	if (opset->_type & E_FINISH_OP)
	{
		opset->_aop.Complete(serviceimpl, event);
		return;
	}
	bool flag = false;
	if (opset->_type & E_READ_OP &&
		(event->events&M_EPOLLIN || event->events&M_EPOLLERR || event->events&M_EPOLLHUP))
	{
		flag = true;
		opset->_rop->Complete(serviceimpl, event);
	}
	if (opset->_type & E_WRITE_OP &&
		(event->events&M_EPOLLOUT || event->events&M_EPOLLERR || event->events&M_EPOLLHUP))
	{
		flag = true;
		opset->_wop->Complete(serviceimpl, event);
		return;
	}
	if (!flag)
	{
		M_DEBUG_PRINT("type: " << opset->_type);
		assert(0);
	}*/
}

M_SOCKET_DECL void EpollService::Access::Run(EpollService& service, SocketError& error)
{
	//EpollService::IoServiceImpl* impl = new EpollService::IoServiceImpl(service);
	//CreateEpoll(*impl, error);
	//if (error)
	//{
	//	delete impl;
	//	return;
	//}
	//MutexLock& lock = service._mutex;
	//lock.lock();
	//service._implvector.push_back(impl);
	//service._implmap[impl->_handler] = impl;
	//++service._implcnt;
	//lock.unlock();

	//const int max_events = 100;
	//epoll_event_t events[max_events];
	//for (;;)
	//{
	//	/*for (ImplVector::iterator iter=impl->_closeimplvector.begin(); iter!=impl->_closeimplvector.end();
	//		)
	//	{
	//		Impl& imple = *iter;
	//		if (M_IMPL_OP(imple)._aop)
	//			M_IMPL_OP(imple)._aop->Clear();
	//		if (M_IMPL_OP(imple)._cop)
	//			M_IMPL_OP(imple)._cop->Clear();
	//		if (M_IMPL_OP(imple)._rop)
	//			M_IMPL_OP(imple)._rop->Clear();
	//		if (M_IMPL_OP(imple)._wop)
	//			M_IMPL_OP(imple)._wop->Clear();

	//		iter = impl->_closeimplvector.erase(iter);
	//	}*/

	//	g_setlasterr(0);
	//	g_bzero(&events, sizeof(events));
	//	s_int32_t ret = g_epoll_wait(impl->_handler, events, max_events, -1);
	//	if (ret == 0)
	//	{
	//		1; // time out
	//		continue;
	//	}
	//	if (ret < 0 && M_ERR_LAST != M_EINTR)
	//	{
	//		M_DEFAULT_SOCKET_ERROR2(error);
	//		break;;
	//	}
	//	bool brk = false;
	//	for (s_int32_t idx = 0; idx < ret; ++idx)
	//	{
	//		EpollService::OperationSet* opset = (EpollService::OperationSet*)events[idx].data.ptr;
	//		ExecOp(*impl, opset, &events[idx]);
	//		if (opset->_type & E_FINISH_OP)
	//		{
	//			brk = true;
	//			delete opset;
	//		}
	//	}
	//	if (brk)
	//		break;
	//}

	//lock.lock();
	//service._implvector.erase(std::find(service._implvector.begin(), service._implvector.end(), impl));
	//service._implmap.erase(impl->_handler);
	//--service._implcnt;
	//lock.unlock();
	//DestroyEpoll(*impl);
	//delete impl;
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

M_SOCKET_DECL void EpollService::Access::AsyncAccept(EpollService& service, Impl& accept_impl, Impl& sock_impl
	, const M_COMMON_HANDLER_TYPE(IocpService2)& handler, SocketError& error)
{
	//if (M_IMPL_FD(accept_impl) == M_INVALID_SOCKET)
	//{
	//	error = SocketError(M_ERR_BAD_DESCRIPTOR);
	//	return;
	//}
	//if (M_IMPL_G_ACCEPT_FLAG(accept_impl))
	//{
	//	error = SocketError(M_ERR_POSTED_ACCEPT);
	//	return;
	//}
	//if (!M_IMPL_G_NONBLOCK(accept_impl))
	//{
	//	if (!detail::Util::SetNonBlock(M_IMPL_FD(accept_impl)))
	//	{
	//		M_DEFAULT_SOCKET_ERROR2(error);
	//		return;
	//	}
	//	M_IMPL_S_NONBLOCK(accept_impl)
	//}

	//typedef EpollService::AcceptOperation2 OperationType;
	//OperationType* accept_op = 0;// dynamic_cast<OperationType*>(M_IMPL_OP(accept_impl)._aop);
	//if (!accept_op)
	//{
	//	EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(accept_impl), E_ACCEPT_OP);
	//	//accept_op = dynamic_cast<OperationType*>(M_IMPL_OP(accept_impl)._aop);
	//	M_IMPL_OP(accept_impl)._type = E_ACCEPT_OP;
	//}
	//accept_op->_handler = handler;
	//accept_op->_acpt_impl = accept_impl;
	//accept_op->_cli_impl = sock_impl;

	//M_IMPL_S_ACCEPT_FLAG(accept_impl);
	//if (!M_IMPL_G_BIND(accept_impl))
	//{
	//	M_IMPL_S_BIND(accept_impl);
	//	CtlEpoll(service, accept_impl, &M_IMPL_OP(accept_impl), M_EPOLL_CTL_ADD, M_EPOLLIN, error);
	//}
	//if (error)
	//{
	//	M_IMPL_C_ACCEPT_FLAG(accept_impl);
	//	M_IMPL_S_UNBIND(accept_impl);
	//	accept_op->Clear();
	//	return;
	//}
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::Connect(EpollService& service, EpollService::Impl& impl, const EndPoint& ep, SocketError& error)
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
		if (M_IMPL_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL_S_CONNECT_FLAG(impl);
		mlock.unlock();

		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		s_int32_t ret = g_connect(M_IMPL_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
		
		mlock.lock();
		M_IMPL_C_CONNECT_FLAG(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret != 0, error);
		return;
	} 
	while (false);
	mlock.unlock();
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::AsyncConnect(EpollService& service, Impl& impl, const EndPoint& ep
	, const M_COMMON_HANDLER_TYPE(EpollService)& handler, SocketError& error)
{
	/*if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_CONNECT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_CONNECT);
		return;
	}
	if (!M_IMPL_G_NONBLOCK(impl))
	{
		if (!detail::Util::SetNonBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_NONBLOCK(impl)
	}

	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_connect(M_IMPL_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	if (ret == -1 && M_ERR_LAST != M_EINPROGRESS)
	{
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}

	typedef EpollService::ConnectOperation2 OperationType;
	OperationType* connect_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._cop);
	if (!connect_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_CONNECT_OP);
		connect_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._cop);
		M_IMPL_OP(impl)._type = E_CONNECT_OP;
	}
	connect_op->_handler = handler;
	connect_op->_impl = impl;

	M_IMPL_S_CONNECT_FLAG(impl);
	if (!M_IMPL_G_BIND(impl))
	{
		M_IMPL_S_BIND(impl);
		CtlEpoll(service, impl, &M_IMPL_OP(impl), M_EPOLL_CTL_ADD, M_EPOLLOUT | M_EPOLLONESHOT, error);
	}
	if (error)
	{
		M_IMPL_C_CONNECT_FLAG(impl);
		M_IMPL_S_UNBIND(impl);
		connect_op->Clear();
		return;
	}*/
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
	/*if (!M_IMPL_G_NONBLOCK(impl))
	{
		if (!detail::Util::SetNonBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_NONBLOCK(impl)
	}
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_READ_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_READ);
		return;
	}

	typedef EpollService::ReadOperation2 OperationType;
	OperationType* read_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._rop);
	if (!read_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_READ_OP);
		read_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._rop);
	}
	read_op->_wsabuf.buf = data;
	read_op->_wsabuf.len = size;
	read_op->_handler = hander;
	read_op->_impl = impl;

	M_IMPL_OP(impl)._type = E_READ_OP;
	int flag = M_EPOLLIN;
	int ctl = M_EPOLL_CTL_MOD;
	if (!M_IMPL_G_BIND(impl))
	{
		M_IMPL_S_BIND(impl);
		ctl = M_EPOLL_CTL_ADD;
	}
	if (M_IMPL_G_WRITE_FLAG(impl))
	{
		flag |= M_EPOLLOUT;
		M_IMPL_OP(impl)._type |= E_WRITE_OP;
	}

	M_IMPL_S_READ_FLAG(impl);
	CtlEpoll(service, impl, &M_IMPL_OP(impl), ctl, flag, error);
	if (error)
	{
		M_IMPL_C_READ_FLAG(impl);
		M_IMPL_S_UNBIND(impl);
		read_op->Clear();
	}*/
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
	/*if (!M_IMPL_G_NONBLOCK(impl))
	{
		if (!detail::Util::SetNonBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_NONBLOCK(impl)
	}
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_WRITE_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_WRITE);
		return;
	}

	typedef EpollService::WriteOperation2 OperationType;
	OperationType* write_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._wop);
	if (!write_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_WRITE_OP);
		write_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._wop);
	}
	write_op->_wsabuf.buf = const_cast<s_byte_t*>(data);
	write_op->_wsabuf.len = size;
	write_op->_handler = hander;
	write_op->_impl = impl;

	M_IMPL_OP(impl)._type = E_WRITE_OP;
	int flag = M_EPOLLOUT;
	int ctl = M_EPOLL_CTL_MOD;
	if (!M_IMPL_G_BIND(impl))
	{
		M_IMPL_S_BIND(impl);
		ctl = M_EPOLL_CTL_ADD;
	}
	if (M_IMPL_G_READ_FLAG(impl))
	{
		flag |= M_EPOLLIN;
		M_IMPL_OP(impl)._type |= E_READ_OP;
	}

	M_IMPL_S_WRITE_FLAG(impl);
	CtlEpoll(service, impl, &M_IMPL_OP(impl), ctl, flag, error);
	if (error)
	{
		M_IMPL_C_WRITE_FLAG(impl);
		M_IMPL_S_UNBIND(impl);
		write_op->Clear();
	}*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

M_SOCKET_DECL bool EpollService::AcceptOperation2::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	/*if (M_IMPL_FD(this->_acpt_impl) != M_INVALID_SOCKET)
		M_IMPL_C_ACCEPT_FLAG(this->_acpt_impl);

	SocketError error;
	socket_t fd = g_accept(M_IMPL_FD(this->_acpt_impl), 0, 0);
	if (fd < 0)
	{
		Opts::SoError error_opt;
		EpollService::Access::GetOption(serviceimpl.GetService(), this->_acpt_impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_ACCEPT_FAIL) : SocketError(error_opt.Value());
	}
	else{
		M_IMPL_FD(this->_cli_impl) = fd;
		if (M_IMPL_G_V(this->_acpt_impl)) {
			M_IMPL_S_V4(this->_cli_impl);
		}
		else {
			M_IMPL_S_V6(this->_cli_impl);
		}
	}

	function_t<void(SocketError)> handler = this->_handler;

	this->Clear();
	handler(error);
	return true;*/
}

//template<typename Handler>
M_SOCKET_DECL void EpollService::AcceptOperation2::Clear()
{
	this->_handler = 0;
	this->_acpt_impl = this->_cli_impl = Impl();
}

M_SOCKET_DECL bool EpollService::ConnectOperation2::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	/*if (M_IMPL_FD(this->_impl) != M_INVALID_SOCKET)
		M_IMPL_C_CONNECT_FLAG(this->_impl);

	SocketError error;
	if (event->events & M_EPOLLERR)
	{
		Opts::SoError error_opt;
		EpollService::Access::GetOption(serviceimpl.GetService(), this->_impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_CONNECT_FAIL) : SocketError(error_opt.Value());
	}

	function_t <void(SocketError)> handler = this->_handler;
	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	opset->_type &= ~(E_CONNECT_OP);

	this->Clear();
	handler(error);
	return true;*/
}

M_SOCKET_DECL void EpollService::ConnectOperation2::Clear()
{
	this->_handler = 0;
	this->_impl = Impl();
}

M_SOCKET_DECL bool EpollService::WriteOperation2::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	/*if (M_IMPL_FD(this->_impl) != M_INVALID_SOCKET)
		M_IMPL_C_WRITE_FLAG(this->_impl);

	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;

	SocketError error;
	s_int32_t ret = g_send(M_IMPL_FD(this->_impl), data, size, MSG_NOSIGNAL);
	if (ret <= 0)
	{
		Opts::SoError error_opt;
		Access::GetOption(serviceimpl.GetService(), this->_impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_WRITE_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	s_int32_t flag = 0;
	if (opset->_type & E_READ_OP)
		flag = M_EPOLLIN;
	opset->_type &= ~(E_WRITE_OP);
	Access::CtlEpoll(serviceimpl.GetService(), this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);

	function_t <void(s_uint32_t, SocketError)> handler = this->_handler;
	this->Clear();
	handler(ret < 0 ? 0 : ret, error);
	return true;*/
}

M_SOCKET_DECL void EpollService::WriteOperation2::Clear()
{
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

M_SOCKET_DECL bool EpollService::ReadOperation2::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	/*if (M_IMPL_FD(this->_impl) != M_INVALID_SOCKET)
		M_IMPL_C_READ_FLAG(this->_impl);

	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;

	SocketError error;
	s_int32_t ret = g_recv(M_IMPL_FD(this->_impl), data, size, 0);
	if (ret <= 0)
	{
		Opts::SoError error_opt;
		Access::GetOption(serviceimpl.GetService(), this->_impl, error_opt, error);
		error = (error) ? SocketError(M_ERR_ASYNC_READ_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	s_int32_t flag = 0;
	if (opset->_type & E_WRITE_OP)
		flag = M_EPOLLOUT;
	opset->_type &= ~(E_READ_OP);
	Access::CtlEpoll(serviceimpl.GetService(), this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);

	function_t <void(s_uint32_t, SocketError)> handler = this->_handler;
	this->Clear();
	handler(ret < 0 ? 0 : ret, error);
	return true;*/
}

M_SOCKET_DECL void EpollService::ReadOperation2::Clear()
{
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

M_SOCKET_DECL bool EpollService::FinishOperation::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	/*g_closesocket(this->_fd);
	this->_fd = M_INVALID_SOCKET;
	return true;*/
}

M_SOCKET_DECL void EpollService::FinishOperation::Clear()
{
}

template<typename T>
M_SOCKET_DECL void EpollService::OperationAlloc<T>::Alloc(OperationSet* opset, s_int32_t type)
{
	///*if (type & E_ACCEPT_OP && !opset->_aop)
	//{
	//	opset->_aop = new T;
	//}
	//else */if (type & E_CONNECT_OP && !opset->_cop)
	//{
	//	opset->_cop = new T;
	//}
	//else if (type & E_READ_OP && !opset->_rop)
	//{
	//	opset->_rop = new T;
	//}
	//else if (type & E_WRITE_OP && !opset->_wop)
	//{
	//	opset->_wop = new T;
	//}
	//else
	//{
	//	assert(0);
	//}
}

#endif
#endif
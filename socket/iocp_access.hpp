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

#ifndef M_IOCP_ACCESS_INCLUDE
#define M_IOCP_ACCESS_INCLUDE

#ifdef M_PLATFORM_WIN

// ipv4 or ipv6 bit(0)
#define M_IMPL2_S_V4(impl)\
	M_CLR_BIT(impl._core->_state,0)
#define M_IMPL2_S_V6(impl)\
	M_SET_BIT(impl._core->_state,0)
#define M_IMPL2_G_V(impl)\
	M_GET_BIT(impl._core->_state,0)

// bind bit(1)
#define M_IMPL2_S_BIND(impl)\
	M_SET_BIT(impl._core->_state,1)
#define M_IMPL2_S_UNBIND(impl)\
	M_CLR_BIT(impl._core->_state,1)
#define M_IMPL2_G_BIND(impl)\
	M_GET_BIT(impl._core->_state,1)

// block mode bit(2)
#define M_IMPL2_S_BLOCK(impl)\
	M_SET_BIT(impl._core->_state,2)
#define M_IMPL2_G_BLOCK(impl)\
	M_GET_BIT(impl._core->_state,2)
#define M_IMPL2_S_NONBLOCK(impl)\
	M_CLR_BIT(impl._core->_state,2)
#define M_IMPL2_G_NONBLOCK(impl)\
	(!M_GET_BIT(impl._core->_state,2))

// type bit(3~4)
#define M_IMPL2_S_TYPE(impl,type)\
	M_SET_MBIT(impl._core->_state,type,3,4)
#define M_IMPL2_G_TYPE(impl)\
	M_GET_MBIT(impl._core->_state,3,4)

// accept flag bit(5)
#define M_IMPL2_G_ACCEPT_FLAG(impl)\
	M_GET_BIT(impl._core->_state,5)
#define M_IMPL2_S_ACCEPT_FLAG(impl)\
	M_SET_BIT(impl._core->_state,5)
#define M_IMPL2_C_ACCEPT_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,5)

// connect flag bit(6)
#define M_IMPL2_G_CONNECT_FLAG(impl)\
	M_GET_BIT(impl._core->_state,6)
#define M_IMPL2_S_CONNECT_FLAG(impl)\
	M_SET_BIT(impl._core->_state,6)
#define M_IMPL2_C_CONNECT_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,6)

// write flag bit(7)
#define M_IMPL2_G_WRITE_FLAG(impl)\
	M_GET_BIT(impl._core->_state,7)
#define M_IMPL2_S_WRITE_FLAG(impl)\
	M_SET_BIT(impl._core->_state,7)
#define M_IMPL2_C_WRITE_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,7)

// read flag bit(8)
#define M_IMPL2_G_READ_FLAG(impl)\
	M_GET_BIT(impl._core->_state,8)
#define M_IMPL2_S_READ_FLAG(impl)\
	M_SET_BIT(impl._core->_state,8)
#define M_IMPL2_C_READ_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,8)

// close flag bit(9)
#define M_IMPL2_G_CLOSE_FLAG(impl)\
	M_GET_BIT(impl._core->_state,9)
#define M_IMPL2_S_CLOSE_FLAG(impl)\
	M_SET_BIT(impl._core->_state,9)
#define M_IMPL2_C_CLOSE_FLAG(impl)\
	M_CLR_BIT(impl._core->_state,9)

#define M_IMPL2_FD(impl)\
	impl._core->_fd
#define M_IMPL2_STATE(impl)\
	impl._core->_state
#define M_IMPL2_IOCP(impl)\
	impl._core->_iocp
#define M_IMPL2_OP(impl)\
	impl._core->_op
#define M_IMPL2_ROP(impl)\
	impl._core->_op._rop
#define M_IMPL2_WOP(impl)\
	impl._core->_op._wop
#define M_IMPL2_AOP(impl)\
	impl._core->_op._aop
#define M_IMPL2_COP(impl)\
	impl._core->_op._cop

#define M_IMPL2_MUTEX(impl)\
	(*impl._core->_mutex)
#define M_IMPL2_INIT_MUTEX(impl)\
	impl._core->_mutex.reset(new MutexLock);

#define M_FOREACH_CLOSEREQ(item,reqlist)\
	for (std::list<ImplCloseReq*>::iterator item=reqlist.begin();\
		item!=reqlist.end(); ++item)

////////////////////////////////////////////////////////////////////////////////

M_SOCKET_DECL void IocpService2::Access::ConstructImpl(IocpService2& service, Impl& impl, s_uint16_t type){
	M_IMPL2_IOCP(impl) = 0;
	M_IMPL2_FD(impl) = M_INVALID_SOCKET;
	M_IMPL2_STATE(impl) = 0;
	M_IMPL2_S_V4(impl);
	M_IMPL2_S_UNBIND(impl);
	M_IMPL2_S_BLOCK(impl);
	M_IMPL2_S_TYPE(impl, (s_uint8_t)type);
	M_IMPL2_INIT_MUTEX(impl);
}

M_SOCKET_DECL void IocpService2::Access::DestroyImpl(IocpService2& service, Impl& impl){
	ConstructImpl(service, impl, E_NULL_SOCKET_TYPE);
}

M_SOCKET_DECL bool IocpService2::Access::IsOpen(IocpService2& service, Impl& impl, SocketError& error){
	return (M_IMPL2_FD(impl) != M_INVALID_SOCKET);
}

template<typename GettableOptionType>
M_SOCKET_DECL void IocpService2::Access::GetOption(IocpService2& service, Impl& impl, GettableOptionType& opt, SocketError& error){
	socklen_t len = opt.Size();
	s_int32_t ret = g_getsockopt(M_IMPL2_FD(impl), opt.Level(), opt.Name(), (char*)opt.Data(), &len);
	M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
}

template<typename SettableOptionType>
M_SOCKET_DECL void IocpService2::Access::SetOption(IocpService2& service, Impl& impl, const SettableOptionType& opt, SocketError& error){
	s_int32_t ret = g_setsockopt(M_IMPL2_FD(impl), opt.Level(), opt.Name(), (char*)opt.Data(), opt.Size());
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint IocpService2::Access::RemoteEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error){
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetRemoteEndPoint(M_IMPL2_FD(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint IocpService2::Access::LocalEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error){
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetLocalEndPoint(M_IMPL2_FD(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

M_SOCKET_DECL void IocpService2::Access::Cancel(IocpService2& service, Impl& impl, SocketError& error){
	if (M_IMPL2_G_BIND(impl)){
		if (!g_cancelio((HANDLE)M_IMPL2_FD(impl)))
			M_DEFAULT_SOCKET_ERROR2(error);
	}
}

M_SOCKET_DECL void IocpService2::Access::CreateIocp(IocpService2::IoServiceImpl& impl, SocketError& error){
	if (impl._handler != 0){
		error = SocketError(M_ERR_IOCP_EXIST);
		return;
	}
	impl._handler = g_createiocompletionport(INVALID_HANDLE_VALUE, 0, 0, 0);
	M_DEFAULT_SOCKET_ERROR(impl._handler == 0, error);
}

M_SOCKET_DECL void IocpService2::Access::DestroyIocp(IocpService2::IoServiceImpl& impl){
	if (impl._handler != 0){
		g_closehandle(impl._handler);
		impl._handler = 0;
		impl._fdcnt = 0;
	}
}

M_SOCKET_DECL void IocpService2::Access::BindIocp(IocpService2& service, Impl& impl, SocketError& error){
	if (M_IMPL2_IOCP(impl) == 0){
		ScopedLock scoped_l(service._mutex);
		if (!service._implvector.empty()) {
			service._implidx++;
			s_uint32_t size = (s_uint32_t)service._implvector.size();
			IocpService2::IoServiceImpl& serviceimpl = *(service._implvector[service._implidx%size]);
			M_IMPL2_IOCP(impl) = serviceimpl._handler;
			serviceimpl._fdcnt++;
		}
	}
	if (M_IMPL2_IOCP(impl) == 0){
		error = SocketError(M_ERR_NOT_SERVICE);
		return;
	}
	HANDLE ret = g_createiocompletionport((HANDLE)M_IMPL2_FD(impl), M_IMPL2_IOCP(impl), 0, 0);
	M_DEFAULT_SOCKET_ERROR(M_IMPL2_IOCP(impl) != ret, error);
}

M_SOCKET_DECL void IocpService2::Access::ExecOp(IocpService2& service, IocpService2::Operation* op, s_uint32_t transbyte, bool ok){
	SocketError error;
	if (!ok){
		// op->Internal is not error code
		M_DEFAULT_SOCKET_ERROR2(error);
	}
	op->_oper->Complete(service, transbyte, error);
}

M_SOCKET_DECL void IocpService2::Access::Run(IocpService2& service, SocketError& error)
{
	IocpService2::IoServiceImpl* simpl = new IocpService2::IoServiceImpl(service);
	CreateIocp(*simpl, error);
	if (error) {
		delete simpl;
		return;
	}

	service._mutex.lock();
	service._implmap[simpl->_handler] = simpl;
	service._implvector.push_back(simpl);
	++service._implcnt;
	service._mutex.unlock();

	std::list<ImplCloseReq*> closereqs, closereqs2;
	for (;;){
		simpl->_mutex.lock();
		closereqs.swap(simpl->_closereqs);
		simpl->_mutex.unlock();

		M_FOREACH_CLOSEREQ(iter, closereqs) {
			_ExecClose((*iter));
			closereqs2.push_back((*iter));
		}

		simpl->_mutex.lock();
		simpl->_closereqs2.merge(closereqs2);
		simpl->_mutex.unlock();

		closereqs.clear();
		closereqs2.clear();

		DWORD trans_bytes = 0;
		ULONG_PTR comple_key = 0;
		overlapped_t* overlapped = 0;
		g_setlasterr(0);

		BOOL ret = g_getqueuedcompletionstatus(simpl->_handler, &trans_bytes,
			&comple_key, &overlapped, 500);
		if (overlapped){
			IocpService2::Operation* op = (IocpService2::Operation*)overlapped;
			if (op->_type & E_FINISH_OP){
				delete op;
				break;
			}
			ExecOp(service, op, trans_bytes, ret ? true : false);
			continue;
		}
		if (!ret){
			if (M_ERR_LAST != WAIT_TIMEOUT) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
		}
	}

	M_FOREACH_CLOSEREQ(iter, simpl->_closereqs) {
		(*iter)->Clear();
		delete (*iter);
	}
	M_FOREACH_CLOSEREQ(iter, simpl->_closereqs2) {
		(*iter)->Clear();
		delete (*iter);
	}

	service._mutex.lock();
	service._implvector.erase(std::find(service._implvector.begin(), service._implvector.end(), simpl));
	service._implcnt--;
	service._mutex.unlock();
}

M_SOCKET_DECL void IocpService2::Access::Stop(IocpService2& service, SocketError& error){
	for (IocpService2::IoServiceImplVector::iterator iter = service._implvector.begin();
		iter != service._implvector.end(); ++iter){
		Operation* op = new Operation;
		op->_type = E_FINISH_OP;
		if (!g_postqueuedcompletionstatus((*iter)->_handler, 0, 0, op)){
			delete op;
			M_DEFAULT_SOCKET_ERROR2(error);
		}
	}
}

M_SOCKET_DECL bool IocpService2::Access::Stopped(const IocpService2& service){
	ScopedLock scoped_lock(service._mutex);
	return (service._implvector.empty() ? true : false);
}

M_SOCKET_DECL s_uint32_t IocpService2::Access::GetServiceCount(const IocpService2& service){
	ScopedLock scoped(service._mutex);
	return (s_uint32_t)service._implvector.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

M_SOCKET_DECL void IocpService2::Access::Close(IocpService2& service, Impl& impl, SocketError& error) {
	Close(service, impl, 0, error);
}

M_SOCKET_DECL void IocpService2::Access::Close(IocpService2& service, Impl& impl, function_t<void()> handler, SocketError& error) {
	MutexLock& mlock = M_IMPL2_MUTEX(impl);
	mlock.lock();
	if (M_IMPL2_FD(impl) != M_INVALID_SOCKET && !M_IMPL2_G_CLOSE_FLAG(impl)) {
		service._mutex.lock();
		IocpService2::IoServiceImpl* simpl = _GetIoServiceImpl(service, impl);
		if (simpl)
			--simpl->_fdcnt;
		service._mutex.unlock();

		if (M_IMPL2_G_BLOCK(impl)) {
			if (g_closesocket(M_IMPL2_FD(impl)) == M_SOCKET_ERROR)
				M_DEFAULT_SOCKET_ERROR2(error);
			M_IMPL2_FD(impl) = M_INVALID_SOCKET;
			M_IMPL2_STATE(impl) = 0;
		}
		else {
			M_IMPL2_S_CLOSE_FLAG(impl);
		}
		mlock.unlock();

		if (simpl) {
			ScopedLock scoped_l(simpl->_mutex);
			ImplCloseReq* req = 0;
			if (!simpl->_closereqs2.empty()) {
				req = simpl->_closereqs2.front();
				simpl->_closereqs.pop_front();
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
M_SOCKET_DECL void IocpService2::Access::Open(IocpService2& service, Impl& impl, const ProtocolType& pt, SocketError& error) {
	if (M_IMPL2_FD(impl) != M_INVALID_SOCKET) {
		error = SocketError(M_ERR_SOCKET_OPEN);
		return;
	}
	if (pt.Family() == M_AF_INET) {
		M_IMPL2_S_V4(impl);
	}
	else {
		M_IMPL2_S_V6(impl);
	}

	M_IMPL2_FD(impl) = g_socket(pt.Family(), pt.Type(), pt.Protocol());
	if (M_IMPL2_FD(impl) == M_INVALID_SOCKET) {
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}
}

template<typename EndPoint>
M_SOCKET_DECL void IocpService2::Access::Bind(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error) {
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_bind(M_IMPL2_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void IocpService2::Access::Listen(IocpService2& service, Impl& impl, s_int32_t flag, SocketError& error) {
	s_int32_t ret = g_listen(M_IMPL2_FD(impl), flag);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void IocpService2::Access::Shutdown(IocpService2& service, Impl& impl, EShutdownType what, SocketError& error) {
	if (0 != g_shutdown(M_IMPL2_FD(impl), what))
		M_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void IocpService2::Access::Accept(IocpService2& service, Impl& impl, Impl& peer, SocketError& error)
{
	MutexLock& lock = M_IMPL2_MUTEX(impl);
	do 
	{
		lock.lock();
		if (M_INVALID_SOCKET == M_IMPL2_FD(impl)) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_ACCEPT_FLAG(impl)) {
			error = SocketError(M_ERR_POSTED_ACCEPT);
			break;
		}
		if (M_IMPL2_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL2_S_ACCEPT_FLAG(impl);
		lock.unlock();

		for (;;) {
			M_IMPL2_FD(peer) = g_accept(M_IMPL2_FD(impl), 0, 0);
			if (M_INVALID_SOCKET != M_IMPL2_FD(peer)) {
				break;
			}
			if (M_ECONNRESET == M_ERR_LAST) {
				break;
			}
			M_DEFAULT_SOCKET_ERROR2(error);
			break;
		}
		
		lock.lock();
		M_IMPL2_C_ACCEPT_FLAG(impl);
		lock.unlock();
		return;

	} 
	while (false);
	lock.unlock();
}

M_SOCKET_DECL void IocpService2::Access::AsyncAccept(IocpService2& service, Impl& accept_impl, Impl& client_impl, 
	M_COMMON_HANDLER_TYPE(IocpService2) handler, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(accept_impl);
	do 
	{
		mlock.lock();
		if (M_IMPL2_FD(accept_impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_ACCEPT_FLAG(accept_impl)){
			error = SocketError(M_ERR_POSTED_ACCEPT);
			break;
		}
		if (!M_IMPL2_G_NONBLOCK(accept_impl)){
			if (!detail::Util::SetNonBlock(M_IMPL2_FD(accept_impl))){
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_IMPL2_S_NONBLOCK(accept_impl)
		}
		if (!M_IMPL2_G_BIND(accept_impl)){
			BindIocp(service, accept_impl, error);
			if (error)
				break;
			M_IMPL2_S_BIND(accept_impl);
		}
		
		ConstructImpl(service, client_impl, E_SOCKET_TYPE);
		Open(service, client_impl, M_IMPL2_G_V(accept_impl) ? Tcp::V6() : Tcp::V4(), error);
		if (error)
			break;

		M_IMPL2_S_ACCEPT_FLAG(accept_impl);
		mlock.unlock();

		typedef IocpService2::AcceptOperation2<M_COMMON_HANDLER_TYPE(IocpService2)> OperationType;
		IocpService2::OperationAlloc<OperationType>::AllocOp(M_IMPL2_AOP(accept_impl),
			E_ACCEPT_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_IMPL2_AOP(accept_impl)._oper);
		g_bzero(op->_buf, sizeof(op->_buf));
		op->_bytes   = 0;
		op->_handler = handler;
		op->_impl    = client_impl;
		op->_accept_impl = accept_impl;

		for (;;){
			
			// If no error occurs, the AcceptEx function completed successfully and a value of TRUE is returned.
			s_int32_t ret = gAcceptEx(M_IMPL2_FD(accept_impl), M_IMPL2_FD(op->_impl), op->_buf,
				0, sizeof(sockaddr_storage_t), sizeof(sockaddr_storage_t), (LPDWORD)&op->_bytes, &M_IMPL2_AOP(accept_impl));

			if (!ret && M_ERR_LAST == M_ECONNRESET){
				M_DEBUG_PRINT("peer connect reset");
				continue;
			}
			if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING){
				M_DEBUG_PRINT("acceptex fail");
				M_DEFAULT_SOCKET_ERROR2(error);
				
				SocketError error2;
				Close(service, client_impl, error2);
				op->Clear();

				mlock.lock();
				M_IMPL2_C_ACCEPT_FLAG(accept_impl);
				mlock.unlock();
			}
			break;
		}
		return;
	} 
	while (false);
	mlock.unlock();
}

template<typename EndPoint>
M_SOCKET_DECL void IocpService2::Access::Connect(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL2_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_CONNECT_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_CONNECT);
			break;
		}
		if (!ep.Valid()){
			error = SocketError(M_ERR_ENDPOINT_INVALID);
			break;
		}
		if (M_IMPL2_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL2_S_CONNECT_FLAG(impl);
		mlock.unlock();

		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		s_int32_t ret = g_connect(M_IMPL2_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
		
		mlock.lock();
		M_IMPL2_C_CONNECT_FLAG(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret != 0, error);
		return;
	} 
	while (false);
	mlock.unlock();
}

template<typename EndPoint>
M_SOCKET_DECL void IocpService2::Access::AsyncConnect(IocpService2& service, Impl& impl, const EndPoint& ep, 
	M_COMMON_HANDLER_TYPE(IocpService2) handler, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(impl);
	do 
	{
		if (M_IMPL2_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_CONNECT_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_CONNECT);
			break;
		}
		if (!ep.Valid()) {
			error = SocketError(M_ERR_ENDPOINT_INVALID);
			break;
		}
		if (!M_IMPL2_G_NONBLOCK(impl)){
			if (!detail::Util::SetNonBlock(M_IMPL2_FD(impl))){
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_IMPL2_S_NONBLOCK(impl)
		}
		if (!M_IMPL2_G_BIND(impl)){
			BindIocp(service, impl, error);
			if (error)
				break;
			M_IMPL2_S_BIND(impl);
		}

		M_IMPL2_S_CONNECT_FLAG(impl);
		mlock.unlock();

		sockaddr_storage_t addr;
		g_bzero(&addr, sizeof(addr));
		addr.ss_family = ep.Protocol().Family();
		g_bind(M_IMPL2_FD(impl), (sockaddr_t*)&addr, sizeof(addr));

		typedef IocpService2::ConnectOperation2<M_COMMON_HANDLER_TYPE(IocpService2)> OperationType;
		IocpService2::OperationAlloc<OperationType>::AllocOp(M_IMPL2_COP(impl), E_CONNECT_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_IMPL2_COP(impl)._oper);
		op->_handler = handler;
		op->_impl = impl;

		DWORD send_bytes = 0;
		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		s_int32_t ret = gConnectEx(M_IMPL2_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep), 
			0, 0, &send_bytes, &M_IMPL2_COP(impl));
		if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING){
			M_DEFAULT_SOCKET_ERROR2(error);
			op->Clear();

			mlock.lock();
			M_IMPL2_C_CONNECT_FLAG(impl);
			mlock.unlock();
		}
		return;
	} 
	while (false);
	mlock.unlock();
}

M_SOCKET_DECL s_int32_t IocpService2::Access::RecvSome(IocpService2& service, Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL2_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_READ_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_READ);
			break;
		}
		if (M_IMPL2_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL2_S_READ_FLAG(impl);
		mlock.unlock();

		DWORD trans_bytes = 0;
		wsabuf_t wsabuf;
		wsabuf.buf = data;
		wsabuf.len = static_cast<DWORD>(size);
		DWORD flag = 0;
		s_int32_t ret = g_wsarecv(M_IMPL2_FD(impl), &wsabuf, 1, &trans_bytes, &flag, 0, 0);
		
		mlock.lock();
		M_IMPL2_C_READ_FLAG(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return trans_bytes;
	} 
	while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

M_SOCKET_DECL void IocpService2::Access::AsyncRecvSome(IocpService2& service, Impl& impl, s_byte_t* data, s_uint32_t size, 
	M_RW_HANDLER_TYPE(IocpService2) hander, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL2_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_READ_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_READ);
			break;
		}
		if (!M_IMPL2_G_NONBLOCK(impl)){
			if (!detail::Util::SetNonBlock(M_IMPL2_FD(impl))){
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_IMPL2_S_NONBLOCK(impl)
		}
		if (!M_IMPL2_G_BIND(impl)){
			BindIocp(service, impl, error);
			if (error)
				break;
			M_IMPL2_S_BIND(impl);
		}

		M_IMPL2_S_READ_FLAG(impl);
		mlock.unlock();

		typedef IocpService2::ReadOperation2<M_RW_HANDLER_TYPE(IocpService2)> OperationType;
		IocpService2::OperationAlloc<OperationType>::AllocOp(M_IMPL2_ROP(impl), E_READ_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_IMPL2_ROP(impl)._oper);
		op->_handler = hander;
		op->_impl = impl;
		op->_wsabuf.buf = data;
		op->_wsabuf.len = size;

		DWORD flag = 0;
		s_int32_t ret = g_wsarecv(M_IMPL2_FD(impl), &op->_wsabuf, 1, 0, &flag, &M_IMPL2_ROP(impl), 0);
		if (ret == M_SOCKET_ERROR  && M_ERR_LAST != M_ERROR_IO_PENDING){
			M_DEFAULT_SOCKET_ERROR2(error);
			op->Clear();

			mlock.lock();
			M_IMPL2_C_READ_FLAG(impl);
			mlock.unlock();
		}
		return;
	} 
	while (false);
	mlock.unlock();
}

M_SOCKET_DECL s_int32_t IocpService2::Access::SendSome(IocpService2& service, Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL2_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_WRITE_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_WRITE);
			break;
		}
		if (M_IMPL2_G_NONBLOCK(impl)) {
			error = SocketError(M_ERR_IS_NONBLOCK);
			break;
		}
		M_IMPL2_S_WRITE_FLAG(impl);
		mlock.unlock();

		DWORD trans_bytes = 0;
		wsabuf_t wsabuf;
		wsabuf.buf = const_cast<s_byte_t*>(data);
		wsabuf.len = static_cast<DWORD>(size);;
		s_int32_t ret = g_wsasend(M_IMPL2_FD(impl), &wsabuf, 1, &trans_bytes, 0, 0, 0);
		
		mlock.lock();
		M_IMPL2_C_WRITE_FLAG(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return trans_bytes;
	} 
	while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

M_SOCKET_DECL void IocpService2::Access::AsyncSendSome(IocpService2& service, Impl& impl, const s_byte_t* data, s_uint32_t size, 
	M_RW_HANDLER_TYPE(IocpService2) hander, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(impl);
	do 
	{
		mlock.lock();
		if (M_IMPL2_FD(impl) == M_INVALID_SOCKET){
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_IMPL2_G_WRITE_FLAG(impl)){
			error = SocketError(M_ERR_POSTED_WRITE);
			return;
		}
		if (!M_IMPL2_G_NONBLOCK(impl)){
			if (!detail::Util::SetNonBlock(M_IMPL2_FD(impl))){
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_IMPL2_S_NONBLOCK(impl)
		}
		if (!M_IMPL2_G_BIND(impl)){
			BindIocp(service, impl, error);
			if (error)
				break;
			M_IMPL2_S_BIND(impl);
		}

		M_IMPL2_S_WRITE_FLAG(impl);
		mlock.unlock();

		typedef IocpService2::WriteOperation2< M_RW_HANDLER_TYPE(IocpService2)> OperationType;
		IocpService2::OperationAlloc<OperationType>::AllocOp(M_IMPL2_WOP(impl), E_WRITE_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_IMPL2_WOP(impl)._oper);
		op->_handler = hander;
		op->_impl = impl;
		op->_wsabuf.buf = const_cast<s_byte_t*>(data);
		op->_wsabuf.len = size;

		DWORD send_bytes = 0;
		// If no error occurs and the send operation has completed immediately, WSASend returns zero
		s_int32_t ret = g_wsasend(M_IMPL2_FD(impl), &op->_wsabuf, 1, 0, 0, &M_IMPL2_WOP(impl), 0);
		if (ret == M_SOCKET_ERROR && M_ERR_LAST != M_ERROR_IO_PENDING){
			M_DEFAULT_SOCKET_ERROR2(error);
			op->Clear();

			mlock.lock();
			M_IMPL2_C_WRITE_FLAG(impl);
			mlock.unlock();
		}
	} 
	while (false);
	mlock.unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

M_SOCKET_DECL void IocpService2::Access::_ExecClose(ImplCloseReq* req) {
	g_closesocket(M_IMPL2_FD(req->_impl));
	if (req->_handler)
		req->_handler();
	req->Clear();
}

M_SOCKET_DECL IocpService2::IoServiceImpl* IocpService2::Access::_GetIoServiceImpl(IocpService2& service, Impl& impl) {
	if (M_IMPL2_IOCP(impl) == 0)
		return 0;

	IocpService2::IoServiceImplMap::iterator iter = service._implmap.find(M_IMPL2_IOCP(impl));
	if (iter == service._implmap.end())
		return 0;
	return iter->second;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Handler>
M_SOCKET_DECL bool IocpService2::AcceptOperation2<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(this->_accept_impl);
	mlock.lock();
	if (M_IMPL2_FD(this->_accept_impl) != M_INVALID_SOCKET)
		M_IMPL2_C_ACCEPT_FLAG(this->_accept_impl);
	mlock.unlock();

	if (error) {
		SocketError error2;
		Access::Close(service, this->_impl, error2);
	}
	else {
		if (M_IMPL2_FD(this->_accept_impl) != M_INVALID_SOCKET) {
			g_setsockopt(M_IMPL2_FD(this->_impl), M_SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, 
				(char*)&(M_IMPL2_FD(this->_accept_impl)),sizeof(M_IMPL2_FD(this->_accept_impl)));
		}
	}
	Handler handler = this->_handler;
	this->Clear();
	handler(error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void IocpService2::AcceptOperation2<Handler>::Clear()
{
	this->_handler = 0;
	this->_accept_impl = this->_impl = Impl();
}

template<typename Handler>
M_SOCKET_DECL bool IocpService2::ConnectOperation2<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(this->_impl);
	mlock.lock();
	if (M_IMPL2_FD(this->_impl) != M_INVALID_SOCKET)
		M_IMPL2_C_CONNECT_FLAG(this->_impl);
	mlock.unlock();

	if (!error){
		g_setsockopt(M_IMPL2_FD(this->_impl), M_SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
	}

	Handler handler = this->_handler;
	this->Clear();
	handler(error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void IocpService2::ConnectOperation2<Handler>::Clear()
{
	this->_handler = 0;
	this->_impl = Impl();
}

template<typename Handler>
M_SOCKET_DECL bool IocpService2::WriteOperation2<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(this->_impl);
	mlock.lock();
	if (M_IMPL2_FD(this->_impl) != M_INVALID_SOCKET)
		M_IMPL2_C_WRITE_FLAG(this->_impl);
	mlock.unlock();

	Handler handler = this->_handler;
	this->Clear();
	handler(transbyte, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void IocpService2::WriteOperation2<Handler>::Clear()
{
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService2::ReadOperation2<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	MutexLock& mlock = M_IMPL2_MUTEX(this->_impl);
	mlock.lock();
	if (M_IMPL2_FD(this->_impl) != M_INVALID_SOCKET)
		M_IMPL2_C_READ_FLAG(this->_impl);
	mlock.unlock();

	Handler handler = this->_handler;
	this->Clear();
	handler(transbyte, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void IocpService2::ReadOperation2<Handler>::Clear()
{
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

template<typename T>
M_SOCKET_DECL void IocpService2::OperationAlloc<T>::AllocOp(Operation& op, s_int32_t type) {
	if (!op._oper) {
		op._oper = new T;
	}
	g_bzero(&op, sizeof(wsaoverlapped_t));
	op._type = type;
}

#endif
#endif
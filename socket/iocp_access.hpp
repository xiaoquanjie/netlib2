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
	M_CLR_BIT(impl._state,0)
#define M_IMPL2_S_V6(impl)\
	M_SET_BIT(impl._state,0)
#define M_IMPL2_G_V(impl)\
	M_GET_BIT(impl._state,0)

// bind bit(1)
#define M_IMPL2_S_BIND(impl)\
	M_SET_BIT(impl._state,1)
#define M_IMPL2_S_UNBIND(impl)\
	M_CLR_BIT(impl._state,1)
#define M_IMPL2_G_BIND(impl)\
	M_GET_BIT(impl._state,1)

// block mode bit(2)
#define M_IMPL2_S_BLOCK(impl)\
	M_SET_BIT(impl._state,2)
#define M_IMPL2_G_BLOCK(impl)\
	M_GET_BIT(impl._state,2)
#define M_IMPL2_S_NONBLOCK(impl)\
	M_CLR_BIT(impl._state,2)
#define M_IMPL2_G_NONBLOCK(impl)\
	(!M_GET_BIT(impl._state,2))

// type bit(3~4)
#define M_IMPL2_S_TYPE(impl,type)\
	M_SET_MBIT(impl._state,type,3,4)
#define M_IMPL2_G_TYPE(impl)\
	M_GET_MBIT(impl._state,3,4)

// accept flag bit(5)
#define M_IMPL2_G_ACCEPT_FLAG(impl)\
	M_GET_BIT(impl._state,5)
#define M_IMPL2_S_ACCEPT_FLAG(impl)\
	M_SET_BIT(impl._state,5)
#define M_IMPL2_C_ACCEPT_FLAG(impl)\
	M_CLR_BIT(impl._state,5)

// connect flag bit(6)
#define M_IMPL2_G_CONNECT_FLAG(impl)\
	M_GET_BIT(impl._state,6)
#define M_IMPL2_S_CONNECT_FLAG(impl)\
	M_SET_BIT(impl._state,6)
#define M_IMPL2_C_CONNECT_FLAG(impl)\
	M_CLR_BIT(impl._state,6)

// write flag bit(7)
#define M_IMPL2_G_WRITE_FLAG(impl)\
	M_GET_BIT(impl._state,7)
#define M_IMPL2_S_WRITE_FLAG(impl)\
	M_SET_BIT(impl._state,7)
#define M_IMPL2_C_WRITE_FLAG(impl)\
	M_CLR_BIT(impl._state,7)

// read flag bit(8)
#define M_IMPL2_G_READ_FLAG(impl)\
	M_GET_BIT(impl._state,8)
#define M_IMPL2_S_READ_FLAG(impl)\
	M_SET_BIT(impl._state,8)
#define M_IMPL2_C_READ_FLAG(impl)\
	M_CLR_BIT(impl._state,8)

M_SOCKET_DECL void IocpService2::Access::Construct(IocpService2& service, Impl& impl, s_uint16_t type)
{
	impl._iocp = 0;
	impl._fd = M_INVALID_SOCKET;
	impl._state = 0;
	M_IMPL2_S_V4(impl);
	M_IMPL2_S_UNBIND(impl);
	M_IMPL2_S_BLOCK(impl);
	M_IMPL2_S_TYPE(impl, (s_uint8_t)type);
	impl._operation._accept_op = impl._operation._connect_op = impl._operation._read_op =
		impl._operation._write_op = 0;
}

M_SOCKET_DECL void IocpService2::Access::Destroy(IocpService2& service, Impl& impl)
{
	impl._fd = M_INVALID_SOCKET;
	impl._state = 0;
	M_IMPL2_S_V4(impl);
	M_IMPL2_S_UNBIND(impl);
	M_IMPL2_S_BLOCK(impl);
	M_IMPL2_S_TYPE(impl, E_NULL_SOCKET_TYPE);

	delete impl._operation._accept_op;
	delete impl._operation._connect_op;
	delete impl._operation._read_op;
	delete impl._operation._write_op;
	impl._operation._accept_op = impl._operation._connect_op = impl._operation._read_op =
		impl._operation._write_op = 0;
}

M_SOCKET_DECL void IocpService2::Access::Close(IocpService2& service, Impl& impl, SocketError& error)
{
	if (impl._fd != M_INVALID_SOCKET)
	{
		IocpService2::IoServiceImpl* serviceimpl = GetIoServiceImpl(service, impl);
		if (serviceimpl)
			::InterlockedDecrement(&serviceimpl->_fdcnt);

		if (impl._fd != M_INVALID_SOCKET)
		{
			if (g_closesocket(impl._fd) == M_SOCKET_ERROR)
				M_DEFAULT_SOCKET_ERROR2(error);
			impl._fd = M_INVALID_SOCKET;
		}
	}
}

M_SOCKET_DECL bool IocpService2::Access::IsOpen(IocpService2& service, Impl& impl, SocketError& error)
{
	return (impl._fd != M_INVALID_SOCKET);
}

template<typename GettableOptionType>
M_SOCKET_DECL void IocpService2::Access::GetOption(IocpService2& service, Impl& impl, GettableOptionType& opt, SocketError& error)
{
	socklen_t len = opt.Size();
	s_int32_t ret = g_getsockopt(impl._fd, opt.Level(), opt.Name(), (char*)opt.Data(), &len);
	M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
}

template<typename SettableOptionType>
M_SOCKET_DECL void IocpService2::Access::SetOption(IocpService2& service, Impl& impl, const SettableOptionType& opt, SocketError& error)
{
	s_int32_t ret = g_setsockopt(impl._fd, opt.Level(), opt.Name(), (char*)opt.Data(), opt.Size());
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint IocpService2::Access::RemoteEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetRemoteEndPoint(impl._fd, ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint IocpService2::Access::LocalEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetLocalEndPoint(impl._fd, ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

M_SOCKET_DECL void IocpService2::Access::Shutdown(IocpService2& service, Impl& impl, EShutdownType what, SocketError& error)
{
	if (0 != g_shutdown(impl._fd, what))
		M_DEFAULT_SOCKET_ERROR2(error);
}

template<typename ProtocolType>
M_SOCKET_DECL void IocpService2::Access::Open(IocpService2& service, Impl& impl, ProtocolType pt, SocketError& error)
{
	if (impl._fd != M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_SOCKET_OPEN);
		return;
	}
	if (pt.Family() == M_AF_INET) {
		M_IMPL2_S_V4(impl);
	}
	else {
		M_IMPL2_S_V6(impl);
	}

	impl._fd = g_socket(pt.Family(), pt.Type(), pt.Protocol());
	if (impl._fd == M_INVALID_SOCKET)
	{
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}
}

template<typename EndPoint>
M_SOCKET_DECL void IocpService2::Access::Bind(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_bind(impl._fd, ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void IocpService2::Access::Cancel(IocpService2& service, Impl& impl, SocketError& error)
{
	if (M_IMPL2_G_BIND(impl))
	{
		if (!g_cancelio((HANDLE)impl._fd))
			M_DEFAULT_SOCKET_ERROR2(error);
	}
}

M_SOCKET_DECL void IocpService2::Access::Listen(IocpService2& service, Impl& impl, s_int32_t flag, SocketError& error)
{
	s_int32_t ret = g_listen(impl._fd, flag);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void IocpService2::Access::Accept(IocpService2& service, Impl& impl, Impl& peer, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL2_G_ACCEPT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_ACCEPT);
		return;
	}
	if (!M_IMPL2_G_BLOCK(impl))
	{
		if (!detail::Util::SetBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL2_S_BLOCK(impl);
	}

	for (;;)
	{
		M_IMPL2_S_ACCEPT_FLAG(impl);
		peer._fd = g_accept(impl._fd, 0, 0);
		M_IMPL2_C_ACCEPT_FLAG(impl);

		if (peer._fd != M_INVALID_SOCKET)
			break;
		if (M_ERR_LAST == M_ECONNRESET)
			continue;

		M_DEFAULT_SOCKET_ERROR2(error);
		break;
	}
}

template<typename AcceptHandler>
M_SOCKET_DECL void IocpService2::Access::AsyncAccpet(IocpService2& service, M_HANDLER_SOCKET_PTR(AcceptHandler) accept_ptr, AcceptHandler handler, SocketError& error)
{
	IocpService2::Impl& impl = accept_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL2_G_ACCEPT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_ACCEPT);
		return;
	}
	if (!M_IMPL2_G_NONBLOCK(impl))
	{
		if (!detail::Util::SetNonBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL2_S_NONBLOCK(impl)
	}

	if (!M_IMPL2_G_BIND(impl))
	{
		BindIocp(service, impl, error);
		if (error)
			return;
		M_IMPL2_S_BIND(impl);
	}

	IocpService2::Impl client_impl;
	Construct(service, client_impl, E_SOCKET_TYPE);
	Open(service, client_impl, M_IMPL2_G_V(impl) ? Tcp::V6() : Tcp::V4(), error);
	if (error)
	{
		return;
	}

	typedef IocpService2::AcceptOperation<AcceptHandler> OperationType;
	IocpService2::OperationAlloc<OperationType>::Alloc(&impl._operation, E_ACCEPT_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._operation._accept_op->_oper);
	g_bzero(op->_buf, sizeof(op->_buf));
	op->_socket_ptr = accept_ptr;
	op->_handler = handler;
	op->_bytes = 0;
	op->_impl = client_impl;

	for (;;)
	{
		M_IMPL2_S_ACCEPT_FLAG(impl);
		// If no error occurs, the AcceptEx function completed successfully and a value of TRUE is returned.
		s_int32_t ret = gAcceptEx(impl._fd, op->_impl._fd, op->_buf,
			0, sizeof(sockaddr_storage_t), sizeof(sockaddr_storage_t), (LPDWORD)&op->_bytes, impl._operation._accept_op);

		if (!ret && M_ERR_LAST == M_ECONNRESET)
		{
			M_DEBUG_PRINT("peer connect reset");
			continue;
		}
		if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING)
		{
			M_DEBUG_PRINT("acceptex fail");
			M_DEFAULT_SOCKET_ERROR2(error);
			M_IMPL2_C_ACCEPT_FLAG(impl);

			op->_socket_ptr.reset();
			SocketError error2;
			Close(service, op->_impl, error2);
			Destroy(service, op->_impl);
		}
		break;
	}
}

M_SOCKET_DECL s_int32_t IocpService2::Access::RecvSome(IocpService2& service, Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return M_SOCKET_ERROR;
	}
	if (M_IMPL2_G_READ_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_READ);
		return M_SOCKET_ERROR;
	}
	if (!M_IMPL2_G_BLOCK(impl))
	{
		if (!detail::Util::SetBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return M_SOCKET_ERROR;
		}
		M_IMPL2_S_BLOCK(impl);
	}

	M_IMPL2_S_READ_FLAG(impl);
	DWORD trans_bytes = 0;
	DWORD data_size = static_cast<DWORD>(size);
	wsabuf_t wsabuf;
	wsabuf.buf = data;
	wsabuf.len = data_size;
	DWORD flag = 0;
	s_int32_t ret = g_wsarecv(impl._fd, &wsabuf, 1, &trans_bytes, &flag, 0, 0);
	M_IMPL2_C_READ_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
	return trans_bytes;
}

M_SOCKET_DECL s_int32_t IocpService2::Access::SendSome(IocpService2& service, Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return M_SOCKET_ERROR;
	}
	if (M_IMPL2_G_WRITE_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_WRITE);
		return M_SOCKET_ERROR;
	}
	if (!M_IMPL2_G_BLOCK(impl))
	{
		if (!detail::Util::SetBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return M_SOCKET_ERROR;
		}
		M_IMPL2_S_BLOCK(impl);
	}

	M_IMPL2_S_WRITE_FLAG(impl);
	DWORD trans_bytes = 0;
	DWORD data_size = static_cast<DWORD>(size);
	wsabuf_t wsabuf;
	wsabuf.buf = const_cast<s_byte_t*>(data);
	wsabuf.len = data_size;
	s_int32_t ret = g_wsasend(impl._fd, &wsabuf, 1, &trans_bytes, 0, 0, 0);
	M_IMPL2_C_WRITE_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
	return trans_bytes;
}

template<typename ReadHandler>
M_SOCKET_DECL void IocpService2::Access::AsyncRecvSome(IocpService2& service, M_HANDLER_SOCKET_PTR(ReadHandler) socket_ptr, s_byte_t* data, s_uint32_t size, ReadHandler hander, SocketError& error)
{
	IocpService2::Impl& impl = socket_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL2_G_READ_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_READ);
		return;
	}
	if (!M_IMPL2_G_NONBLOCK(impl))
	{
		if (!detail::Util::SetNonBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL2_S_NONBLOCK(impl)
	}
	if (!M_IMPL2_G_BIND(impl))
	{
		BindIocp(service, impl, error);
		if (error)
			return;
		M_IMPL2_S_BIND(impl);
	}

	typedef IocpService2::ReadOperation<ReadHandler> OperationType;
	IocpService2::OperationAlloc<OperationType>::Alloc(&impl._operation, E_READ_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._operation._read_op->_oper);
	op->_handler = hander;
	op->_socket_ptr = socket_ptr;
	op->_wsabuf.buf = data;
	op->_wsabuf.len = size;

	DWORD flag = 0;
	M_IMPL2_S_READ_FLAG(impl);
	s_int32_t ret = g_wsarecv(impl._fd, &op->_wsabuf, 1, 0, &flag, impl._operation._read_op, 0);
	if (ret == M_SOCKET_ERROR  && M_ERR_LAST != M_ERROR_IO_PENDING)
	{
		op->_socket_ptr.reset();
		M_IMPL2_C_READ_FLAG(impl);
		M_DEFAULT_SOCKET_ERROR2(error);
	}
}

template<typename WriteHandler>
M_SOCKET_DECL void IocpService2::Access::AsyncSendSome(IocpService2& service, M_HANDLER_SOCKET_PTR(WriteHandler) socket_ptr, const s_byte_t* data, s_uint32_t size, WriteHandler hander, SocketError& error)
{
	IocpService2::Impl& impl = socket_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL2_G_WRITE_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_WRITE);
		return;
	}
	if (!M_IMPL2_G_NONBLOCK(impl))
	{
		if (!detail::Util::SetNonBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL2_S_NONBLOCK(impl)
	}

	if (!M_IMPL2_G_BIND(impl))
	{
		BindIocp(service, impl, error);
		if (error)
			return;
		M_IMPL2_S_BIND(impl);
	}

	typedef IocpService2::WriteOperation<WriteHandler> OperationType;
	IocpService2::OperationAlloc<OperationType>::Alloc(&impl._operation, E_WRITE_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._operation._write_op->_oper);
	op->_handler = hander;
	op->_socket_ptr = socket_ptr;
	op->_wsabuf.buf = const_cast<s_byte_t*>(data);
	op->_wsabuf.len = size;

	DWORD send_bytes = 0;
	M_IMPL2_S_WRITE_FLAG(impl);
	// If no error occurs and the send operation has completed immediately, WSASend returns zero
	s_int32_t ret = g_wsasend(impl._fd, &op->_wsabuf, 1, 0, 0, impl._operation._write_op, 0);
	if (ret == M_SOCKET_ERROR && M_ERR_LAST != M_ERROR_IO_PENDING)
	{
		op->_socket_ptr.reset();
		M_IMPL2_C_WRITE_FLAG(impl);
		M_DEFAULT_SOCKET_ERROR2(error);
	}
}

template<typename EndPoint>
M_SOCKET_DECL void IocpService2::Access::Connect(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL2_G_CONNECT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_CONNECT);
		return;
	}
	if (!ep.Valid())
	{
		error = SocketError(M_ERR_ENDPOINT_INVALID);
		return;
	}
	if (!M_IMPL2_G_BLOCK(impl))
	{
		if (!detail::Util::SetBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL2_S_BLOCK(impl);
	}

	M_IMPL2_S_CONNECT_FLAG(impl);
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_connect(impl._fd, ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_IMPL2_C_CONNECT_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename ConnectHandler, typename EndPoint>
M_SOCKET_DECL void IocpService2::Access::AsyncConnect(IocpService2& service, M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error)
{
	IocpService2::Impl& impl = connect_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL2_G_CONNECT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_CONNECT);
		return;
	}
	if (!M_IMPL2_G_NONBLOCK(impl))
	{
		if (!detail::Util::SetNonBlock(impl._fd))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL2_S_NONBLOCK(impl)
	}
	if (!M_IMPL2_G_BIND(impl))
	{
		BindIocp(service, impl, error);
		if (error)
			return;
		M_IMPL2_S_BIND(impl);
	}

	sockaddr_storage_t addr;
	g_bzero(&addr, sizeof(addr));
	addr.ss_family = ep.Protocol().Family();
	g_bind(impl._fd, (sockaddr_t*)&addr, sizeof(addr));

	typedef IocpService2::ConnectOperation<ConnectHandler> OperationType;
	IocpService2::OperationAlloc<OperationType>::Alloc(&impl._operation, E_CONNECT_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._operation._connect_op->_oper);
	op->_handler = handler;
	op->_socket_ptr = connect_ptr;

	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	DWORD send_bytes = 0;

	M_IMPL2_S_CONNECT_FLAG(impl);
	// On success, the ConnectEx function returns TRUE. On failure, the function returns FALSE
	s_int32_t ret = gConnectEx(impl._fd, ep_access::SockAddr(ep), ep_access::SockAddrLen(ep), 0, 0, &send_bytes, impl._operation._connect_op);
	if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING)
	{
		op->_socket_ptr.reset();
		M_IMPL2_C_CONNECT_FLAG(impl);
		M_DEFAULT_SOCKET_ERROR2(error);
	}
}

M_SOCKET_DECL void IocpService2::Access::CreateIocp(IocpService2::IoServiceImpl& impl, SocketError& error)
{
	if (impl._handler != 0)
	{
		error = SocketError(M_ERR_IOCP_EXIST);
		return;
	}

	impl._handler = g_createiocompletionport(INVALID_HANDLE_VALUE, 0, 0, 0);
	M_DEFAULT_SOCKET_ERROR(impl._handler == 0, error);
}

M_SOCKET_DECL void IocpService2::Access::DestroyIocp(IocpService2::IoServiceImpl& impl)
{
	if (impl._handler != 0)
	{
		g_closehandle(impl._handler);
		impl._handler = 0;
		impl._fdcnt = 0;
	}
}

M_SOCKET_DECL void IocpService2::Access::BindIocp(IocpService2& service, Impl& impl, SocketError& error)
{
	if (impl._iocp == 0)
	{
		service._mutex.lock();
		if (service._implvector.empty())
		{
			service._mutex.unlock();
			error = SocketError(M_ERR_NOT_SERVICE);
			return;
		}

		service._implidx++;
		s_int32_t size = (s_int32_t)service._implvector.size();
		IocpService2::IoServiceImpl& serviceimpl = *(service._implvector[service._implidx%size]);
		service._mutex.unlock();

		::InterlockedIncrement(&serviceimpl._fdcnt);
		impl._iocp = serviceimpl._handler;
	}
	if (impl._iocp == 0)
	{
		error = SocketError(M_ERR_NOT_SERVICE);
		return;
	}
	HANDLE ret = g_createiocompletionport((HANDLE)impl._fd, impl._iocp, 0, 0);
	M_DEFAULT_SOCKET_ERROR(impl._iocp != ret, error);
}

M_SOCKET_DECL void IocpService2::Access::ExecOp(IocpService2& service, IocpService2::Operation* op, s_uint32_t transbyte, bool ok)
{
	SocketError error;
	if (!ok)
	{
		// op->Internal is not error code
		M_DEFAULT_SOCKET_ERROR2(error);
	}
	op->_oper->Complete(service, transbyte, error);
}

M_SOCKET_DECL void IocpService2::Access::Run(IocpService2& service, SocketError& error)
{
	IocpService2::IoServiceImpl* impl = new IocpService2::IoServiceImpl(service);
	CreateIocp(*impl, error);
	if (error)
	{
		delete impl;
		return;
	}
	MutexLock& mutex = service._mutex;
	mutex.lock();
	service._implvector.push_back(impl);
	service._implmap[impl->_handler] = impl;
	++service._implcnt;
	mutex.unlock();

	for (;;)
	{
		DWORD trans_bytes = 0;
		ULONG_PTR comple_key = 0;
		overlapped_t* overlapped = 0;
		g_setlasterr(0);

		BOOL ret = g_getqueuedcompletionstatus(impl->_handler, &trans_bytes, &comple_key, &overlapped, -1);
		if (overlapped)
		{
			IocpService2::Operation* op = (IocpService2::Operation*)overlapped;
			if (op->_type & E_FINISH_OP)
			{
				delete op;
				break;
			}
			ExecOp(service, op, trans_bytes, ret ? true : false);
			continue;
		}
		if (!ret)
		{
			if (M_ERR_LAST == WAIT_TIMEOUT)
				1;// time out

			M_DEFAULT_SOCKET_ERROR2(error);
			break;
		}
	}

	mutex.lock();
	service._implvector.erase(std::find(service._implvector.begin(), service._implvector.end(), impl));
	service._implmap.erase(impl->_handler);
	--service._implcnt;
	mutex.unlock();
	DestroyIocp(*impl);
	delete impl;
}

M_SOCKET_DECL void IocpService2::Access::Stop(IocpService2& service, SocketError& error)
{
	for (IocpService2::IoServiceImplVector::iterator iter = service._implvector.begin();
		iter != service._implvector.end(); ++iter)
	{
		Operation* op = new Operation;
		op->_type = E_FINISH_OP;
		if (!g_postqueuedcompletionstatus((*iter)->_handler, 0, 0, op))
		{
			delete op;
			M_DEFAULT_SOCKET_ERROR2(error);
		}
	}
}

M_SOCKET_DECL bool IocpService2::Access::Stopped(const IocpService2& service)
{
	ScopedLock scoped_lock(service._mutex);
	return (service._implvector.empty() ? true : false);
}

M_SOCKET_DECL IocpService2::IoServiceImpl* IocpService2::Access::GetIoServiceImpl(IocpService2& service, Impl& impl)
{
	if (impl._iocp == 0)
		return 0;

	IocpService2::IoServiceImplMap::iterator iter = service._implmap.find(impl._iocp);
	if (iter == service._implmap.end())
		return 0;
	return iter->second;
}

M_SOCKET_DECL s_uint32_t IocpService2::Access::GetServiceCount(const IocpService2& service)
{
	ScopedLock scoped(service._mutex);
	return (s_uint32_t)service._implvector.size();
}

M_SOCKET_DECL IocpService2::Operation::Operation() :_oper(0) {}

M_SOCKET_DECL IocpService2::Operation::~Operation()
{
	delete _oper;
	_oper = 0;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService2::AcceptOperation<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	if (error)
	{
		SocketError error2;
		Access::Close(service, _impl, error2);
	}

	M_IMPL2_C_ACCEPT_FLAG(this->_socket_ptr->GetImpl());
	shard_ptr_t<TcpSocket<IocpService2> > SocketPtr(new TcpSocket<IocpService2>(service));
	IocpService2::Impl& impl = SocketPtr->GetImpl();
	impl = _impl;
	Handler handler = this->_handler;
	M_HANDLER_SOCKET_PTR(Handler) acceptor_ptr = this->_socket_ptr;
	this->_socket_ptr.reset();
	handler(acceptor_ptr, SocketPtr, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService2::ConnectOperation<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	M_IMPL2_C_CONNECT_FLAG(this->_socket_ptr->GetImpl());
	M_HANDLER_SOCKET_PTR(Handler) connect_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_socket_ptr.reset();
	if (!error)
	{
		IocpService2::Impl& impl = connect_ptr->GetImpl();
		g_setsockopt(impl._fd, M_SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
	}
	handler(connect_ptr, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService2::WriteOperation<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	M_IMPL2_C_WRITE_FLAG(this->_socket_ptr->GetImpl());
	M_HANDLER_SOCKET_PTR(Handler) write_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_socket_ptr.reset();
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	handler(write_ptr, data, size, transbyte, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService2::ReadOperation<Handler>::Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error)
{
	M_IMPL2_C_READ_FLAG(this->_socket_ptr->GetImpl());
	M_HANDLER_SOCKET_PTR(Handler) read_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_socket_ptr.reset();
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	handler(read_ptr, data, size, transbyte, error);
	return true;
}

template<typename T>
M_SOCKET_DECL void IocpService2::OperationAlloc<T>::Alloc(OperationSet* opset, s_int32_t type)
{
	if (type == E_ACCEPT_OP)
	{
		if (!opset->_accept_op)
		{
			opset->_accept_op = new Operation;
			g_bzero(opset->_accept_op, sizeof(Operation));
			opset->_accept_op->_oper = new T;
		}
		else
			g_bzero(opset->_accept_op, sizeof(wsaoverlapped_t));
		opset->_accept_op->_type = type;
	}
	else if (type == E_CONNECT_OP)
	{
		if (!opset->_connect_op)
		{
			opset->_connect_op = new Operation;
			g_bzero(opset->_connect_op, sizeof(Operation));
			opset->_connect_op->_oper = new T;
		}
		else
			g_bzero(opset->_connect_op, sizeof(wsaoverlapped_t));
		opset->_connect_op->_type = type;
	}
	else if (type == E_READ_OP)
	{
		if (!opset->_read_op)
		{
			opset->_read_op = new Operation;
			g_bzero(opset->_read_op, sizeof(Operation));
			opset->_read_op->_oper = new T;
		}
		else
			g_bzero(opset->_read_op, sizeof(wsaoverlapped_t));
		opset->_read_op->_type = type;
	}
	else if (type == E_WRITE_OP)
	{
		if (!opset->_write_op)
		{
			opset->_write_op = new Operation;
			g_bzero(opset->_write_op, sizeof(Operation));
			opset->_write_op->_oper = new T;
		}
		else
			g_bzero(opset->_write_op, sizeof(wsaoverlapped_t));
		opset->_write_op->_type = type;
	}
	else
	{
		assert(0);
	}
}

#endif
#endif
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

#ifndef M_WIN_ACCESS_INCLUDE
#define M_WIN_ACCESS_INCLUDE

#ifdef M_PLATFORM_WIN

// ipv4 or ipv6 bit(0)
#define M_IMPL_S_V4(impl)\
	M_CLR_BIT(impl._state,0)
#define M_IMPL_S_V6(impl)\
	M_SET_BIT(impl._state,0)
#define M_IMPL_G_V(impl)\
	M_GET_BIT(impl._state,0)

// bind bit(1)
#define M_IMPL_S_BIND(impl)\
	M_SET_BIT(impl._state,1)
#define M_IMPL_S_UNBIND(impl)\
	M_CLR_BIT(impl._state,1)
#define M_IMPL_G_BIND(impl)\
	M_GET_BIT(impl._state,1)

// block mode bit(2)
#define M_IMPL_S_BLOCK(impl)\
	M_SET_BIT(impl._state,2)
#define M_IMPL_G_BLOCK(impl)\
	M_GET_BIT(impl._state,2)
#define M_IMPL_S_NONBLOCK(impl)\
	M_CLR_BIT(impl._state,2)
#define M_IMPL_G_NONBLOCK(impl)\
	(!M_GET_BIT(impl._state,2))

// type bit(3~4)
#define M_IMPL_S_TYPE(impl,type)\
	M_SET_MBIT(impl._state,type,3,4)
#define M_IMPL_G_TYPE(impl)\
	M_GET_MBIT(impl._state,3,4)

// accept flag
#define M_IMPL_G_ACCEPT_FLAG(impl)\
	(impl._accept_flag)
#define M_IMPL_S_ACCEPT_FLAG(impl)\
	impl._accept_flag = 1
#define M_IMPL_C_ACCEPT_FLAG(impl)\
	impl._accept_flag = 0

// connect flag
#define M_IMPL_G_CONNECT_FLAG(impl)\
	(impl._connect_flag)
#define M_IMPL_S_CONNECT_FLAG(impl)\
	impl._connect_flag = 1
#define M_IMPL_C_CONNECT_FLAG(impl)\
	impl._connect_flag = 0

// write flag
#define M_IMPL_G_WRITE_FLAG(impl)\
	(impl._write_flag)
#define M_IMPL_S_WRITE_FLAG(impl)\
	impl._write_flag = 1
#define M_IMPL_C_WRITE_FLAG(impl)\
	impl._write_flag = 0

// read flag
#define M_IMPL_G_READ_FLAG(impl)\
	(impl._read_flag)
#define M_IMPL_S_READ_FLAG(impl)\
	impl._read_flag = 1
#define M_IMPL_C_READ_FLAG(impl)\
	impl._read_flag = 0

M_SOCKET_DECL void IocpService::Access::Construct(IocpService& service, Impl& impl, s_uint16_t type)
{
	impl._fd = M_INVALID_SOCKET;
	impl._state = 0;
	impl._accept_op = impl._connect_op = impl._read_op = impl._write_op = 0;
	impl._accept_flag = impl._connect_flag = impl._write_flag = impl._read_flag = 0;
	M_IMPL_S_V4(impl);
	M_IMPL_S_UNBIND(impl);
	M_IMPL_S_BLOCK(impl);
	M_IMPL_S_TYPE(impl, (s_uint8_t)type);
}

M_SOCKET_DECL void IocpService::Access::Destroy(IocpService& service, Impl& impl)
{
	impl._fd = M_INVALID_SOCKET;
	impl._state = 0;
	M_IMPL_S_V4(impl);
	M_IMPL_S_UNBIND(impl);
	M_IMPL_S_BLOCK(impl);
	M_IMPL_S_TYPE(impl, E_NULL_SOCKET_TYPE);

	delete impl._accept_op;
	delete impl._connect_op;
	delete impl._write_op;
	delete impl._read_op;
	impl._accept_op = impl._connect_op = impl._read_op = impl._write_op = 0;
	impl._accept_flag = impl._connect_flag = impl._write_flag = impl._read_flag = 0;
}

M_SOCKET_DECL void IocpService::Access::Close(IocpService& service, Impl& impl, SocketError& error)
{
	if (impl._fd != M_INVALID_SOCKET)
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (impl._fd != M_INVALID_SOCKET)
		{
			if (g_closesocket(impl._fd) == M_SOCKET_ERROR)
				M_DEFAULT_SOCKET_ERROR2(error);
			impl._fd = M_INVALID_SOCKET;
		}
	}
}

M_SOCKET_DECL bool IocpService::Access::IsOpen(IocpService& service, Impl& impl, SocketError& error)
{
	return (impl._fd != M_INVALID_SOCKET);
}

template<typename GettableOptionType>
M_SOCKET_DECL void IocpService::Access::GetOption(IocpService& service, Impl& impl, GettableOptionType& opt, SocketError& error)
{
	socklen_t len = opt.Size();
	s_int32_t ret = g_getsockopt(impl._fd, opt.Level(), opt.Name(), (char*)opt.Data(), &len);
	M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
}

template<typename SettableOptionType>
M_SOCKET_DECL void IocpService::Access::SetOption(IocpService& service, Impl& impl, const SettableOptionType& opt, SocketError& error)
{
	s_int32_t ret = g_setsockopt(impl._fd, opt.Level(), opt.Name(), (char*)opt.Data(), opt.Size());
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint IocpService::Access::RemoteEndPoint(EndPoint, IocpService& service, const Impl& impl, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetRemoteEndPoint(impl._fd, ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint IocpService::Access::LocalEndPoint(EndPoint, IocpService& service, const Impl& impl, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetLocalEndPoint(impl._fd, ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

M_SOCKET_DECL void IocpService::Access::Shutdown(IocpService& service, Impl& impl, EShutdownType what, SocketError& error)
{
	if (0 != g_shutdown(impl._fd, what))
		M_DEFAULT_SOCKET_ERROR2(error);
}

template<typename ProtocolType>
M_SOCKET_DECL void IocpService::Access::Open(IocpService& service, Impl& impl, ProtocolType pt, SocketError& error)
{
	if (impl._fd != M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_SOCKET_OPEN);
		return;
	}
	if (pt.Family() == M_AF_INET) {
		M_IMPL_S_V4(impl);
	}
	else {
		M_IMPL_S_V6(impl);
	}

	impl._fd = g_socket(pt.Family(), pt.Type(), pt.Protocol());
	if (impl._fd == M_INVALID_SOCKET)
	{
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}
}

template<typename EndPoint>
M_SOCKET_DECL void IocpService::Access::Bind(IocpService& service, Impl& impl, const EndPoint& ep, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_bind(impl._fd, ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void IocpService::Access::Cancel(IocpService& service, Impl& impl, SocketError& error)
{
	if (M_IMPL_G_BIND(impl))
	{
		if (!g_cancelio((HANDLE)impl._fd))
			M_DEFAULT_SOCKET_ERROR2(error);
	}
}

M_SOCKET_DECL void IocpService::Access::Listen(IocpService& service, Impl& impl, s_int32_t flag, SocketError& error)
{
	s_int32_t ret = g_listen(impl._fd, flag);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void IocpService::Access::Accept(IocpService& service, Impl& impl, Impl& peer, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_ACCEPT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_ACCEPT);
		return;
	}
	if (!M_IMPL_G_BLOCK(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BLOCK(impl))
		{
			if (!detail::Util::SetBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return;
			}
			M_IMPL_S_BLOCK(impl);
		}
	}

	for (;;)
	{
		M_IMPL_S_ACCEPT_FLAG(impl);
		peer._fd = g_accept(impl._fd, 0, 0);
		M_IMPL_C_ACCEPT_FLAG(impl);

		if (peer._fd != M_INVALID_SOCKET)
			break;
		if (M_ERR_LAST == M_ECONNRESET)
			continue;

		M_DEFAULT_SOCKET_ERROR2(error);
		break;
	}
}

template<typename AcceptHandler>
M_SOCKET_DECL void IocpService::Access::AsyncAccept(IocpService& service, M_HANDLER_SOCKET_PTR(AcceptHandler) accept_ptr, AcceptHandler handler, SocketError& error)
{
	IocpService::Impl& impl = accept_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_ACCEPT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_ACCEPT);
		return;
	}
	if (!M_IMPL_G_NONBLOCK(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_NONBLOCK(impl))
		{
			if (!detail::Util::SetNonBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return;
			}
			M_IMPL_S_NONBLOCK(impl)
		}
	}
	
	if (!M_IMPL_G_BIND(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BIND(impl))
		{
			BindIocp(service, impl, error);
			if (error)
				return;
			M_IMPL_S_BIND(impl);
		}
	}

	IocpService::Impl client_impl;
	Construct(service, client_impl, E_SOCKET_TYPE);
	Open(service, client_impl, M_IMPL_G_V(impl) ? Tcp::V6() : Tcp::V4(), error);
	if (error)
	{
		return;
	}

	typedef IocpService::AcceptOperation<AcceptHandler> OperationType;
	impl._accept_op = IocpService::OperationAlloc<OperationType>::Alloc(impl._accept_op, E_ACCEPT_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._accept_op->_oper);
	g_bzero(op->_buf, sizeof(op->_buf));
	op->_socket_ptr = accept_ptr;
	op->_handler = handler;
	op->_bytes = 0;
	op->_impl = client_impl;

	for (;;)
	{
		M_IMPL_S_ACCEPT_FLAG(impl);
		// If no error occurs, the AcceptEx function completed successfully and a value of TRUE is returned.
		s_int32_t ret = gAcceptEx(impl._fd, op->_impl._fd, op->_buf, 
			0,sizeof(sockaddr_storage_t), sizeof(sockaddr_storage_t), (LPDWORD)&op->_bytes, impl._accept_op);
	
		if (!ret && M_ERR_LAST == M_ECONNRESET)
		{
			M_DEBUG_PRINT("peer connect reset");
			continue;
		}
		if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING)
		{
			M_DEBUG_PRINT("acceptex fail");
			M_DEFAULT_SOCKET_ERROR2(error);
			M_IMPL_C_ACCEPT_FLAG(impl);

			op->_socket_ptr.reset();
			SocketError error2;
			Close(service, op->_impl, error2);
			Destroy(service, op->_impl);
		}
		break;
	}
}

M_SOCKET_DECL s_int32_t IocpService::Access::RecvSome(IocpService& service, Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return M_SOCKET_ERROR;
	}
	if (M_IMPL_G_READ_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_READ);
		return M_SOCKET_ERROR;
	}
	if (!M_IMPL_G_BLOCK(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BLOCK(impl))
		{
			if (!detail::Util::SetBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return M_SOCKET_ERROR;
			}
			M_IMPL_S_BLOCK(impl);
		}
	}

	M_IMPL_S_READ_FLAG(impl);
	DWORD trans_bytes = 0;
	DWORD data_size = static_cast<DWORD>(size);
	wsabuf_t wsabuf;
	wsabuf.buf = data;
	wsabuf.len = data_size;
	DWORD flag = 0;
	s_int32_t ret = g_wsarecv(impl._fd, &wsabuf, 1, &trans_bytes, &flag, 0, 0);
	M_IMPL_C_READ_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
	return trans_bytes;
}

M_SOCKET_DECL s_int32_t IocpService::Access::SendSome(IocpService& service, Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return M_SOCKET_ERROR;
	}
	if (M_IMPL_G_WRITE_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_WRITE);
		return M_SOCKET_ERROR;
	}
	if (!M_IMPL_G_BLOCK(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BLOCK(impl))
		{
			if (!detail::Util::SetBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return M_SOCKET_ERROR;
			}
			M_IMPL_S_BLOCK(impl);
		}
	}

	M_IMPL_S_WRITE_FLAG(impl);
	DWORD trans_bytes = 0;
	DWORD data_size = static_cast<DWORD>(size);
	wsabuf_t wsabuf;
	wsabuf.buf = const_cast<s_byte_t*>(data);
	wsabuf.len = data_size;
	s_int32_t ret = g_wsasend(impl._fd, &wsabuf, 1, &trans_bytes, 0, 0, 0);
	M_IMPL_C_WRITE_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
	return trans_bytes;
}

template<typename ReadHandler>
M_SOCKET_DECL void IocpService::Access::AsyncRecvSome(IocpService& service, M_HANDLER_SOCKET_PTR(ReadHandler) socket_ptr, s_byte_t* data, s_uint32_t size, ReadHandler hander, SocketError& error)
{
	IocpService::Impl& impl = socket_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_READ_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_READ);
		return;
	}
	if (!M_IMPL_G_NONBLOCK(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_NONBLOCK(impl))
		{
			if (!detail::Util::SetNonBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return;
			}
			M_IMPL_S_NONBLOCK(impl)
		}
	}

	if (!M_IMPL_G_BIND(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BIND(impl))
		{
			BindIocp(service, impl, error);
			if (error)
				return;
			M_IMPL_S_BIND(impl);
		}
	}

	typedef IocpService::ReadOperation<ReadHandler> OperationType;
	impl._read_op = IocpService::OperationAlloc<OperationType>::Alloc(impl._read_op, E_READ_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._read_op->_oper);
	op->_handler = hander;
	op->_socket_ptr = socket_ptr;
	op->_wsabuf.buf = data;
	op->_wsabuf.len = size;

	DWORD flag = 0;
	M_IMPL_S_READ_FLAG(impl);
	s_int32_t ret = g_wsarecv(impl._fd, &op->_wsabuf, 1, 0, &flag, impl._read_op, 0);
	if (ret==M_SOCKET_ERROR  && M_ERR_LAST != M_ERROR_IO_PENDING)
	{
		op->_socket_ptr.reset();
		M_IMPL_C_READ_FLAG(impl);
		M_DEFAULT_SOCKET_ERROR2(error);
	}
}

template<typename WriteHandler>
M_SOCKET_DECL void IocpService::Access::AsyncSendSome(IocpService& service, M_HANDLER_SOCKET_PTR(WriteHandler) socket_ptr, const s_byte_t* data, s_uint32_t size, WriteHandler hander, SocketError& error)
{
	IocpService::Impl& impl = socket_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_WRITE_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_WRITE);
		return;
	}
	if (!M_IMPL_G_NONBLOCK(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_NONBLOCK(impl))
		{
			if (!detail::Util::SetNonBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return;
			}
			M_IMPL_S_NONBLOCK(impl)
		}
	}

	if (!M_IMPL_G_BIND(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BIND(impl))
		{
			BindIocp(service, impl, error);
			if (error)
				return;
			M_IMPL_S_BIND(impl);
		}
	}

	typedef IocpService::WriteOperation<WriteHandler> OperationType;
	impl._write_op = IocpService::OperationAlloc<OperationType>::Alloc(impl._write_op, E_WRITE_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._write_op->_oper);
	op->_handler = hander;
	op->_socket_ptr = socket_ptr;
	op->_wsabuf.buf = const_cast<s_byte_t*>(data);
	op->_wsabuf.len = size;

	DWORD send_bytes = 0;
	M_IMPL_S_WRITE_FLAG(impl);
	// If no error occurs and the send operation has completed immediately, WSASend returns zero
	s_int32_t ret = g_wsasend(impl._fd, &op->_wsabuf, 1, 0, 0, impl._write_op, 0);
	if (ret==M_SOCKET_ERROR && M_ERR_LAST != M_ERROR_IO_PENDING)
	{
		op->_socket_ptr.reset();
		M_IMPL_C_WRITE_FLAG(impl);
		M_DEFAULT_SOCKET_ERROR2(error);
	}
}

template<typename EndPoint>
M_SOCKET_DECL void IocpService::Access::Connect(IocpService& service, Impl& impl, const EndPoint& ep, SocketError& error)
{
	if (impl._fd == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_CONNECT_FLAG(impl))
	{
		error = SocketError(M_ERR_POSTED_CONNECT);
		return;
	}
	if (!ep.Valid())
	{
		error = SocketError(M_ERR_ENDPOINT_INVALID);
		return;
	}

	if (!M_IMPL_G_BLOCK(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BLOCK(impl))
		{
			if (!detail::Util::SetBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return;
			}
			M_IMPL_S_BLOCK(impl);
		}
	}

	M_IMPL_S_CONNECT_FLAG(impl);
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_connect(impl._fd, ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_IMPL_C_CONNECT_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename ConnectHandler, typename EndPoint>
M_SOCKET_DECL void IocpService::Access::AsyncConnect(IocpService& service, M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error)
{
	IocpService::Impl& impl = connect_ptr->GetImpl();
	if (impl._fd == M_INVALID_SOCKET)
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
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_NONBLOCK(impl))
		{
			if (!detail::Util::SetNonBlock(impl._fd))
			{
				M_DEFAULT_SOCKET_ERROR2(error);
				return;
			}
			M_IMPL_S_NONBLOCK(impl)
		}
	}

	if (!M_IMPL_G_BIND(impl))
	{
		M_DISPATCHER_SCOPED_LOCK(impl._fd);
		if (!M_IMPL_G_BIND(impl))
		{
			BindIocp(service, impl, error);
			if (error)
				return;
			M_IMPL_S_BIND(impl);
		}
	}

	sockaddr_storage_t addr;
	g_bzero(&addr, sizeof(addr));
	addr.ss_family = ep.Protocol().Family();
	g_bind(impl._fd, (sockaddr_t*)&addr, sizeof(addr));

	typedef IocpService::ConnectOperation<ConnectHandler> OperationType;
	impl._connect_op = IocpService::OperationAlloc<OperationType>::Alloc(impl._connect_op, E_CONNECT_OP);
	OperationType* op = dynamic_cast<OperationType*>(impl._connect_op->_oper);
	op->_handler = handler;
	op->_socket_ptr = connect_ptr;

	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	DWORD send_bytes = 0;

	M_IMPL_S_CONNECT_FLAG(impl);
	// On success, the ConnectEx function returns TRUE. On failure, the function returns FALSE
	s_int32_t ret = gConnectEx(impl._fd, ep_access::SockAddr(ep), ep_access::SockAddrLen(ep), 0, 0, &send_bytes, impl._connect_op);
	if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING)
	{
		op->_socket_ptr.reset();
		M_IMPL_C_CONNECT_FLAG(impl);
		M_DEFAULT_SOCKET_ERROR2(error);
	}
}

M_SOCKET_DECL void IocpService::Access::CreateIocp(IocpService& service, SocketError& error)
{
	if (service._impl._handler != 0)
	{
		error = SocketError(M_ERR_IOCP_EXIST);
		return;
	}
	service._impl._handler = g_createiocompletionport(INVALID_HANDLE_VALUE, 0, 0, 0);
	M_DEFAULT_SOCKET_ERROR(service._impl._handler == 0, error);
	if (!error)
	{
		service._impl._stopped = 0;
	}
}

M_SOCKET_DECL void IocpService::Access::DestroyIocp(IocpService& service)
{
	if (service._impl._handler)
	{
		g_closehandle(service._impl._handler);
		service._impl._handler = 0;
	}
}

M_SOCKET_DECL void IocpService::Access::BindIocp(IocpService& service, Impl& impl, SocketError& error)
{
	if (service._impl._handler == 0)
	{
		error = SocketError(M_ERR_IOCP_INVALID);
		return;
	}
	HANDLE ret = g_createiocompletionport((HANDLE)impl._fd, service._impl._handler, 0, 0);
	M_DEFAULT_SOCKET_ERROR(service._impl._handler != ret, error);
}

M_SOCKET_DECL void IocpService::Access::ExecOp(IocpService& service, IocpService::Operation* op, s_uint32_t transbyte, bool ok)
{
	SocketError error;
	if (!ok)
	{
		// op->Internal is not error code
		M_DEFAULT_SOCKET_ERROR2(error);
	}
	op->_oper->Complete(service, transbyte, error);
}

M_SOCKET_DECL void IocpService::Access::Run(IocpService& service, SocketError& error)
{
	for (;;)
	{
		DWORD trans_bytes = 0;
		ULONG_PTR comple_key = 0;
		overlapped_t* overlapped = 0;
		g_setlasterr(0);

		BOOL ret = g_getqueuedcompletionstatus(service._impl._handler, &trans_bytes, &comple_key, &overlapped, -1);
		if (overlapped)
		{
			IocpService::Operation* op = (IocpService::Operation*)overlapped;
			if (op->_type & E_FINISH_OP)
			{
				if (!g_postqueuedcompletionstatus(service._impl._handler, 0, 0, op))
					delete op;
				return;
			}
			ExecOp(service, op, trans_bytes, ret ? true : false);
			continue;
		}
		if (!ret)
		{
			if (M_ERR_LAST == WAIT_TIMEOUT)
				1;// time out

			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
	}
}

M_SOCKET_DECL void IocpService::Access::Stop(IocpService& service, SocketError& error)
{
	if (::InterlockedExchange(&service._impl._stopped, 1) == 0)
	{
		Operation* op = new Operation;
		op->_type = E_FINISH_OP;
		if (!g_postqueuedcompletionstatus(service._impl._handler, 0, 0, op))
		{
			delete op;
			M_DEFAULT_SOCKET_ERROR2(error);
		}
	}
}

M_SOCKET_DECL bool IocpService::Access::Stopped(const IocpService& service)
{
	ScopedLock scoped_lock(service._impl._mutex);
	return (service._impl._stopped != 0 ? true : false);
}

M_SOCKET_DECL IocpService::Operation::Operation():_oper(0){}

M_SOCKET_DECL IocpService::Operation::~Operation()
{
	delete _oper;
	_oper = 0;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService::AcceptOperation<Handler>::Complete(IocpService& service, s_uint32_t transbyte, SocketError& error)
{
	if (error)
	{
		SocketError error2;
		Access::Close(service, _impl, error2);
	}

	M_IMPL_C_ACCEPT_FLAG(this->_socket_ptr->GetImpl());
	shard_ptr_t<TcpSocket<IocpService> > SocketPtr(new TcpSocket<IocpService>(service));
	IocpService::Impl& impl = SocketPtr->GetImpl();
	impl = _impl;
	Handler handler = this->_handler;
	M_HANDLER_SOCKET_PTR(Handler) acceptor_ptr = this->_socket_ptr;
	this->_socket_ptr.reset();
	handler(acceptor_ptr, SocketPtr, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService::ConnectOperation<Handler>::Complete(IocpService& service, s_uint32_t transbyte, SocketError& error)
{
	M_IMPL_C_CONNECT_FLAG(this->_socket_ptr->GetImpl());
	M_HANDLER_SOCKET_PTR(Handler) connect_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_socket_ptr.reset();
	if (!error)
	{
		IocpService::Impl& impl = connect_ptr->GetImpl();
		g_setsockopt(impl._fd, M_SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
	}
	handler(connect_ptr, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService::WriteOperation<Handler>::Complete(IocpService& service, s_uint32_t transbyte, SocketError& error)
{
	M_IMPL_C_WRITE_FLAG(this->_socket_ptr->GetImpl());
	M_HANDLER_SOCKET_PTR(Handler) write_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_socket_ptr.reset();
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	handler(write_ptr, data, size, transbyte, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL bool IocpService::ReadOperation<Handler>::Complete(IocpService& service, s_uint32_t transbyte, SocketError& error)
{
	M_IMPL_C_READ_FLAG(this->_socket_ptr->GetImpl());
	M_HANDLER_SOCKET_PTR(Handler) read_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_socket_ptr.reset();
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	handler(read_ptr, data, size, transbyte, error);
	return true;
}

template<typename T>
M_SOCKET_DECL IocpService::Operation* IocpService::OperationAlloc<T>::Alloc(Operation* ptr, s_int32_t type)
{
	if (!ptr)
	{
		ptr = new Operation;
		g_bzero(ptr, sizeof(Operation));
		ptr->_type = type;
		ptr->_oper = new T;
	}
	else
	{
		g_bzero(ptr, sizeof(wsaoverlapped_t));
		ptr->_type = type;
	}
	return ptr;
}


#endif
#endif
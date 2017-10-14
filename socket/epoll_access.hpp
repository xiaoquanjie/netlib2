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

#define M_IMPL_FD(impl)\
	impl._core->_fd
#define M_IMPL_STATE(impl)\
	impl._core->_state
#define M_IMPL_EPOLL(impl)\
	impl._core->_epoll
#define M_IMPL_OP(impl)\
	impl._core->_operation

M_SOCKET_DECL void EpollService::Access::Construct(EpollService& service, EpollService::Impl& impl, s_uint16_t type)
{
	M_IMPL_EPOLL(impl) = -1;
	M_IMPL_FD(impl) = M_INVALID_SOCKET;
	M_IMPL_STATE(impl) = 0;
	M_IMPL_S_V4(impl);
	M_IMPL_S_UNBIND(impl);
	M_IMPL_S_BLOCK(impl);
	M_IMPL_S_TYPE(impl, (s_uint8_t)type);
	M_IMPL_OP(impl)._type = 0;
	M_IMPL_OP(impl)._accept_op = M_IMPL_OP(impl)._connect_op = M_IMPL_OP(impl)._read_op =
		M_IMPL_OP(impl)._write_op = 0;
}

M_SOCKET_DECL void EpollService::Access::Destroy(EpollService& service, EpollService::Impl& impl)
{
	M_IMPL_EPOLL(impl) = -1;
	M_IMPL_FD(impl) = M_INVALID_SOCKET;
	M_IMPL_STATE(impl) = 0;
	M_IMPL_S_V4(impl);
	M_IMPL_S_UNBIND(impl);
	M_IMPL_S_BLOCK(impl);
	M_IMPL_S_TYPE(impl, E_NULL_SOCKET_TYPE);

	delete M_IMPL_OP(impl)._accept_op;
	delete M_IMPL_OP(impl)._connect_op;
	delete M_IMPL_OP(impl)._read_op;
	delete M_IMPL_OP(impl)._write_op;
	M_IMPL_OP(impl)._type = 0;
	M_IMPL_OP(impl)._accept_op = M_IMPL_OP(impl)._connect_op = M_IMPL_OP(impl)._read_op =
		M_IMPL_OP(impl)._write_op = 0;
}

M_SOCKET_DECL void EpollService::Access::Close(EpollService& service, EpollService::Impl& impl, SocketError& error)
{
	if (M_IMPL_FD(impl) != M_INVALID_SOCKET)
	{
		service._mutex.lock();
		EpollService::IoServiceImpl* serviceimpl = GetIoServiceImpl(service, impl);
		if (serviceimpl)
			__sync_sub_and_fetch(&serviceimpl->_fdcnt, 1);
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
			if (M_IMPL_OP(impl)._accept_op)
				M_IMPL_OP(impl)._accept_op->Clear();
			if (M_IMPL_OP(impl)._connect_op)
				M_IMPL_OP(impl)._connect_op->Clear();
			if (M_IMPL_OP(impl)._read_op)
				M_IMPL_OP(impl)._read_op->Clear();
			if (M_IMPL_OP(impl)._write_op)
				M_IMPL_OP(impl)._write_op->Clear();
		}
	}
}

M_SOCKET_DECL bool EpollService::Access::IsOpen(EpollService& service, EpollService::Impl& impl, SocketError& error)
{
	return (M_IMPL_FD(impl) != M_INVALID_SOCKET);
}

template<typename GettableOptionType>
M_SOCKET_DECL void EpollService::Access::GetOption(EpollService& service, EpollService::Impl& impl, GettableOptionType& opt, SocketError& error)
{
	socklen_t len = opt.Size();
	s_int32_t ret = g_getsockopt(M_IMPL_FD(impl), opt.Level(), opt.Name(), (char*)opt.Data(), &len);
	M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
}

template<typename SettableOptionType>
M_SOCKET_DECL void EpollService::Access::SetOption(EpollService& service, EpollService::Impl& impl, const SettableOptionType& opt, SocketError& error)
{
	s_int32_t ret = g_setsockopt(M_IMPL_FD(impl), opt.Level(), opt.Name(), (char*)opt.Data(), opt.Size());
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint EpollService::Access::RemoteEndPoint(EndPoint, EpollService& service, const EpollService::Impl& impl, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetRemoteEndPoint(M_IMPL_FD(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template<typename EndPoint>
M_SOCKET_DECL EndPoint EpollService::Access::LocalEndPoint(EndPoint, EpollService& service, const EpollService::Impl& impl, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetLocalEndPoint(M_IMPL_FD(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

M_SOCKET_DECL void EpollService::Access::Shutdown(EpollService& service, EpollService::Impl& impl, EShutdownType what, SocketError& error)
{
	if (0 != g_shutdown(M_IMPL_FD(impl), what))
		M_DEFAULT_SOCKET_ERROR2(error);
}

template<typename ProtocolType>
M_SOCKET_DECL void EpollService::Access::Open(EpollService& service, EpollService::Impl& impl, ProtocolType pt, SocketError& error)
{
	if (M_IMPL_FD(impl) != M_INVALID_SOCKET)
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

	M_IMPL_FD(impl) = g_socket(pt.Family(), pt.Type(), pt.Protocol());
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
	{
		M_DEFAULT_SOCKET_ERROR2(error);
		return;
	}
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::Bind(EpollService& service, EpollService::Impl& impl, const EndPoint& ep, SocketError& error)
{
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_bind(M_IMPL_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void EpollService::Access::Cancel(EpollService& service, EpollService::Impl& impl, SocketError& error)
{
	CtlEpoll(service, impl, 0, M_EPOLL_CTL_DEL, 0, error);
}

M_SOCKET_DECL void EpollService::Access::Listen(EpollService& service, EpollService::Impl& impl, s_int32_t flag, SocketError& error)
{
	s_int32_t ret = g_listen(M_IMPL_FD(impl), flag);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL void EpollService::Access::Accept(EpollService& service, EpollService::Impl& impl, EpollService::Impl& peer, SocketError& error)
{
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
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
		if (!detail::Util::SetBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_BLOCK(impl);
	}

	for (;;)
	{
		M_IMPL_S_ACCEPT_FLAG(impl);
		M_IMPL_FD(peer) = g_accept(M_IMPL_FD(impl), 0, 0);
		M_IMPL_C_ACCEPT_FLAG(impl);

		if (M_IMPL_FD(peer) != M_INVALID_SOCKET)
			break;
		if (M_ERR_LAST == M_ECONNRESET)
			continue;

		M_DEFAULT_SOCKET_ERROR2(error);
		break;
	}
}

template<typename AcceptHandler>
M_SOCKET_DECL void EpollService::Access::AsyncAccept(EpollService& service, M_HANDLER_SOCKET_PTR(AcceptHandler) accept_ptr, AcceptHandler handler, SocketError& error)
{
	EpollService::Impl& impl = accept_ptr->GetImpl();
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
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
		if (!detail::Util::SetNonBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_NONBLOCK(impl)
	}

	typedef EpollService::AcceptOperation<AcceptHandler> OperationType;
	OperationType* accept_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._accept_op);
	if (!accept_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_ACCEPT_OP);
		accept_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._accept_op);
		M_IMPL_OP(impl)._type = E_ACCEPT_OP;
	}
	accept_op->_handler = handler;
	accept_op->_socket_ptr = accept_ptr;

	M_IMPL_S_ACCEPT_FLAG(impl);
	if (!M_IMPL_G_BIND(impl))
	{
		M_IMPL_S_BIND(impl);
		CtlEpoll(service, impl, &M_IMPL_OP(impl), M_EPOLL_CTL_ADD, M_EPOLLIN, error);
	}
	if (error)
	{
		M_IMPL_C_ACCEPT_FLAG(impl);
		M_IMPL_S_UNBIND(impl);
		accept_op->Clear();
		return;
	}
}

template<typename AcceptHandler>
M_SOCKET_DECL void EpollService::Access::AsyncAccept(EpollService& service, Impl& accept_impl, Impl& sock_impl, AcceptHandler handler, SocketError& error)
{
	if (M_IMPL_FD(accept_impl) == M_INVALID_SOCKET)
	{
		error = SocketError(M_ERR_BAD_DESCRIPTOR);
		return;
	}
	if (M_IMPL_G_ACCEPT_FLAG(accept_impl))
	{
		error = SocketError(M_ERR_POSTED_ACCEPT);
		return;
	}
	if (!M_IMPL_G_NONBLOCK(accept_impl))
	{
		if (!detail::Util::SetNonBlock(M_IMPL_FD(accept_impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_NONBLOCK(accept_impl)
	}

	typedef EpollService::AcceptOperation2<AcceptHandler> OperationType;
	OperationType* accept_op = dynamic_cast<OperationType*>(M_IMPL_OP(accept_impl)._accept_op);
	if (!accept_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(accept_impl), E_ACCEPT_OP);
		accept_op = dynamic_cast<OperationType*>(M_IMPL_OP(accept_impl)._accept_op);
		M_IMPL_OP(accept_impl)._type = E_ACCEPT_OP;
	}
	accept_op->_handler = handler;
	accept_op->_acpt_impl = accept_impl;
	accept_op->_cli_impl = sock_impl;

	M_IMPL_S_ACCEPT_FLAG(accept_impl);
	if (!M_IMPL_G_BIND(accept_impl))
	{
		M_IMPL_S_BIND(accept_impl);
		CtlEpoll(service, accept_impl, &M_IMPL_OP(accept_impl), M_EPOLL_CTL_ADD, M_EPOLLIN, error);
	}
	if (error)
	{
		M_IMPL_C_ACCEPT_FLAG(accept_impl);
		M_IMPL_S_UNBIND(accept_impl);
		accept_op->Clear();
		return;
	}
}

M_SOCKET_DECL s_int32_t EpollService::Access::RecvSome(EpollService& service, EpollService::Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
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
		if (!detail::Util::SetBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return M_SOCKET_ERROR;
		}
		M_IMPL_S_BLOCK(impl);
	}

	M_IMPL_S_READ_FLAG(impl);
	s_int32_t ret = g_recv(M_IMPL_FD(impl), data, size, 0);
	M_IMPL_C_READ_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
	return ret;
}

M_SOCKET_DECL s_int32_t EpollService::Access::SendSome(EpollService& service, EpollService::Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
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
		if (!detail::Util::SetBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return M_SOCKET_ERROR;
		}
		M_IMPL_S_BLOCK(impl);
	}

	M_IMPL_S_WRITE_FLAG(impl);
	s_int32_t ret = g_send(M_IMPL_FD(impl), data, size, MSG_NOSIGNAL);
	M_IMPL_C_WRITE_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
	return ret;
}

template<typename ReadHandler>
M_SOCKET_DECL void EpollService::Access::AsyncRecvSome(EpollService& service, M_HANDLER_SOCKET_PTR(ReadHandler) socket_ptr, s_byte_t* data, s_uint32_t size, ReadHandler hander, SocketError& error)
{
	EpollService::Impl& impl = socket_ptr->GetImpl();
	if (!M_IMPL_G_NONBLOCK(impl))
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

	typedef EpollService::ReadOperation<ReadHandler> OperationType;
	OperationType* read_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._read_op);
	if (!read_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_READ_OP);
		read_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._read_op);
	}
	read_op->_wsabuf.buf = data;
	read_op->_wsabuf.len = size;
	read_op->_handler = hander;
	read_op->_socket_ptr = socket_ptr;

	M_IMPL_OP(impl)._type = E_READ_OP;
	int flag = M_EPOLLIN;
	int ctl  = M_EPOLL_CTL_MOD;
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
	}
}

M_SOCKET_DECL void EpollService::Access::AsyncRecvSome(EpollService& service, Impl& impl, s_byte_t* data, s_uint32_t size, M_RW_HANDLER_TYPE(EpollService) hander, SocketError& error)
{
	if (!M_IMPL_G_NONBLOCK(impl))
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

	typedef M_RW_HANDLER_TYPE(EpollService) ReadHandler;
	typedef EpollService::ReadOperation2<ReadHandler> OperationType;
	OperationType* read_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._read_op);
	if (!read_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_READ_OP);
		read_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._read_op);
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
	}
}

template<typename WriteHandler>
M_SOCKET_DECL void EpollService::Access::AsyncSendSome(EpollService& service, M_HANDLER_SOCKET_PTR(WriteHandler) socket_ptr, const s_byte_t* data, s_uint32_t size, WriteHandler hander, SocketError& error)
{
	EpollService::Impl& impl = socket_ptr->GetImpl();
	if (!M_IMPL_G_NONBLOCK(impl))
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

	typedef EpollService::WriteOperation<WriteHandler> OperationType;
	OperationType* write_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._write_op);
	if (!write_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_WRITE_OP);
		write_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._write_op);
	}
	write_op->_wsabuf.buf = const_cast<s_byte_t*>(data);
	write_op->_wsabuf.len = size;
	write_op->_handler = hander;
	write_op->_socket_ptr = socket_ptr;

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
	}
}

M_SOCKET_DECL void EpollService::Access::AsyncSendSome(EpollService& service, Impl& impl, const s_byte_t* data, s_uint32_t size, M_RW_HANDLER_TYPE(EpollService) hander, SocketError& error)
{
	if (!M_IMPL_G_NONBLOCK(impl))
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

	typedef M_RW_HANDLER_TYPE(EpollService) WriteHandler;
	typedef EpollService::WriteOperation2<WriteHandler> OperationType;
	OperationType* write_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._write_op);
	if (!write_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_WRITE_OP);
		write_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._write_op);
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
	}
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::Connect(EpollService& service, EpollService::Impl& impl, const EndPoint& ep, SocketError& error)
{
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
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
		if (!detail::Util::SetBlock(M_IMPL_FD(impl)))
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			return;
		}
		M_IMPL_S_BLOCK(impl);
	}

	M_IMPL_S_CONNECT_FLAG(impl);
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_connect(M_IMPL_FD(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_IMPL_C_CONNECT_FLAG(impl);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename ConnectHandler, typename EndPoint>
M_SOCKET_DECL void EpollService::Access::AsyncConnect(EpollService& service, M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error)
{
	EpollService::Impl& impl = connect_ptr->GetImpl();
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
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

	typedef EpollService::ConnectOperation<ConnectHandler> OperationType;
	OperationType* connect_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._connect_op);
	if (!connect_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_CONNECT_OP);
		connect_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._connect_op);
		M_IMPL_OP(impl)._type = E_CONNECT_OP;
	}
	connect_op->_handler = handler;
	connect_op->_socket_ptr = connect_ptr;

	M_IMPL_S_CONNECT_FLAG(impl);
	if (!M_IMPL_G_BIND(impl))
	{
		M_IMPL_S_BIND(impl);
		CtlEpoll(service, impl, &M_IMPL_OP(impl), M_EPOLL_CTL_ADD, M_EPOLLOUT|M_EPOLLONESHOT, error);
	}
	if (error)
	{
		M_IMPL_C_CONNECT_FLAG(impl);
		M_IMPL_S_UNBIND(impl);
		connect_op->Clear();
		return;
	}
}

template<typename EndPoint>
M_SOCKET_DECL void EpollService::Access::AsyncConnect(EpollService& service, Impl& impl, const EndPoint& ep, M_COMMON_HANDLER_TYPE(EpollService) handler, SocketError& error)
{
	if (M_IMPL_FD(impl) == M_INVALID_SOCKET)
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

	typedef M_COMMON_HANDLER_TYPE(EpollService) ConnectHandler;
	typedef EpollService::ConnectOperation2<ConnectHandler> OperationType;
	OperationType* connect_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._connect_op);
	if (!connect_op)
	{
		EpollService::OperationAlloc<OperationType>::Alloc(&M_IMPL_OP(impl), E_CONNECT_OP);
		connect_op = dynamic_cast<OperationType*>(M_IMPL_OP(impl)._connect_op);
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
	}
}

M_SOCKET_DECL void EpollService::Access::ExecOp(EpollService::IoServiceImpl& serviceimpl, EpollService::OperationSet* opset, epoll_event_t* event)
{
	if (opset->_type & E_ACCEPT_OP)
	{
		opset->_accept_op->Complete(serviceimpl, event);
		return;
	}
	if (opset->_type & E_CONNECT_OP)
	{
		opset->_connect_op->Complete(serviceimpl, event);
		return;
	}
	if (opset->_type & E_FINISH_OP)
	{
		opset->_accept_op->Complete(serviceimpl, event);
		return;
	}
	bool flag = false;
	if (opset->_type & E_READ_OP &&
		(event->events&M_EPOLLIN || event->events&M_EPOLLERR || event->events&M_EPOLLHUP))
	{
		flag = true;
		opset->_read_op->Complete(serviceimpl, event);
	}
	if (opset->_type & E_WRITE_OP &&
		(event->events&M_EPOLLOUT || event->events&M_EPOLLERR || event->events&M_EPOLLHUP))
	{
		flag = true;
		opset->_write_op->Complete(serviceimpl, event);
		return;
	}
	if (!flag)
	{
		M_DEBUG_PRINT("type: " << opset->_type);
		assert(0);
	}
}

M_SOCKET_DECL void EpollService::Access::Run(EpollService& service, SocketError& error)
{
	EpollService::IoServiceImpl* impl = new EpollService::IoServiceImpl(service);
	CreateEpoll(*impl, error);
	if (error)
	{
		delete impl;
		return;
	}
	MutexLock& lock = service._mutex;
	lock.lock();
	service._implvector.push_back(impl);
	service._implmap[impl->_handler] = impl;
	++service._implcnt;
	lock.unlock();

	const int max_events = 100;
	epoll_event_t events[max_events];
	for (;;)
	{
		g_setlasterr(0);
		g_bzero(&events, sizeof(events));
		s_int32_t ret = g_epoll_wait(impl->_handler, events, max_events, -1);
		if (ret == 0)
		{
			1; // time out
			continue;
		}
		if (ret < 0 && M_ERR_LAST != M_EINTR)
		{
			M_DEFAULT_SOCKET_ERROR2(error);
			break;;
		}
		bool brk = false;
		for (s_int32_t idx = 0; idx < ret; ++idx)
		{
			EpollService::OperationSet* opset = (EpollService::OperationSet*)events[idx].data.ptr;
			ExecOp(*impl, opset, &events[idx]);
			if (opset->_type & E_FINISH_OP)
			{
				brk = true;
				delete opset;
			}
		}
		if (brk)
			break;;
	}

	lock.lock();
	service._implvector.erase(std::find(service._implvector.begin(), service._implvector.end(), impl));
	service._implmap.erase(impl->_handler);
	--service._implcnt;
	lock.unlock();
	DestroyEpoll(*impl);
	delete impl;
}

M_SOCKET_DECL void EpollService::Access::Stop(EpollService& service, SocketError& error)
{
	for (EpollService::IoServiceImplVector::iterator iter=service._implvector.begin();
		iter!=service._implvector.end(); ++iter)
	{
		EpollService::OperationSet* opset = new EpollService::OperationSet;
		opset->_type = E_FINISH_OP;
		EpollService::FinishOperation* op = new EpollService::FinishOperation;
		opset->_accept_op = op;
		op->_fd = g_socket(M_AF_INET, M_SOCK_STREAM, M_IPPROTO_IP);
		epoll_event_t event;
		event.events = M_EPOLLERR | M_EPOLLIN | M_EPOLLOUT;
		event.data.ptr = opset;
		s_int32_t ret = g_epoll_ctl((*iter)->_handler, M_EPOLL_CTL_ADD, op->_fd, &event);
		if (ret != 0)
			delete opset;
	}
}

M_SOCKET_DECL bool EpollService::Access::Stopped(const EpollService& service)
{
	ScopedLock scoped_lock(service._mutex);
	return (service._implvector.empty() ? true : false);
}

M_SOCKET_DECL void EpollService::Access::CreateEpoll(EpollService::IoServiceImpl& impl, SocketError& error)
{
	if (impl._handler != -1)
	{
		error = SocketError(M_ERR_EPOLL_EXIST);
		return;
	}
	impl._handler = g_epoll_create(1);
	M_DEFAULT_SOCKET_ERROR(impl._handler < 0, error);
	if (!error)
	{
		impl._fdcnt = 0;
	}
}

M_SOCKET_DECL void EpollService::Access::DestroyEpoll(EpollService::IoServiceImpl& impl)
{
	if (impl._handler != -1)
	{
		g_closesocket(impl._handler);
		impl._handler = -1;
		impl._fdcnt = 0;
	}
}

M_SOCKET_DECL void EpollService::Access::CtlEpoll(EpollService& service, Impl& impl, EpollService::OperationSet* opset, s_int32_t flag, s_int32_t events, SocketError& error)
{
	if (M_EPOLL_CTL_ADD == flag && M_IMPL_EPOLL(impl) ==-1)
	{
		service._mutex.lock();
		if (service._implvector.empty())
		{
			service._mutex.unlock();
			error = SocketError(M_ERR_NOT_SERVICE);
			return;
		}

		service._implidx++;
		s_int32_t size = service._implvector.size();
		EpollService::IoServiceImpl& serviceimpl = *(service._implvector[service._implidx%size]);
		__sync_add_and_fetch(&serviceimpl._fdcnt, 1);
		M_IMPL_EPOLL(impl) = serviceimpl._handler;
		service._mutex.unlock();
	}
	if (M_IMPL_EPOLL(impl) == -1)
	{
		error = SocketError(M_ERR_NOT_SERVICE);
		return;
	}

	epoll_event_t event;
	event.events = events;
	event.data.ptr = opset;
	s_int32_t ret = g_epoll_ctl(M_IMPL_EPOLL(impl), flag, M_IMPL_FD(impl), &event);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

M_SOCKET_DECL EpollService::IoServiceImpl* EpollService::Access::GetIoServiceImpl(EpollService& service, Impl& impl)
{
	if (M_IMPL_EPOLL(impl) == -1)
		return 0;
	
	EpollService::IoServiceImplMap::iterator iter = service._implmap.find(M_IMPL_EPOLL(impl));
	if (iter == service._implmap.end())
		return 0;
	return iter->second;
}

M_SOCKET_DECL s_uint32_t EpollService::Access::GetServiceCount(const EpollService& service)
{
	ScopedLock scoped(service._mutex);
	return service._implvector.size();
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::AcceptOperation<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	Handler handler = this->_handler;
	M_HANDLER_SOCKET_PTR(Handler) socket_ptr = this->_socket_ptr;
	
	SocketError error;
	EpollService::Impl& impl = socket_ptr->GetImpl();
	shard_ptr_t<TcpSocket<EpollService> > 
		clisocket_ptr(new TcpSocket<EpollService>(serviceimpl.GetService()));

	M_IMPL_C_ACCEPT_FLAG(impl);
	socket_t fd = g_accept(M_IMPL_FD(impl), 0, 0);
	if (fd < 0) {
		Opts::SoError error_opt;
		socket_ptr->GetOption(error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_ACCEPT_FAIL) : SocketError(error_opt.Value());
	}
	else {
		EpollService::Impl& cli_impl = clisocket_ptr->GetImpl();
		M_IMPL_FD(cli_impl) = fd;
		if (M_IMPL_G_V(impl)){
			M_IMPL_S_V4(cli_impl);
		}
		else{
			M_IMPL_S_V6(cli_impl);
		}
	}
		
	this->Clear();
	handler(socket_ptr, clisocket_ptr, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::AcceptOperation<Handler>::Clear()
{
	this->_handler = 0;
	this->_socket_ptr.reset();
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::AcceptOperation2<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	Handler handler = this->_handler;
	this->_handler = 0;

	EpollService::Impl& impl = this->_acpt_impl;
	if (!M_IMPL_G_ACCEPT_FLAG(impl))
		return false;

	SocketError error;
	socket_t fd = g_accept(M_IMPL_FD(impl), 0, 0);
	if (fd < 0)
	{
		Opts::SoError error_opt;
		EpollService::Access::GetOption(serviceimpl.GetService(), impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_ACCEPT_FAIL) : SocketError(error_opt.Value());
	}
	else
	{
		EpollService::Impl& cli_impl = this->_cli_impl;
		M_IMPL_FD(cli_impl) = fd;
		if (M_IMPL_G_V(impl)) {
			M_IMPL_S_V4(cli_impl);
		}
		else {
			M_IMPL_S_V6(cli_impl);
		}
	}

	M_IMPL_C_ACCEPT_FLAG(impl);
	handler(error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::AcceptOperation2<Handler>::Clear()
{
	this->_handler = 0;
	this->_acpt_impl = this->_cli_impl = Impl();
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::ConnectOperation<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	M_HANDLER_SOCKET_PTR(Handler) socket_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_handler = 0;
	this->_socket_ptr.reset();
	if (!socket_ptr)
		return false;

	EpollService::Impl& impl = socket_ptr->GetImpl();
	if (!M_IMPL_G_CONNECT_FLAG(impl))
		return false;

	SocketError error;
	if (event->events & M_EPOLLERR)
	{
		Opts::SoError error_opt;
		socket_ptr->GetOption(error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_CONNECT_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	opset->_type &= ~(E_CONNECT_OP);

	M_IMPL_C_CONNECT_FLAG(impl);
	handler(socket_ptr, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::ConnectOperation<Handler>::Clear()
{
	this->_handler = 0;
	this->_socket_ptr.reset();
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::ConnectOperation2<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	Handler handler = this->_handler;
	this->_handler = 0;
	
	EpollService::Impl& impl = this->_impl;
	if (!M_IMPL_G_CONNECT_FLAG(impl))
		return false;

	SocketError error;
	if (event->events & M_EPOLLERR)
	{
		Opts::SoError error_opt;
		EpollService::Access::GetOption(serviceimpl.GetService(), impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_CONNECT_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	opset->_type &= ~(E_CONNECT_OP);

	M_IMPL_C_CONNECT_FLAG(impl);
	handler(error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::ConnectOperation2<Handler>::Clear()
{
	this->_handler = 0;
	this->_impl = Impl();
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::WriteOperation<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
	M_HANDLER_SOCKET_PTR(Handler) socket_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_handler = 0;
	this->_socket_ptr.reset();
	if (!socket_ptr)
		return false;

	EpollService::Impl& impl = socket_ptr->GetImpl();
	if (!M_IMPL_G_WRITE_FLAG(impl))
		return false;

	SocketError error;
	s_int32_t ret = g_send(M_IMPL_FD(impl), data, size, MSG_NOSIGNAL);
	if (ret <= 0)
	{
		Opts::SoError error_opt;
		socket_ptr->GetOption(error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_WRITE_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	s_int32_t flag = 0;
	if (opset->_type & E_READ_OP)
		flag = M_EPOLLIN;
	opset->_type &= ~(E_WRITE_OP);
	Access::CtlEpoll(serviceimpl.GetService(), impl, opset, M_EPOLL_CTL_MOD, flag, error);

	M_IMPL_C_WRITE_FLAG(impl);
	handler(socket_ptr, data, size, ret < 0 ? 0 : ret, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::WriteOperation<Handler>::Clear()
{
	this->_handler = 0;
	this->_socket_ptr.reset();
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::WriteOperation2<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
	Handler handler = this->_handler;
	this->_handler = 0;
	
	EpollService::Impl& impl = this->_impl;
	if (!M_IMPL_G_WRITE_FLAG(impl))
		return false;

	SocketError error;
	s_int32_t ret = g_send(M_IMPL_FD(impl), data, size, MSG_NOSIGNAL);
	if (ret <= 0)
	{
		Opts::SoError error_opt;
		Access::GetOption(serviceimpl.GetService(), impl, error_opt, error);
		error = error ? SocketError(M_ERR_ASYNC_WRITE_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	s_int32_t flag = 0;
	if (opset->_type & E_READ_OP)
		flag = M_EPOLLIN;
	opset->_type &= ~(E_WRITE_OP);
	Access::CtlEpoll(serviceimpl.GetService(), impl, opset, M_EPOLL_CTL_MOD, flag, error);

	M_IMPL_C_WRITE_FLAG(impl);
	handler(ret < 0 ? 0 : ret, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::WriteOperation2<Handler>::Clear()
{
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::ReadOperation<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
	M_HANDLER_SOCKET_PTR(Handler) socket_ptr = this->_socket_ptr;
	Handler handler = this->_handler;
	this->_handler = 0;
	this->_socket_ptr.reset();
	if (!socket_ptr)
		return false;
	
	EpollService::Impl& impl = socket_ptr->GetImpl();
	if (!M_IMPL_G_READ_FLAG(impl))
		return false;

	SocketError error;
	s_int32_t ret = g_recv(M_IMPL_FD(impl), data, size, 0);
	if (ret <= 0)
	{
		Opts::SoError error_opt;
		socket_ptr->GetOption(error_opt, error);
		error = (error) ? SocketError(M_ERR_ASYNC_READ_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	s_int32_t flag = 0;
	if (opset->_type & E_WRITE_OP)
		flag = M_EPOLLOUT;
	opset->_type &= ~(E_READ_OP);
	Access::CtlEpoll(serviceimpl.GetService(), impl, opset, M_EPOLL_CTL_MOD, flag, error);

	M_IMPL_C_READ_FLAG(impl);
	handler(socket_ptr, data, size, ret < 0 ? 0 : ret, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::ReadOperation<Handler>::Clear()
{
	this->_handler = 0;
	this->_socket_ptr.reset();
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

template<typename Handler>
M_SOCKET_DECL bool EpollService::ReadOperation2<Handler>::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	s_byte_t* data = this->_wsabuf.buf;
	s_uint32_t size = (s_uint32_t)this->_wsabuf.len;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
	Handler handler = this->_handler;
	this->_handler = 0;
	
	EpollService::Impl& impl = this->_impl;
	if (!M_IMPL_G_READ_FLAG(impl))
		return false;

	SocketError error;
	s_int32_t ret = g_recv(M_IMPL_FD(impl), data, size, 0);
	if (ret <= 0)
	{
		Opts::SoError error_opt;
		Access::GetOption(serviceimpl.GetService(), impl, error_opt, error);
		error = (error) ? SocketError(M_ERR_ASYNC_READ_FAIL) : SocketError(error_opt.Value());
	}

	EpollService::OperationSet* opset = (EpollService::OperationSet*)event->data.ptr;
	s_int32_t flag = 0;
	if (opset->_type & E_WRITE_OP)
		flag = M_EPOLLOUT;
	opset->_type &= ~(E_READ_OP);
	Access::CtlEpoll(serviceimpl.GetService(), impl, opset, M_EPOLL_CTL_MOD, flag, error);

	M_IMPL_C_READ_FLAG(impl);
	handler(ret < 0 ? 0 : ret, error);
	return true;
}

template<typename Handler>
M_SOCKET_DECL void EpollService::ReadOperation2<Handler>::Clear()
{
	this->_impl = Impl();
	this->_handler = 0;
	this->_wsabuf.buf = 0;
	this->_wsabuf.len = 0;
}

M_SOCKET_DECL bool EpollService::FinishOperation::Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event)
{
	g_closesocket(this->_fd);
	this->_fd = M_INVALID_SOCKET;
	return true;
}

M_SOCKET_DECL void EpollService::FinishOperation::Clear()
{
}

template<typename T>
M_SOCKET_DECL void EpollService::OperationAlloc<T>::Alloc(OperationSet* opset, s_int32_t type)
{
	if (type & E_ACCEPT_OP && !opset->_accept_op)
	{
		opset->_accept_op = new T;
	}
	else if (type & E_CONNECT_OP && !opset->_connect_op)
	{
		opset->_connect_op = new T;
	}
	else if (type & E_READ_OP && !opset->_read_op)
	{
		opset->_read_op = new T;
	}
	else if (type & E_WRITE_OP && !opset->_write_op)
	{
		opset->_write_op = new T;
	}
	else
	{
		assert(0);
	}
}

#endif
#endif
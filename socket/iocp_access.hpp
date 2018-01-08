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

#include "coroutine/coroutine.hpp"
#include "socket/win_iocp.hpp"
#ifdef M_PLATFORM_WIN

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
#define M_Impl_Iocp(impl)\
	impl._core->_iocp
#define M_Impl_Op(impl)\
	impl._core->_op
#define M_Impl_Rop(impl)\
	impl._core->_op._rop
#define M_Impl_Wop(impl)\
	impl._core->_op._wop
#define M_Impl_Aop(impl)\
	impl._core->_op._aop
#define M_Impl_Cop(impl)\
	impl._core->_op._cop

#define M_Impl_Mutex(impl)\
	(*impl._core->_mutex)
#define M_Impl_Init_Mutex(impl)\
	impl._core->_mutex.reset(new MutexLock);

#define M_Swap_Handler(type,handler1,handler2)\
	handler1.swap(const_cast<type>(handler2));

using namespace coroutine;

M_SOCKET_NAMESPACE_BEGIN

inline void IocpService::Access::ConstructImpl(IocpService& service, SocketImpl& impl, s_uint16_t type) {
	impl.Init();
	M_Impl_Iocp(impl) = 0;
	M_Impl_Fd(impl) = M_INVALID_SOCKET;
	M_Impl_State(impl) = 0;
	M_Impl_S_V4(impl);
	M_Impl_S_Unbind(impl);
	M_Impl_S_Block(impl);
	M_Impl_S_Type(impl, (s_uint8_t)type);
	M_Impl_Init_Mutex(impl);
}

inline void IocpService::Access::DestroyImpl(IocpService& service, SocketImpl& impl) {
	ConstructImpl(service, impl, E_NULL_SOCKET_TYPE);
}

inline bool IocpService::Access::IsOpen(IocpService& service, SocketImpl& impl, SocketError& error) {
	return (M_Impl_Fd(impl) != M_INVALID_SOCKET);
}

template<typename GettableOptionType>
inline void IocpService::Access::GetOption(IocpService& service, SocketImpl& impl,
	GettableOptionType& opt, SocketError& error) {
	socklen_t len = opt.Size();
	s_int32_t ret = g_getsockopt(M_Impl_Fd(impl), opt.Level(), opt.Name(), (char*)opt.Data(), &len);
	M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
}

template<typename SettableOptionType>
inline void IocpService::Access::SetOption(IocpService& service, SocketImpl& impl,
	const SettableOptionType& opt, SocketError& error) {
	s_int32_t ret = g_setsockopt(M_Impl_Fd(impl), opt.Level(), opt.Name(), (char*)opt.Data(), opt.Size());
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

template<typename EndPoint>
inline EndPoint IocpService::Access::RemoteEndPoint(EndPoint, IocpService& service,
	const SocketImpl& impl, SocketError& error) {
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetRemoteEndPoint(M_Impl_Fd(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template<typename EndPoint>
inline EndPoint IocpService::Access::LocalEndPoint(EndPoint, IocpService& service,
	const SocketImpl& impl, SocketError& error) {
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	EndPoint ep;
	if (!ep_access::GetLocalEndPoint(M_Impl_Fd(impl), ep))
		M_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

inline void IocpService::Access::Cancel(IocpService& service, SocketImpl& impl, SocketError& error) {
	if (M_Impl_G_Bind(impl)) {
		if (!g_cancelio((HANDLE)M_Impl_Fd(impl)))
			M_DEFAULT_SOCKET_ERROR2(error);
	}
}

inline void IocpService::Access::BindIocp(IocpService& service, SocketImpl& impl, SocketError& error) {
	if (M_Impl_Iocp(impl) == 0) {
		ScopedLock scoped_l(service._mutex);
		if (!service._implvector.empty()) {
			service._implidx++;
			s_uint32_t size = (s_uint32_t)service._implvector.size();
			IoServiceImpl& serviceimpl = *(service._implvector[service._implidx%size]);
			M_Impl_Iocp(impl) = serviceimpl._handler;
			serviceimpl._fdcnt++;
		}
	}
	if (M_Impl_Iocp(impl) == 0) {
		error = SocketError(M_ERR_NOT_SERVICE);
		return;
	}
	HANDLE ret = g_createiocompletionport((HANDLE)M_Impl_Fd(impl), M_Impl_Iocp(impl), 0, 0);
	M_DEFAULT_SOCKET_ERROR(M_Impl_Iocp(impl) != ret, error);
}

inline void IocpService::Access::Run(IocpService& service, SocketError& error) {
	IoServiceImpl* psimpl = _CreateIoImpl(service, error);
	if (psimpl) {
		_DoRun(service, *psimpl, false, error);
		_ReleaseIoImpl(service, psimpl);
	}
}

inline void IocpService::Access::CoRun(IocpService& service, SocketError& error) {
	IoServiceImpl* psimpl = _CreateIoImpl(service, error);
	if (psimpl) {
		Coroutine::initEnv();
		_DoRun(service, *psimpl, true, error);
		_ReleaseIoImpl(service, psimpl);
		CoroutineTask::clrTask();
		Coroutine::close();
	}
}

inline void IocpService::Access::Stop(IocpService& service, SocketError& error) {
	service._mutex.lock();
	for (IocpService::IoServiceImplVector::iterator iter = service._implvector.begin();
		iter != service._implvector.end(); ++iter) {
		Operation* op = new Operation;
		op->_type = E_FINISH_OP;
		if (!g_postqueuedcompletionstatus((*iter)->_handler, 0, 0, op)) {
			delete op;
			M_DEFAULT_SOCKET_ERROR2(error);
		}
	}
	service._mutex.unlock();
}

inline bool IocpService::Access::Stopped(const IocpService& service) {
	ScopedLock scoped_lock(service._mutex);
	return (service._implvector.empty() ? true : false);
}

inline s_uint32_t IocpService::Access::GetServiceCount(const IocpService& service) {
	ScopedLock scoped(service._mutex);
	return (s_uint32_t)service._implvector.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void IocpService::Access::Close(IocpService& service, SocketImpl& impl, SocketError& error) {
	Close(service, impl, 0, error);
}

inline void IocpService::Access::Close(IocpService& service, SocketImpl& impl, function_t<void()> handler, 
	SocketError& error) 
{
	MutexLock& mlock = M_Impl_Mutex(impl);
	mlock.lock();
	if (M_Impl_Fd(impl) != M_INVALID_SOCKET 
		&& !M_Impl_G_Close_Flag(impl)) 
	{
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
inline void IocpService::Access::Open(IocpService& service, SocketImpl& impl,
	const ProtocolType& pt, SocketError& error) 
{
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
inline void IocpService::Access::Bind(IocpService& service, SocketImpl& impl, const EndPoint& ep, 
	SocketError& error) {
	typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
	s_int32_t ret = g_bind(M_Impl_Fd(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

inline void IocpService::Access::Listen(IocpService& service, SocketImpl& impl, s_int32_t flag, 
	SocketError& error) {
	s_int32_t ret = g_listen(M_Impl_Fd(impl), flag);
	M_DEFAULT_SOCKET_ERROR(ret != 0, error);
}

inline void IocpService::Access::Shutdown(IocpService& service, SocketImpl& impl,
	EShutdownType what, SocketError& error) {
	if (0 != g_shutdown(M_Impl_Fd(impl), what))
		M_DEFAULT_SOCKET_ERROR2(error);
}

inline void IocpService::Access::Accept(IocpService& service, SocketImpl& impl,
	SocketImpl& peer, SocketError& error)
{
	MutexLock& lock = M_Impl_Mutex(impl);
	do
	{
		lock.lock();
		if (M_INVALID_SOCKET == M_Impl_Fd(impl)) {
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
		lock.unlock();

		for (;;) {
			M_Impl_Fd(peer) = g_accept(M_Impl_Fd(impl), 0, 0);
			if (M_INVALID_SOCKET != M_Impl_Fd(peer)) {
				break;
			}
			if (M_ECONNRESET == M_ERR_LAST) {
				continue;
			}
			M_DEFAULT_SOCKET_ERROR2(error);
			break;
		}

		lock.lock();
		M_Impl_C_Accept_Flag(impl);
		lock.unlock();
		return;

	} while (false);
	lock.unlock();
}

inline void IocpService::Access::AsyncAccept(IocpService& service, SocketImpl& accept_impl, SocketImpl& client_impl
	, const M_COMMON_HANDLER_TYPE(IocpService)& handler, SocketError& error)
{
	MutexLock& mlock = M_Impl_Mutex(accept_impl);
	do
	{
		mlock.lock();
		if (M_Impl_Fd(accept_impl) == M_INVALID_SOCKET) {
			error = SocketError(M_ERR_BAD_DESCRIPTOR);
			break;
		}
		if (M_Impl_G_Accept_Flag(accept_impl)) {
			error = SocketError(M_ERR_POSTED_ACCEPT);
			break;
		}
		if (!M_Impl_G_Nonblock(accept_impl)) {
			if (!detail::Util::SetNonBlock(M_Impl_Fd(accept_impl))) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_Impl_S_Nonblock(accept_impl)
		}
		if (!M_Impl_G_Bind(accept_impl)) {
			BindIocp(service, accept_impl, error);
			if (error)
				break;
			M_Impl_S_Bind(accept_impl);
		}

		ConstructImpl(service, client_impl, E_SOCKET_TYPE);
		Open(service, client_impl, M_Impl_G_V(accept_impl) ? Tcp::V6() : Tcp::V4(), error);
		if (error)
			break;

		M_Impl_S_Accept_Flag(accept_impl);
		mlock.unlock();

		typedef AptOpType<M_COMMON_HANDLER_TYPE(IocpService)> OperationType;
		OpAlloc<OperationType>::AllocOp(M_Impl_Aop(accept_impl),E_ACCEPT_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_Impl_Aop(accept_impl)._oper);
		g_bzero(op->_buf, sizeof(op->_buf));
		op->_bytes = 0;
		op->_impl = client_impl;
		op->_accept_impl = accept_impl;
		M_Swap_Handler(M_COMMON_HANDLER_TYPE(IocpService)&, op->_handler, handler);

		for (;;) {

			// If no error occurs, the AcceptEx function completed successfully and a value of TRUE is returned.
			s_int32_t ret = gAcceptEx(M_Impl_Fd(accept_impl), M_Impl_Fd(op->_impl), op->_buf,
				0, sizeof(sockaddr_storage_t), sizeof(sockaddr_storage_t), (LPDWORD)&op->_bytes, &M_Impl_Aop(accept_impl));

			if (!ret && M_ERR_LAST == M_ECONNRESET) {
				M_DEBUG_PRINT("peer connect reset");
				continue;
			}
			if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING) {
				M_DEBUG_PRINT("acceptex fail");
				M_DEFAULT_SOCKET_ERROR2(error);

				SocketError error2;
				Close(service, client_impl, error2);
				op->Clear();

				mlock.lock();
				M_Impl_C_Accept_Flag(accept_impl);
				mlock.unlock();
			}
			break;
		}
		return;
	} while (false);
	mlock.unlock();
}

template<typename EndPoint>
inline void IocpService::Access::Connect(IocpService& service, SocketImpl& impl, const EndPoint& ep,
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
			if (M_ERR_LAST == M_EWOULDBLOCK) {
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
inline void IocpService::Access::AsyncConnect(IocpService& service, SocketImpl& impl, const EndPoint& ep
	, const M_COMMON_HANDLER_TYPE(IocpService)& handler, SocketError& error)
{
	MutexLock& mlock = M_Impl_Mutex(impl);
	do
	{
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
		if (!M_Impl_G_Nonblock(impl)) {
			if (!detail::Util::SetNonBlock(M_Impl_Fd(impl))) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_Impl_S_Nonblock(impl)
		}
		if (!M_Impl_G_Bind(impl)) {
			BindIocp(service, impl, error);
			if (error)
				break;
			M_Impl_S_Bind(impl);
		}

		M_Impl_S_Connect_Flag(impl);
		mlock.unlock();

		sockaddr_storage_t addr;
		g_bzero(&addr, sizeof(addr));
		addr.ss_family = ep.Protocol().Family();
		g_bind(M_Impl_Fd(impl), (sockaddr_t*)&addr, sizeof(addr));

		typedef CoOpType<M_COMMON_HANDLER_TYPE(IocpService)> OperationType;
		OpAlloc<OperationType>::AllocOp(M_Impl_Cop(impl), E_CONNECT_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_Impl_Cop(impl)._oper);
		op->_impl = impl;
		M_Swap_Handler(M_COMMON_HANDLER_TYPE(IocpService)&, op->_handler, handler);

		DWORD send_bytes = 0;
		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		s_int32_t ret = gConnectEx(M_Impl_Fd(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep),
			0, 0, &send_bytes, &M_Impl_Cop(impl));
		if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING) {
			M_DEFAULT_SOCKET_ERROR2(error);
			op->Clear();

			mlock.lock();
			M_Impl_C_Connect_Flag(impl);
			mlock.unlock();
		}
		return;
	} while (false);
	mlock.unlock();
}

inline s_int32_t IocpService::Access::RecvSome(IocpService& service, SocketImpl& impl,
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

		DWORD trans_bytes = 0;
		wsabuf_t wsabuf;
		wsabuf.buf = data;
		wsabuf.len = static_cast<DWORD>(size);
		DWORD flag = 0;
		s_int32_t ret = g_wsarecv(M_Impl_Fd(impl), &wsabuf, 1, &trans_bytes, &flag, 0, 0);

		mlock.lock();
		M_Impl_C_Read_Flag(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return trans_bytes;
	} while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

inline void IocpService::Access::AsyncRecvSome(IocpService& service, SocketImpl& impl, s_byte_t* data, s_uint32_t size
	, const M_RW_HANDLER_TYPE(IocpService)& hander, SocketError& error)
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
		if (!M_Impl_G_Nonblock(impl)) {
			if (!detail::Util::SetNonBlock(M_Impl_Fd(impl))) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_Impl_S_Nonblock(impl)
		}
		if (!M_Impl_G_Bind(impl)) {
			BindIocp(service, impl, error);
			if (error)
				break;
			M_Impl_S_Bind(impl);
		}

		M_Impl_S_Read_Flag(impl);
		mlock.unlock();

		typedef RdOpType<M_RW_HANDLER_TYPE(IocpService)> OperationType;
		OpAlloc<OperationType>::AllocOp(M_Impl_Rop(impl), E_READ_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_Impl_Rop(impl)._oper);
		op->_impl = impl;
		op->_wsabuf.buf = data;
		op->_wsabuf.len = size;
		M_Swap_Handler(M_RW_HANDLER_TYPE(IocpService)&, op->_handler, hander);

		DWORD flag = 0;
		s_int32_t ret = g_wsarecv(M_Impl_Fd(impl), &op->_wsabuf, 1, 0, &flag, &M_Impl_Rop(impl), 0);
		if (ret == M_SOCKET_ERROR  && M_ERR_LAST != M_ERROR_IO_PENDING) {
			M_DEFAULT_SOCKET_ERROR2(error);
			op->Clear();

			mlock.lock();
			M_Impl_C_Read_Flag(impl);
			mlock.unlock();
		}
		return;
	} while (false);
	mlock.unlock();
}

inline s_int32_t IocpService::Access::SendSome(IocpService& service, SocketImpl& impl,
	const s_byte_t* data, s_uint32_t size, SocketError& error)
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

		DWORD trans_bytes = 0;
		wsabuf_t wsabuf;
		wsabuf.buf = const_cast<s_byte_t*>(data);
		wsabuf.len = static_cast<DWORD>(size);;
		s_int32_t ret = g_wsasend(M_Impl_Fd(impl), &wsabuf, 1, &trans_bytes, 0, 0, 0);

		mlock.lock();
		M_Impl_C_Write_Flag(impl);
		mlock.unlock();

		M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
		return trans_bytes;
	} while (false);
	mlock.unlock();
	return M_SOCKET_ERROR;
}

inline void IocpService::Access::AsyncSendSome(IocpService& service, SocketImpl& impl, const s_byte_t* data, s_uint32_t size
	, const M_RW_HANDLER_TYPE(IocpService)& hander, SocketError& error)
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
			return;
		}
		if (!M_Impl_G_Nonblock(impl)) {
			if (!detail::Util::SetNonBlock(M_Impl_Fd(impl))) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			M_Impl_S_Nonblock(impl)
		}
		if (!M_Impl_G_Bind(impl)) {
			BindIocp(service, impl, error);
			if (error)
				break;
			M_Impl_S_Bind(impl);
		}

		M_Impl_S_Write_Flag(impl);
		mlock.unlock();

		typedef WrOpType< M_RW_HANDLER_TYPE(IocpService)> OperationType;
		OpAlloc<OperationType>::AllocOp(M_Impl_Wop(impl), E_WRITE_OP);

		OperationType* op = dynamic_cast<OperationType*>(M_Impl_Wop(impl)._oper);
		op->_impl = impl;
		op->_wsabuf.buf = const_cast<s_byte_t*>(data);
		op->_wsabuf.len = size;
		M_Swap_Handler(M_RW_HANDLER_TYPE(IocpService)&, op->_handler, hander);

		DWORD send_bytes = 0;
		// If no error occurs and the send operation has completed immediately, WSASend returns zero
		s_int32_t ret = g_wsasend(M_Impl_Fd(impl), &op->_wsabuf, 1, 0, 0, &M_Impl_Wop(impl), 0);
		if (ret == M_SOCKET_ERROR && M_ERR_LAST != M_ERROR_IO_PENDING) {
			M_DEFAULT_SOCKET_ERROR2(error);
			op->Clear();

			mlock.lock();
			M_Impl_C_Write_Flag(impl);
			mlock.unlock();
		}
	} while (false);
	mlock.unlock();
}

inline s_int32_t IocpService::Access::Select(SocketImpl& impl, bool rd_or_wr, s_uint32_t timeo_sec, SocketError& error) {
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline IocpService::IoServiceImpl* 
IocpService::Access::_CreateIoImpl(IocpService& service, SocketError& error) {
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

inline void IocpService::Access::_ReleaseIoImpl(
	IocpService& service, IoServiceImpl* simpl) 
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

inline void IocpService::Access::_DoRun(IocpService& service, IoServiceImpl& simpl, bool isco, 
	SocketLib::SocketError& error) 
{
	base::slist<SocketClose*> closes1;
	base::slist<SocketClose*> closes2;
	DWORD trans_bytes = 0;
	ULONG_PTR comple_key = 0;
	overlapped_t* overlapped = 0;
	for (;;) {
		_DoClose(&simpl, closes1, closes2);
		trans_bytes = 0;
		comple_key = 0;
		overlapped = 0;
		g_setlasterr(0);

		BOOL ret = g_getqueuedcompletionstatus(simpl._handler, &trans_bytes,
			&comple_key, &overlapped, 500);
		if (overlapped) {
			Operation* op = (Operation*)overlapped;
			if (op->_type & E_FINISH_OP) {
				delete op;
				break;
			}
			_ExecOp(isco, service, &simpl, op, trans_bytes, 
				ret ? true : false);
			continue;
		}
		if (!ret) {
			if (M_ERR_LAST != WAIT_TIMEOUT) {
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
		}
	}
}

inline void IocpService::Access::_ExecOp(bool isco, IocpService& service, IoServiceImpl* simpl, 
	Operation* operation,s_uint32_t tb, bool opstate) 
{
	if (!isco) {
		_DoExecOp(&service, operation, tb, opstate);
	}
	else {
		CoEventTask* task = 0;
		if (simpl->_taskvec.size()) {
			task = simpl->_taskvec.back();
			simpl->_taskvec.pop_back();
		}
		else 
			task = (CoEventTask*)malloc(sizeof(CoEventTask));
		
		task->service = &service;
		task->op = operation;
		task->tb = tb;
		task->ok = opstate;
		coroutine::CoroutineTask::doTask(_DoExecCoOp, task);
		if (simpl->_taskvec.size() < 1024)
			simpl->_taskvec.push_back(task);
		else
			free(task);
	}
}

inline void IocpService::Access::_DoExecCoOp(void* param) {
	CoEventTask* task = (CoEventTask*)param;
	_DoExecOp(task->service, task->op, task->tb, task->ok);
}

inline void IocpService::Access::_DoExecOp(IocpService* service, Operation* operation,
	s_uint32_t tb, bool opstate) 
{
	// op->Internal is not error code
	SocketError error;
	if (!opstate) {
		M_DEFAULT_SOCKET_ERROR2(error);
	}
	operation->_oper->Complete(*service, tb, error);
}

inline void IocpService::Access::_DoClose(IocpService::IoServiceImpl* simpl,
	base::slist<SocketClose*>&closereqs, base::slist<SocketClose*>&closereqs2)
{
	if (simpl->_closereqs.size()) {
		simpl->_mutex.lock();
		closereqs.swap(simpl->_closereqs);
		simpl->_mutex.unlock();
	}

	while (closereqs.size()) {
		SocketClose* close = closereqs.front();
		MutexLock& mlock = M_Impl_Mutex(close->_impl);
		mlock.lock();
		g_closesocket(M_Impl_Fd(close->_impl));
		M_Impl_Fd(close->_impl) = M_INVALID_SOCKET;
		M_Impl_State(close->_impl) = 0;
		mlock.unlock();
		if (close->_handler)
			close->_handler();
		close->Clear();

		closereqs2.push_back(close);
		closereqs.pop_front();
	}

	if (closereqs2.size()) {
		simpl->_mutex.lock();
		simpl->_closereqs2.join(closereqs2);
		simpl->_mutex.unlock();
	}
}

inline IocpService::IoServiceImpl* 
IocpService::Access::_GetIoServiceImpl(IocpService& service, SocketImpl& impl) {
	if (M_Impl_Iocp(impl) == 0)
		return 0;
	IocpService::IoServiceImplMap::iterator iter = service._implmap.find(M_Impl_Iocp(impl));
	if (iter == service._implmap.end())
		return 0;
	return iter->second;
}

///////////////////////////////////////////////////////////////////////////////////////

namespace iodetail {
	// struct AcceptOperation
	template<typename Handler>
	inline bool AcceptOperation<Handler>::Complete(IocpService& service,
		s_uint32_t transbyte, SocketError& error) {
		MutexLock& mlock = M_Impl_Mutex(this->_accept_impl);
		mlock.lock();
		if (M_Impl_Fd(this->_accept_impl) != M_INVALID_SOCKET)
			M_Impl_C_Accept_Flag(this->_accept_impl);
		mlock.unlock();

		if (error) {
			SocketError error2;
			IocpService::Access::Close(service, this->_impl, error2);
		}
		else {
			if (M_Impl_Fd(this->_accept_impl) != M_INVALID_SOCKET) {
				g_setsockopt(M_Impl_Fd(this->_impl), M_SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
					(char*)&(M_Impl_Fd(this->_accept_impl)), sizeof(M_Impl_Fd(this->_accept_impl)));
			}
		}
		Handler handler;
		handler.swap(this->_handler);
		this->Clear();
		handler(error);
		return true;
	}

	template<typename Handler>
	inline void AcceptOperation<Handler>::Clear() {
		this->_handler = 0;
		this->_accept_impl = this->_impl = SocketImpl();
	}

	// struct ConnectOperation
	template<typename Handler>
	inline bool ConnectOperation<Handler>::Complete(IocpService& service,
		s_uint32_t transbyte, SocketError& error) {
		bool notify = false;
		MutexLock& mlock = M_Impl_Mutex(this->_impl);
		mlock.lock();
		if (M_Impl_Fd(this->_impl) != M_INVALID_SOCKET) {
			M_Impl_C_Connect_Flag(this->_impl);
			notify = true;
		}
		mlock.unlock();

		if (!error) {
			g_setsockopt(M_Impl_Fd(this->_impl), M_SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
		}
		Handler handler;
		handler.swap(this->_handler);
		this->Clear();
		if (notify)
			handler(error);
		return true;
	}

	template<typename Handler>
	inline void ConnectOperation<Handler>::Clear() {
		this->_handler = 0;
		this->_impl = SocketImpl();
	}

	// struct WriteOperation
	template<typename Handler>
	inline bool WriteOperation<Handler>::Complete(IocpService& service,
		s_uint32_t transbyte, SocketError& error) {
		bool notify = false;
		MutexLock& mlock = M_Impl_Mutex(this->_impl);
		mlock.lock();
		if (M_Impl_Fd(this->_impl) != M_INVALID_SOCKET) {
			notify = true;
			M_Impl_C_Write_Flag(this->_impl);
		}
		mlock.unlock();

		Handler handler;
		handler.swap(this->_handler);
		this->Clear();
		if (notify)
			handler(transbyte, error);
		return true;
	}

	template<typename Handler>
	inline void WriteOperation<Handler>::Clear() {
		this->_impl = SocketImpl();
		this->_handler = 0;
		this->_wsabuf.buf = 0;
		this->_wsabuf.len = 0;
	}

	// struct ReadOperation
	template<typename Handler>
	inline bool ReadOperation<Handler>::Complete(IocpService& service,
		s_uint32_t transbyte, SocketError& error) {
		bool notify = false;
		MutexLock& mlock = M_Impl_Mutex(this->_impl);
		mlock.lock();
		if (M_Impl_Fd(this->_impl) != M_INVALID_SOCKET) {
			notify = true;
			M_Impl_C_Read_Flag(this->_impl);
		}
		mlock.unlock();

		Handler handler;
		handler.swap(this->_handler);
		this->Clear();
		if (notify)
			handler(transbyte, error);
		return true;
	}

	template<typename Handler>
	inline void ReadOperation<Handler>::Clear() {
		this->_impl = SocketImpl();
		this->_handler = 0;
		this->_wsabuf.buf = 0;
		this->_wsabuf.len = 0;
	}

	template<typename T>
	inline void OperationAlloc<T>::AllocOp(Operation& op, s_int32_t type) {
		if (!op._oper) {
			op._oper = new T;
		}
		g_bzero(&op, sizeof(wsaoverlapped_t));
		op._type = type;
	}

	/////////////////////////////////////////////////////////////////////////////

	inline SocketImpl::SocketImpl() {
	}
	inline void SocketImpl::Init() {
		if (!_core) {
			_core.reset(new core);
			_core->_fd = M_INVALID_SOCKET;
			_core->_iocp = 0;
			_core->_state = 0;
		}
	}
}

M_SOCKET_NAMESPACE_END
#endif
#endif
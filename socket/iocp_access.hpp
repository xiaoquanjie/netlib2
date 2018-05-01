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

using namespace coroutine;

M_SOCKET_NAMESPACE_BEGIN

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
				M_DEBUG_PRINT("peer connect reset\n");
				continue;
			}
			if (!ret && M_ERR_LAST != M_ERROR_IO_PENDING) {
				M_DEBUG_PRINT("acceptex fail\n");
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void IocpService::Access::_DoRun(IocpService& service, IoServiceImpl& simpl, bool isco, 
	SocketLib::SocketError& error) 
{
	function_t<void()>& runhandler = service.GetRunCallback();
	base::slist<SocketClose*> closes1;
	base::slist<SocketClose*> closes2;
	DWORD trans_bytes = 0;
	ULONG_PTR comple_key = 0;
	overlapped_t* overlapped = 0;
	for (;;) {
		if (runhandler)
			runhandler();
		_DoClose(&simpl, closes1, closes2);
		if (isco)
			CoroutineTask::doThrResume();
		trans_bytes = 0;
		comple_key = 0;
		overlapped = 0;
		g_setlasterr(0);

		BOOL ret = g_getqueuedcompletionstatus(simpl._handler, &trans_bytes,
			&comple_key, &overlapped, 20);
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
		CoroutineTask::doTask(_DoExecCoOp, task);
		if (simpl->_taskvec.size() < 1024)
			simpl->_taskvec.push_back(task);
		else
			free(task);
	}
}

inline void IocpService::Access::_DoExecCoOp(void* param) {
	CoEventTask* task = (CoEventTask*)param;
	_DoExecOp((IocpService*)task->service, task->op, task->tb, task->ok);
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
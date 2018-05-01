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

using namespace coroutine;

M_SOCKET_NAMESPACE_BEGIN

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
	else {
		epoll_event_t event;
		event.events = events;
		event.data.ptr = opset;
		s_int32_t ret = g_epoll_ctl(M_Impl_Epoll(impl), flag, M_Impl_Fd(impl), &event);
		M_DEFAULT_SOCKET_ERROR(ret != 0, error);
	}
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

//////////////////////////////////////////////////////////////////////////////////////////

inline void EpollService::Access::_DoRun(EpollService& service, IoServiceImpl& simpl, bool isco,
	SocketLib::SocketError& error) 
{
	function_t<void()>& runhandler = service.GetRunCallback();
	base::slist<SocketClose*> closes1;
	base::slist<SocketClose*> closes2;
	const int max_events = 128;
	epoll_event_t events[max_events];
	for (;;) {
		if (runhandler)
			runhandler();
		_DoClose(&simpl, closes1, closes2);
		if (isco)
			CoroutineTask::doThrResume();
		g_setlasterr(0);
		g_bzero(&events, sizeof(events));
		s_int32_t ret = g_epoll_wait(simpl._handler, events, max_events, 20);
		if (ret == 0) {
			// time out
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
			EpollService::Access::GetOption(*serviceimpl.GetService(), 
				this->_acpt_impl, error_opt, error);
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
		// if (event->events & M_EPOLLERR) {
			Opts::SoError error_opt;
			EpollService::Access::GetOption(*serviceimpl.GetService(), 
				this->_impl, error_opt, error);
			error = error ? SocketError(M_ERR_ASYNC_CONNECT_FAIL) : SocketError(error_opt.Value());
		//}

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
		 	EpollService::Access::CtlEpoll((EpollService&)*serviceimpl.GetService(), 
				this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);
		}
		mlock.unlock();

		s_int32_t ret = g_send(M_Impl_Fd(this->_impl), this->_wsabuf.buf
			, (s_uint32_t)this->_wsabuf.len, MSG_NOSIGNAL);
		if (ret <= 0) {
			Opts::SoError error_opt;
			EpollService::Access::GetOption((EpollService&)*serviceimpl.GetService(), 
				this->_impl, error_opt, error);
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
			EpollService::Access::CtlEpoll((EpollService&)*serviceimpl.GetService(), 
				this->_impl, opset, M_EPOLL_CTL_MOD, flag, error);
		}
		mlock.unlock();

		s_int32_t ret = g_recv(M_Impl_Fd(this->_impl), this->_wsabuf.buf
			, (s_uint32_t)this->_wsabuf.len, 0);
		if (ret <= 0)
		{
			Opts::SoError error_opt;
			EpollService::Access::GetOption((EpollService&)*serviceimpl.GetService(), 
				this->_impl, error_opt, error);
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
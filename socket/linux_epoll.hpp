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

#ifndef M_LINUX_EPOLL_INCLUDE
#define M_LINUX_EPOLL_INCLUDE

#include "config.hpp"
#include <map>
#include <vector>
#include <algorithm>
#ifndef M_PLATFORM_WIN
M_SOCKET_NAMESPACE_BEGIN

class EpollService
{
public:
	class  Access;
	struct Impl;
	struct ImplCloseReq;

	struct IoServiceImpl {
		friend class Access;
		M_SOCKET_DECL IoServiceImpl(EpollService& service);
		M_SOCKET_DECL EpollService& GetService();

	private:
		EpollService&   _service;
		s_int32_t		_handler;
		s_uint32_t		_fdcnt;
		MutexLock	    _mutex;
		slist<ImplCloseReq*> _closereqs;
		slist<ImplCloseReq*> _closereqs2;
	};
	typedef std::vector<IoServiceImpl*> IoServiceImplVector;
	typedef std::map<s_int32_t, IoServiceImpl*> IoServiceImplMap;

	struct Oper {
		virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event) = 0;
		virtual void Clear() = 0;
		virtual ~Oper() {}
	};

	struct Operation{
		M_SOCKET_DECL Operation();
		M_SOCKET_DECL ~Operation();
		Oper* _oper;
	};

	struct OperationSet{
		s_uint8_t  _type;
		Operation _aop;
		Operation _cop;
		Operation _wop;
		Operation _rop;
	};

	struct AcceptOperation2;
	struct ConnectOperation2;
	struct WriteOperation2;
	struct ReadOperation2;

	M_SOCKET_DECL EpollService();

	M_SOCKET_DECL ~EpollService();

	M_SOCKET_DECL void Run();

	M_SOCKET_DECL void Run(SocketError& error);

	M_SOCKET_DECL void Stop();

	M_SOCKET_DECL void Stop(SocketError& error);

	M_SOCKET_DECL bool Stopped()const;

	M_SOCKET_DECL s_int32_t ServiceCount()const;

protected:
	EpollService(const EpollService&);
	EpollService& operator=(const EpollService&);

private:
	IoServiceImplVector _implvector;
	IoServiceImplMap	_implmap;
	s_int32_t			_implcnt;
	s_int32_t			_implidx;
	mutable MutexLock	_mutex;
};

M_SOCKET_DECL EpollService::Operation::Operation() :_oper(0){
}

M_SOCKET_DECL EpollService::Operation::~Operation() {
	delete _oper;
	_oper = 0;
}

struct EpollService::Impl {
	friend struct AcceptOperation2;
	friend struct ConnectOperation2;
	friend struct WriteOperation2;
	friend struct ReadOperation2;
	friend class Access;

	struct core {
		s_int32_t	 _epoll;
		socket_t	 _fd;
		s_uint16_t   _state;
		OperationSet _op;
		MutexLock    _mutex;
	};
	template<int N>
	void Init();
private:
	shard_ptr_t<core> _core;
};

struct EpollService::AcceptOperation2 : public EpollService::Oper {
	M_SOCKET_DECL virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event);
	M_SOCKET_DECL virtual void Clear();
	Impl  _acpt_impl;
	Impl  _cli_impl;
	function_t <void(SocketError)>
		_handler;
};

struct EpollService::ConnectOperation2 : public EpollService::Oper {
	M_SOCKET_DECL virtual bool Complete(IoServiceImpl& serviceimpl, epoll_event_t* event);
	M_SOCKET_DECL virtual void Clear();
	Impl  _impl;
	function_t <void(SocketError)>
		  _handler;
};

struct EpollService::WriteOperation2 : public EpollService::Oper {
	M_SOCKET_DECL virtual bool Complete(IoServiceImpl& serviceimpl, epoll_event_t* event);
	M_SOCKET_DECL virtual void Clear();
	wsabuf_t _wsabuf;
	Impl     _impl;
	function_t <void(s_uint32_t, SocketError)>
		     _handler;
};

struct EpollService::ReadOperation2 : public EpollService::Oper {
	M_SOCKET_DECL virtual bool Complete(IoServiceImpl& serviceimpl, epoll_event_t* event);
	M_SOCKET_DECL virtual void Clear();
	wsabuf_t _wsabuf;
	Impl     _impl;
	function_t <void(s_uint32_t, SocketError)>
			 _handler;
};

template<int N>
void EpollService::Impl::Init() {
	if (!_core) {
		_core.reset(new core);
		_core->_epoll = -1;
		_core->_fd = M_INVALID_SOCKET;
		_core->_state = 0;
		_core->_op._type = 0;
		_core->_op._aop._oper = new AcceptOperation2;
		_core->_op._cop._oper = new ConnectOperation2;
		_core->_op._wop._oper = new WriteOperation2;
		_core->_op._rop._oper = new ReadOperation2;
	}
}

struct EpollService::ImplCloseReq {
	Impl _impl;
	function_t<void()> _handler;
	void Clear() {
		_handler = 0;
		_impl = Impl();
	}
};

class EpollService::Access
{
public:
	M_SOCKET_DECL static void ConstructImpl(EpollService& service, Impl& impl, s_uint16_t type);

	M_SOCKET_DECL static void DestroyImpl(EpollService& service, Impl& impl);

	M_SOCKET_DECL static bool IsOpen(EpollService& service, Impl& impl, SocketError& error);

	template<typename GettableOptionType>
	M_SOCKET_DECL static void GetOption(EpollService& service, Impl& impl, GettableOptionType& opt, SocketError& error);

	template<typename SettableOptionType>
	M_SOCKET_DECL static void SetOption(EpollService& service, Impl& impl, const SettableOptionType& opt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint RemoteEndPoint(EndPoint, EpollService& service, const Impl& impl, SocketError& error);
	
	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint LocalEndPoint(EndPoint, EpollService& service, const Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Cancel(EpollService& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void CreateEpoll(IoServiceImpl& impl, SocketError& error);

	M_SOCKET_DECL static void DestroyEpoll(IoServiceImpl& impl);

	M_SOCKET_DECL static void CtlEpoll(EpollService& service, Impl& impl, EpollService::OperationSet* opset, s_int32_t flag, s_int32_t events, SocketError& error);

	M_SOCKET_DECL static void ExecOp(IoServiceImpl& serviceimpl, EpollService::OperationSet* op, epoll_event_t* event);

	M_SOCKET_DECL static void Run(EpollService& service, SocketError& error);

	M_SOCKET_DECL static void Stop(EpollService& service, SocketError& error);

	M_SOCKET_DECL static bool Stopped(const EpollService& service);

	M_SOCKET_DECL static s_uint32_t GetServiceCount(const EpollService& service);

////////////////
	M_SOCKET_DECL static void Close(EpollService& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Close(EpollService& service, Impl& impl, function_t<void()> handler
		, SocketError& error);

	template<typename ProtocolType>
	M_SOCKET_DECL static void Open(EpollService& service, Impl& impl, ProtocolType pt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Bind(EpollService& service, Impl& impl, const EndPoint& ep, SocketError& error);

	M_SOCKET_DECL static void Listen(EpollService& service, Impl& impl, s_int32_t flag, SocketError& error);

	M_SOCKET_DECL static void Shutdown(EpollService& service, Impl& impl, EShutdownType what, SocketError& error);

	M_SOCKET_DECL static void Accept(EpollService& service, Impl& impl, Impl& peer, SocketError& error);

	M_SOCKET_DECL static void AsyncAccept(EpollService& service, Impl& accept_impl, Impl& sock_impl
		, const M_COMMON_HANDLER_TYPE(IocpService2)& handler, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Connect(EpollService& service, Impl& impl, const EndPoint& ep, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void AsyncConnect(EpollService& service, Impl& impl, const EndPoint& ep
		, const M_COMMON_HANDLER_TYPE(EpollService)& handler, SocketError& error);

	M_SOCKET_DECL static s_int32_t RecvSome(EpollService& service, Impl& impl
		, s_byte_t* data, s_uint32_t size, SocketError& error);

	M_SOCKET_DECL static void AsyncRecvSome(EpollService& service, Impl& impl, s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error);

	M_SOCKET_DECL static s_int32_t SendSome(EpollService& service, Impl& impl
		, const s_byte_t* data, s_uint32_t size, SocketError& error);

	M_SOCKET_DECL static void AsyncSendSome(EpollService& service, Impl& impl, const s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error);

protected:
	M_SOCKET_DECL static EpollService::IoServiceImpl* _GetIoServiceImpl(EpollService& service, Impl& impl);

	M_SOCKET_DECL static void _DoClose(IoServiceImpl* simpl
		, slist<ImplCloseReq*>&closereqs, slist<ImplCloseReq*>&closereqs2);
};

M_SOCKET_DECL EpollService::IoServiceImpl::IoServiceImpl(EpollService& service)
	:_service(service), _handler(-1), _fdcnt(0){
}

M_SOCKET_DECL EpollService& EpollService::IoServiceImpl::GetService(){
	return _service;
}

M_SOCKET_DECL EpollService::EpollService()
	: _implcnt(0), _implidx(0){
}

M_SOCKET_DECL EpollService::~EpollService(){
	SocketError error;
	Stop(error);
	while (ServiceCount()){
		g_sleep(1);
	}
	ScopedLock scoped(_mutex);
	for (IoServiceImplMap::iterator iter = _implmap.begin(); iter != _implmap.end(); ++iter) {
		IoServiceImpl& impl = (IoServiceImpl&)(*iter->second);
		Access::DestroyEpoll(impl);
	}
	_implmap.clear();
	_implvector.clear();
}

M_SOCKET_DECL void EpollService::Run(){
	SocketError error;
	this->Run(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void EpollService::Run(SocketError& error){
	Access::Run(*this, error);
}

M_SOCKET_DECL void EpollService::Stop(){
	SocketError error;
	this->Stop(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void EpollService::Stop(SocketError& error){
	Access::Stop(*this, error);
}

M_SOCKET_DECL bool EpollService::Stopped()const{
	return Access::Stopped(*this);
}

M_SOCKET_DECL s_int32_t EpollService::ServiceCount()const{
	return Access::GetServiceCount(*this);
}

#include "linuxsock_init.hpp"
#include "epoll_access.hpp"
M_SOCKET_NAMESPACE_END
#endif
#endif

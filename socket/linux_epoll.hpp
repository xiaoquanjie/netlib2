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
	friend class Access;

	struct IoServiceImpl
	{
		friend class Access;
		M_SOCKET_DECL IoServiceImpl(EpollService& service);
		M_SOCKET_DECL EpollService& GetService();

	private:
		EpollService&		_service;
		s_int32_t			_handler;
		s_uint32_t			_fdcnt;
	};
	typedef std::vector<IoServiceImpl*> IoServiceImplVector;
	typedef std::map<s_int32_t, IoServiceImpl*> IoServiceImplMap;

	struct Operation
	{
		virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event) = 0;
		virtual void Clear() = 0;
		virtual ~Operation() {}
	};

	template<typename Handler>
	struct AcceptOperation : public Operation {
		Handler    _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event);
		M_SOCKET_DECL virtual void Clear();
	};

	template<typename Handler>
	struct ConnectOperation : public Operation {
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event);
		M_SOCKET_DECL virtual void Clear();
	};

	template<typename Handler>
	struct WriteOperation : public Operation {
		wsabuf_t _wsabuf;
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event);
		M_SOCKET_DECL virtual void Clear();
	};

	template<typename Handler>
	struct ReadOperation : public Operation {
		wsabuf_t _wsabuf;
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event);
		M_SOCKET_DECL virtual void Clear();
	};

	struct FinishOperation : public Operation {
		s_int32_t _fd;
		M_SOCKET_DECL virtual bool Complete(EpollService::IoServiceImpl& serviceimpl, epoll_event_t* event);
		M_SOCKET_DECL virtual void Clear();
	};

	struct OperationSet
	{
		s_uint8_t  _type;
		Operation* _accept_op;
		Operation* _connect_op;
		Operation* _write_op;
		Operation* _read_op;
	};

	template<typename T>
	struct OperationAlloc
	{
		M_SOCKET_DECL static void Alloc(OperationSet* opset, s_int32_t type);
	};

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

struct EpollService::Impl
{
	friend class Access;
	template<typename T>
	friend struct AcceptOperation;
	template<typename T>
	friend struct ConnectOperation;
	template<typename T>
	friend struct WriteOperation;
	template<typename T>
	friend struct ReadOperation;

	struct core{
		s_int32_t	 _epoll;
		socket_t	 _fd;
		s_uint16_t   _state;
		OperationSet _operation;
	};

	Impl(){
		_core = shard_ptr_t<core>(new core);
	}

private:
	shard_ptr_t<core> _core;
};

class EpollService::Access
{
public:
	M_SOCKET_DECL static void Construct(EpollService& service, EpollService::Impl& impl, s_uint16_t type);

	M_SOCKET_DECL static void Destroy(EpollService& service, EpollService::Impl& impl);

	M_SOCKET_DECL static void Close(EpollService& service, EpollService::Impl& impl, SocketError& error);

	M_SOCKET_DECL static bool IsOpen(EpollService& service, EpollService::Impl& impl, SocketError& error);

	template<typename GettableOptionType>
	M_SOCKET_DECL static void GetOption(EpollService& service, EpollService::Impl& impl, GettableOptionType& opt, SocketError& error);

	template<typename SettableOptionType>
	M_SOCKET_DECL static void SetOption(EpollService& service, EpollService::Impl& impl, const SettableOptionType& opt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint RemoteEndPoint(EndPoint, EpollService& service, const EpollService::Impl& impl, SocketError& error);
	
	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint LocalEndPoint(EndPoint, EpollService& service, const EpollService::Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Shutdown(EpollService& service, EpollService::Impl& impl, EShutdownType what, SocketError& error);

	template<typename ProtocolType>
	M_SOCKET_DECL static void Open(EpollService& service, EpollService::Impl& impl, ProtocolType pt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Bind(EpollService& service, EpollService::Impl& impl, const EndPoint& ep, SocketError& error);

	M_SOCKET_DECL static void Cancel(EpollService& service, EpollService::Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Listen(EpollService& service, EpollService::Impl& impl, s_int32_t flag, SocketError& error);

	M_SOCKET_DECL static void Accept(EpollService& service, EpollService::Impl& impl, Impl& peer, SocketError& error);

	template<typename AcceptHandler>
	M_SOCKET_DECL static void AsyncAccept(EpollService& service, M_HANDLER_SOCKET_PTR(AcceptHandler) accept_ptr, AcceptHandler handler, SocketError& error);

	template<typename AcceptHandler>
	M_SOCKET_DECL static void AsyncAccept(EpollService& service, Impl& accept_impl, Impl& sock_impl, AcceptHandler handler, SocketError& error);

	M_SOCKET_DECL static s_int32_t RecvSome(EpollService& service, EpollService::Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error);

	M_SOCKET_DECL static s_int32_t SendSome(EpollService& service, EpollService::Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error);

	template<typename ReadHandler>
	M_SOCKET_DECL static void AsyncRecvSome(EpollService& service, M_HANDLER_SOCKET_PTR(ReadHandler) socket_ptr, s_byte_t* data, s_uint32_t size, ReadHandler hander, SocketError& error);

	M_SOCKET_DECL static void AsyncRecvSome(EpollService& service, Impl& impl, s_byte_t* data, s_uint32_t size, M_RW_HANDLER_TYPE(EpollService) hander, SocketError& error);

	template<typename WriteHandler>
	M_SOCKET_DECL static void AsyncSendSome(EpollService& service, M_HANDLER_SOCKET_PTR(WriteHandler) socket_ptr, const s_byte_t* data, s_uint32_t size, WriteHandler hander, SocketError& error);

	M_SOCKET_DECL static void AsyncSendSome(EpollService& service, Impl& impl, const s_byte_t* data, s_uint32_t size, M_RW_HANDLER_TYPE(EpollService) hander, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Connect(EpollService& service, EpollService::Impl& impl, const EndPoint& ep, SocketError& error);

	template<typename ConnectHandler, typename EndPoint>
	M_SOCKET_DECL static void AsyncConnect(EpollService& service, M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void AsyncConnect(EpollService& service, Impl& impl, const EndPoint& ep, M_COMMON_HANDLER_TYPE(EpollService) handler, SocketError& error);

	M_SOCKET_DECL static void ExecOp(EpollService::IoServiceImpl& serviceimpl, EpollService::OperationSet* op, epoll_event_t* event);

	M_SOCKET_DECL static void Run(EpollService& service, SocketError& error);

	M_SOCKET_DECL static void Stop(EpollService& service, SocketError& error);

	M_SOCKET_DECL static bool Stopped(const EpollService& service);

	template<typename SOCKET_TYPE>
	M_SOCKET_DECL static void SetImplState(SOCKET_TYPE& st, s_uint8_t state);

	M_SOCKET_DECL static void CreateEpoll(EpollService::IoServiceImpl& impl, SocketError& error);

	M_SOCKET_DECL static void DestroyEpoll(EpollService::IoServiceImpl& impl);

	M_SOCKET_DECL static void CtlEpoll(EpollService& service, Impl& impl, EpollService::OperationSet* opset, s_int32_t flag, s_int32_t events, SocketError& error);

	M_SOCKET_DECL static EpollService::IoServiceImpl* GetIoServiceImpl(EpollService& service, Impl& impl);

	M_SOCKET_DECL static s_uint32_t GetServiceCount(const EpollService& service);
};

M_SOCKET_DECL EpollService::IoServiceImpl::IoServiceImpl(EpollService& service)
	:_service(service), _handler(-1), _fdcnt(0)
{
}

M_SOCKET_DECL EpollService& EpollService::IoServiceImpl::GetService()
{
	return _service;
}

M_SOCKET_DECL EpollService::EpollService()
	: _implcnt(0), _implidx(0)
{
}

M_SOCKET_DECL EpollService::~EpollService()
{
	ScopedLock scoped(_mutex);
	SocketError error;
	Stop(error);
	for (IoServiceImplVector::iterator iter=_implvector.begin(); iter!=_implvector.end(); ++iter)
	{
		IoServiceImpl& impl = (IoServiceImpl&)**iter;
		Access::DestroyEpoll(impl);
	}
	_implmap.clear();
	_implvector.clear();
}

M_SOCKET_DECL void EpollService::Run()
{
	SocketError error;
	this->Run(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void EpollService::Run(SocketError& error)
{
	Access::Run(*this, error);
}

M_SOCKET_DECL void EpollService::Stop()
{
	SocketError error;
	this->Stop(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void EpollService::Stop(SocketError& error)
{
	Access::Stop(*this, error);
}

M_SOCKET_DECL bool EpollService::Stopped()const
{
	return Access::Stopped(*this);
}

M_SOCKET_DECL s_int32_t EpollService::ServiceCount()const
{
	return Access::GetServiceCount(*this);
}

#include "linuxsock_init.hpp"
#include "epoll_access.hpp"
M_SOCKET_NAMESPACE_END
#endif
#endif


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

#ifndef M_WIN_IOCP2_INCLUDE
#define M_WIN_IOCP2_INCLUDE

#include "config.hpp"
#include <map>
#include <vector>
#include <algorithm>
#ifdef M_PLATFORM_WIN
M_SOCKET_NAMESPACE_BEGIN

class IocpService2
{
public:
	class  Access;
	struct Impl;
	friend class Access;

	struct IoServiceImpl
	{
		friend class Access;
		M_SOCKET_DECL IoServiceImpl(IocpService2& service);
		M_SOCKET_DECL IocpService2& GetService();
	
	private:
		IocpService2&	_service;
		HANDLE			_handler;
		s_uint32_t		_fdcnt;
	};
	typedef std::vector<IoServiceImpl*> IoServiceImplVector;
	typedef std::map<HANDLE, IoServiceImpl*> IoServiceImplMap;

	struct Oper
	{
		virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error) = 0;
		virtual ~Oper() {}
	};

	struct Operation : public wsaoverlapped_t
	{
		s_uint8_t _type;
		Oper* _oper;
		M_SOCKET_DECL Operation();
		M_SOCKET_DECL ~Operation();
	};

	template<typename Handler>
	struct AcceptOperation : public Oper {
		s_byte_t   _buf[sizeof(sockaddr_storage_t) * 2];
		s_uint32_t _bytes;
		Impl	   _impl;
		Handler    _handler;

		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;
		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct ConnectOperation : public Oper {
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct WriteOperation : public Oper {
		wsabuf_t _wsabuf;
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct ReadOperation : public Oper {
		wsabuf_t _wsabuf;
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(IocpService2& service, s_uint32_t transbyte, SocketError& error);
	};

	struct OperationSet
	{
		Operation* _accept_op;
		Operation* _connect_op;
		Operation* _write_op;
		Operation* _read_op;
	};

	template<typename T>
	struct OperationAlloc
	{
		M_SOCKET_DECL static void Alloc(OperationSet* ptr, s_int32_t type);
	};

	M_SOCKET_DECL IocpService2();

	M_SOCKET_DECL ~IocpService2();

	M_SOCKET_DECL void Run();

	M_SOCKET_DECL void Run(SocketError& error);

	M_SOCKET_DECL void Stop();

	M_SOCKET_DECL void Stop(SocketError& error);

	M_SOCKET_DECL bool Stopped()const;

	M_SOCKET_DECL s_int32_t ServiceCount()const;

protected:
	IocpService2(const IocpService2&);
	IocpService2& operator=(const IocpService2&);

private:
	IoServiceImplVector _implvector;
	IoServiceImplMap	_implmap;
	s_int32_t			_implcnt;
	s_int32_t			_implidx;
	mutable MutexLock	_mutex;
};

struct IocpService2::Impl
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

private:
	HANDLE		 _iocp;
	socket_t	 _fd;
	s_uint16_t	 _state;
	OperationSet _operation;
};

class IocpService2::Access
{
public:
	M_SOCKET_DECL static void Construct(IocpService2& service, Impl& impl, s_uint16_t type);

	M_SOCKET_DECL static void Destroy(IocpService2& service, Impl& impl);

	M_SOCKET_DECL static void Close(IocpService2& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static bool IsOpen(IocpService2& service, Impl& impl, SocketError& error);

	template<typename GettableOptionType>
	M_SOCKET_DECL static void GetOption(IocpService2& service, Impl& impl, GettableOptionType& opt, SocketError& error);

	template<typename SettableOptionType>
	M_SOCKET_DECL static void SetOption(IocpService2& service, Impl& impl, const SettableOptionType& opt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint RemoteEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint LocalEndPoint(EndPoint, IocpService2& service, const Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Shutdown(IocpService2& service, Impl& impl, EShutdownType what, SocketError& error);

	template<typename ProtocolType>
	M_SOCKET_DECL static void Open(IocpService2& service, Impl& impl, ProtocolType pt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Bind(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error);

	M_SOCKET_DECL static void Cancel(IocpService2& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Listen(IocpService2& service, Impl& impl, s_int32_t flag, SocketError& error);

	M_SOCKET_DECL static void Accept(IocpService2& service, Impl& impl, Impl& peer, SocketError& error);

	template<typename AcceptHandler>
	M_SOCKET_DECL static void AsyncAccpet(IocpService2& service, M_HANDLER_SOCKET_PTR(AcceptHandler) accept_ptr, AcceptHandler handler, SocketError& error);

	M_SOCKET_DECL static s_int32_t RecvSome(IocpService2& service, Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error);

	M_SOCKET_DECL static s_int32_t SendSome(IocpService2& service, Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error);

	template<typename ReadHandler>
	M_SOCKET_DECL static void AsyncRecvSome(IocpService2& service, M_HANDLER_SOCKET_PTR(ReadHandler) socket_ptr, s_byte_t* data, s_uint32_t size, ReadHandler hander, SocketError& error);

	template<typename WriteHandler>
	M_SOCKET_DECL static void AsyncSendSome(IocpService2& service, M_HANDLER_SOCKET_PTR(WriteHandler) socket_ptr, const s_byte_t* data, s_uint32_t size, WriteHandler hander, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Connect(IocpService2& service, Impl& impl, const EndPoint& ep, SocketError& error);

	template<typename ConnectHandler, typename EndPoint>
	M_SOCKET_DECL static void AsyncConnect(IocpService2& service, M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error);

	M_SOCKET_DECL static void CreateIocp(IocpService2::IoServiceImpl& impl, SocketError& error);

	M_SOCKET_DECL static void DestroyIocp(IocpService2::IoServiceImpl& impl);

	M_SOCKET_DECL static void BindIocp(IocpService2& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void ExecOp(IocpService2& service, IocpService2::Operation* op, s_uint32_t transbyte, bool ok);

	M_SOCKET_DECL static void Run(IocpService2& service, SocketError& error);

	M_SOCKET_DECL static void Stop(IocpService2& service, SocketError& error);

	M_SOCKET_DECL static bool Stopped(const IocpService2& service);

	M_SOCKET_DECL static IocpService2::IoServiceImpl* GetIoServiceImpl(IocpService2& service, Impl& impl);

	M_SOCKET_DECL static s_uint32_t GetServiceCount(const IocpService2& service);
};

M_SOCKET_DECL IocpService2::IoServiceImpl::IoServiceImpl(IocpService2& service)
	:_service(service),_handler(0),_fdcnt(0)
{
}

M_SOCKET_DECL IocpService2& IocpService2::IoServiceImpl::GetService()
{
	return _service;
}

M_SOCKET_DECL IocpService2::IocpService2()
	:_implcnt(0),_implidx(0)
{
}

M_SOCKET_DECL IocpService2::~IocpService2()
{
	ScopedLock scoped(_mutex);
	SocketError error;
	Stop(error);
	for (IoServiceImplVector::iterator iter = _implvector.begin(); iter != _implvector.end(); ++iter)
	{
		IoServiceImpl& impl = (IoServiceImpl&)**iter;
		Access::DestroyIocp(impl);
	}
	_implmap.clear();
	_implvector.clear();
}

M_SOCKET_DECL void IocpService2::Run()
{
	SocketError error;
	this->Run(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void IocpService2::Run(SocketError& error)
{
	Access::Run(*this, error);
}

M_SOCKET_DECL void IocpService2::Stop()
{
	SocketError error;
	this->Stop(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void IocpService2::Stop(SocketError& error)
{
	Access::Stop(*this, error);
}

M_SOCKET_DECL bool IocpService2::Stopped()const
{
	return Access::Stopped(*this);
}

M_SOCKET_DECL s_int32_t IocpService2::ServiceCount()const
{
	return 1;
}

#include "winsock_init.hpp"
#include "iocp_access.hpp"
M_SOCKET_NAMESPACE_END
#endif
#endif

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

#ifndef M_WIN_IOCP_INCLUDE
#define M_WIN_IOCP_INCLUDE

#include "config.hpp"
#ifdef M_PLATFORM_WIN
M_SOCKET_NAMESPACE_BEGIN

class IocpService
{
public:
	class  Access;
	struct Impl;
	friend class Access;

	struct IoServiceImpl
	{
		friend class Access;
		IoServiceImpl():_handler(0), _stopped(1){}
	private:
		HANDLE _handler;
		mutable long _stopped;
		mutable MutexLock _mutex;
	};

	struct Oper
	{
		virtual bool Complete(IocpService& service,s_uint32_t transbyte, SocketError& error) = 0;
		virtual ~Oper(){}
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
		s_byte_t   _buf[sizeof(sockaddr_storage_t)*2];
		s_uint32_t _bytes;
		Impl	   _impl;
		Handler    _handler;

		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;
		M_SOCKET_DECL virtual bool Complete(IocpService& service, s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct ConnectOperation : public Oper {
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(IocpService& service, s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct WriteOperation : public Oper {
		wsabuf_t _wsabuf;
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(IocpService& service, s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct ReadOperation : public Oper {
		wsabuf_t _wsabuf;
		Handler _handler;
		M_HANDLER_SOCKET_PTR(Handler) _socket_ptr;

		M_SOCKET_DECL virtual bool Complete(IocpService& service, s_uint32_t transbyte, SocketError& error);
	};

	template<typename T>
	struct OperationAlloc
	{
		M_SOCKET_DECL static Operation* Alloc(Operation* ptr,s_int32_t type);
	};

	M_SOCKET_DECL IocpService();

	M_SOCKET_DECL ~IocpService();

	M_SOCKET_DECL void Run();

	M_SOCKET_DECL void Run(SocketError& error);

	M_SOCKET_DECL void Stop();

	M_SOCKET_DECL void Stop(SocketError& error);

	M_SOCKET_DECL bool Stopped()const;

	M_SOCKET_DECL s_int32_t ServiceCount()const;

protected:
	IocpService(const IocpService&);
	IocpService& operator=(const IocpService&);

private:
	IoServiceImpl _impl;
};

struct IocpService::Impl
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
	socket_t	_fd;
	s_uint8_t  _state;

	Operation* _accept_op;
	Operation* _connect_op;
	Operation* _write_op;
	Operation* _read_op;

	s_uint8_t _accept_flag;
	s_uint8_t _connect_flag;
	s_uint8_t _read_flag;
	s_uint8_t _write_flag;
};

class IocpService::Access
{
public:
	M_SOCKET_DECL static void Construct(IocpService& service, Impl& impl, s_uint16_t type);

	M_SOCKET_DECL static void Destroy(IocpService& service, Impl& impl);

	M_SOCKET_DECL static void Close(IocpService& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static bool IsOpen(IocpService& service, Impl& impl, SocketError& error);

	template<typename GettableOptionType>
	M_SOCKET_DECL static void GetOption(IocpService& service, Impl& impl, GettableOptionType& opt, SocketError& error);

	template<typename SettableOptionType>
	M_SOCKET_DECL static void SetOption(IocpService& service, Impl& impl, const SettableOptionType& opt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint RemoteEndPoint(EndPoint, IocpService& service, const Impl& impl, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static EndPoint LocalEndPoint(EndPoint, IocpService& service, const Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Shutdown(IocpService& service, Impl& impl, EShutdownType what, SocketError& error);

	template<typename ProtocolType>
	M_SOCKET_DECL static void Open(IocpService& service, Impl& impl, ProtocolType pt, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Bind(IocpService& service, Impl& impl, const EndPoint& ep, SocketError& error);

	M_SOCKET_DECL static void Cancel(IocpService& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void Listen(IocpService& service, Impl& impl, s_int32_t flag, SocketError& error);

	M_SOCKET_DECL static void Accept(IocpService& service, Impl& impl, Impl& peer, SocketError& error);

	template<typename AcceptHandler>
	M_SOCKET_DECL static void AsyncAccpet(IocpService& service, M_HANDLER_SOCKET_PTR(AcceptHandler) accept_ptr, AcceptHandler handler, SocketError& error);

	M_SOCKET_DECL static s_int32_t RecvSome(IocpService& service, Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error);

	M_SOCKET_DECL static s_int32_t SendSome(IocpService& service, Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error);

	template<typename ReadHandler>
	M_SOCKET_DECL static void AsyncRecvSome(IocpService& service, M_HANDLER_SOCKET_PTR(ReadHandler) socket_ptr, s_byte_t* data, s_uint32_t size, ReadHandler hander, SocketError& error);

	template<typename WriteHandler>
	M_SOCKET_DECL static void AsyncSendSome(IocpService& service, M_HANDLER_SOCKET_PTR(WriteHandler) socket_ptr, const s_byte_t* data, s_uint32_t size, WriteHandler hander, SocketError& error);

	template<typename EndPoint>
	M_SOCKET_DECL static void Connect(IocpService& service, Impl& impl, const EndPoint& ep, SocketError& error);

	template<typename ConnectHandler, typename EndPoint>
	M_SOCKET_DECL static void AsyncConnect(IocpService& service, M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error);

	M_SOCKET_DECL static void CreateIocp(IocpService& service, SocketError& error);

	M_SOCKET_DECL static void DestroyIocp(IocpService& service);

	M_SOCKET_DECL static void BindIocp(IocpService& service, Impl& impl, SocketError& error);

	M_SOCKET_DECL static void ExecOp(IocpService& service, IocpService::Operation* op, s_uint32_t transbyte,bool ok);

	M_SOCKET_DECL static void Run(IocpService& service,SocketError& error);

	M_SOCKET_DECL static void Stop(IocpService& service, SocketError& error);

	M_SOCKET_DECL static bool Stopped(const IocpService& service);

};

M_SOCKET_DECL IocpService::IocpService()
{
	SocketError error;
	Access::CreateIocp(*this,error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL IocpService::~IocpService()
{
	SocketError error;
	Stop(error);
	Access::DestroyIocp(*this);
}

M_SOCKET_DECL void IocpService::Run()
{
	SocketError error;
	this->Run(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void IocpService::Run(SocketError& error)
{
	Access::Run(*this, error);
}

M_SOCKET_DECL void IocpService::Stop()
{
	SocketError error;
	this->Stop(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

M_SOCKET_DECL void IocpService::Stop(SocketError& error)
{
	Access::Stop(*this, error);
}

M_SOCKET_DECL bool IocpService::Stopped()const
{
	return Access::Stopped(*this);
}

M_SOCKET_DECL s_int32_t IocpService::ServiceCount()const
{
	return 1;
}

#include "winsock_init.hpp"
#include "win_access.hpp"
M_SOCKET_NAMESPACE_END
#endif
#endif

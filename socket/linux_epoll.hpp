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

#include "socket/config.hpp"
#include <map>
#include <vector>
#include <algorithm>
#include "base/tls.hpp"
#ifndef M_PLATFORM_WIN
M_SOCKET_NAMESPACE_BEGIN

class EpollService;
namespace iodetail {
	struct SocketImpl;
	struct SocketClose;

	struct IoServiceImpl {
		EpollService* _service;
		s_int32_t _handler;
		s_uint32_t _fdcnt;
		MutexLock _mutex;
		base::slist<SocketClose*>
			_closereqs;
		base::slist<SocketClose*>
			_closereqs2;
		IoServiceImpl() {
			_fdcnt = 0;
			_service = 0;
			_handler = g_epoll_create(1);
			assert(_handler >= 0);
		}
		~IoServiceImpl() {
			g_closesocket(_handler);
			_handler = -1;
			_fdcnt = 0;
			_service = 0;
		}
		void Close() {
			g_closesocket(_handler);
			_fdcnt = 0;
			_service = 0;
			_handler = g_epoll_create(1);
			assert(_handler >= 0);
		}
		EpollService* GetService() {
			return _service;
		}
	};

	typedef std::vector<IoServiceImpl*> IoServiceImplVector;
	typedef std::map<s_int32_t, IoServiceImpl*> IoServiceImplMap;

	struct Oper {
		virtual bool Complete(IoServiceImpl& serviceimpl,
			epoll_event_t* event) = 0;
		virtual void Clear() = 0;
		virtual ~Oper() {}
	};

	struct Operation {
		Operation();
		~Operation();
		Oper* _oper;
	};

	struct OperationSet {
		s_uint8_t _type;
		Operation _aop;
		Operation _cop;
		Operation _wop;
		Operation _rop;
	};

	struct SocketImpl {
		struct core {
			s_int32_t _epoll;
			socket_t _fd;
			s_uint16_t _state;
			OperationSet _op;
			MutexLock _mutex;
		};
		void Init();
		shard_ptr_t<core> _core;
	};

	struct AcceptOperation : public Oper {
		SocketImpl _acpt_impl;
		SocketImpl _cli_impl;
		function_t <void(SocketError)>
			_handler;
		virtual bool Complete(IoServiceImpl& serviceimpl,
			epoll_event_t* event);
		virtual void Clear();
	};

	struct ConnectOperation : public Oper {
		SocketImpl _impl;
		function_t <void(SocketError)>
			_handler;
		virtual bool Complete(IoServiceImpl& serviceimpl,
			epoll_event_t* event);
		virtual void Clear();
	};

	struct WriteOperation : public Oper {
		wsabuf_t _wsabuf;
		SocketImpl _impl;
		function_t <void(s_uint32_t, SocketError)>
			_handler;
		virtual bool Complete(IoServiceImpl& serviceimpl,
			epoll_event_t* event);
		virtual void Clear();
	};

	struct ReadOperation : public Oper {
		wsabuf_t _wsabuf;
		SocketImpl _impl;
		function_t <void(s_uint32_t, SocketError)>
			_handler;
		virtual bool Complete(IoServiceImpl& serviceimpl,
			epoll_event_t* event);
		virtual void Clear();
	};

	struct SocketClose {
		SocketImpl _impl;
		function_t<void()> _handler;
		void Clear() {
			_handler = 0;
			_impl = SocketImpl();
		}
	};
}

#ifndef AptOpType
#define AptOpType iodetail::AcceptOperation
#endif
#ifndef RdOpType 
#define RdOpType iodetail::ReadOperation
#endif
#ifndef WrOpType
#define WrOpType iodetail::WriteOperation
#endif
#ifndef CoOpType
#define CoOpType iodetail::ConnectOperation
#endif

class EpollService
{
public:
	typedef iodetail::IoServiceImplVector IoServiceImplVector;
	typedef iodetail::IoServiceImplMap IoServiceImplMap;
	typedef iodetail::IoServiceImpl IoServiceImpl;
	typedef iodetail::SocketImpl SocketImpl;
	typedef iodetail::Operation Operation;
	typedef iodetail::SocketClose SocketClose;
	typedef iodetail::OperationSet OperationSet;
	typedef iodetail::SocketImpl Impl;

	class  Access;

	EpollService();

	~EpollService();

	void Run();

	void Run(SocketError& error);

	void Stop();

	void Stop(SocketError& error);

	bool Stopped()const;

	s_int32_t ServiceCount()const;

	static IoServiceImpl& GetServiceImpl();

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class EpollService::Access
{
public:
	static void ConstructImpl(EpollService& service, SocketImpl& impl, s_uint16_t type);

	static void DestroyImpl(EpollService& service, SocketImpl& impl);

	static bool IsOpen(EpollService& service, SocketImpl& impl, SocketError& error);

	template<typename GettableOptionType>
	static void GetOption(EpollService& service, SocketImpl& impl, GettableOptionType& opt, SocketError& error);

	template<typename SettableOptionType>
	static void SetOption(EpollService& service, SocketImpl& impl, const SettableOptionType& opt, SocketError& error);

	template<typename EndPoint>
	static EndPoint RemoteEndPoint(EndPoint, EpollService& service, const SocketImpl& impl, SocketError& error);

	template<typename EndPoint>
	static EndPoint LocalEndPoint(EndPoint, EpollService& service, const SocketImpl& impl, SocketError& error);

	static void Cancel(EpollService& service, SocketImpl& impl, SocketError& error);

	static void CtlEpoll(EpollService& service, SocketImpl& impl, EpollService::OperationSet* opset, s_int32_t flag, s_int32_t events, SocketError& error);

	static void ExecOp(IoServiceImpl& serviceimpl, EpollService::OperationSet* op, epoll_event_t* event);

	static void Run(EpollService& service, SocketError& error);

	static void Stop(EpollService& service, SocketError& error);

	static bool Stopped(const EpollService& service);

	static s_uint32_t GetServiceCount(const EpollService& service);

	////////////////
	static void Close(EpollService& service, SocketImpl& impl, SocketError& error);

	static void Close(EpollService& service, SocketImpl& impl, function_t<void()> handler
		, SocketError& error);

	template<typename ProtocolType>
	static void Open(EpollService& service, SocketImpl& impl, ProtocolType pt, SocketError& error);

	template<typename EndPoint>
	static void Bind(EpollService& service, SocketImpl& impl, const EndPoint& ep, SocketError& error);

	static void Listen(EpollService& service, SocketImpl& impl, s_int32_t flag, SocketError& error);

	static void Shutdown(EpollService& service, SocketImpl& impl, EShutdownType what, SocketError& error);

	static void Accept(EpollService& service, SocketImpl& impl, SocketImpl& peer, SocketError& error);

	static void AsyncAccept(EpollService& service, SocketImpl& accept_impl, SocketImpl& sock_impl
		, const M_COMMON_HANDLER_TYPE(IocpService2)& handler, SocketError& error);

	template<typename EndPoint>
	static void Connect(EpollService& service, SocketImpl& impl, const EndPoint& ep, 
		SocketError& error, s_uint32_t timeo_sec);

	template<typename EndPoint>
	static void AsyncConnect(EpollService& service, SocketImpl& impl, const EndPoint& ep
		, const M_COMMON_HANDLER_TYPE(EpollService)& handler, SocketError& error);

	static s_int32_t RecvSome(EpollService& service, SocketImpl& impl
		, s_byte_t* data, s_uint32_t size, SocketError& error);

	static void AsyncRecvSome(EpollService& service, SocketImpl& impl, s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error);

	static s_int32_t SendSome(EpollService& service, SocketImpl& impl
		, const s_byte_t* data, s_uint32_t size, SocketError& error);

	static void AsyncSendSome(EpollService& service, SocketImpl& impl, const s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(EpollService)& hander, SocketError& error);

	// -1 == time out,0 == ok,other == error
	static s_int32_t Select(SocketImpl& impl, bool rd_or_wr, s_uint32_t timeo_sec, SocketError& error);

protected:
	static IoServiceImpl* _GetIoServiceImpl(EpollService& service, SocketImpl& impl);

	static void _DoClose(IoServiceImpl* simpl
		, base::slist<SocketClose*>&closereqs, base::slist<SocketClose*>&closereqs2);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline EpollService::EpollService()
	: _implcnt(0), _implidx(0) {
}

inline EpollService::~EpollService() {
	SocketError error;
	Stop(error);
	while (ServiceCount()) {
		g_sleep(1);
	}
	ScopedLock scoped(_mutex);
	_implmap.clear();
	_implvector.clear();
}

inline void EpollService::Run() {
	SocketError error;
	this->Run(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

inline void EpollService::Run(SocketError& error) {
	Access::Run(*this, error);
}

inline void EpollService::Stop() {
	SocketError error;
	this->Stop(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

inline void EpollService::Stop(SocketError& error) {
	Access::Stop(*this, error);
}

inline bool EpollService::Stopped()const {
	return Access::Stopped(*this);
}

inline s_int32_t EpollService::ServiceCount()const {
	return Access::GetServiceCount(*this);
}

inline EpollService::IoServiceImpl& EpollService::GetServiceImpl() {
	return base::tlsdata<EpollService::IoServiceImpl>::data();
}

#include "socket/linuxsock_init.hpp"
M_SOCKET_NAMESPACE_END
#endif
#endif

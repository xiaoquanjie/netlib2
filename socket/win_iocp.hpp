
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

#include "socket/config.hpp"
#include <map>
#include <vector>
#include <algorithm>
#include "base/tls.hpp"
#ifdef M_PLATFORM_WIN
M_SOCKET_NAMESPACE_BEGIN

class IocpService;

namespace iodetail {
	struct SocketImpl;
	struct SocketClose;
	struct Operation;

	struct CoEventTask {
		IocpService* service;
		Operation* op;
		s_uint32_t tb;
		bool ok;
	};

	struct IoServiceImpl {
		IocpService* _service;
		HANDLE _handler;
		s_uint32_t _fdcnt;
		MutexLock _mutex;
		base::slist<SocketClose*>
			_closereqs;
		base::slist<SocketClose*>
			_closereqs2;
		base::svector<CoEventTask*>
			_taskvec;
		IoServiceImpl() {
			_fdcnt = 0;
			_service = 0;
			_handler = g_createiocompletionport(INVALID_HANDLE_VALUE, 0, 0, 0);
			assert(_handler != 0);
		}
		~IoServiceImpl() {
			g_closehandle(_handler);
			_handler = 0;
			_fdcnt = 0;
			_service = 0;
		}
		void Close() {
			g_closehandle(_handler);
			_fdcnt = 0;
			_service = 0;
			_handler = g_createiocompletionport(INVALID_HANDLE_VALUE, 0, 0, 0);
			assert(_handler != 0);
		}
		IocpService* GetService() {
			return _service;
		}
	};

	typedef std::vector<IoServiceImpl*> IoServiceImplVector;
	typedef std::map<HANDLE, IoServiceImpl*> IoServiceImplMap;

	struct Oper {
		virtual bool Complete(
			IocpService& service,
			s_uint32_t transbyte,
			SocketError& error) = 0;
		virtual void Clear() = 0;
		virtual ~Oper() {}
	};

	struct Operation : public wsaoverlapped_t {
		s_uint8_t _type;
		Oper* _oper;
		Operation() {
			_oper = 0;
			_type = E_NULL_OP;
		}
		~Operation() {
			delete _oper;
			_oper = 0;
		}
	};

	struct OperationSet {
		Operation _aop;
		Operation _cop;
		Operation _wop;
		Operation _rop;
	};

	template<typename Handler>
	struct AcceptOperation : public Oper {
		s_byte_t _buf[sizeof(sockaddr_storage_t) * 2];
		s_uint32_t _bytes;
		SocketImpl _impl;
		SocketImpl _accept_impl;
		Handler _handler;
		virtual void Clear();
		virtual bool Complete(IocpService& service,
			s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct ConnectOperation : public Oper {
		Handler _handler;
		SocketImpl _impl;
		virtual void Clear();
		virtual bool Complete(IocpService& service,
			s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct WriteOperation : public Oper {
		wsabuf_t _wsabuf;
		Handler _handler;
		SocketImpl _impl;
		virtual void Clear();
		virtual bool Complete(IocpService& service,
			s_uint32_t transbyte, SocketError& error);
	};

	template<typename Handler>
	struct ReadOperation : public Oper {
		wsabuf_t _wsabuf;
		Handler _handler;
		SocketImpl _impl;
		virtual void Clear();
		virtual bool Complete(IocpService& service,
			s_uint32_t transbyte, SocketError& error);
	};

	template<typename T>
	struct OperationAlloc {
		static void AllocOp(Operation& op, s_int32_t type);
	};

	struct SocketImpl {
		struct core {
			HANDLE _iocp;
			socket_t _fd;
			s_uint16_t _state;
			OperationSet _op;
			shard_ptr_t<MutexLock> _mutex;
		};
		SocketImpl();
		void Init();
		shard_ptr_t<core> _core;
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
#ifndef OpAlloc 
#define OpAlloc iodetail::OperationAlloc
#endif

class IocpService
{
public:
	typedef iodetail::IoServiceImplVector IoServiceImplVector;
	typedef iodetail::IoServiceImplMap IoServiceImplMap;
	typedef iodetail::IoServiceImpl IoServiceImpl;
	typedef iodetail::SocketImpl SocketImpl;
	typedef iodetail::Operation Operation;
	typedef iodetail::SocketClose SocketClose;
	typedef iodetail::SocketImpl Impl;
	typedef iodetail::CoEventTask CoEventTask;

	class  Access;

	IocpService();

	~IocpService();

	void Run();

	void Run(SocketError& error);

	void CoRun();

	void CoRun(SocketError& error);

	void Stop();

	void Stop(SocketError& error);

	bool Stopped()const;

	s_int32_t ServiceCount()const;

	static IoServiceImpl& GetServiceImpl();

protected:
	IocpService(const IocpService&);
	IocpService& operator=(const IocpService&);

private:
	IoServiceImplVector _implvector;
	IoServiceImplMap _implmap;
	s_int32_t _implcnt;
	s_int32_t _implidx;
	mutable MutexLock _mutex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

class IocpService::Access
{
public:
	static void ConstructImpl(IocpService& service, SocketImpl& impl, s_uint16_t type);

	static void DestroyImpl(IocpService& service, SocketImpl& impl);

	static bool IsOpen(IocpService& service, SocketImpl& impl, SocketError& error);

	template<typename GettableOptionType>
	static void GetOption(IocpService& service, SocketImpl& impl, GettableOptionType& opt, SocketError& error);

	template<typename SettableOptionType>
	static void SetOption(IocpService& service, SocketImpl& impl, const SettableOptionType& opt, SocketError& error);

	template<typename EndPoint>
	static EndPoint RemoteEndPoint(EndPoint, IocpService& service, const SocketImpl& impl, SocketError& error);

	template<typename EndPoint>
	static EndPoint LocalEndPoint(EndPoint, IocpService& service, const SocketImpl& impl, SocketError& error);

	static void Cancel(IocpService& service, SocketImpl& impl, SocketError& error);

	static void BindIocp(IocpService& service, SocketImpl& impl, SocketError& error);

	static void Run(IocpService& service, SocketError& error);

	static void CoRun(IocpService& service, SocketError& error);

	static void Stop(IocpService& service, SocketError& error);

	static bool Stopped(const IocpService& service);

	static s_uint32_t GetServiceCount(const IocpService& service);

	static void Close(IocpService& service, SocketImpl& impl, SocketError& error);

	static void Close(IocpService& service, SocketImpl& impl, function_t<void()> handler, SocketError& error);

	template<typename ProtocolType>
	static void Open(IocpService& service, SocketImpl& impl, const ProtocolType& pt, SocketError& error);

	template<typename EndPoint>
	static void Bind(IocpService& service, SocketImpl& impl, const EndPoint& ep, SocketError& error);

	static void Listen(IocpService& service, SocketImpl& impl, s_int32_t flag, SocketError& error);

	static void Shutdown(IocpService& service, SocketImpl& impl, EShutdownType what, SocketError& error);

	static void Accept(IocpService& service, SocketImpl& impl, SocketImpl& peer, SocketError& error);

	static void AsyncAccept(IocpService& service, SocketImpl& accept_impl, SocketImpl& client_impl
		, const M_COMMON_HANDLER_TYPE(IocpService)& handler, SocketError& error);

	template<typename EndPoint>
	static void Connect(IocpService& service, SocketImpl& impl, const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec);

	template<typename EndPoint>
	static void AsyncConnect(IocpService& service, SocketImpl& impl, const EndPoint& ep
		, const M_COMMON_HANDLER_TYPE(IocpService)& handler, SocketError& error);

	static s_int32_t RecvSome(IocpService& service, SocketImpl& impl, s_byte_t* data, s_uint32_t size, SocketError& error);

	static void AsyncRecvSome(IocpService& service, SocketImpl& impl, s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(IocpService)& hander, SocketError& error);

	static s_int32_t SendSome(IocpService& service, SocketImpl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error);

	static void AsyncSendSome(IocpService& service, SocketImpl& impl, const s_byte_t* data, s_uint32_t size
		, const M_RW_HANDLER_TYPE(IocpService)& hander, SocketError& error);

	// -1 == time out,0 == ok,other == error
	static s_int32_t Select(SocketImpl& impl, bool rd_or_wr, s_uint32_t timeo_sec, SocketError& error);

protected:
	static IoServiceImpl* _CreateIoImpl(IocpService& service, SocketError& error);

	static void _ReleaseIoImpl(IocpService& service, IoServiceImpl* simpl);

	static void _DoRun(IocpService& service, IoServiceImpl& simpl, bool isco,
		SocketLib::SocketError& error);

	static void _ExecOp(bool isco, IocpService& service, IoServiceImpl* simpl, Operation* operation,
		s_uint32_t tb, bool opstate);

	static void _ExecCoOp(void* param);

	static void _DoExecOp(IocpService* service, Operation* operation,
		s_uint32_t tb, bool opstate);

	static void _DoClose(IoServiceImpl* simpl
		, base::slist<SocketClose*>&closereqs, base::slist<SocketClose*>&closereqs2);

	static IoServiceImpl* _GetIoServiceImpl(IocpService& service, SocketImpl& impl);
};

///////////////////////////////////////////////////////////////////////////////////////////////////

inline IocpService::IocpService()
	:_implcnt(0), _implidx(0) {
}

inline IocpService::~IocpService() {
	SocketError error;
	Stop(error);
	while (ServiceCount()) {
		g_sleep(200);
	}
	ScopedLock scoped(_mutex);
	_implmap.clear();
	_implvector.clear();
}

inline void IocpService::Run() {
	SocketError error;
	this->Run(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

inline void IocpService::Run(SocketError& error) {
	Access::Run(*this, error);
}

inline void IocpService::CoRun() {
	SocketError error;
	this->CoRun(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

inline void IocpService::CoRun(SocketError& error) {
	Access::CoRun(*this, error);
}

inline void IocpService::Stop() {
	SocketError error;
	this->Stop(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

inline void IocpService::Stop(SocketError& error) {
	Access::Stop(*this, error);
}

inline bool IocpService::Stopped()const {
	return Access::Stopped(*this);
}

inline s_int32_t IocpService::ServiceCount()const {
	return Access::GetServiceCount(*this);
}

inline IocpService::IoServiceImpl& IocpService::GetServiceImpl() {
	return base::tlsdata<IocpService::IoServiceImpl>::data();
}

#include "socket/winsock_init.hpp"
M_SOCKET_NAMESPACE_END
#endif
#endif

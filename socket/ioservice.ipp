
struct IIoService {
	typedef std::vector<iodetail::IoServiceImpl*>
		IoServiceImplVector;
	#ifdef M_PLATFORM_WIN
	typedef std::map<HANDLE, iodetail::IoServiceImpl*>
		IoServiceImplMap;
	#else
	typedef std::map<s_int32_t, iodetail::IoServiceImpl*> 
		IoServiceImplMap;
	#endif

	friend struct IoAccess;

	IIoService() {
		_implcnt = 0;
		_implidx = 0;
	}

	virtual void Run() = 0;

	virtual void Run(SocketError&) = 0;

	virtual void CoRun() = 0;

	virtual void CoRun(SocketError&) = 0;

	virtual void Stop() = 0;

	virtual void Stop(SocketError&) = 0;

	virtual bool Stopped()const = 0;

	virtual s_int32_t ServiceCount()const = 0;

	virtual iodetail::IoServiceImpl& GetServiceImpl() = 0;

	void SetRunCallback(function_t<void()> handler) {
		_runhandler = handler;
	}

	function_t<void()>& GetRunCallback() {
		return _runhandler;
	}

protected:
	IIoService(const IIoService&);
	IIoService& operator=(const IIoService&);

protected:
	IoServiceImplVector 
		_implvector;
	IoServiceImplMap 
		_implmap;
	s_int32_t _implcnt;
	s_int32_t _implidx;
	mutable MutexLock _mutex;
	function_t<void()> _runhandler;
};

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
	(impl._core->_mutex)

#else

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
#define M_Impl_Epoll(impl)\
	impl._core->_epoll
#define M_Impl_Op(impl)\
	impl._core->_op
#define M_Impl_Aop(impl)\
	impl._core->_op._aop._oper
#define M_Impl_Cop(impl)\
	impl._core->_op._cop._oper
#define M_Impl_Rop(impl)\
	impl._core->_op._rop._oper
#define M_Impl_Wop(impl)\
	impl._core->_op._wop._oper

#define M_Impl_Mutex(impl)\
	(impl._core->_mutex)

#endif

#ifdef M_PLATFORM_WIN
#define M_Impl_Io M_Impl_Iocp
#else
#define M_Impl_Io M_Impl_Epoll
#endif

#define M_Swap_Handler(type,handler1,handler2)\
	handler1.swap(const_cast<type>(handler2));

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
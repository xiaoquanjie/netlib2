struct IoAccess {
	typedef iodetail::SocketImpl SocketImpl;
	typedef iodetail::IoServiceImpl IoServiceImpl;
	typedef iodetail::SocketClose SocketClose;

	static void ConstructImpl(IIoService& service, SocketImpl& impl, 
			s_uint16_t type){
		impl.Init();
		M_Impl_S_V4(impl);
		M_Impl_S_Unbind(impl);
		M_Impl_S_Block(impl);
		M_Impl_S_Type(impl, (s_uint8_t)type);
	}

	static void DestroyImpl(IIoService& service, SocketImpl& impl){
		ConstructImpl(service, impl, E_NULL_SOCKET_TYPE);
	}

	static bool IsOpen(IIoService& service, SocketImpl& impl, 
			SocketError& error) {
		return (M_Impl_Fd(impl) != M_INVALID_SOCKET);
	}

	template<typename ProtocolType>
	static void Open(IIoService& service, SocketImpl& impl, 
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
	static void Bind(IIoService& service, SocketImpl& impl, 
			const EndPoint& ep, SocketError& error){
		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		s_int32_t ret = g_bind(M_Impl_Fd(impl), ep_access::SockAddr(ep), 
							ep_access::SockAddrLen(ep));
		M_DEFAULT_SOCKET_ERROR(ret != 0, error);
	}

	static void Listen(IIoService& service, SocketImpl& impl, 
			s_int32_t flag, SocketError& error){
		s_int32_t ret = g_listen(M_Impl_Fd(impl), flag);
		M_DEFAULT_SOCKET_ERROR(ret != 0, error);
	}
	
	template<typename GettableOptionType>
	static void GetOption(IIoService& service, SocketImpl& impl, 
			GettableOptionType& opt, SocketError& error){
		socklen_t len = opt.Size();
		s_int32_t ret = g_getsockopt(M_Impl_Fd(impl), opt.Level(), 
							opt.Name(), (char*)opt.Data(), &len);
		M_DEFAULT_SOCKET_ERROR(ret != 0 || len != opt.Size(), error);
	}

	template<typename SettableOptionType>
	static void SetOption(IIoService& service, SocketImpl& impl, 
			const SettableOptionType& opt, SocketError& error){
		s_int32_t ret = g_setsockopt(M_Impl_Fd(impl), opt.Level(), 
							opt.Name(), (char*)opt.Data(), opt.Size());
		M_DEFAULT_SOCKET_ERROR(ret != 0, error);
	}

	template<typename EndPoint>
	static EndPoint RemoteEndPoint(EndPoint, IIoService& service, 
			const SocketImpl& impl, SocketError& error)
	{
		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		EndPoint ep;
		if (!ep_access::GetRemoteEndPoint(M_Impl_Fd(impl), ep))
			M_DEFAULT_SOCKET_ERROR2(error);
		return ep;
	}

	template<typename EndPoint>
	static EndPoint LocalEndPoint(EndPoint, IIoService& service, 
			const SocketImpl& impl, SocketError& error)
	{
		typedef typename EndPoint::Impl::endpoint_impl_access ep_access;
		EndPoint ep;
		if (!ep_access::GetLocalEndPoint(M_Impl_Fd(impl), ep))
			M_DEFAULT_SOCKET_ERROR2(error);
		return ep;
	}

	static void Shutdown(IIoService& service, SocketImpl& impl, 
			EShutdownType what, SocketError& error){
		if (0 != g_shutdown(M_Impl_Fd(impl), what))
			M_DEFAULT_SOCKET_ERROR2(error);
	}

	static void Close(IIoService& service, SocketImpl& impl, 
			SocketError& error){
		Close(service, impl, 0, error);
	}

	static void Close(IIoService& service, SocketImpl& impl, 
			function_t<void()> handler, SocketError& error)
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

	template<typename EndPoint>
	static void Connect(IIoService& service, SocketImpl& impl, 
		const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec)
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
				if (M_Impl_G_Block(impl))
					detail::Util::SetNonBlock(M_Impl_Fd(impl));
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
			s_int32_t ret = 
				g_connect(M_Impl_Fd(impl), ep_access::SockAddr(ep), ep_access::SockAddrLen(ep));
			if (ret == -1 && timeo_sec != -1) {
				if (M_ERR_LAST == M_ESHOULDBLOCK) {
					ret = Select(impl, false, timeo_sec, error);
					if (ret != 0)
						Close(service, impl, error);
				}
			}
			if (ret != 0 && !error){
				M_DEFAULT_SOCKET_ERROR(ret != 0, error);
			}
			mlock.lock();
			detail::Util::SetBlock(M_Impl_Fd(impl));
			M_Impl_C_Connect_Flag(impl);
			mlock.unlock();
			return;
		} while (false);
		mlock.unlock();
	}

	static void Accept(IIoService& service, SocketImpl& impl, 
			SocketImpl& peer, SocketError& error){
		MutexLock& mlock = M_Impl_Mutex(impl);
		do
		{
			mlock.lock();
			if (M_Impl_Fd(impl) == M_INVALID_SOCKET) {
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
			mlock.unlock();
			for (;;) {
				M_Impl_Fd(peer) = g_accept(M_Impl_Fd(impl), 0, 0);
				if (M_Impl_Fd(peer) != M_INVALID_SOCKET) {
					break;
				}
				if (M_ERR_LAST == M_ECONNRESET) {
					continue;
				}
				M_DEFAULT_SOCKET_ERROR2(error);
				break;
			}
			mlock.lock();
			M_Impl_C_Accept_Flag(impl);
			mlock.unlock();
			return;
		} while (false);
		mlock.unlock();
	}

	static s_int32_t SendSome(IIoService& service, SocketImpl& impl, 
			const s_byte_t* data, s_uint32_t size, SocketError& error){
		#ifdef M_PLATFORM_WIN
			int flag = 0;
		#else
			int flag = MSG_NOSIGNAL;
		#endif
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

			s_int32_t ret = g_send(M_Impl_Fd(impl), data, size, flag);
			mlock.lock();
			M_Impl_C_Write_Flag(impl);
			mlock.unlock();

			M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
			return ret;
		}while (false);
		mlock.unlock();
		return M_SOCKET_ERROR;		
	}

	static s_int32_t RecvSome(IIoService& service, SocketImpl& impl, 
			s_byte_t* data, s_uint32_t size, SocketError& error){
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

			s_int32_t ret = g_recv(M_Impl_Fd(impl), data, size, 0);
			mlock.lock();
			M_Impl_C_Read_Flag(impl);
			mlock.unlock();

			M_DEFAULT_SOCKET_ERROR(ret == M_SOCKET_ERROR, error);
			return ret;
		} while (false);
		mlock.unlock();
		return M_SOCKET_ERROR;
	}

	static bool Stopped(const IIoService& service){
		ScopedLock scoped_lock(service._mutex);
		return (service._implvector.empty() ? true : false);
	}

	static s_uint32_t GetServiceCount(const IIoService& service){
		ScopedLock scoped(service._mutex);
		return (s_uint32_t)service._implvector.size();
	}	

protected:
	static iodetail::IoServiceImpl* _GetIoServiceImpl(IIoService& service, 
			SocketImpl& impl)
	{
		if (M_Impl_Io(impl) == 0)
			return 0;
		IIoService::IoServiceImplMap::iterator iter = service._implmap.find(M_Impl_Io(impl));
		if (iter == service._implmap.end())
			return 0;
		return iter->second;
	}

	static iodetail::IoServiceImpl* _CreateIoImpl(IIoService& service, 
			SocketError& error){
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

	static void _ReleaseIoImpl(IIoService& service, iodetail::IoServiceImpl* simpl){
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

	static void _DoClose(iodetail::IoServiceImpl* simpl
		, base::slist<SocketClose*>&closereqs, base::slist<SocketClose*>&closereqs2){
		if (simpl->_closereqs.size()) {
			simpl->_mutex.lock();
			closereqs.swap(simpl->_closereqs);
			simpl->_mutex.unlock();
		}
		while (closereqs.size()) {
			SocketClose* cls = closereqs.front();
			MutexLock& mlock = M_Impl_Mutex(cls->_impl);
			mlock.lock();
			g_closesocket(M_Impl_Fd(cls->_impl));
			M_Impl_Fd(cls->_impl) = M_INVALID_SOCKET;
			M_Impl_State(cls->_impl) = 0;
			mlock.unlock();
			if (cls->_handler)
				cls->_handler();
			cls->Clear();
			closereqs2.push_back(cls);
			closereqs.pop_front();
		}
		if (closereqs2.size()) {
			simpl->_mutex.lock();
			simpl->_closereqs2.join(closereqs2);
			simpl->_mutex.unlock();
		}
		assert(closereqs.size() == 0);
		assert(closereqs2.size() == 0);		
	}

	// -1 == time out,0 == ok,other == error
	static s_int32_t Select(SocketImpl& impl, bool rd_or_wr, s_uint32_t timeo_sec, 
			SocketError& error){
		socket_t fd = M_Impl_Fd(impl);
		timeval tm;
		tm.tv_sec = timeo_sec;
		tm.tv_usec = 0;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(fd, &set);
		int ret = 0;
		if (rd_or_wr)
			ret = select(fd+1, &set, NULL, NULL, &tm);
		else
			ret = select(fd+1, NULL, &set, NULL, &tm);
	
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
		g_getsockopt(fd, M_SOL_SOCKET, M_SO_ERROR, (char*)&code, 
			(socklen_t *)&code_len);
		if (code == 0)
			return 0;
		else {
			error = SocketError(code);
			return -2;
		}
	}

};

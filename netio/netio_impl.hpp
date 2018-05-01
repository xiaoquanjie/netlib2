/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/11
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_IMPL_INCLUDE
#define M_NETIO_NETIO_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

template<typename NetIoType>
BaseNetIo<NetIoType>::BaseNetIo()
	:_backlog(20) {
	_endian = SocketLib::detail::Util::LocalEndian();
	function_t<void()> handler = bind_t(&BaseNetIo<NetIoType>::RunHandler, this);
	_ioservice.SetRunCallback(handler);
}

template<typename NetIoType>
BaseNetIo<NetIoType>::BaseNetIo(SocketLib::s_uint32_t backlog)
	: _backlog(backlog) {
	_endian = SocketLib::detail::Util::LocalEndian();
	function_t<void()> handler = bind_t(&BaseNetIo<NetIoType>::RunHandler, this);
	_ioservice.SetRunCallback(handler);
}

template<typename NetIoType>
BaseNetIo<NetIoType>::~BaseNetIo() {}

template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		TcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		TcpSocketPtr clisock(new TcpSocket(*this));
		acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOne(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOne(ep);
}

// 建立一个http监听
template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOneHttp(const SocketLib::Tcp::EndPoint& ep) {
	try {
		TcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		HttpSocketPtr clisock(new HttpSocket(*this));
		acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHttpHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

template<typename NetIoType>
bool BaseNetIo<NetIoType>::ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOneHttp(ep);
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(*this));
		connector->AsyncConnect(ep);
		connector.reset();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOne(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ConnectOne(ep);
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOneHttp(const SocketLib::Tcp::EndPoint& ep) {
	try {
		netiolib::HttpConnectorPtr connector(new netiolib::HttpConnector(*this));
		connector->AsyncConnect(ep);
		connector.reset();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::ConnectOneHttp(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ConnectOneHttp(ep);
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::Start(unsigned int thread_cnt, bool isco) {
	if (_threadlist.empty()) {
		for (unsigned int idx = 0; idx < thread_cnt; ++idx) {
			bool* pb = new bool(isco);
			base::thread* pthread = new base::thread(&BaseNetIo::_Start, this, pb);
			_threadlist.push_back(pthread);
		}
	}
	while (_ioservice.ServiceCount()
		!= _threadlist.size());
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::Stop() {
	try {
		_ioservice.Stop();
		while (_threadlist.size()) {
			base::thread* pthread = _threadlist.front();
			pthread->join();
			_threadlist.pop_front();
		}
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		M_NETIO_LOGGER("stop happend error:"M_ERROR_DESC_STR(error));
	}
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::RunHandler() {
}

template<typename NetIoType>
size_t BaseNetIo<NetIoType>::ServiceCount() {
	return _ioservice.ServiceCount();
}

template<typename NetIoType>
inline SocketLib::SocketError BaseNetIo<NetIoType>::GetLastError()const {
	return lasterror;
}

template<typename NetIoType>
inline SocketLib::IoService& BaseNetIo<NetIoType>::GetIoService() {
	return _ioservice;
}

template<typename NetIoType>
inline SocketLib::s_uint32_t BaseNetIo<NetIoType>::LocalEndian()const {
	return _endian;
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::_Start(void*p) {
	printf("%d thread is starting..............\n", base::thread::ctid());
	bool* pb = (bool*)p;
	try {
		if (*pb)
			_ioservice.CoRun();
		else
			_ioservice.Run();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		M_NETIO_LOGGER("run happend error:"M_ERROR_DESC_STR(error));
	}
	delete pb;
	printf("%d thread is leaving..............\n", base::thread::ctid());
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::_AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, TcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_ERROR_DESC_STR(error));
	}
	else {
		clisock->Init();
	}
	TcpSocketPtr newclisock(new TcpSocket(*this));
	acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
	if (error)
		lasterror = error;
}

template<typename NetIoType>
void BaseNetIo<NetIoType>::_AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, TcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_ERROR_DESC_STR(error));
	}
	else {
		clisock->Init();
	}
	HttpSocketPtr newclisock(new HttpSocket(*this));
	acceptor->AsyncAccept(bind_t(&BaseNetIo<NetIoType>::_AcceptHttpHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
	if (error)
		lasterror = error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*以下三个函数定义为虚函数，以便根据实际业务的模式来做具体模式的消息包分发处理。
*保证同一个socket，以下三个函数的调用遵循OnConnected -> OnReceiveData -> OnDisconnected的顺序。
*保证同一个socket，以下后两个函数的调用都在同一个线程中
*/

// 连线通知,这个函数里不要处理业务，防止堵塞
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(TcpSocketPtr& clisock) {
	M_PRINT("tcp socket | OnConnected one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port());
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(TcpConnectorPtr& clisock, SocketLib::SocketError error) {
	if (error) {
		M_PRINT("tcp connector | connect fail :%d %s\n", error.Code(), error.What().c_str());
	}
	else {
		M_PRINT("tcp connector | connect success : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
			clisock->RemoteEndpoint().Port());
	}
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(HttpSocketPtr& clisock) {
	M_PRINT("http socket | OnConnected one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port());
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnConnected(HttpConnectorPtr& clisock, SocketLib::SocketError error) {
	if (error) {
		M_PRINT("http connector | connect fail :%d %s\n", error.Code(), error.What().c_str());
	}
	else {
		M_PRINT("http connector | connect success : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
			clisock->RemoteEndpoint().Port());
	}
}


// 掉线通知,这个函数里不要处理业务，防止堵塞
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(TcpSocketPtr& clisock) {
	M_PRINT("tcp socket | OnDisconnected one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port())
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(TcpConnectorPtr& clisock) {
	M_PRINT("tcp connector | OnDisconnected one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port())
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(HttpSocketPtr& clisock) {
	M_PRINT("http socket | OnDisconnected one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port())
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnDisconnected(HttpConnectorPtr& clisock) {
	M_PRINT("http connector | OnDisconnected one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port())
}

// 数据包通知,这个函数里不要处理业务，防止堵塞
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {
	M_PRINT("tcp socket | OnReceiveData one : %s %d %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port(), buffer.Length());
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(TcpConnectorPtr& clisock, SocketLib::Buffer& buffer) {
	M_PRINT("tcp connector | OnReceiveData one : %s %d %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port(), buffer.Length());
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(HttpSocketPtr& clisock, HttpSvrRecvMsg& httpmsg) {
	M_PRINT("http socket | OnReceiveData one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port());
}
template<typename NetIoType>
void BaseNetIo<NetIoType>::OnReceiveData(HttpConnectorPtr& clisock, HttpCliRecvMsg& httpmsg) {
	M_PRINT("http connector | OnReceiveData one : %s %d\n", clisock->RemoteEndpoint().Address().c_str(),
		clisock->RemoteEndpoint().Port());
}

M_NETIO_NAMESPACE_END
#endif
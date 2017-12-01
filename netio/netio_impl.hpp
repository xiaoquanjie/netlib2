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

NetIo::NetIo() 
	:_backlog(20){
	_endian = SocketLib::detail::Util::LocalEndian();
}

NetIo::NetIo(SocketLib::s_uint32_t backlog) 
	: _backlog(backlog) {
	_endian = SocketLib::detail::Util::LocalEndian();
}

NetIo::~NetIo() {}

bool NetIo::ListenOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		TcpSocketPtr clisock(new TcpSocket(*this));
		acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

bool NetIo::ListenOne(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOne(ep);
}

// 建立一个http监听
bool NetIo::ListenOneHttp(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		HttpSocketPtr clisock(new HttpSocket(*this));
		acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHttpHandler, this, placeholder_1, clisock, acceptor), clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
	return true;
}

bool NetIo::ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ListenOneHttp(ep);
}

void NetIo::ConnectOne(const SocketLib::Tcp::EndPoint& ep, unsigned int data) {
	try {
		netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(*this));
		connector->SetData(data);
		connector->AsyncConnect("127.0.0.1", 3001);
		connector.reset();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}
void NetIo::ConnectOne(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return ConnectOne(ep,data);
}

void NetIo::Run() {
	try {
		_ioservice.Run();
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		M_NETIO_LOGGER("run happend error:"M_ERROR_DESC_STR(error));
	}
}

void NetIo::Stop() {
	try{
		_ioservice.Stop();
	}
	catch (SocketLib::SocketError& error){
		lasterror = error;
		M_NETIO_LOGGER("stop happend error:"M_ERROR_DESC_STR(error));
	}
}

inline SocketLib::SocketError NetIo::GetLastError()const {
	return lasterror;
}

inline SocketLib::IoService& NetIo::GetIoService() {
	return _ioservice;
}

inline SocketLib::s_uint32_t NetIo::LocalEndian()const {
	return _endian;
}

void NetIo::_AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_NETIO_LOGGER(error));
	}
	else {
		clisock->Init();
	}
	TcpSocketPtr newclisock(new TcpSocket(*this));
	acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
	if (error)
		lasterror = error;
}

void NetIo::_AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor) {
	if (error) {
		M_NETIO_LOGGER("accept handler happend error:" << M_NETIO_LOGGER(error));
	}
	else {
		clisock->Init();
	}
	HttpSocketPtr newclisock(new HttpSocket(*this));
	acceptor->AsyncAccept(bind_t(&NetIo::_AcceptHttpHandler, this, placeholder_1, newclisock, acceptor), newclisock->GetSocket(), error);
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
void NetIo::OnConnected(const TcpSocketPtr& clisock) {
}
void NetIo::OnConnected(const TcpConnectorPtr& clisock, SocketLib::SocketError error) {
}
void NetIo::OnConnected(HttpSocketPtr clisock) {
}

// 掉线通知,这个函数里不要处理业务，防止堵塞
void NetIo::OnDisconnected(const TcpSocketPtr& clisock) {
}
void NetIo::OnDisconnected(const TcpConnectorPtr& clisock) {
}
void NetIo::OnDisconnected(HttpSocketPtr clisock) {
}

// 数据包通知,这个函数里不要处理业务，防止堵塞
void NetIo::OnReceiveData(const TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {
}
void NetIo::OnReceiveData(const TcpConnectorPtr& clisock, SocketLib::Buffer& buffer) {
}
void NetIo::OnReceiveData(HttpSocketPtr clisock, HttpSvrRecvMsg& httpmsg) {
}

M_NETIO_NAMESPACE_END
#endif
/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/14
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_TCONNECTOR_IMPL_INCLUDE
#define M_NETIO_TCONNECTOR_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

TcpConnector::TcpConnector(NetIo& netio, MessageChecker2 checker)
	:TcpBaseSocket(netio, checker) {

}

SocketLib::TcpConnector<SocketLib::IoService>& TcpConnector::GetSocket() {
	return *this->_socket;
}

bool TcpConnector::Connect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		this->_socket->Connect(ep);
		return true;
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
}

bool TcpConnector::Connect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return Connect(ep);
}

void TcpConnector::AsyncConnect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		function_t<void(SocketLib::SocketError)> handler = bind_t(&TcpConnector::_ConnectHandler, this,
			placeholder_1, this->shared_from_this());
		this->_socket->AsyncConnect(handler, ep);
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

void TcpConnector::AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return AsyncConnect(ep);
}

void TcpConnector::_ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector) {
	if (error) {
		lasterror = error;
		this->_netio.OnConnected(this->shared_from_this(), error);
	}
	else {
		_remoteep = _socket->RemoteEndPoint();
		_localep = _socket->LocalEndPoint();
		_flag = E_TCPSOCKET_STATE_START;
		this->_netio.OnConnected(this->shared_from_this(), error);
		_flag |= E_TCPSOCKET_STATE_READ;
		function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
			bind_t(&TcpConnector::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2);
		_socket->AsyncRecvSome(handler, _reader.readbuf, M_READ_SIZE);
	}
}

M_NETIO_NAMESPACE_END
#endif
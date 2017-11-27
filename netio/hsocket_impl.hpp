/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/26
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
//----------------------------------------------------------------*/

#ifndef M_NETIO_HSOCKET_IMPL_INCLUDE
#define M_NETIO_HSOCKET_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

HttpSocket::HttpSocket(NetIo& netio)
	:TcpBaseSocket(netio, 0) {
}

SocketLib::TcpSocket<SocketLib::IoService>& HttpSocket::GetSocket() {
	return (*this->_socket);
}

void HttpSocket::Init() {
	try {
		_remoteep = _socket->RemoteEndPoint();
		_localep = _socket->LocalEndPoint();
		_flag = E_STATE_START;
		_netio.OnConnected(this->shared_from_this());
		this->_TryRecvHttpData();
	}
	catch (const SocketLib::SocketError& e) {
		lasterror = e;
	}
}


M_NETIO_NAMESPACE_END
#endif
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

template<typename ConnectorType>
BaseTcpConnector<ConnectorType>::BaseTcpConnector(BaseNetIo<NetIo>& netio)
	:TcpStreamSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService> >(netio)
	, _data(0) {
}

template<typename ConnectorType>
SocketLib::TcpConnector<SocketLib::IoService>& BaseTcpConnector<ConnectorType>::GetSocket() {
	return *this->_socket;
}

template<typename ConnectorType>
bool BaseTcpConnector<ConnectorType>::Connect(const SocketLib::Tcp::EndPoint& ep, SocketLib::s_uint32_t timeo_sec) {
	try {
		this->_socket->Connect(ep,timeo_sec);
		return true;
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
}

template<typename ConnectorType>
bool BaseTcpConnector<ConnectorType>::Connect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::s_uint32_t timeo_sec) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return Connect(ep,timeo_sec);
}

template<typename ConnectorType>
void BaseTcpConnector<ConnectorType>::AsyncConnect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		function_t<void(SocketLib::SocketError)> handler = bind_t(&BaseTcpConnector<ConnectorType>::_ConnectHandler, this,
			placeholder_1, this->shared_from_this());
		this->_socket->AsyncConnect(handler, ep);
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename ConnectorType>
void BaseTcpConnector<ConnectorType>::AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return AsyncConnect(ep);
}

template<typename ConnectorType>
inline void BaseTcpConnector<ConnectorType>::SetData(unsigned int data) {
	_data = data;
}

template<typename ConnectorType>
inline unsigned int BaseTcpConnector<ConnectorType>::GetData()const {
	return _data;
}

template<typename ConnectorType>
void BaseTcpConnector<ConnectorType>::_ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector) {
	if (error) {
		lasterror = error;
		this->_netio.OnConnected(this->shared_from_this(), error);
		return;
	}
	try {
		this->_remoteep = this->_socket->RemoteEndPoint();
		this->_localep = this->_socket->LocalEndPoint();
		this->_flag = E_STATE_START;
		this->_netio.OnConnected(this->shared_from_this(), error);
		function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
			bind_t(&BaseTcpConnector<ConnectorType>::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2);
		this->_socket->AsyncRecvSome(handler, this->_reader.readbuf, M_READ_SIZE);
	}
	catch (SocketLib::SocketError& err) {
		lasterror = err;
	}
}

M_NETIO_NAMESPACE_END
#endif
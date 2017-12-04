/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/12/02
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_HCONNECTOR_IMPL_INCLUDE
#define M_NETIO_HCONNECTOR_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

template<typename ConnectorType>
BaseHttpConnector<ConnectorType>::BaseHttpConnector(BaseNetIo<NetIo>& netio)
	:HttpBaseSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService>
	, HttpCliRecvMsg>(netio) {
}

template<typename ConnectorType>
SocketLib::TcpConnector<SocketLib::IoService>& BaseHttpConnector<ConnectorType>::GetSocket() {
	return (*this->_socket);
}

template<typename ConnectorType>
bool BaseHttpConnector<ConnectorType>::Connect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		this->_socket->Connect(ep);
		return true;
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
}

template<typename ConnectorType>
bool BaseHttpConnector<ConnectorType>::Connect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return Connect(ep);
}

template<typename ConnectorType>
void BaseHttpConnector<ConnectorType>::AsyncConnect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		function_t<void(SocketLib::SocketError)> handler = bind_t(&BaseHttpConnector<ConnectorType>::_ConnectHandler, this,
			placeholder_1, this->shared_from_this());
		this->_socket->AsyncConnect(handler, ep);
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename ConnectorType>
void BaseHttpConnector<ConnectorType>::AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return AsyncConnect(ep);
}

template<typename ConnectorType>
inline void BaseHttpConnector<ConnectorType>::SetData(unsigned int data) {
	_data = data;
}

template<typename ConnectorType>
inline unsigned int BaseHttpConnector<ConnectorType>::GetData()const {
	return _data;
}

template<typename ConnectorType>
void BaseHttpConnector<ConnectorType>::_ConnectHandler(const SocketLib::SocketError& error, HttpConnectorPtr conector) {
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
			bind_t(&BaseHttpConnector<ConnectorType>::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2);
		this->_socket->AsyncRecvSome(handler, this->_reader.readbuf, M_READ_SIZE);
	}
	catch (SocketLib::SocketError& err) {
		lasterror = err;
	}
}

M_NETIO_NAMESPACE_END
#endif
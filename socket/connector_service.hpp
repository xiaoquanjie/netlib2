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

#ifndef M_CONNECTOR_SERVICE_INCLUDE
#define M_CONNECTOR_SERVICE_INCLUDE

#include "stream_service.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename Protocol, typename IoServiceType>
class TcpConnectorService : public StreamSocketService<Protocol, IoServiceType>
{
public:
	typedef typename IoServiceType::Impl Impl;
	typedef typename IoServiceType::Access		Access;
	typedef typename Protocol::EndPoint EndPoint;

	M_SOCKET_DECL TcpConnectorService(IoServiceType& ioservice);

	M_SOCKET_DECL void Connect(Impl& impl, const EndPoint& ep, SocketError& error);

	template<typename ConnectHandler>
	M_SOCKET_DECL void AsyncConnect(M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error);

};

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL TcpConnectorService<Protocol, IoServiceType>::TcpConnectorService(IoServiceType& ioservice)
	:StreamSocketService<Protocol, IoServiceType>(ioservice)
{
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void TcpConnectorService<Protocol, IoServiceType>::Connect(Impl& impl, const EndPoint& ep, SocketError& error)
{
	Access::Connect(this->_ioservice, impl, ep, error);
}

template<typename Protocol, typename IoServiceType>
template<typename ConnectHandler>
M_SOCKET_DECL void TcpConnectorService<Protocol, IoServiceType>::AsyncConnect(M_HANDLER_SOCKET_PTR(ConnectHandler) connect_ptr, const EndPoint& ep, ConnectHandler handler, SocketError& error)
{
	Access::AsyncConnect(this->_ioservice, connect_ptr, ep, handler, error);
}

M_SOCKET_NAMESPACE_END
#endif
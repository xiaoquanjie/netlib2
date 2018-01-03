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

#include "socket/stream_service.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename Protocol, typename IoServiceType>
class TcpConnectorService : public StreamSocketService<Protocol, IoServiceType>
{
public:
	typedef typename IoServiceType::Impl Impl;
	typedef typename IoServiceType::Access		Access;
	typedef typename Protocol::EndPoint EndPoint;

	M_SOCKET_DECL TcpConnectorService(IoServiceType& ioservice);

	M_SOCKET_DECL void Connect(Impl& impl, const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec);

	M_SOCKET_DECL void AsyncConnect(Impl& impl, const M_COMMON_HANDLER_TYPE(IoServiceType)& handler, const EndPoint& ep, SocketError& error);
};

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL TcpConnectorService<Protocol, IoServiceType>::TcpConnectorService(IoServiceType& ioservice)
	:StreamSocketService<Protocol, IoServiceType>(ioservice)
{
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void TcpConnectorService<Protocol, IoServiceType>::Connect(Impl& impl, const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec)
{
	Access::Connect(this->_ioservice, impl, ep, error, timeo_sec);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void TcpConnectorService<Protocol, IoServiceType>::AsyncConnect(Impl& impl, const M_COMMON_HANDLER_TYPE(IoServiceType)& handler, const EndPoint& ep, SocketError& error)
{
	Access::AsyncConnect(this->_ioservice, impl, ep, handler, error);
}


M_SOCKET_NAMESPACE_END
#endif
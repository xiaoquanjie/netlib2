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

#ifndef M_SOCKET_CONNECTOR_INCLUDE
#define M_SOCKET_CONNECTOR_INCLUDE

#include "socket/socket_stream.hpp"
#include "socket/protocol.hpp"
#include "socket/connector_service.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename IoServiceType>
class TcpConnector : public StreamSocket<Tcp, TcpConnectorService<Tcp, IoServiceType> >
	//,public enable_shared_from_this_t<TcpConnector<IoServiceType> >
{
public:
	typedef TcpConnector<IoServiceType> MySelf;
	typedef StreamSocket<Tcp, TcpConnectorService<Tcp, IoServiceType> > SocketType;
	typedef Tcp::EndPoint EndPoint;

	M_SOCKET_DECL TcpConnector(IoServiceType& ioservice);

	M_SOCKET_DECL void Connect(const EndPoint& ep, s_uint32_t timeo_sec = -1);

	M_SOCKET_DECL void Connect(const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec = -1);
		
	/*ConnectHandler: void(SocketError)*/
	M_SOCKET_DECL void AsyncConnect(const M_COMMON_HANDLER_TYPE(IoServiceType)& handler, const EndPoint& ep);

	/*ConnectHandler: void(SocketError)*/
	M_SOCKET_DECL void AsyncConnect(const M_COMMON_HANDLER_TYPE(IoServiceType)& handler, const EndPoint& ep, SocketError& error);

	/*ReadHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncRecvSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler,s_byte_t* data, s_uint32_t size);

	/*ReadHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncRecvSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler, s_byte_t* data, s_uint32_t size, SocketError& error);

	/*WriteHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncSendSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler, s_byte_t* data, s_uint32_t size);

	/*WriteHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncSendSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler, const s_byte_t* data, s_uint32_t size, SocketError& error);
};

template<typename IoServiceType>
M_SOCKET_DECL TcpConnector<IoServiceType>::TcpConnector(IoServiceType& ioservice)
	:SocketType(ioservice)
{
	this->GetObjectService().Construct(this->GetImpl(), E_CONNECTOR_TYPE);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::Connect(const EndPoint& ep, s_uint32_t timeo_sec)
{
	SocketError error;
	this->Connect(ep, error,timeo_sec);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::Connect(const EndPoint& ep, SocketError& error, s_uint32_t timeo_sec)
{
	if (!this->IsOpen()){
		Tcp pt = ep.Protocol();
		this->GetObjectService().Open(this->GetImpl(), pt, error);
		if (error)
			return;
	}
	this->GetObjectService().Connect(this->GetImpl(), ep, error, timeo_sec);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::AsyncConnect(const M_COMMON_HANDLER_TYPE(IoServiceType)& handler, const EndPoint& ep)
{
	SocketError error;
	this->AsyncConnect(handler, ep, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::AsyncConnect(const M_COMMON_HANDLER_TYPE(IoServiceType)& handler, const EndPoint& ep, SocketError& error)
{
	if (!this->IsOpen())
	{
		Tcp pt = ep.Protocol();
		this->GetObjectService().Open(this->GetImpl(), pt, error);
		if (error)
			return;
	}
	this->GetObjectService().AsyncConnect(this->GetImpl(), handler, ep, error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::AsyncRecvSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler, s_byte_t* data, s_uint32_t size)
{
	SocketError error;
	this->AsyncRecvSome(handler, data, size, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::AsyncRecvSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	this->GetObjectService().AsyncRecvSome(this->GetImpl(), data, size, handler, error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::AsyncSendSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler, s_byte_t* data, s_uint32_t size)
{
	SocketError error;
	this->AsyncSendSome(handler, data, size, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpConnector<IoServiceType>::AsyncSendSome(const M_RW_HANDLER_TYPE(IoServiceType)& handler, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	this->GetObjectService().AsyncSendSome(this->GetImpl(), data, size, handler, error);
}

M_SOCKET_NAMESPACE_END
#endif
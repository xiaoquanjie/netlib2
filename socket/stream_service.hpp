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

#ifndef M_STREAM_SERVICE_INCLUDE
#define M_STREAM_SERVICE_INCLUDE

#include "socket_service.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename Protocol, typename IoServiceType>
class StreamSocketService : public BaseSocketService<Protocol, IoServiceType>
{
public:
	typedef typename IoServiceType::Impl	Impl;
	typedef typename IoServiceType::Access	Access;
	typedef typename Protocol::EndPoint		EndPoint;

	M_SOCKET_DECL StreamSocketService(IoServiceType& ioservice);

	M_SOCKET_DECL s_int32_t RecvSome(Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error);

	M_SOCKET_DECL s_int32_t SendSome(Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error);

	template<typename ReadHandler>
	M_SOCKET_DECL void AsyncRecvSome(M_HANDLER_SOCKET_PTR(ReadHandler) impl, s_byte_t* data, s_uint32_t size, ReadHandler handler, SocketError& error);

	template<typename WriteHandler>
	M_SOCKET_DECL void AsyncSendSome(M_HANDLER_SOCKET_PTR(WriteHandler) impl, const s_byte_t* data, s_uint32_t size, WriteHandler handler, SocketError& error);
};


template<typename Protocol, typename IoServiceType>
class TcpSocketService : public StreamSocketService<Protocol, IoServiceType>
{
public:
	typedef typename IoServiceType::Impl Impl;
	typedef typename Protocol::EndPoint EndPoint;

	M_SOCKET_DECL TcpSocketService(IoServiceType& ioservice);
};

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL StreamSocketService<Protocol, IoServiceType>::StreamSocketService(IoServiceType& ioservice)
	:BaseSocketService<Protocol, IoServiceType>(ioservice)
{
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL s_int32_t StreamSocketService<Protocol, IoServiceType>::RecvSome(Impl& impl, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	return Access::RecvSome(this->_ioservice, impl, data, size, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL s_int32_t StreamSocketService<Protocol, IoServiceType>::SendSome(Impl& impl, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	return Access::SendSome(this->_ioservice, impl, data, size, error);
}

template<typename Protocol, typename IoServiceType>
template<typename ReadHandler>
M_SOCKET_DECL void StreamSocketService<Protocol, IoServiceType>::AsyncRecvSome(M_HANDLER_SOCKET_PTR(ReadHandler) impl, s_byte_t* data, s_uint32_t size, ReadHandler handler, SocketError& error)
{
	Access::AsyncRecvSome(this->_ioservice, impl, data, size, handler, error);
}

template<typename Protocol, typename IoServiceType>
template<typename WriteHandler>
M_SOCKET_DECL void StreamSocketService<Protocol, IoServiceType>::AsyncSendSome(M_HANDLER_SOCKET_PTR(WriteHandler) impl, const s_byte_t* data, s_uint32_t size, WriteHandler handler, SocketError& error)
{
	Access::AsyncSendSome(this->_ioservice, impl, data, size, handler, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL TcpSocketService<Protocol, IoServiceType>::TcpSocketService(IoServiceType& ioservice)
	:StreamSocketService<Protocol, IoServiceType>(ioservice)
{
}

M_SOCKET_NAMESPACE_END
#endif
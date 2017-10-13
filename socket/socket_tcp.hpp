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

#ifndef M_SOCKET_TCP_INCLUDE
#define M_SOCKET_TCP_INCLUDE

#include "config.hpp"
#include "socket_stream.hpp"
#include "protocol.hpp"
#include "stream_service.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename IoServiceType>
class TcpSocket : public StreamSocket<Tcp, TcpSocketService<Tcp, IoServiceType> >,
	public enable_shared_from_this_t<TcpSocket<IoServiceType> >
{
public:
	typedef StreamSocket<Tcp, TcpSocketService<Tcp, IoServiceType> > SocketType;

	M_SOCKET_DECL TcpSocket(IoServiceType& ioservice);

	/*ReadHandler: void(TcpSocketPtr connector, s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)*/
	template<typename ReadHandler>
	M_SOCKET_DECL void AsyncRecvSome(s_byte_t* data, s_uint32_t size, ReadHandler handler);

	/*ReadHandler: void(TcpSocketPtr connector, s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)*/
	template<typename ReadHandler>
	M_SOCKET_DECL void AsyncRecvSome(s_byte_t* data, s_uint32_t size, ReadHandler handler, SocketError& error);

	/*ReadHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncRecvSome(M_RW_HANDLER_TYPE(IoServiceType) handler, s_byte_t* data, s_uint32_t size);

	/*ReadHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncRecvSome(M_RW_HANDLER_TYPE(IoServiceType) handler, s_byte_t* data, s_uint32_t size, SocketError& error);

	/*WriteHandler: void(TcpSocketPtr connector, const s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)*/
	template<typename WriteHandler>
	M_SOCKET_DECL void AsyncSendSome(const s_byte_t* data, s_uint32_t size, WriteHandler handler);

	/*WriteHandler: void(TcpSocketPtr connector, const s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)*/
	template<typename WriteHandler>
	M_SOCKET_DECL void AsyncSendSome(const s_byte_t* data, s_uint32_t size, WriteHandler handler, SocketError& error);

	/*WriteHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncSendSome(M_RW_HANDLER_TYPE(IoServiceType) handler, s_byte_t* data, s_uint32_t size);

	/*WriteHandler: void(s_uint32 trans,SocketError error)*/
	M_SOCKET_DECL void AsyncSendSome(M_RW_HANDLER_TYPE(IoServiceType) handler, const s_byte_t* data, s_uint32_t size, SocketError& error);
};

template<typename IoServiceType>
M_SOCKET_DECL TcpSocket<IoServiceType>::TcpSocket(IoServiceType& ioservice)
	:SocketType(ioservice)
{
	this->GetObjectService().Construct(this->GetImpl(), E_SOCKET_TYPE);
}

template<typename IoServiceType>
template<typename ReadHandler>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncRecvSome(s_byte_t* data, s_uint32_t size, ReadHandler handler)
{
	M_CHECK_READ_HANDLER(handler, IoServiceType);
	SocketError error;
	this->AsyncRecvSome(data, size, handler, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
template<typename ReadHandler>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncRecvSome(s_byte_t* data, s_uint32_t size, ReadHandler handler, SocketError& error)
{
	M_CHECK_READ_HANDLER(handler, IoServiceType);
	this->GetObjectService().AsyncRecvSome(this->shared_from_this(), data, size, handler, error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncRecvSome(M_RW_HANDLER_TYPE(IoServiceType) handler, s_byte_t* data, s_uint32_t size)
{
	SocketError error;
	this->AsyncRecvSome(handler, data, size, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncRecvSome(M_RW_HANDLER_TYPE(IoServiceType) handler, s_byte_t* data, s_uint32_t size, SocketError& error)
{
	this->GetObjectService().AsyncRecvSome(this->GetImpl(), data, size, handler, error);
}

template<typename IoServiceType>
template<typename WriteHandler>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncSendSome(const s_byte_t* data, s_uint32_t size, WriteHandler handler)
{
	M_CHECK_WRITE_HANDLER(handler, IoServiceType);
	SocketError error;
	this->AsyncSendSome(data, size, handler, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
template<typename WriteHandler>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncSendSome(const s_byte_t* data, s_uint32_t size, WriteHandler handler, SocketError& error)
{
	M_CHECK_WRITE_HANDLER(handler, IoServiceType);
	this->GetObjectService().AsyncSendSome(this->shared_from_this(), data, size, handler, error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncSendSome(M_RW_HANDLER_TYPE(IoServiceType) handler, s_byte_t* data, s_uint32_t size)
{
	SocketError error;
	this->AsyncSendSome(handler, data, size, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
M_SOCKET_DECL void TcpSocket<IoServiceType>::AsyncSendSome(M_RW_HANDLER_TYPE(IoServiceType) handler, const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	this->GetObjectService().AsyncSendSome(this->GetImpl(), data, size, handler, error);
}

M_SOCKET_NAMESPACE_END
#endif
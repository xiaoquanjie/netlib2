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

#ifndef M_SOCKET_STREAM_INCLUDE
#define M_SOCKET_STREAM_INCLUDE

#include "socket/basic_socket.hpp"
M_SOCKET_NAMESPACE_BEGIN

template <typename Protocol, typename SocketService>
class StreamSocket : public BasicSocket<Protocol, SocketService>
{
public:
	typedef typename SocketService::IoServiceType	IoServiceType;

	M_SOCKET_DECL StreamSocket(IoServiceType& ioservice);

	M_SOCKET_DECL s_int32_t RecvSome(s_byte_t* data, s_uint32_t size);

	M_SOCKET_DECL s_int32_t RecvSome(s_byte_t* data, s_uint32_t size, SocketError& error);

	M_SOCKET_DECL s_int32_t SendSome(const s_byte_t* data, s_uint32_t size);

	M_SOCKET_DECL s_int32_t SendSome(const s_byte_t* data, s_uint32_t size, SocketError& error);
};

template <typename Protocol, typename SocketService>
M_SOCKET_DECL StreamSocket<Protocol, SocketService>::StreamSocket(IoServiceType& ioservice)
	:BasicSocket<Protocol, SocketService>(ioservice)
{}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL s_int32_t StreamSocket<Protocol, SocketService>::RecvSome(s_byte_t* data, s_uint32_t size)
{
	SocketError error;
	s_int32_t ret = this->RecvSome(data, size, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
	return ret;
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL s_int32_t StreamSocket<Protocol, SocketService>::RecvSome(s_byte_t* data, s_uint32_t size, SocketError& error)
{
	return this->GetObjectService().RecvSome(this->GetImpl(), data, size, error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL s_int32_t StreamSocket<Protocol, SocketService>::SendSome(const s_byte_t* data, s_uint32_t size)
{
	SocketError error;
	s_int32_t ret = this->SendSome(data, size,error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
	return ret;
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL s_int32_t StreamSocket<Protocol, SocketService>::SendSome(const s_byte_t* data, s_uint32_t size, SocketError& error)
{
	return this->GetObjectService().SendSome(this->GetImpl(), data, size, error);
}


M_SOCKET_NAMESPACE_END
#endif
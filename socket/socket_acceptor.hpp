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

#ifndef M_SOCKET_ACCEPTOR_INCLUDE
#define M_SOCKET_ACCEPTOR_INCLUDE

#include "basic_socket.hpp"
#include "protocol.hpp"
#include "acceptor_service.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename IoServiceType>
class TcpAcceptor : public BasicSocket<Tcp, TcpAcceptorService<Tcp, IoServiceType> >
		//,public enable_shared_from_this_t<TcpAcceptor<IoServiceType> >
{
public:
	typedef TcpAcceptor<IoServiceType> MySelf;
	typedef BasicSocket<Tcp, TcpAcceptorService<Tcp, IoServiceType> > SocketType;
	typedef typename Tcp::EndPoint EndPoint;
	typedef typename IoServiceType::Impl ImplType;
	typedef StreamSocket<Tcp, TcpSocketService<Tcp, IoServiceType> > StreamSocketType;
	
	M_SOCKET_DECL TcpAcceptor(IoServiceType& ioservice, const EndPoint& ep);

	M_SOCKET_DECL TcpAcceptor(IoServiceType& ioservice, const EndPoint& ep, s_uint32_t flag);

	template<typename Socket_Type>
	M_SOCKET_DECL void Accept(Socket_Type& peer);

	template<typename Socket_Type>
	M_SOCKET_DECL void Accept(Socket_Type& peer, SocketError& error);

	/*AcceptHandler: void(SocketError&)*/
	template<typename AcceptHandler>
	M_SOCKET_DECL void AsyncAccept(AcceptHandler handler, StreamSocketType& sock);

	/*AcceptHandler: void(SocketError&)*/
	template<typename AcceptHandler>
	M_SOCKET_DECL void AsyncAccept(AcceptHandler handler, StreamSocketType& sock, SocketError& error);
};

template<typename IoServiceType>
M_SOCKET_DECL TcpAcceptor<IoServiceType>::TcpAcceptor(IoServiceType& ioservice, const EndPoint& ep)
	:SocketType(ioservice)
{
	this->GetObjectService().Construct(this->GetImpl(), E_ACCEPTOR_TYPE);
	Tcp pt = ep.Protocol();
	SocketError error;
	this->GetObjectService().Open(this->GetImpl(), pt,error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);

	typename SocketType::ReuseAddress reuse(true);
	this->SetOption(reuse, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);

	this->GetObjectService().Bind(this->GetImpl(), ep, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);

	this->GetObjectService().Listen(this->GetImpl(), 100, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
M_SOCKET_DECL TcpAcceptor<IoServiceType>::TcpAcceptor(IoServiceType& ioservice, const EndPoint& ep, s_uint32_t flag)
	:SocketType(ioservice)
{
	this->GetObjectService().Construct(this->GetImpl(), E_ACCEPTOR_TYPE);
	Tcp pt = ep.Protocol();
	SocketError error;
	this->GetObjectService().Open(this->GetImpl(), pt, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);

	typename SocketType::ReuseAddress reuse(true);
	this->SetOption(reuse, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);

	this->GetObjectService().Bind(this->GetImpl(), ep, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);

	this->GetObjectService().Listen(this->GetImpl(), flag, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
template<typename Socket_Type>
M_SOCKET_DECL void TcpAcceptor<IoServiceType>::Accept(Socket_Type& peer)
{
	SocketError error;
	this->Accept(peer, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
template<typename Socket_Type>
M_SOCKET_DECL void TcpAcceptor<IoServiceType>::Accept(Socket_Type& peer, SocketError& error)
{
	this->GetObjectService().Accept(this->GetImpl(), peer.GetImpl(), error);
}

template<typename IoServiceType>
template<typename AcceptHandler>
M_SOCKET_DECL void TcpAcceptor<IoServiceType>::AsyncAccept(AcceptHandler handler,StreamSocketType& sock)
{
	M_CHECK_COMMON_HANDLER(handler, IoServiceType);
	SocketError error;
	this->AsyncAccept(handler, sock, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template<typename IoServiceType>
template<typename AcceptHandler>
M_SOCKET_DECL void TcpAcceptor<IoServiceType>::AsyncAccept(AcceptHandler handler, StreamSocketType& sock, SocketError& error)
{
	M_CHECK_COMMON_HANDLER(handler, IoServiceType);
	this->GetObjectService().AsyncAccept(this->GetImpl(), sock.GetImpl(), handler, error);
}

M_SOCKET_NAMESPACE_END
#endif
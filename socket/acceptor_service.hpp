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

#ifndef M_ACCEPTOR_SERVICE_INCLUDE
#define M_ACCEPTOR_SERVICE_INCLUDE

#include "socket_service.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename Protocol, typename IoServiceType>
class TcpAcceptorService : public BaseSocketService<Protocol, IoServiceType>
{
public:
	typedef typename IoServiceType::Impl Impl;
	typedef typename IoServiceType::Access		Access;
	typedef typename Protocol::EndPoint EndPoint;
	typedef BasicSocket<Protocol, TcpAcceptorService<Protocol, IoServiceType> > AcceptorType;
	typedef StreamSocket<Protocol, TcpSocketService<Protocol, IoServiceType> >  SocketType;

	M_SOCKET_DECL TcpAcceptorService(IoServiceType& ioservice);

	M_SOCKET_DECL void Listen(Impl& impl, s_int32_t flag, SocketError& error);

	M_SOCKET_DECL void Accept(Impl& impl, Impl& peer, SocketError& error);

	template<typename AcceptHandler>
	M_SOCKET_DECL void AsyncAccept(Impl& impl_acceptor, Impl& impl_sock, const AcceptHandler& handler, SocketError& error);
};

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL TcpAcceptorService<Protocol, IoServiceType>::TcpAcceptorService(IoServiceType& ioservice)
	:BaseSocketService<Protocol, IoServiceType>(ioservice)
{
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void TcpAcceptorService<Protocol, IoServiceType>::Listen(Impl& impl, s_int32_t flag, SocketError& error)
{
	Access::Listen(this->_ioservice, impl, flag, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void TcpAcceptorService<Protocol, IoServiceType>::Accept(Impl& impl, Impl& peer, SocketError& error)
{
	Access::Accept(this->_ioservice, impl, peer, error);
}

template<typename Protocol, typename IoServiceType>
template<typename AcceptHandler>
M_SOCKET_DECL void TcpAcceptorService<Protocol, IoServiceType>::AsyncAccept(Impl& impl_acceptor, Impl& impl_sock
	, const AcceptHandler& handler, SocketError& error)
{
	Access::AsyncAccept(this->_ioservice, impl_acceptor, impl_sock, handler, error);
}

M_SOCKET_NAMESPACE_END
#endif
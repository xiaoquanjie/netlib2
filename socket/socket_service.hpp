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

#ifndef M_SOCKET_SERVICE_INCLUDE
#define M_SOCKET_SERVICE_INCLUDE

#include "config.hpp"
M_SOCKET_NAMESPACE_BEGIN

enum
{
	E_NULL_SOCKET_TYPE = 0,
	E_SOCKET_TYPE = 1,
	E_ACCEPTOR_TYPE = 2,
	E_CONNECTOR_TYPE = 3,
};
enum
{
	E_NULL_OP = 0,			// null op,0
	E_ACCEPT_OP = (1<<0),	// accept op,1
	E_CONNECT_OP = (1<<1),	// connect op,2
	E_WRITE_OP = (1<<2),	// write op,4
	E_READ_OP = (1<<3),		// read op,8
	E_FINISH_OP = (1<<4),	// finish op,16
};
enum
{
	E_NULL_STATE = 0,				// non state,0
	E_ACCEPTING_STATE = (1<< 0),	// accepting state,1
	E_CONNECTING_STATE = (1<<1),	// connecting state,2
	E_WRITING_STATE = (1<<2),		// writing state,4
	E_READING_STATE = (1 << 3),		// reading state,8
};

template<typename Protocol,typename IoService>
class BaseSocketService
{
public:
	typedef Protocol  ProtocolType;
	typedef IoService IoServiceType;
	typedef typename IoServiceType::Impl		Impl;
	typedef typename IoServiceType::Access		Access;
	typedef typename  Protocol::EndPoint		EndPoint;

	M_SOCKET_DECL BaseSocketService(IoServiceType& ioservice);

	M_SOCKET_DECL void Construct(Impl& impl,s_uint16_t type);

	M_SOCKET_DECL void Destroy(Impl& impl);

	M_SOCKET_DECL void Close(Impl& impl,SocketError& error);

	M_SOCKET_DECL bool IsOpen(Impl& impl, SocketError& error)const;

	template<typename GettableOptionType>
	M_SOCKET_DECL void GetOption(Impl& impl, GettableOptionType& opt, SocketError& error)const;

	template<typename SettableOptionType>
	M_SOCKET_DECL void SetOption(Impl& impl, const SettableOptionType& opt, SocketError& error);

	M_SOCKET_DECL EndPoint RemoteEndPoint(const Impl& impl, SocketError& error)const;

	M_SOCKET_DECL EndPoint LocalEndPoint(const Impl& impl, SocketError& error)const;

	M_SOCKET_DECL void Shutdown(Impl& impl, EShutdownType what, SocketError& error);

	M_SOCKET_DECL void Open(Impl& impl, ProtocolType pt,SocketError& error);

	M_SOCKET_DECL void Bind(Impl& impl, const EndPoint& ep, SocketError& error);

	M_SOCKET_DECL void Cancel(Impl& impl, SocketError& error);

	M_SOCKET_DECL IoServiceType& GetIoService() {
		return _ioservice;
	}

protected:
	IoServiceType& _ioservice;
};

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL BaseSocketService<Protocol, IoServiceType>::BaseSocketService(IoServiceType& ioservice)
	:_ioservice(ioservice)
{}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::Construct(Impl& impl, s_uint16_t type)
{
	Access::ConstructImpl(_ioservice, impl, type);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::Destroy(Impl& impl)
{
	Access::DestroyImpl(_ioservice, impl);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::Close(Impl& impl, SocketError& error)
{
	Access::Close(_ioservice, impl, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL bool BaseSocketService<Protocol, IoServiceType>::IsOpen(Impl& impl, SocketError& error)const
{
	return Access::IsOpen(_ioservice, impl, error);
}

template<typename Protocol, typename IoServiceType>
template<typename GettableOptionType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::GetOption(Impl& impl, GettableOptionType& opt, SocketError& error)const
{
	Access::GetOption(_ioservice, impl, opt, error);
}

template<typename Protocol, typename IoServiceType>
template<typename SettableOptionType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::SetOption(Impl& impl, const SettableOptionType& opt, SocketError& error)
{
	Access::SetOption(_ioservice, impl, opt, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL typename BaseSocketService<Protocol, IoServiceType>::EndPoint BaseSocketService<Protocol, IoServiceType>::RemoteEndPoint(const Impl& impl, SocketError& error)const
{
	 return Access::RemoteEndPoint(EndPoint(), _ioservice, impl, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL typename BaseSocketService<Protocol, IoServiceType>::EndPoint BaseSocketService<Protocol, IoServiceType>::LocalEndPoint(const Impl& impl, SocketError& error)const
{
	return IoServiceType::Access::LocalEndPoint(EndPoint(), _ioservice, impl, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::Shutdown(Impl& impl, EShutdownType what, SocketError& error)
{
	Access::Shutdown(_ioservice, impl, what, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::Open(Impl& impl, ProtocolType pt,SocketError& error)
{
	Access::Open(_ioservice, impl, pt, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::Bind(Impl& impl, const EndPoint& ep, SocketError& error)
{
	Access::Bind(_ioservice, impl, ep, error);
}

template<typename Protocol, typename IoServiceType>
M_SOCKET_DECL void BaseSocketService<Protocol, IoServiceType>::Cancel(Impl& impl, SocketError& error)
{
	Access::Cancel(_ioservice, impl, error);
}


M_SOCKET_NAMESPACE_END
#endif
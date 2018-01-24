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

#ifndef M_BASIC_SOCKET_INCLUDE
#define M_BASIC_SOCKET_INCLUDE

#include "socket/io_object.hpp"
#include "socket/socket_service.hpp"
#include "socket/option.hpp"
M_SOCKET_NAMESPACE_BEGIN

template <typename Protocol, typename SocketService>
class BasicSocket : public SocketIoObject<SocketService> , public Opts
{
public:
	typedef typename SocketService::IoServiceType	IoServiceType;
	typedef typename Protocol::EndPoint				EndPoint;
	
	M_SOCKET_DECL BasicSocket(IoServiceType& ioservice);

	M_SOCKET_DECL virtual ~BasicSocket();

	// 此函数是非线程安全函数,调用要慎重。如果外部可以不调用，则最好不要调用，由析构函数自动调用
	M_SOCKET_DECL void Close();
	
	M_SOCKET_DECL void Close(SocketError& error);

	M_SOCKET_DECL void Close(function_t<void()> handler);

	M_SOCKET_DECL void Close(function_t<void()> handler, SocketError& error);

	M_SOCKET_DECL void Shutdown(EShutdownType what);

	M_SOCKET_DECL void Shutdown(EShutdownType what,SocketError& error);

	M_SOCKET_DECL bool IsOpen();

	template<typename GettableOptionType>
	M_SOCKET_DECL void GetOption(GettableOptionType& opt);
	
	template<typename GettableOptionType>
	M_SOCKET_DECL void GetOption(GettableOptionType& opt,SocketError& error);

	template<typename SettableOptionType>
	M_SOCKET_DECL void SetOption(const SettableOptionType& opt);
	
	template<typename SettableOptionType>
	M_SOCKET_DECL void SetOption(const SettableOptionType& opt,SocketError& error);

	M_SOCKET_DECL EndPoint RemoteEndPoint()const;

	M_SOCKET_DECL EndPoint RemoteEndPoint(SocketError& error)const;
	
	M_SOCKET_DECL EndPoint LocalEndPoint()const;

	M_SOCKET_DECL EndPoint LocalEndPoint(SocketError& error)const;

	socket_t GetFd()const;
};

template <typename Protocol, typename SocketService>
M_SOCKET_DECL BasicSocket<Protocol, SocketService>::BasicSocket(IoServiceType& ioservice)
	:SocketIoObject<SocketService>(ioservice)
{
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL BasicSocket<Protocol, SocketService>::~BasicSocket()
{
	SocketError error;
	this->Close(error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::Close()
{
	SocketError error;
	this->Close(0,error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::Close(SocketError& error)
{
	this->Close(0, error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::Close(function_t<void()> handler) {
	SocketError error;
	this->Close(handler, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::Close(function_t<void()> handler, SocketError& error) {
	this->GetObjectService().Close(this->GetImpl(), handler, error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::Shutdown(EShutdownType what)
{
	SocketError error;
	this->Shutdown(what, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::Shutdown(EShutdownType what, SocketError& error)
{
	this->GetObjectService().Shutdown(this->GetImpl(), what, error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL bool BasicSocket<Protocol, SocketService>::IsOpen()
{
	SocketError error;
	return this->GetObjectService().IsOpen(this->GetImpl(), error);
}

template <typename Protocol, typename SocketService>
template<typename GettableOptionType>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::GetOption(GettableOptionType& opt)
{
	SocketError error;
	GetOption(opt, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template <typename Protocol, typename SocketService>
template<typename GettableOptionType>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::GetOption(GettableOptionType& opt, SocketError& error)
{
	this->GetObjectService().GetOption(this->GetImpl(), opt, error);
}

template <typename Protocol, typename SocketService>
template<typename SettableOptionType>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::SetOption(const SettableOptionType& opt)
{
	SocketError error;
	this->SetOption(opt, error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
}

template <typename Protocol, typename SocketService>
template<typename SettableOptionType>
M_SOCKET_DECL void BasicSocket<Protocol, SocketService>::SetOption(const SettableOptionType& opt, SocketError& error)
{
	this->GetObjectService().SetOption(this->GetImpl(), opt, error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL typename BasicSocket<Protocol, SocketService>::EndPoint BasicSocket<Protocol, SocketService>::RemoteEndPoint()const
{
	SocketError error;
	EndPoint ep = this->RemoteEndPoint(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL typename BasicSocket<Protocol, SocketService>::EndPoint BasicSocket<Protocol, SocketService>::RemoteEndPoint(SocketError& error)const
{
	return this->GetObjectService().RemoteEndPoint(this->GetImpl(), error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL typename BasicSocket<Protocol, SocketService>::EndPoint BasicSocket<Protocol, SocketService>::LocalEndPoint()const
{
	SocketError error;
	EndPoint ep = this->LocalEndPoint(error);
	M_THROW_DEFAULT_SOCKET_ERROR2(error);
	return ep;
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL typename BasicSocket<Protocol, SocketService>::EndPoint BasicSocket<Protocol, SocketService>::LocalEndPoint(SocketError& error)const
{
	return this->GetObjectService().LocalEndPoint(this->GetImpl(), error);
}

template <typename Protocol, typename SocketService>
M_SOCKET_DECL socket_t BasicSocket<Protocol, SocketService>::GetFd()const {
	return this->GetObjectService().GetFd(this->GetImpl());
}

M_SOCKET_NAMESPACE_END
#endif
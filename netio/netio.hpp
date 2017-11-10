/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_INCLUDE
#define M_NETIO_NETIO_INCLUDE

#include "config.hpp"
#include "tcp_socket.hpp"

M_NETIO_NAMESPACE_BEGIN

typedef shard_ptr_t<SocketLib::TcpAcceptor<IoService> > NetIoTcpAcceptorPtr;

class NetIo 
{
public:
	NetIo();

	NetIo(SocketLib::s_uint32_t backlog);

	virtual ~NetIo();

	// 建立一个监听
	bool ListenOne(const SocketLib::Tcp::EndPoint& ep);

	void Run();

	void Stop();

	inline SocketLib::SocketError GetLastError()const;

	inline SocketLib::IoService& GetIoService();

protected:
	virtual void AcceptHandler(SocketLib::SocketError error, TcpSocketPtr clisock, NetIoTcpAcceptorPtr acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

private:
	SocketLib::IoService  _ioservice;
	SocketLib::s_uint32_t _backlog;
	SocketLib::SocketError _lasterror;
};

/////////////////////////////////////////////////////////////////////////////////////

NetIo::NetIo():_backlog(20){
}

NetIo::NetIo(SocketLib::s_uint32_t backlog) : _backlog(backlog) {
}

NetIo::~NetIo(){}

bool NetIo::ListenOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		TcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		//TcpSocketPtr clisock(new TcpSocket<SocketLib::IoService>(_ioservice));
		//function_t<void(SocketLib::SocketError)> handler = bind_t(&NetIo::AcceptHandler, this, placeholder_1, clisock, acceptor);
	}
	catch (SocketLib::SocketError& error) {
		return false;
	}
	return true;
}

void NetIo::Run(){

}

void NetIo::Stop() {
	try {
		_ioservice.Run();
	}
	catch (SocketLib::SocketError& error) {

	}
}

inline SocketLib::SocketError NetIo::GetLastError()const {
	return _lasterror;
}

inline SocketLib::IoService& NetIo::GetIoService() {
	return _ioservice;
}

void NetIo::AcceptHandler(SocketLib::SocketError error, TcpSocketPtr clisock, NetIoTcpAcceptorPtr acceptor) {

}

M_NETIO_NAMESPACE_END
#endif
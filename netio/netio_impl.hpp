/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/11
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_IMPL_INCLUDE
#define M_NETIO_NETIO_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

NetIo::NetIo() :_backlog(20) {
}

NetIo::NetIo(SocketLib::s_uint32_t backlog) : _backlog(backlog) {
}

NetIo::~NetIo() {}

bool NetIo::ListenOne(const SocketLib::Tcp::EndPoint& ep) {
	try {
		NetIoTcpAcceptorPtr acceptor(new SocketLib::TcpAcceptor<SocketLib::IoService>(_ioservice, ep, _backlog));
		TcpSocketPtr clisock(new TcpSocket(*this, 0, 0));
		function_t<void(SocketLib::SocketError)> handler = bind_t(&NetIo::AcceptHandler, this, placeholder_1, clisock, acceptor);
		acceptor->AsyncAccept(handler, clisock->GetSocket());
	}
	catch (SocketLib::SocketError& error) {
		return false;
	}
	return true;
}

void NetIo::Run() {

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
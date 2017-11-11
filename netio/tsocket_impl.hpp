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

#ifndef M_NETIO_TSOCKET_IMPL_INCLUDE
#define M_NETIO_TSOCKET_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

TcpSocket::TcpSocket(NetIo& netio, MessageReceiver receiver, MessageHeaderChecker checker)
	:_netio(netio), _message_receiver(receiver), _header_checker(checker) {
	_socket = new SocketLib::TcpSocket<SocketLib::IoService>(_netio.GetIoService());
}

SocketLib::TcpSocket<SocketLib::IoService>& TcpSocket::GetSocket() {
	return *_socket;
}

void TcpSocket::WriteHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {

}

void TcpSocket::ReadHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {

}


M_NETIO_NAMESPACE_END
#endif
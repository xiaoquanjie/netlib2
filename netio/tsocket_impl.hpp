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

#define M_READ_SIZE (4*1024)

TcpSocket::_readerinfo_::_readerinfo_() {
	readbuf = new SocketLib::s_byte_t[M_READ_SIZE];
	g_memset(readbuf, 0, M_READ_SIZE);
}

TcpSocket::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

TcpSocket::_writerinfo_::_writerinfo_() {
	writing = false;
}

TcpSocket::TcpSocket(NetIo& netio, MessageReceiver receiver, MessageHeaderChecker checker)
	:_netio(netio), _message_receiver(receiver), _header_checker(checker) {
	_stopped = true;
	_socket = new SocketLib::TcpSocket<SocketLib::IoService>(_netio.GetIoService());
}

TcpSocket::~TcpSocket() {
	delete _socket;
}

SocketLib::TcpSocket<SocketLib::IoService>& TcpSocket::GetSocket() {
	return *_socket;
}

void TcpSocket::Init() {
	try {
		_localep = _socket->LocalEndPoint();
		_remoteep = _socket->RemoteEndPoint();
		_stopped = false;
		_netio.OnConnected(shared_from_this());

		function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler = 
			bind_t(&TcpSocket::_ReadHandler, shared_from_this(), placeholder_1, placeholder_2);
		_socket->AsyncRecvSome(handler,_reader.readbuf,M_READ_SIZE);
	}
	catch (const SocketLib::SocketError& e) {
		lasterror = e;
	}
}

const SocketLib::Tcp::EndPoint& TcpSocket::LocalEndpoint()const {
	return _localep;
}

const SocketLib::Tcp::EndPoint& TcpSocket::RemoteEndpoint()const {
	return _remoteep;
}

void TcpSocket::PostClose() {
	SocketLib::ScopedLock scoped_r(_reader.lock);
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_Close();
}

void TcpSocket::_Close() {
	if (!_stopped) {
		_stopped = true;
	}
}

void TcpSocket::Send(SocketLib::Buffer* buffer) {
	SocketLib::ScopedLock scoped_r(_writer.lock);
	if (!_stopped) {
		_writer.buffer_pool.push_back(buffer);
		_TrySendData();
	}
}

void TcpSocket::_WriteHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {

}

void TcpSocket::_ReadHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {
	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("read handler happend error:" << M_ERROR_DESC_STR(error));
		PostClose();
	}
	else if (tran_byte<=0){
		// 对方关闭写
		PostClose();
	}
	else {
		_reader.msgbuffer.Write(_reader.readbuf, tran_byte);
		_reader.readbuf[0] = 0;
		if (_CutMsgPack()) 
		{
			SocketLib::ScopedLock scoped_r(_reader.lock);
			if (!_stopped) {
				function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
					bind_t(&TcpSocket::_ReadHandler, shared_from_this(), placeholder_1, placeholder_2);
				_socket->AsyncRecvSome(handler, _reader.readbuf, M_READ_SIZE);
			}
		}
		else {
			// 数据检查出错，主动断开连接
			PostClose();
		}
	}
}

bool TcpSocket::_CutMsgPack() {
	return true;
}

bool TcpSocket::_TrySendData() {
	if (!_writer.writing && _writer.buffer_pool.size()) 
	{
		_writer.writing = true;
		SocketLib::Buffer* pbuffer = _writer.buffer_pool.front();
		_writer.msgbuffer.reset(pbuffer);

		function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
			bind_t(&TcpSocket::_WriteHandler, shared_from_this(), placeholder_1, placeholder_2);
		_socket->AsyncSendSome(handler, _writer.msgbuffer->Data(), _writer.msgbuffer->Length());
		return true;
	}
	return false;
}

M_NETIO_NAMESPACE_END
#endif
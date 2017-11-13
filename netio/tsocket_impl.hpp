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
	g_memset(&curheader, 0, sizeof(curheader));
	readbuf = new SocketLib::s_byte_t[M_READ_SIZE];
	g_memset(readbuf, 0, M_READ_SIZE);
}

TcpSocket::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

TcpSocket::_writerinfo_::_writerinfo_() {
	writing = false;
}

TcpSocket::TcpSocket(NetIo& netio, MessageChecker checker)
	:_netio(netio), _msgchecker(checker){
	_flag = E_TCPSOCKET_STATE_STOP;
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
		_remoteep = _socket->RemoteEndPoint();
		_localep = _socket->LocalEndPoint();
		_flag = E_TCPSOCKET_STATE_START;
		_netio.OnConnected(shared_from_this());
		_flag |= E_TCPSOCKET_STATE_READ;
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

void TcpSocket::Close() {
	_PostClose(E_TCPSOCKET_STATE_START);
}

void TcpSocket::_PostClose(unsigned int state) {
	// 这里一定要写双锁，可以思考下重要性
	SocketLib::ScopedLock scoped_r(_reader.lock);
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_Close(state);
}

void TcpSocket::_Close(unsigned int state) {
	if (_flag & state) {
		unsigned int tmp_flag = _flag;
		_flag &= ~state;
		if (_flag == E_TCPSOCKET_STATE_STOP
			&& tmp_flag != E_TCPSOCKET_STATE_STOP) {
			// 通知连接断开
			_netio.OnDisconnected(shared_from_this());
		}
	}
}

void TcpSocket::Send(SocketLib::Buffer* buffer) {
	SocketLib::ScopedLock scoped_w(_writer.lock);
	if (_flag & E_TCPSOCKET_STATE_START) {
		_writer.buffer_pool.push_back(buffer);
		_TrySendData();
	}
	delete buffer;
}

void TcpSocket::Send(SocketLib::s_byte_t* data, SocketLib::s_uint16_t len) {
	MessageHeader hdr;
	hdr.endian = _netio.LocalEndian();
	hdr.size = len;
	hdr.timestamp = (unsigned int)time(0);

	SocketLib::Buffer* buffer = new SocketLib::Buffer();
	buffer->Write(hdr);
	buffer->Write(data, len);
	Send(buffer);
}

void TcpSocket::_WriteHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {
	_writer.writing = false;
	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("write handler happend error:"M_ERROR_DESC_STR(error));
		_PostClose(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_WRITE);
	}
	else if (tran_byte <= 0) {
		// 连接已经关闭
		_PostClose(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_WRITE);
	}
	else {
		SocketLib::ScopedLock scoped_w(_writer.lock);
		_writer.msgbuffer->RemoveData(tran_byte);
		if (!_TrySendData() && !(_flag & E_TCPSOCKET_STATE_START)){
			// 数据发送完后，如果状态不是E_TCPSOCKET_STATE_START，则需要关闭写
			_socket->Shutdown(SocketLib::E_Shutdown_WR);
			_Close(E_TCPSOCKET_STATE_WRITE);
		}
	}
}

void TcpSocket::_ReadHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {
	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("read handler happend error:" << M_ERROR_DESC_STR(error));
		_PostClose(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_READ);
	}
	else if (tran_byte<=0){
		// 对方关闭写
		_PostClose(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_READ);
	}
	else {
		if (_CutMsgPack(_reader.readbuf, tran_byte))
		{
			SocketLib::ScopedLock scoped_r(_reader.lock);
			if (_flag & E_TCPSOCKET_STATE_START){
				function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
					bind_t(&TcpSocket::_ReadHandler, shared_from_this(), placeholder_1, placeholder_2);
				_socket->AsyncRecvSome(handler, _reader.readbuf, M_READ_SIZE);
			}
			else {
				_Close(E_TCPSOCKET_STATE_READ);
			}
		}
		else {
			// 数据检查出错，主动断开连接
			_socket->Shutdown(SocketLib::E_Shutdown_RD);
			_PostClose(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_READ);
		}
	}
}

bool TcpSocket::_CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte) {
	// 减少内存拷贝是此函数的设计关键
	SocketLib::s_uint32_t hdrlen = (SocketLib::s_uint32_t)sizeof(MessageHeader);
	SocketLib::s_uint32_t datalen = _reader.msgbuffer.Length();

	// 算出头部长度
	if (_reader.curheader.size == 0) {
		if (tran_byte + datalen < hdrlen) {
			_reader.msgbuffer.Write(buf, tran_byte);
			return true;
		}
		else {
			_reader.msgbuffer.Write(buf, hdrlen - datalen);
			buf += (hdrlen - datalen);
			tran_byte -= (hdrlen - datalen);

			_reader.msgbuffer.Read(_reader.curheader);
			if (_reader.curheader.endian != _netio.LocalEndian()) {
				_reader.curheader.size = g_htons(_reader.curheader.size);
				_reader.curheader.timestamp = g_htonl(_reader.curheader.timestamp);
			}

			// 校验
			if (_reader.curheader.size > (0xFFFF - hdrlen))
				return false;
		}
	}

	// 拷贝body
	datalen = _reader.msgbuffer.Length();
	if (tran_byte + datalen < _reader.curheader.size) {
		_reader.msgbuffer.Write(buf, tran_byte);
	}
	else {
		_reader.msgbuffer.Write(buf, _reader.curheader.size - datalen);
		buf += (_reader.curheader.size - datalen);
		tran_byte -= (_reader.curheader.size - datalen);

		BufferPtr tmp_bufferptr(new SocketLib::Buffer());
		tmp_bufferptr->Write(buf, tran_byte);
		// swap
		tmp_bufferptr->Swap(_reader.msgbuffer);
		if (_msgchecker)
			if (!_msgchecker(shared_from_this(), _reader.curheader, tmp_bufferptr))
				return false;
		// notify
		_netio.OnReceiveData(shared_from_this(), tmp_bufferptr);
	}
	return true;
}

bool TcpSocket::_TrySendData() {
	if (!_writer.writing) 
	{
		if (_writer.msgbuffer->Length() == 0 && _writer.buffer_pool.size() > 0) {
			SocketLib::Buffer* pbuffer = _writer.buffer_pool.front();
			_writer.msgbuffer.reset(pbuffer);
		}
		if (_writer.msgbuffer->Length() != 0) {
			_writer.writing = true;
			_flag |= E_TCPSOCKET_STATE_WRITE;
			function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
				bind_t(&TcpSocket::_WriteHandler, shared_from_this(), placeholder_1, placeholder_2);
			_socket->AsyncSendSome(handler, _writer.msgbuffer->Data(), _writer.msgbuffer->Length());
			return true;
		}
		else {
			_writer.writing = false;
		}
	}
	return false;
}

M_NETIO_NAMESPACE_END
#endif
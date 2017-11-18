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

template<typename T, typename SocketType, typename CheckerType>
TcpBaseSocket<T, SocketType, CheckerType>::_readerinfo_::_readerinfo_() {
	g_memset(&curheader, 0, sizeof(curheader));
	readbuf = new SocketLib::s_byte_t[M_READ_SIZE];
	g_memset(readbuf, 0, M_READ_SIZE);
}

template<typename T, typename SocketType, typename CheckerType>
TcpBaseSocket<T, SocketType, CheckerType>::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

template<typename T, typename SocketType, typename CheckerType>
TcpBaseSocket<T, SocketType, CheckerType>::_writerinfo_::_writerinfo_() {
	writing = false;
}

template<typename T, typename SocketType, typename CheckerType>
TcpBaseSocket<T, SocketType, CheckerType>::_writerinfo_::~_writerinfo_() {
	for (std::list<SocketLib::Buffer*>::iterator iter = buffer_pool.begin();
		iter != buffer_pool.end(); ++iter)
		delete (*iter);
	buffer_pool.clear();
}

template<typename T, typename SocketType, typename CheckerType>
TcpBaseSocket<T, SocketType, CheckerType>::TcpBaseSocket(NetIo& netio, CheckerType checker)
	:_netio(netio) ,_msgchecker(checker){
	_flag = E_TCPSOCKET_STATE_STOP;
	_socket = new SocketType(_netio.GetIoService());
}

template<typename T, typename SocketType, typename CheckerType>
TcpBaseSocket<T, SocketType, CheckerType>::~TcpBaseSocket() {
	delete _socket;
}

template<typename T, typename SocketType, typename CheckerType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType, CheckerType>::LocalEndpoint()const {
	return _localep;
}

template<typename T, typename SocketType, typename CheckerType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType, CheckerType>::RemoteEndpoint()const {
	return _remoteep;
}

template<typename T, typename SocketType, typename CheckerType>
void TcpBaseSocket<T, SocketType, CheckerType>::Close() {
	_PostClose(E_TCPSOCKET_STATE_START);
}

template<typename T, typename SocketType, typename CheckerType>
void TcpBaseSocket<T, SocketType, CheckerType>::_PostClose(unsigned int state) {
	// 这里一定要写双锁，可以思考下重要性
	SocketLib::ScopedLock scoped_r(_reader.lock);
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_Close(state);
}

template<typename T, typename SocketType, typename CheckerType>
void TcpBaseSocket<T, SocketType, CheckerType>::_Close(unsigned int state) {
	if (_flag & state) {
		unsigned int tmp_flag = _flag;
		_flag &= ~state;
		// if _flag is equal to stop flag or just equal to read flag ,have to close the link
		if ((_flag == E_TCPSOCKET_STATE_STOP || _flag == E_TCPSOCKET_STATE_READ)
			&& tmp_flag != E_TCPSOCKET_STATE_STOP) {
			// 通知连接断开
			_flag = E_TCPSOCKET_STATE_STOP;
			_socket->Shutdown(SocketLib::E_Ehutdown_BOTH);

			SocketLib::SocketError error;
			function_t<void()> handler = bind_t(&TcpBaseSocket::_CloseHandler, this->shared_from_this());
			_socket->Close(handler, error);
		}
	}
}

template<typename T, typename SocketType, typename CheckerType>
void TcpBaseSocket<T, SocketType, CheckerType>::Send(SocketLib::Buffer* buffer) {
	SocketLib::ScopedLock scoped_w(_writer.lock);
	SocketLib::ScopedLock scoped_r(_reader.lock);
	if (_flag & E_TCPSOCKET_STATE_START) {
		_writer.buffer_pool.push_back(buffer);
		_TrySendData();
	}
	else
		delete buffer;
}

template<typename T, typename SocketType, typename CheckerType>
void TcpBaseSocket<T, SocketType, CheckerType>::Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len) {
	if (len <= 0)
		return;
	MessageHeader hdr;
	hdr.endian = _netio.LocalEndian();
	hdr.size = len;
	hdr.timestamp = (unsigned int)time(0);

	SocketLib::Buffer* buffer = new SocketLib::Buffer();
	buffer->Write(hdr);
	buffer->Write((void*)data, len);
	Send(buffer);
}

template<typename T, typename SocketType, typename CheckerType>
void TcpBaseSocket<T, SocketType, CheckerType>::_WriteHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {
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
		_writer.writing = false;
		_writer.msgbuffer->RemoveData(tran_byte);
		if (!_TrySendData() && !(_flag & E_TCPSOCKET_STATE_START)) {
			// 数据发送完后，如果状态不是E_TCPSOCKET_STATE_START，则需要关闭写
			_socket->Shutdown(SocketLib::E_Shutdown_WR);
			_Close(E_TCPSOCKET_STATE_WRITE);
		}
	}
}

template<typename T, typename SocketType, typename CheckerType>
void TcpBaseSocket<T, SocketType, CheckerType>::_ReadHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error) {
	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("read handler happend error:" << M_ERROR_DESC_STR(error));
		_PostClose(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_READ);
	}
	else if (tran_byte <= 0) {
		// 对方关闭写
		_PostClose(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_READ);
	}
	else {
		SocketLib::ScopedLock scoped_r(_reader.lock);
		if (_flag & E_TCPSOCKET_STATE_START) {
			if (_CutMsgPack(_reader.readbuf, tran_byte)) {
				function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
					bind_t(&TcpBaseSocket::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2);
				_socket->AsyncRecvSome(handler, _reader.readbuf, M_READ_SIZE);
			}
			else {
				// 数据检查出错，主动断开连接
				_socket->Shutdown(SocketLib::E_Shutdown_RD);
				_Close(E_TCPSOCKET_STATE_START | E_TCPSOCKET_STATE_READ);
			}
		}
		else
			_Close(E_TCPSOCKET_STATE_READ);
	}
}

template<typename T, typename SocketType, typename CheckerType>
inline void TcpBaseSocket<T, SocketType, CheckerType>::_CloseHandler() {
	_netio.OnDisconnected(this->shared_from_this());
}

template<typename T, typename SocketType, typename CheckerType>
bool TcpBaseSocket<T, SocketType, CheckerType>::_CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte) {
	// 减少内存拷贝是此函数的设计关键
	SocketLib::s_uint32_t hdrlen = (SocketLib::s_uint32_t)sizeof(MessageHeader);
	do 
	{
		// 算出头部长度
		SocketLib::s_uint32_t datalen = _reader.msgbuffer.Length();
		if (_reader.curheader.size == 0) {
			if (tran_byte + datalen < hdrlen) {
				_reader.msgbuffer.Write(buf, tran_byte);
				return true;
			}
			else if (datalen >= hdrlen) {
				_reader.msgbuffer.Read(_reader.curheader);
			}
			else {
				_reader.msgbuffer.Write(buf, hdrlen - datalen);
				buf += (hdrlen - datalen);
				tran_byte -= (hdrlen - datalen);
				_reader.msgbuffer.Read(_reader.curheader);
			}

			// convert byte order
			if (_reader.curheader.endian != _netio.LocalEndian()) {
				_reader.curheader.size = g_htons(_reader.curheader.size);
				_reader.curheader.timestamp = g_htonl(_reader.curheader.timestamp);
			}
			// check
			if (_reader.curheader.size > (0xFFFF - hdrlen))
				return false;
		}

		// copy body data
		datalen = _reader.msgbuffer.Length();
		if (tran_byte + datalen < _reader.curheader.size) {
			_reader.msgbuffer.Write(buf, tran_byte);
			return true;
		}
		else {
			_reader.msgbuffer.Write(buf, _reader.curheader.size - datalen);
			buf += (_reader.curheader.size - datalen);
			tran_byte -= (_reader.curheader.size - datalen);

			// swap
			BufferPtr tmp_bufferptr(new SocketLib::Buffer());
			tmp_bufferptr->Swap(_reader.msgbuffer);
			// check
			if (_msgchecker)
				if (!_msgchecker(this->shared_from_this(), _reader.curheader, tmp_bufferptr))
					return false;

			// notify
			_reader.curheader.size = 0;
			_netio.OnReceiveData(this->shared_from_this(), tmp_bufferptr);
		}

	} while (1);

	return true;
}

template<typename T, typename SocketType, typename CheckerType>
bool TcpBaseSocket<T, SocketType, CheckerType>::_TrySendData() {
	if (!_writer.writing)
	{
		if ((!_writer.msgbuffer || _writer.msgbuffer->Length() == 0)
			&& _writer.buffer_pool.size() > 0) {
			SocketLib::Buffer* pbuffer = _writer.buffer_pool.front();
			_writer.msgbuffer.reset(pbuffer);
			_writer.buffer_pool.pop_front();
		}
		if (_writer.msgbuffer && _writer.msgbuffer->Length() != 0) {
			_writer.writing = true;
			_flag |= E_TCPSOCKET_STATE_WRITE;
			function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
				bind_t(&TcpBaseSocket::_WriteHandler, this->shared_from_this(), placeholder_1, placeholder_2);
			_socket->AsyncSendSome(handler, _writer.msgbuffer->Data(), _writer.msgbuffer->Length());
			return true;
		}
		else {
			_flag &= ~E_TCPSOCKET_STATE_WRITE;
			_writer.writing = false;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////

TcpSocket::TcpSocket(NetIo& netio, MessageChecker checker)
	:TcpBaseSocket(netio,checker){
}

SocketLib::TcpSocket<SocketLib::IoService>& TcpSocket::GetSocket() {
	return (*this->_socket);
}

void TcpSocket::Init() {
	try {
		_remoteep = _socket->RemoteEndPoint();
		_localep = _socket->LocalEndPoint();
		_flag = E_TCPSOCKET_STATE_START;
		_netio.OnConnected(this->shared_from_this());
		_flag |= E_TCPSOCKET_STATE_READ;
		function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler = 
			bind_t(&TcpSocket::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2);
		_socket->AsyncRecvSome(handler,_reader.readbuf,M_READ_SIZE);
	}
	catch (const SocketLib::SocketError& e) {
		lasterror = e;
	}
}


M_NETIO_NAMESPACE_END
#endif
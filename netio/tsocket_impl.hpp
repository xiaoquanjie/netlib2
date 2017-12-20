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

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::_writerinfo_::_writerinfo_() {
	writing = false;
	msgbuffer = 0;
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::_writerinfo_::~_writerinfo_() {
	SocketLib::Buffer* pbuffer;
	while (buffer_pool.size()) {
		pbuffer = buffer_pool.front();
		buffer_pool.pop_front();
		delete pbuffer;
	}
	while (buffer_pool2.size()) {
		pbuffer = buffer_pool2.back();
		buffer_pool2.pop_back();
		delete pbuffer;
	}
	delete msgbuffer;
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::TcpBaseSocket(BaseNetIo<NetIo>& netio)
	:_netio(netio) {
	_flag = E_STATE_STOP;
	_socket = new SocketType(_netio.GetIoService());
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::~TcpBaseSocket() {
	delete _socket;
}

template<typename T, typename SocketType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType>::LocalEndpoint()const {
	return _localep;
}

template<typename T, typename SocketType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType>::RemoteEndpoint()const {
	return _remoteep;
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::Close() {
	_PostClose(E_STATE_START);
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_PostClose(unsigned int state) {
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_Close(state);
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_Close(unsigned int state) {
	if (_flag == E_STATE_START || _writer.writing) {
		if (state == E_STATE_START)
			_flag = E_STATE_STOP;
		else if (state == E_STATE_WRITE) {
			_writer.writing = false;
			_flag = E_STATE_STOP;
		}
		if (_flag == E_STATE_STOP && !_writer.writing) {
			SocketLib::SocketError error;
			_socket->Close(bind_t(&TcpBaseSocket::_CloseHandler, this->shared_from_this()), error);
		}
	}
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::Send(SocketLib::Buffer* buffer) {
	SocketLib::ScopedLock scoped_w(_writer.lock);
	if (_flag == E_STATE_START) {
		SocketLib::Buffer* pBuffer;
		if (_writer.buffer_pool2.size()) {
			pBuffer = _writer.buffer_pool2.back();
			_writer.buffer_pool2.pop_back();
		}
		else
			pBuffer = new SocketLib::Buffer();

		pBuffer->Swap(*buffer);
		_writer.buffer_pool.push_back(pBuffer);
		_TrySendData();
	}
	else
		delete buffer;
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len) {
	if (len > 0) {
		SocketLib::ScopedLock scoped_w(_writer.lock);
		if (_flag != E_STATE_START)
			return;

		MessageHeader hdr;
		hdr.endian = _netio.LocalEndian();
		hdr.size = len;
		hdr.timestamp = (unsigned int)time(0);

		SocketLib::Buffer* buffer;
		if (_writer.buffer_pool2.size()) {
			buffer = _writer.buffer_pool2.back();
			_writer.buffer_pool2.pop_back();
		}
		else
			buffer = new SocketLib::Buffer();

		buffer->Clear();
		buffer->Write(hdr);
		buffer->Write((void*)data, len);
		_writer.buffer_pool.push_back(buffer);
		_TrySendData();
	}
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error) {
	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("write handler happend error:"M_ERROR_DESC_STR(error));
		_PostClose(E_STATE_WRITE);
	}
	else if (tran_byte <= 0) {
		// 连接已经关闭
		_PostClose(E_STATE_WRITE);
	}
	else {
		SocketLib::ScopedLock scoped_w(_writer.lock);
		_writer.msgbuffer->RemoveData(tran_byte);
		if (!_TrySendData(true) && !(_flag == E_STATE_START)) {
			// 数据发送完后，如果状态不是E_TCPSOCKET_STATE_START，则需要关闭写
			_socket->Shutdown(SocketLib::E_Shutdown_WR, error);
			_Close(E_STATE_WRITE);
		}
	}
}

template<typename T, typename SocketType>
inline void TcpBaseSocket<T, SocketType>::_CloseHandler() {
	_netio.OnDisconnected(this->shared_from_this());
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::_TrySendData(bool ignore) {
	if (!_writer.writing || ignore)
	{
		if ((!_writer.msgbuffer || _writer.msgbuffer->Length() == 0)
			&& _writer.buffer_pool.size() > 0) {
			SocketLib::Buffer* pbuffer = _writer.buffer_pool.front();
			if (_writer.msgbuffer) {
				if (_writer.buffer_pool2.size() < 10) {
					_writer.msgbuffer->Clear();
					_writer.buffer_pool2.push_back(_writer.msgbuffer);
				}
				else
					delete _writer.msgbuffer;
			}
			_writer.msgbuffer = pbuffer;
			_writer.buffer_pool.pop_front();
		}
		if (_writer.msgbuffer && _writer.msgbuffer->Length() != 0) {
			_writer.writing = true;
			SocketLib::SocketError error;
			_socket->AsyncSendSome(bind_t(&TcpBaseSocket::_WriteHandler, this->shared_from_this(), placeholder_1, placeholder_2)
				, _writer.msgbuffer->Data(), _writer.msgbuffer->Length(), error);
			if (error)
				_Close(E_STATE_WRITE);
			return (!error);
		}
		else {
			_writer.writing = false;
		}
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::_readerinfo_::_readerinfo_() {
	g_memset(&curheader, 0, sizeof(curheader));
	readbuf = new SocketLib::s_byte_t[M_READ_SIZE];
	g_memset(readbuf, 0, M_READ_SIZE);
}

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

template<typename T, typename SocketType>
void TcpStreamSocket<T, SocketType>::_ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error) {
	if (error) {
		// 出错关闭连接
		M_NETIO_LOGGER("read handler happend error:" << M_ERROR_DESC_STR(error));
		this->_PostClose(E_STATE_START);
	}
	else if (tran_byte <= 0) {
		// 对方关闭写
		this->_PostClose(E_STATE_START);
	}
	else {
		if (this->_flag == E_STATE_START) {
			if (_CutMsgPack(_reader.readbuf, tran_byte)) {
				_TryRecvData();
			}
			else {
				// 数据检查出错，主动断开连接
				this->_socket->Shutdown(SocketLib::E_Shutdown_RD, error);
				this->_PostClose(E_STATE_START);
			}
		}
		else {
			this->_PostClose(E_STATE_START);
		}
	}
}

template<typename T, typename SocketType>
bool TcpStreamSocket<T, SocketType>::_CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte) {
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
			if (_reader.curheader.endian != this->_netio.LocalEndian()) {
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
			_reader.msgbuffer2.Swap(_reader.msgbuffer);

			// notify
			_reader.curheader.size = 0;
			this->_netio.OnReceiveData(this->shared_from_this(), _reader.msgbuffer2);
			_reader.msgbuffer2.Clear();
		}
	} while (true);
	return true;
}

template<typename T, typename SocketType>
void TcpStreamSocket<T, SocketType>::_TryRecvData() {
	SocketLib::SocketError error;
	this->_socket->AsyncRecvSome(bind_t(&TcpStreamSocket::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2)
		, _reader.readbuf, M_READ_SIZE, error);
	if (error)
		this->_PostClose(E_STATE_START);
}

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::TcpStreamSocket(BaseNetIo<NetIo>& netio)
	:TcpBaseSocket<T, SocketType>(netio) {
}


M_NETIO_NAMESPACE_END
#endif
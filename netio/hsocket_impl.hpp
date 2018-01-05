/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/26
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
//----------------------------------------------------------------*/

#ifndef M_NETIO_HSOCKET_IMPL_INCLUDE
#define M_NETIO_HSOCKET_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

template<typename T, typename SocketType, typename HttpMsgType>
HttpBaseSocket<T, SocketType, HttpMsgType>::_readerinfo_::_readerinfo_() {
	readbuf = new SocketLib::s_byte_t[M_READ_SIZE];
	g_memset(readbuf, 0, M_READ_SIZE);
}

template<typename T, typename SocketType, typename HttpMsgType>
HttpBaseSocket<T, SocketType, HttpMsgType>::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

template<typename T, typename SocketType, typename HttpMsgType>
void HttpBaseSocket<T, SocketType, HttpMsgType>::_ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error) {
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

template<typename T, typename SocketType, typename HttpMsgType>
bool HttpBaseSocket<T, SocketType, HttpMsgType>::_CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte) {
	shard_ptr_t<T> ref;
	while (true) {
		SocketLib::s_uint32_t copy_len = (SocketLib::s_uint32_t)_reader.httpmsg.Parse(buf, tran_byte);
		if (copy_len == 0 || copy_len <= tran_byte) {
			if (!ref)
				ref = this->shared_from_this();
			this->_netio.OnReceiveData(ref, _reader.httpmsg);
			_reader.httpmsg.Clear();
		}
		tran_byte -= copy_len;
		if (tran_byte == 0)
			break;
		buf += copy_len;
	}
	return true;
}

template<typename T, typename SocketType, typename HttpMsgType>
void HttpBaseSocket<T, SocketType, HttpMsgType>::_TryRecvData() {
	SocketLib::SocketError error;
	this->_socket->AsyncRecvSome(bind_t(&HttpBaseSocket::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2)
		, _reader.readbuf, M_READ_SIZE, error);
	if (error)
		this->_PostClose(E_STATE_START);
}

template<typename T, typename SocketType, typename HttpMsgType>
HttpBaseSocket<T, SocketType, HttpMsgType>::HttpBaseSocket(BaseNetIo<NetIo>& netio)
	:TcpBaseSocket<T, SocketType>(netio) {
}


M_NETIO_NAMESPACE_END
#endif
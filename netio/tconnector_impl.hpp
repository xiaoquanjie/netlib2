/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/14
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_TCONNECTOR_IMPL_INCLUDE
#define M_NETIO_TCONNECTOR_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

template<typename ConnectorType>
BaseTcpConnector<ConnectorType>::BaseTcpConnector(BaseNetIo<NetIo>& netio)
	:TcpStreamSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService> >(netio)
	, _data(0) {
}

template<typename ConnectorType>
SocketLib::TcpConnector<SocketLib::IoService>& BaseTcpConnector<ConnectorType>::GetSocket() {
	return *this->_socket;
}

template<typename ConnectorType>
bool BaseTcpConnector<ConnectorType>::Connect(const SocketLib::Tcp::EndPoint& ep, SocketLib::s_uint32_t timeo_sec) {
	try {
		this->_socket->Connect(ep,timeo_sec);
		return true;
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
}

template<typename ConnectorType>
bool BaseTcpConnector<ConnectorType>::Connect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::s_uint32_t timeo_sec) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return Connect(ep,timeo_sec);
}

template<typename ConnectorType>
void BaseTcpConnector<ConnectorType>::AsyncConnect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		function_t<void(SocketLib::SocketError)> handler = bind_t(&BaseTcpConnector<ConnectorType>::_ConnectHandler, this,
			placeholder_1, this->shared_from_this());
		this->_socket->AsyncConnect(handler, ep);
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename ConnectorType>
void BaseTcpConnector<ConnectorType>::AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return AsyncConnect(ep);
}

template<typename ConnectorType>
inline void BaseTcpConnector<ConnectorType>::SetData(unsigned int data) {
	_data = data;
}

template<typename ConnectorType>
inline unsigned int BaseTcpConnector<ConnectorType>::GetData()const {
	return _data;
}

template<typename ConnectorType>
void BaseTcpConnector<ConnectorType>::_ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector) {
	if (error) {
		lasterror = error;
		this->_netio.OnConnected(this->shared_from_this(), error);
		return;
	}
	try {
		this->_remoteep = this->_socket->RemoteEndPoint();
		this->_localep = this->_socket->LocalEndPoint();
		this->_flag = E_STATE_START;
		this->_netio.OnConnected(this->shared_from_this(), error);
		function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
			bind_t(&BaseTcpConnector<ConnectorType>::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2);
		this->_socket->AsyncRecvSome(handler, this->_reader.readbuf, M_READ_SIZE);
	}
	catch (SocketLib::SocketError& err) {
		lasterror = err;
	}
}

////////////////////////////////////////////////////////////////////////////////////////

inline SyncTcpConnector::SyncTcpConnector() {
	_socket = new SocketLib::TcpConnector<SocketLib::IoService>(_ioservice);
	_flag = E_STATE_STOP;
	_readbuf = (SocketLib::s_byte_t*)g_malloc(M_READ_SIZE);
	g_memset(_readbuf, 0, M_READ_SIZE);
	_readsize = 0;
	g_memset(&_curheader, 0, sizeof(_curheader));
}

inline SyncTcpConnector::~SyncTcpConnector() {
	g_free(_readbuf);
}

inline bool SyncTcpConnector::Connect(const SocketLib::Tcp::EndPoint& ep, SocketLib::s_uint32_t timeo_sec) {
	try {
		this->_socket->Connect(ep, timeo_sec);
		_flag = E_STATE_START;
		return true;
	}
	catch (SocketLib::SocketError&) {
		return false;
	}
}

inline bool SyncTcpConnector::Connect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::s_uint32_t timeo_sec) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return Connect(ep, timeo_sec);
}

inline const SocketLib::Tcp::EndPoint& SyncTcpConnector::LocalEndpoint()const {
	return _localep;
}

inline const SocketLib::Tcp::EndPoint& SyncTcpConnector::RemoteEndpoint()const {
	return _remoteep;
}

// 调用这个函数不意味着连接立即断开，会等所有的未处理的数据处理完就会断开
inline void SyncTcpConnector::Close() {
	SocketLib::SocketError error;
	_socket->Close(error);
	_flag = E_STATE_STOP;
}

inline bool SyncTcpConnector::Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len) {
	if (_flag != E_STATE_START)
		return  false;
	SocketLib::s_uint32_t hdrlen = (SocketLib::s_uint32_t)sizeof(MessageHeader);
	if (len > 0 && len <= (0xFFFF - hdrlen)) {
		_sndbuffer.Clear();
		MessageHeader hdr;
		hdr.endian = _LocalEndian();
		hdr.size = len;
		hdr.timestamp = (unsigned int)time(0);
		_sndbuffer.Write(hdr);
		_sndbuffer.Write((void*)data, len);
		SocketLib::SocketError error;
		do 
		{
			SocketLib::s_uint32_t snd_cnt = _socket->SendSome(_sndbuffer.Data(), _sndbuffer.Length(), error);
			if (error) {
				Close();
				return false;
			}
			_sndbuffer.RemoveData(snd_cnt);
			if (_sndbuffer.Length() == 0)
				break;
		} while (true);		
		return true;
	}
	return false;
}

inline bool SyncTcpConnector::IsConnected()const {
	return (_flag == E_STATE_START);
}

inline SocketLib::Buffer* SyncTcpConnector::Recv() {
	if (_flag != E_STATE_START)
		return 0;
	_rcvbuffer.Clear();
	SocketLib::Buffer* reply = 0;
	if (_readsize>0)
		reply = _CutMsgPack(_readbuf, _readsize);
	if (!reply) {
		SocketLib::SocketError error;
		do {
			_readsize = _socket->RecvSome(_readbuf, M_READ_SIZE, error);
			if (_readsize == 0 || error) {
				Close();
				return 0;
			}
			reply = _CutMsgPack(_readbuf, _readsize);
			if (reply)
				break;
		} while (true);
	}
	return reply;
}

inline SocketLib::s_uint32_t SyncTcpConnector::_LocalEndian()const {
	static SocketLib::s_uint32_t endian = SocketLib::detail::Util::LocalEndian();
	return endian;
}

inline SocketLib::Buffer* SyncTcpConnector::_CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t& tran_byte) {
	// 减少内存拷贝是此函数的设计关键
	SocketLib::s_uint32_t hdrlen = (SocketLib::s_uint32_t)sizeof(MessageHeader);
	do
	{
		// 算出头部长度
		SocketLib::s_uint32_t datalen = _rcvbuffer.Length();
		if (_curheader.size == 0) {
			if (tran_byte + datalen < hdrlen) {
				_rcvbuffer.Write(buf, tran_byte);
				return 0;
			}
			else if (datalen >= hdrlen) {
				_rcvbuffer.Read(_curheader);
			}
			else {
				_rcvbuffer.Write(buf, hdrlen - datalen);
				buf += (hdrlen - datalen);
				tran_byte -= (hdrlen - datalen);
				_rcvbuffer.Read(_curheader);
			}

			// convert byte order
			if (_curheader.endian != _LocalEndian()) {
				_curheader.size = g_htons(_curheader.size);
				_curheader.timestamp = g_htonl(_curheader.timestamp);
			}
			// check
			if (_curheader.size > (0xFFFF - hdrlen))
				return 0;
		}

		// copy body data
		datalen = _rcvbuffer.Length();
		if (tran_byte + datalen < _curheader.size) {
			_rcvbuffer.Write(buf, tran_byte);
			return 0;
		}
		else {
			_rcvbuffer.Write(buf, _curheader.size - datalen);
			buf += (_curheader.size - datalen);
			tran_byte -= (_curheader.size - datalen);

			// swap
			_curheader.size = 0;
			return &_rcvbuffer;
		}
	} while (true);
	return 0;
}

M_NETIO_NAMESPACE_END
#endif
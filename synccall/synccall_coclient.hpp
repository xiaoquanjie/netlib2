#ifndef M_SYNCCALL_COCLIENT_INCLUDE
#define M_SYNCCALL_COCLIENT_INCLUDE

#include "synccall/config.hpp"
#include "synccall/synccall_client.hpp"
#include "coroutine/coroutine.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class ScIo;

// Э�̰汾
class CoScClient  : public BaseScClient{
	friend class ScServer;
public:
	~CoScClient();
	
	bool Connect(const std::string& ip, unsigned short port, unsigned int timeout);
	
	// 0==ok, -1==time out,-2==connect invalid ,-3 == other error
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply);
	
	// 0==ok, -1==time out,-2==connect invalid, -3 == other error
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len);
	
	bool IsConnected()const;
	
	void Close();

	void SetTimeOut(unsigned int timeout);

protected:
	bool _Reconnect();

	int _Sync(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len,
		netiolib::Buffer*& preply);

	inline _CoScInfo_* _GetScInfo() {
		return (_CoScInfo_*)(_socket->GetExtData());
	}

	CoScClient();
	
	CoScClient(const CoScClient&);
	
	CoScClient& operator=(const CoScClient&);

private:
	netiolib::TcpConnectorPtr _socket;
	ScIo* _io;
};

inline CoScClient::CoScClient() {
	_io = 0;
	_timeo_rw = 10;
}

inline CoScClient::~CoScClient() {
	Close();
	_io = 0;
}

inline bool CoScClient::Connect(const std::string& ip, unsigned short port, unsigned int timeout) {
	if (_socket) {
		return false;
	}
	else {
		_ip = ip;
		_port = port;
		_timeo_c = timeout;
		return _Reconnect();
	}
}

// 0==ok, -1==time out,-2==connect invalid ,-3 == other error
inline int CoScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply) {
	preply = 0;
	return _Sync(M_TWOWAY_TYPE, msg_type, msg, len, preply);
}

// 0==ok, -1==time out,-2==connect invalid, -3 == other error
inline int CoScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len) {
	netiolib::Buffer* preply = 0;
	return _Sync(M_ONEWAY_TYPE, msg_type, msg, len, preply);
}

inline bool CoScClient::IsConnected()const {
	if (_socket)
		return _socket->IsConnected();
	return false;
}

inline void CoScClient::Close() {
	if (_socket) {
		_socket->Close();
		_socket.reset();
	}
}

inline void CoScClient::SetTimeOut(unsigned int timeout) {
	if (_socket) {
		_timeo_rw = timeout;
		_socket->SetKeepAlive(timeout);
	}
}

inline bool CoScClient::_Reconnect() {
	if (!_io) {
		return false;
	}
	else {
		_socket.reset(new netiolib::TcpConnector((netiolib::NetIo&)*_io));
		_CoScInfo_* pscinfo = new _CoScInfo_;
		pscinfo->co_id = -1;
		pscinfo->thr_id = 0;
		pscinfo->valid = false;
		_socket->SetExtData(pscinfo, free_coscinfo);
		if(_socket->Connect(_ip, _port, _timeo_c)) {
			SetTimeOut(_timeo_rw);
			pscinfo->valid = true;
			return true;
		}
		else {
			Close();
			return false;
		}
	}
}

inline int CoScClient::_Sync(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len,
	netiolib::Buffer*& preply) {
	int code = 0;
	for (int cnt = 0; cnt < 2; ++cnt) {
		if (!_socket
			|| !_socket->IsConnected()) {
			if (!_Reconnect()) {
				code = -2;
				continue;
			}
		}
		_CoScInfo_* pscinfo = _GetScInfo();
		pscinfo->mutex.lock();
		if (!pscinfo->valid) {
			pscinfo->mutex.unlock();
		}
		else {
			pscinfo->thr_id = base::thread::ctid();
			pscinfo->co_id = coroutine::Coroutine::curid();
			pscinfo->mutex.unlock();
			_FillRequest(M_TWOWAY_TYPE, msg_type, msg, len);
			_socket->Send(_request.Data(), _request.Length());
			coroutine::Coroutine::yield();
			do {
				if (!_CheckReply(&pscinfo->buffer))
					break;
				preply = &pscinfo->buffer;
				return 0;
			} while (false);
		}
		Close();
		code = -3;
		continue;
	}
	return code;
}


M_SYNCCALL_NAMESPACE_END
#endif
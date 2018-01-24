#ifndef M_SYNCCALL_CLIENT_INCLUDE
#define M_SYNCCALL_CLIENT_INCLUDE

#include "synccall/config.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class BaseScClient {
protected:
	BaseScClient() {
		_port = 0;
		_timeo_c = -1;
		_timeo_rw = 3;
		_packidx = 0;
	}

	void _FillRequest(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len) {
		_request.Clear();
		_request.Write((base::s_uint32_t)way);
		_request.Write((base::s_uint32_t)msg_type);
		_request.Write((base::s_uint32_t)++_packidx);
		_request.Write(msg, len);
	}
	bool _CheckReply(SocketLib::Buffer* reply) {
		if (reply) {
			base::s_uint32_t way_type = 0;
			reply->Read(way_type);
			base::s_uint32_t msg_type = 0;
			reply->Read(msg_type);
			base::s_uint32_t pack_idx = 0;
			reply->Read(pack_idx);
			if (pack_idx != _packidx)
				return false;
			return true;
		}
		else {
			return false;
		}
	}

	std::string _ip;
	unsigned short _port;
	unsigned int _timeo_c;
	unsigned int _timeo_rw;
	unsigned int _packidx;
	netiolib::Buffer _request;
};

class ScClient : public BaseScClient{
public:
	ScClient();
	
	~ScClient();
	
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

	ScClient(const ScClient&);
	ScClient& operator=(const ScClient&);

private:
	netiolib::SyncConnector* _socket;
};

inline ScClient::ScClient() {
	_socket = 0;
}

inline ScClient::~ScClient() {
	Close();
}

inline bool ScClient::Connect(const std::string& ip, unsigned short port, unsigned int timeout) {
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
inline int ScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply) {
	preply = 0;
	return _Sync(M_TWOWAY_TYPE, msg_type, msg, len, preply);
}

// 0==ok, -1==time out,-2==connect invalid, -3 == other error
inline int ScClient::SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len) {
	netiolib::Buffer* preply = 0;
	return _Sync(M_ONEWAY_TYPE, msg_type, msg, len, preply);
}

inline bool ScClient::IsConnected()const {
	if (_socket)
		return _socket->IsConnected();
	return false;
}

inline void ScClient::Close() {
	if (_socket) {
		_socket->Close();
		delete _socket;
		_socket = 0;
	}
}

inline void ScClient::SetTimeOut(unsigned int timeout) {
	if (_socket) {
		_timeo_rw = timeout;
		_socket->SetTimeOut(timeout);
	}
}

inline bool ScClient::_Reconnect() {
	_socket = new netiolib::SyncConnector;
	if (_socket->Connect(_ip, _port, _timeo_c)) {
		SetTimeOut(_timeo_rw);
		return true;
	}
	else {
		Close();
		return false;
	}
}

inline int ScClient::_Sync(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len, 
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
		_FillRequest(way, msg_type, msg, len);
		do {
			if (!_socket->Send(_request.Data(), _request.Length()))
				break;
			preply = _socket->Recv();
			if (!_CheckReply(preply))
				break;
			return 0;
		} while (false);
		Close();
		code = -3;
		continue;
	}
	return code;
}

M_SYNCCALL_NAMESPACE_END
#endif
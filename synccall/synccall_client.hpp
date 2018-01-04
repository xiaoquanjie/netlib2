#ifndef M_SYNCCALL_CLIENT_INCLUDE
#define M_SYNCCALL_CLIENT_INCLUDE

#include "synccall/config.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class SyncCallIo;

class SyncCallClient {
	friend class SyncCallSvr;
public:

	// 0==ok, -1==time out,-2==connect invalid
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer& reply) {
		if (_connector) {
			if (!_connector->IsConnected()) {
				if (!_Reconnect())
					return -2;
			}
			_FillRequest(999, msg_type, msg, len);
			_connector->Send(_request.Data(), _request.Length());
			
			return 0;
		}
		else {
			return -2;
		}
	}

	// 0==ok, -1==time out,-2==connect invalid
	int SyncCall(int msg_type, const char* msg,SocketLib::s_uint32_t len) {
		if (_connector) {
			if (!_connector->IsConnected()) {
				if (!_Reconnect())
					return -2;
			}
			_FillRequest(666, msg_type, msg, len);
			_connector->Send(_request.Data(), _request.Length());


			return 0;
		}
		else {
			return -2;
		}
	}

	bool IsConnected()const {
		if (_connector)
			return _connector->IsConnected();
		return false;
	}

protected:
	bool _Reconnect() {
		_connector.reset(new netiolib::TcpConnector((netiolib::NetIo&)*_io));
		if (_connector->Connect(_ip, _port, _timeo)) {
			return true;
		}
		return false;
	}

	void _FillRequest(int way, int msg_type, const char* msg, SocketLib::s_uint32_t len) {
		_request.Clear();
		_request.Write(way);
		_request.Write(msg_type);
		_request.Write(++_packidx);
		_request.Write(msg, len);
	}

	SyncCallClient() {
		_port = 0;
		_timeo = -1;
		_packidx = 0;
	}

	SyncCallClient(const SyncCallClient&);
	SyncCallClient& operator=(const SyncCallClient&);

private:
	std::string _ip;
	unsigned short _port;
	unsigned int _timeo;
	unsigned int _packidx;
	netiolib::Buffer _request;
	netiolib::Buffer _reply;
	netiolib::TcpConnectorPtr _connector;
	SyncCallIo* _io;
};

M_SYNCCALL_NAMESPACE_END
#endif
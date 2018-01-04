#ifndef M_SYNCCALL_CLIENT_INCLUDE
#define M_SYNCCALL_CLIENT_INCLUDE

#include "synccall/config.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

class SyncCallIo;

class SyncCallClient {
	friend class SyncCallSvr;
public:
	bool Connect(const std::string& ip, unsigned short port, unsigned int timeout) {
		if (_connector) {
			return false;
		}
		else {
			_ip = ip;
			_port = port;
			_timeo = timeout;
			return _Reconnect();
		}
	}

	// 0==ok, -1==time out,-2==connect invalid ,-3 == other error
	int SyncCall(int msg_type, const char* msg, SocketLib::s_uint32_t len, netiolib::Buffer*& preply) {
		preply = 0;
		if (!_connector) {
			return -2;
		}
		if (!_connector->IsConnected()) {
			if (!_Reconnect())
				return -2;
		}
		_FillRequest(999, msg_type, msg, len);
		if (!_connector->Send(_request.Data(), _request.Length())) {
			Close();
			return -3;
		}
		SocketLib::Buffer* reply = _connector->Recv();
		if (_CheckReply(reply)) {
			preply = reply;
			return 0; // ok
		}
		else {
			Close();
			return -3;
		}
	}

	// 0==ok, -1==time out,-2==connect invalid, -3 == other error
	int SyncCall(int msg_type, const char* msg,SocketLib::s_uint32_t len) {
		if (!_connector) {
			return -2;
		}
		if (!_connector->IsConnected()) {
			if (!_Reconnect())
				return -2;
		}
		_FillRequest(M_ONEWAY_TYPE, msg_type, msg, len);
		if (!_connector->Send(_request.Data(), _request.Length())) {
			Close();
			return -3;
		}
		SocketLib::Buffer* reply = _connector->Recv();
		if (_CheckReply(reply)) {
			return 0; // ok
		}
		else {
			Close();
			return -3;
		}
	}

	bool IsConnected()const {
		if (_connector)
			return _connector->IsConnected();
		return false;
	}

	void Close() {
		if (_connector) {
			_connector->Close();
		}
	}

protected:
	bool _Reconnect() {
		_connector.reset(new netiolib::SyncTcpConnector);
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

	bool _CheckReply(SocketLib::Buffer* reply) {
		if (reply) {
			unsigned int way_type = 0;
			reply->Read(way_type);
			unsigned int msg_type = 0;
			reply->Read(msg_type);
			unsigned int pack_idx = 0;
			reply->Read(pack_idx);
			if (pack_idx != _packidx)
				return false;
			return true;
		}
		else {
			return false;
		}
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
	netiolib::SyncTcpConnectorPtr _connector;
};

M_SYNCCALL_NAMESPACE_END
#endif
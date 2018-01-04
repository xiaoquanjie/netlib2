#ifndef M_SYNCCALL_SVR_INCLUDE
#define M_SYNCCALL_SVR_INCLUDE

#include "synccall/config.hpp"
#include "synccall/server_handler.hpp"
#include "synccall/synccall_client.hpp"
#include "base/thread.hpp"
#include <stdlib.h>
M_SYNCCALL_NAMESPACE_BEGIN

class SyncCallClient;
class SyncCallSvr;

struct ISyncCallSvr {
	friend class SyncCallIo;
protected:
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) = 0;
	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) = 0;
	virtual void OnReceiveData(const netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) = 0;
	virtual void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) = 0;
};

class SyncCallIo : public netiolib::NetIo {
public:
	SyncCallIo(ISyncCallSvr* server) {
		_server = server;
	}
protected:
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) {
		_server->OnConnected(clisock);
	}
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		_server->OnConnected(clisock, error);
	}

	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {
		_server->OnDisconnected(clisock);
	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {
		_server->OnDisconnected(clisock);
	}

	virtual void OnReceiveData(const netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
		_server->OnReceiveData(clisock, buffer);
	}
	virtual void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		_server->OnReceiveData(clisock, buffer);
	}
protected:
	ISyncCallSvr* _server;
};

class SyncCallSvr : public ISyncCallSvr {
	friend class SyncCallIo;
	typedef std::map<base::s_uint64_t, IServerHandler*> SvrHandlerMap;

public:
	SyncCallSvr() :_io(this) {
	}

	~SyncCallSvr() {
		for (std::list<base::thread*>::iterator iter = _threads.begin();
			iter != _threads.end(); ++iter) {
			(*iter)->join();
			delete (*iter);
		}
		_threads.clear();
	}

	bool RegisterHandler(const std::string& ip, unsigned short port, IServerHandler* handler) {
		if (_io.ListenOne(ip, port)) {
			base::s_uint64_t id = UniqueId(ip, port);
			_svrhandler_map[id] = handler;
			return true;
		}
		else {
			return false;
		}
	}

	SyncCallClient* CreateClient(const std::string& ip,unsigned short port,unsigned int timeout) {
		SyncCallClient* client = new SyncCallClient;
		client->_ip = ip;
		client->_port = port;
		client->_timeo = timeout;
		client->_io = &_io;
		client->_connector.reset(new netiolib::TcpConnector(_io));
		if (client->_connector->Connect(ip, port, timeout))
			return client;
		else {
			delete client;
			return 0;
		}
	}

	void Start(unsigned int thread_cnt) {
		if (_threads.empty()) {
			for (unsigned int idx = 0; idx < thread_cnt; ++idx) {
				base::thread* pthread = new base::thread(&SyncCallSvr::Run, this, 0);
				_threads.push_back(pthread);
			}
		}
	}

	void Stop() {
		_io.Stop();
	}

protected:
	void Run(void*) {
		printf("%d thread is starting..............\n", base::thread::ctid());
		_io.Run();
		printf("%d thread is leaving..............\n", base::thread::ctid());
	}

	void OnConnected(const netiolib::TcpSocketPtr& clisock) {
		std::string ip = clisock->LocalEndpoint().Address();
		base::s_uint16_t port = clisock->LocalEndpoint().Port();
		base::s_uint64_t* id = new base::s_uint64_t;
		*id = UniqueId(ip, port);
		clisock->GetSocket().SetData(id);
	}
	void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		
	}

	void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {
		base::s_uint64_t* id = (base::s_uint64_t*)clisock->GetSocket().GetData();
		delete id;
	}
	void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {
	}

	void OnReceiveData(const netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
		base::s_uint64_t* id = (base::s_uint64_t*)clisock->GetSocket().GetData();
		if (id) {
			SvrHandlerMap::iterator iter = _svrhandler_map.find(*id);
			if (iter == _svrhandler_map.end()) {
				printf("handler is not exist,id(%lld)\n", *id);
				return;
			}
			unsigned int way_type = 0;
			buffer.Read(way_type);
			unsigned int msg_type = 0;
			buffer.Read(msg_type);
			unsigned int pack_idx = 0;
			buffer.Read(pack_idx);
			// don't ask why the way_the is 666 or 999
			if (way_type == 666) {
				netiolib::Buffer* reply = new netiolib::Buffer;
				reply->Write(way_type);
				reply->Write(msg_type);
				reply->Write(pack_idx);
				iter->second->OnOneWayDealer(msg_type, buffer);
				clisock->Send(reply);
			}
			else if (way_type == 999) {
				netiolib::Buffer* reply = new netiolib::Buffer;
				reply->Write(way_type);
				reply->Write(msg_type);
				reply->Write(pack_idx);
				iter->second->OnTwoWayDealer(msg_type, buffer, *reply);
				clisock->Send(reply);
			}
			else {
				printf("error way_type(%d)\n", way_type);
			}
		}
	}
	void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		
	}

	base::s_uint64_t UniqueId(const std::string& ip, unsigned short port) {
		base::s_uint32_t int_ip = inet_addr(ip.c_str());
		base::s_uint64_t id = (((base::s_uint64_t)int_ip) << 17) + port;
		return id;
	}

private:
	SyncCallIo _io;
	std::list<base::thread*> _threads;
	SvrHandlerMap _svrhandler_map;
};


M_SYNCCALL_NAMESPACE_END
#endif
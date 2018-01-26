#include <transfer/transfer.hpp>
#include <netio/netio.hpp>
#include <iostream>
#include <map>

M_TRANSFER_NAMESPACE_BEGIN

class TransferSvr : netiolib::NetIo {
	struct SocketInfo {
		base::s_int32_t type;
		base::s_int32_t id;
	};

public:
	TransferSvr() {
		_thread = 0;
	}

	virtual void Start(unsigned int thread_cnt, bool isco) {
		_thread = new base::thread(&TransferSvr::_Loop, this, 0);
		netiolib::NetIo::Start(thread_cnt, false);
		_heart_mng.Start(thread_cnt);
	}

	virtual void Stop() {
		netiolib::NetIo::Stop();
		_heart_mng.Stop();
	}

	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) {
		_heart_mng.OnConnected(clisock);
	}

	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) {

	}

	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, 
		SocketLib::Buffer& buffer) {
		_heart_mng.OnReceiveData(clisock);
		_ParseMsg(clisock, buffer);
	}

protected:
	protocol_base* _CreateMsg(base::s_int32_t msgid, SocketLib::Buffer& buffer) {
		protocol_base* msg = 0;
		switch (msgid) {
		case msgid_login:
			msg = new protocol_login;
			break;
		case msgid_logout:
			msg = new protocol_logout;
			break;
		case msgid_single_point:
			msg = new protocol_single_point;
			break;
		case msgid_type_broadcast:
			msg = new protocol_type_broadcast;
			break;
		case msgid_all_broadcast:
			msg = new protocol_all_broadcast;
			break;
		default:
			break;
		}
		if (msg) {
			msg->Read(buffer);
		}
		return msg;
	}

	void _ParseMsg(netiolib::TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {
		base::s_int32_t msgid = 0;
		buffer.Read(msgid);
		protocol_base* msg = _CreateMsg(msgid, buffer);
		_PushMsg(msg);
	}

	void _PushMsg(protocol_base* msg) {
		if (msg) {
			_mutex.lock();
			_msgqueue.push_back(msg);
			_mutex.unlock();
		}
	}

	void _Loop(void*p) {
		protocol_base* msg = 0;
		while (true) {
			if (_msgqueue.size()) {
				_mutex.lock();
				_msgqueue.swap(_msgqueue2);
				_mutex.unlock();
			}
			else {
				_thread->sleep(3);
			}
			while (_msgqueue2.size()) {
				_msgqueue2.pop_front(msg);
				_HandleMsg(msg);
			}
		}
	}

	void _HandleMsg(protocol_base* msg) {
		switch (msg->msgid) {
		case msgid_login:
			_Handle_Login((protocol_login*)msg);
			break;
		case msgid_logout:
			_Handle_Logout((protocol_logout*)msg);
			break;
		case msgid_single_point:
			_Handle_SinglePoint((protocol_single_point*)msg);
			break;
		case msgid_type_broadcast:
			_Handle_TypeBroadcast((protocol_type_broadcast*)msg);
			break;
		case msgid_all_broadcast:
			_Handle_AllBroadcast((protocol_all_broadcast*)msg);
			break;
		default:
			break;
		}
	}

	void _Handle_Login(protocol_login* msg) {

	}

	void _Handle_Logout(protocol_logout* msg) {

	}

	void _Handle_SinglePoint(protocol_single_point* msg) {

	}

	void _Handle_TypeBroadcast(protocol_type_broadcast* msg) {

	}

	void _Handle_AllBroadcast(protocol_all_broadcast* msg) {

	}

private:
	base::thread* _thread;
	netiolib::HeartBeatMng _heart_mng;
	base::circular_queue<protocol_base*> _msgqueue;
	base::circular_queue<protocol_base*> _msgqueue2;
	base::MutexLock _mutex;
	std::map<netiolib::TcpSocketPtr, SocketInfo> _socket_info_map;

};


M_TRANSFER_NAMESPACE_END
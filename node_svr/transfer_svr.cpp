#include <node_svr/node_svr.hpp>
#include <iostream>
#include <map>
#include <fstream>
#include "base/configreader.hpp"

M_NODESVR_NAMESPACE_BEGIN

class TransferSvr 
	: public netiolib::NetIo
{
public:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) {
		_heart_mng.OnConnected(clisock);
	}

	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) {
		std::bind(&TransferSvr::OnConnected, this);
	}

	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {
		_heart_mng.OnReceiveData(clisock);
		base::s_int32_t msgid = 0;
		buffer.Read(msgid);
		if (msg_id_keepalive == msgid) {
			protocol_keepalive_ack ack_msg;
			buffer.Clear();
			buffer.Write(ack_msg);
			clisock->Send(buffer.Data(), buffer.Length());
		}
		else if (msg_id_transfer == msgid) {
			protocol_transfer trans_msg;
			trans_msg.Read(buffer);
			if (protocol_transfer::enum_login_type
				== trans_msg.msgtype) {

			}
			else if (protocol_transfer::enum_logout_type
				== trans_msg.msgtype) {

			}
			else if (protocol_transfer::enum_signle_point_type
				== trans_msg.msgtype) {

			}
			else if (protocol_transfer::enum_type_broadcast_type
				== trans_msg.msgtype) {

			}
			else if (protocol_transfer::enum_all_broadcast_type
				== trans_msg.msgtype) {

			}
		}
	}

	netiolib::HeartBeatMng& GetHeartMng() {
		return _heart_mng;
	}

protected:
	base::MutexLock _mutex;

	netiolib::HeartBeatMng 
		_heart_mng;
};

void start_transfer_svr() {
	TransferSvr svr;
	svr.Start(4);
	svr.GetHeartMng().Start(4);
}

void stop_transfer_svr() {

}

M_NODESVR_NAMESPACE_END
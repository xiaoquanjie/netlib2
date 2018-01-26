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

	}

	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) {

	}

	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, SocketLib::Buffer& buffer) {

	}

	netiolib::HeartBeatMng& GetHeartMng() {
		return _heart_mng;
	}

protected:
	netiolib::HeartBeatMng _heart_mng;
};



M_NODESVR_NAMESPACE_END
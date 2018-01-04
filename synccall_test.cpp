#include "synccall/synccall.hpp"

void print_clock(bool beg);

class RpcHandler : public synccall::IServerHandler {
public:
	virtual void OnOneWayDealer(const int msg_type, netiolib::Buffer& request) {

	}
	virtual void OnTwoWayDealer(const int msg_type, netiolib::Buffer& request, netiolib::Buffer& reply) {
		if (msg_type == 1) {
			std::string info;
			request.Read(info);
			//std::cout << "client request:" << info << std::endl;
			info = "server knows " + info;
			reply.Write(info);
		}
		else {
			std::cout << "error msg_type=" << msg_type << std::endl;
		}
	}
};

void synccall_server(){
	synccall::SyncCallSvr server;
	server.Start(4);
	server.RegisterHandler("127.0.0.1", 4001, new RpcHandler);
	int i = 0;
	std::cin >> i;
	server.Stop();
}

void synccall_client() {
	synccall::SyncCallClient client;
	if (!client.Connect("127.0.0.1", 4001, 2)) {
		std::cout << "connect fail" << std::endl;
		return;
	}

	int i = 0;
	SocketLib::Buffer request;
	std::string info = "this is a test!!!!";
	for (int i = 0; i < 1024; ++i)
		info.append("a");
	print_clock(true);
	while (true) {
		
		//std::cin >> info;
		if (info=="stop")
			break;

		request.Clear();
		request.Write(info);
		SocketLib::Buffer* reply;
		int ret = client.SyncCall(1, request.Data(), request.Length(), reply);
		if (ret != 0) {
			std::cout << "happend error" << std::endl;
			break;
		}
		info.clear();
		reply->Read(info);
		info.clear();
		//std::cout << "server reply:" << info << std::endl;
		++i;
		if (i>=100000)
			break;
	}
	print_clock(false);
}


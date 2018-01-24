#include "synccall/synccall.hpp"
#include <iostream>
using namespace std;

class CoRpcHandler : public synccall::IServerHandler {
public:
	CoRpcHandler(synccall::ScServer& svr)
		:server(svr) {

	}

	virtual void OnOneWayDealer(const int msg_type, netiolib::Buffer& request) {
		std::cout << "error" << std::endl;

	}

	virtual void OnTwoWayDealer(const int msg_type, netiolib::Buffer& request, netiolib::Buffer& reply) {
		if (msg_type == 1) {
			std::string name = "null";
			synccall::CoScClient* client = server.CreateCoScClient();
			if (client->Connect("127.0.0.1", 3001, 2)) {
				netiolib::Buffer* preply = 0;
				if (0 == client->SyncCall(2, 0, 0, preply)) {
					preply->Read(name);
				}
			}
			//delete client;
			reply.Write(name);
		}
		else {
			std::cout << "error msg_type=" << msg_type << std::endl;
		}
	}

private:
	synccall::ScServer& server;
};

class CoRpcHandler2 : public synccall::IServerHandler {
public:
	CoRpcHandler2(synccall::ScServer& svr)
		:server(svr) {

	}

	virtual void OnOneWayDealer(const int msg_type, netiolib::Buffer& request) {
		std::cout << "error" << std::endl;

	}

	virtual void OnTwoWayDealer(const int msg_type, netiolib::Buffer& request, netiolib::Buffer& reply) {
		if (msg_type == 2) {
			//int i;
			//std::cin >> i;
			reply.Write(std::string("xiaoquanjie"));
		}
		else {
			std::cout << "error msg_type=" << msg_type << std::endl;
		}
	}

private:
	synccall::ScServer& server;
};

void co_synccall_server() {
	synccall::ScServer server;
	server.Start(1, true);
	if (!server.Register("0.0.0.0", 4001, new CoRpcHandler(server))) {
		cout << "register handler error" << endl;
		return;
	}
	if (!server.Register("0.0.0.0", 3001, new CoRpcHandler2(server))) {
		cout << "register handler2 error" << endl;
		return;
	}
	int i = 0;
	cin >> i;
	server.Stop();
}

void co_synccall_client() {
	std::string ip;
	std::cout << "input ip:";
	std::cin >> ip;

	synccall::ScClient client;
	if (!client.Connect(ip, 4001, -1)) {
		std::cout << "connect fail" << std::endl;
		return;
	}
	std::cout << "connect success" << std::endl;
	client.SetTimeOut(1);
	std::string name = "";
	SocketLib::Buffer* reply = 0;
	if (0 == client.SyncCall(1, 0, 0, reply)) {
		reply->Read(name);
		cout << name << endl;
	}
	else {
		cout << "happend error" << endl;
	}
}
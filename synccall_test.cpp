#include "synccall/synccall.hpp"

void print_clock(bool beg);

struct CDisplayTime
{
	clock_t     miStartTime;
	const char* mpName;
	CDisplayTime(const char* pName) :mpName(pName){
		miStartTime = clock();
	}
	~CDisplayTime(){
		if (mpName){
			std::cout << mpName << " elapsed=" << ((double)(clock() - miStartTime) / CLOCKS_PER_SEC) << std::endl;
		}
	}
};


#define M_DISPLAYTIME() CDisplayTime loDisTime_##__LINE__(__FUNCTION__);


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
	//server.RegisterHandler("172.16.81.247", 4001, new RpcHandler);
	server.RegisterHandler("127.0.0.1", 4001, new RpcHandler);
	int i = 0;
	std::cin >> i;
	server.Stop();
}

void synccall_client() {
	synccall::SyncCallClient client;
	//if (!client.Connect("172.16.81.247", 4001,-1)) {
	if (!client.Connect("127.0.0.1", 4001, -1)) {
		std::cout << "connect fail" << std::endl;
		return;
	}
	std::cout << "connect success" << std::endl;

	int i = 0;
	SocketLib::Buffer request;
	std::string info = "this is a test!!!!";
	for (int i = 0; i < 1024; ++i)
		info.append("a");
	
	{
		M_DISPLAYTIME();
		while (true) {
			//std::cin >> info;
			if (info == "stop")
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
			if (i >= 100000)
				break;
		}
	}
}


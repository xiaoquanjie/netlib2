#include "netio/netio.hpp"
#include <iostream>
#include "thread.hpp"

using namespace std;

class TestNetIo : public netiolib::NetIo {
public:
	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(netiolib::TcpSocketPtr clisock) {
		cout << "OnConnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnConnected(netiolib::TcpConnectorPtr clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "connect fail :" << error.What() << endl;
		}
		else {
			cout << "connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
			std::string str;
			std::cin >> str;
			clisock->Send(str.c_str(), str.length());
		}
	}

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(netiolib::TcpSocketPtr clisock) {
		cout << "OnDisconnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(netiolib::TcpConnectorPtr clisock) {
		cout << "OnDisconnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(netiolib::TcpSocketPtr clisock, netiolib::BufferPtr buffer) {
		cout << "receive data from :" << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port();
		buffer->Write('\0');
		cout << " : " << buffer->Data() << endl;

		// 回复
		std::string str("svr reply:");
		str.append(buffer->Data(), buffer->Length());
		clisock->Send(str.c_str(), str.length());
	}
	virtual void OnReceiveData(netiolib::TcpConnectorPtr clisock, netiolib::BufferPtr buffer) {
		cout << "receive data from :" << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port();
		buffer->Write('\0');
		cout << " : " << buffer->Data() << endl;

		std::string str;
		std::cin >> str;
		clisock->Send(str.c_str(), str.length());
	}

	void Start(void*) {
		Run();
	}
	
};

void server() {
	TestNetIo test_io;
	thread thr(&TestNetIo::Start, &test_io, 0);
	thread thr2(&TestNetIo::Start, &test_io, 0);
	thr.sleep(200);
	if (!test_io.ListenOne("0.0.0.0", 3001)) {
		cout << test_io.GetLastError().What() << endl;
	}
	thr.join();
	thr2.join();
}

void client() {
	TestNetIo test_io;
	thread thr(&TestNetIo::Start, &test_io, 0);
	thr.sleep(200);
	test_io.ConnectOne("127.0.0.1", 3001);
	thr.join();
}

int main() {

	//server();
	client();
	return 0;
}
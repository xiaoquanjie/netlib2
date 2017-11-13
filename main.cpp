#include "netio/netio.hpp"
#include <iostream>
#include "thread.hpp"

using namespace std;

class TestNetIo : public netiolib::NetIo {
public:
	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(netiolib::TcpSocketPtr clisock) {
		cout << "OnConnected one : " << clisock->LocalEndpoint().Address() << endl;
	}

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(netiolib::TcpSocketPtr clisock) {
		cout << "OnDisconnected one : " << clisock->LocalEndpoint().Address() << endl;
	}

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(netiolib::TcpSocketPtr clisock, netiolib::BufferPtr buffer) {
		cout << "OnReceiveData one : " << clisock->LocalEndpoint().Address() << endl;
	}

	void Start(void*) {
		Run();
	}
	
};

int main() {

	TestNetIo test_io;
	thread thr(&TestNetIo::Start,&test_io,0);
	thr.sleep(200);
	if (!test_io.ListenOne("127.0.0.1", 3001)) {
		cout << test_io.GetLastError().What() << endl;
	}

	thr.join();
	return 0;
}
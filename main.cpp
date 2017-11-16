#include "netio/netio.hpp"
#include <iostream>
#include "thread.hpp"

using namespace std;

void SendData(netiolib::TcpConnectorPtr clisock) {
	unsigned int data = clisock->GetData();
	--data;
	clisock->SetData(data);
	if (data > 0) {
		std::string str = "hello world.........................................................................................";
		clisock->Send(str.c_str(), str.length());
	}
	else {
		cout << "send over" << endl;
	}
}

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
			SendData(clisock);
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
		//cout << "receive data from :" << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port();
		//buffer->Write('\0');
		//cout << " : " << buffer->Data() << endl;

		// 回复
		std::string str("svr reply:");
		str.append(buffer->Data(), buffer->Length());
		clisock->Send(str.c_str(), str.length());
	}
	virtual void OnReceiveData(netiolib::TcpConnectorPtr clisock, netiolib::BufferPtr buffer) {
		//cout << "receive data from :" << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port();
		//buffer->Write('\0');
		//cout << " : " << buffer->Data() << endl;

		SendData(clisock);
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
	std::list<thread*> pthreads;
	for (int i = 0; i<100; ++i)
	{
		pthreads.push_back(new thread(&TestNetIo::Start, &test_io, 0));
	}
	thread::sleep(2000);
	for (int i = 0; i < 100; ++i) {
		test_io.ConnectOne("192.168.10.128", 3001, 200);
	}
	
	for (std::list<thread*>::iterator iter=pthreads.begin(); iter!=pthreads.end();
		++iter)
	{
		(*iter)->join();
	}
}

int main() {

	//server();
	client();
	return 0;
}
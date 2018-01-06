#include "netio/netio.hpp"
#include <iostream>
#include "base/thread.hpp"
#include <list>

using namespace std;

struct TestInfo {
	base::s_uint64_t request;
	base::s_uint64_t reply;
};

class TcpTestIo : public netiolib::NetIo {
public:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) {
		cout << "OnConnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	
	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "connect fail :" << error.What() << endl;
		}
		else {
			cout << "connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
			TestInfo* pinfo = new TestInfo;
			pinfo->request = 0;
			pinfo->reply = 0;
			clisock->GetSocket().SetData(pinfo);

			unsigned int data = clisock->GetData();
			pinfo->request += data;

			SocketLib::Buffer buffer;
			buffer.Write(data);
			--data;
			clisock->SetData(data);
			clisock->Send(buffer.Data(), buffer.Length());
		}
	}

	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}

	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl;
		TestInfo* pinfo = (TestInfo*)clisock->GetSocket().GetData();
		delete pinfo;
	}

	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
		clisock->Send(buffer.Data(), buffer.Length());
	}

	virtual void OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		int count = 0;
		buffer.Read(count);
		TestInfo* pinfo = (TestInfo*)clisock->GetSocket().GetData();
		pinfo->reply += count;

		unsigned int data = clisock->GetData();
		if (data > 0) {
			pinfo->request += data;
			buffer.Clear();
			buffer.Write(data);
			--data;
			clisock->SetData(data);
			clisock->Send(buffer.Data(), buffer.Length());
		}
		else {
			static SocketLib::MutexLock lock;
			SocketLib::ScopedLock scoped(lock); // 输出测试，测性能时要去掉
			cout << clisock->LocalEndpoint().Port() << " request:" << pinfo->request << " reply:" << pinfo->reply << endl;
			clisock->Close();
		}
	}

	void Start(void*) {
		Run();
	}
};

void tcp_pause() {
	int i;
	cin >> i;
}

void async_tcp_server() {
	std::list<base::thread*> threads;
	TcpTestIo test_io;
	cout << "input thread count:";
	int thread_cnt = 0;
	cin >> thread_cnt;
	for (int i = 0; i < thread_cnt; ++i) {
		threads.push_back(new base::thread(&TcpTestIo::Start, &test_io, 0));
	}
	while (thread_cnt != threads.size())
		;

	if (test_io.ListenOne("0.0.0.0", 3001)) {
		cout << "listening....." << endl;
	}
	else {
		cout << test_io.GetLastError().What() << endl;
	}

	tcp_pause();
	test_io.Stop();
	for (std::list<base::thread*>::iterator iter = threads.begin();
		iter != threads.end(); ++iter) {
		(*iter)->join();
		delete (*iter);
	}
	cout << "finish.............." << endl;
}

void async_tcp_client() {
	std::list<base::thread*> threads;
	TcpTestIo test_io;
	cout << "input thread count:";
	int thread_cnt = 0;
	cin >> thread_cnt;
	for (int i = 0; i < thread_cnt; ++i) {
		threads.push_back(new base::thread(&TcpTestIo::Start, &test_io, 0));
	}
	while (thread_cnt != threads.size())
		;

	std::string ip;
	cout << "input ip:";
	cin >> ip;
	for (int i = 0; i < thread_cnt; ++i) {
		test_io.ConnectOne(ip, 3001,10000);
	}

	tcp_pause();
	test_io.Stop();
	for (std::list<base::thread*>::iterator iter = threads.begin();
		iter != threads.end(); ++iter) {
		(*iter)->join();
		delete (*iter);
	}
	cout << "finish.............." << endl;
}
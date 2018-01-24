#include "netio/netio.hpp"
#include <iostream>
#include "base/thread.hpp"
#include <list>

using namespace std;

struct TestInfo {
	base::s_uint32_t count;
	base::s_uint64_t request;
	base::s_uint64_t reply;
};

void free_TestInfo(void* data) {
	delete ((TestInfo*)data);
}

class TcpTestIo : public netiolib::NetIo {
public:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) {
		netiolib::NetIo::OnConnected(clisock);
		cout << "OnConnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	
	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "connect fail :" << error.What() << endl;
		}
		else {
			cout << "connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
			TestInfo* pinfo = new TestInfo;
			pinfo->count = 10000;
			pinfo->request = 0;
			pinfo->reply = 0;
			pinfo->request += pinfo->count;
			clisock->SetExtData(pinfo, free_TestInfo);

			SocketLib::Buffer buffer;
			buffer.Write(pinfo->count);
			--pinfo->count;
			clisock->Send(buffer.Data(), buffer.Length());
		}
	}

	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) {
		netiolib::NetIo::OnDisconnected(clisock);
		cout << "OnDisconnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}

	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl;
		TestInfo* pinfo = (TestInfo*)clisock->GetSocket().GetData();
		delete pinfo;
	}

	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
		netiolib::NetIo::OnReceiveData(clisock, buffer);
		clisock->Send(buffer.Data(), buffer.Length());
	}

	virtual void OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		int count = 0;
		buffer.Read(count);
		TestInfo* pinfo = (TestInfo*)clisock->GetExtData();
		pinfo->reply += count;
		if (pinfo->count > 0) {
			pinfo->request += pinfo->count;
			buffer.Clear();
			buffer.Write(pinfo->count);
			--pinfo->count;
			clisock->Send(buffer.Data(), buffer.Length());
		}
		else {
			static SocketLib::MutexLock lock;
			SocketLib::ScopedLock scoped(lock); // 输出测试，测性能时要去掉
			cout << clisock->LocalEndpoint().Port() << " request:" << pinfo->request << " reply:" << pinfo->reply << endl;
			clisock->Close();
		}
	}
};

void tcp_pause() {
	int i;
	cin >> i;
}

void async_tcp_server() {
	TcpTestIo test_io;
	cout << "input thread count:";
	int thread_cnt = 0;
	cin >> thread_cnt;
	test_io.Start(thread_cnt);

	if (test_io.ListenOne("0.0.0.0", 3001)) {
		cout << "listening....." << endl;
	}
	else {
		cout << test_io.GetLastError().What() << endl;
	}

	tcp_pause();
	test_io.Stop();
	cout << "finish.............." << endl;
}

void async_tcp_client() {
	TcpTestIo test_io;
	cout << "input thread count:";
	int thread_cnt = 0;
	cin >> thread_cnt;
	test_io.Start(thread_cnt);
	
	std::string ip;
	cout << "input ip:";
	cin >> ip;
	for (int i = 0; i < thread_cnt; ++i) {
		test_io.ConnectOne(ip, 3001);
	}

	tcp_pause();
	test_io.Stop();
	cout << "finish.............." << endl;
}
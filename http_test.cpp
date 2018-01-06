#include "netio/netio.hpp"
#include <iostream>
#include "base/thread.hpp"
#include <list>

using namespace std;

class HttpTestIo : public netiolib::NetIo {
public:
	virtual void OnConnected(netiolib::HttpSocketPtr& clisock) {
		cout << "OnConnected one http : " << clisock->RemoteEndpoint().Address()
			<< " " << clisock->RemoteEndpoint().Port() << endl;
	}

	virtual void OnConnected(netiolib::HttpConnectorPtr& clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "http connect fail :" << error.What() << endl;
		}
		else {
			cout << "http connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
			clisock->Close();
		}
	}

	virtual void OnDisconnected(netiolib::HttpSocketPtr& clisock) {
		cout << "OnDisconnected one http : " << clisock->RemoteEndpoint().Address() << " "
			<< clisock->RemoteEndpoint().Port() << endl;
	}

	virtual void OnDisconnected(netiolib::HttpConnectorPtr& clisock) {
		cout << "OnDisconnected one http connector: " << clisock->RemoteEndpoint().Address() << " "
			<< clisock->RemoteEndpoint().Port() << endl;
	}

	virtual void OnReceiveData(netiolib::HttpSocketPtr& clisock, netiolib::HttpSvrRecvMsg& httpmsg) {
		netiolib::HttpSvrSendMsg& msg = clisock->GetSvrMsg();
		msg.SetBody("newxiaoquanjie", 14);
		clisock->SendHttpMsg();
	}

	virtual void OnReceiveData(netiolib::HttpConnectorPtr&, netiolib::HttpCliRecvMsg& httmsg) {
		cout << httmsg.GetRespondLine() << endl;
		cout << httmsg.GetHeader() << endl;
		cout << httmsg.GetBody() << endl;
	}

	void Start(void*) {
		Run();
	}
};

void http_pause() {
	int i;
	cin >> i;
}

void http_server() {
	std::list<base::thread*> threads;
	HttpTestIo test_io;
	cout << "input thread count:";
	int thread_cnt = 0;
	cin >> thread_cnt;
	for (int i = 0; i < thread_cnt; ++i) {
		threads.push_back(new base::thread(&HttpTestIo::Start, &test_io, 0));
	}
	while (thread_cnt != threads.size())
		;

	if (test_io.ListenOneHttp("0.0.0.0", 5001)) {
		cout << "listening....." << endl;
	}
	else {
		cout << test_io.GetLastError().What() << endl;
	}

	http_pause();
	test_io.Stop();
	for (std::list<base::thread*>::iterator iter = threads.begin();
		iter != threads.end(); ++iter) {
		(*iter)->join();
		delete (*iter);
	}
	cout << "finish.............." << endl;
}

void http_client() {
	std::list<base::thread*> threads;
	HttpTestIo test_io;
	cout << "input thread count:";
	int thread_cnt = 0;
	cin >> thread_cnt;
	for (int i = 0; i < thread_cnt; ++i) {
		threads.push_back(new base::thread(&HttpTestIo::Start, &test_io, 0));
	}
	while (thread_cnt != threads.size())
		;

	std::string ip;
	cout << "input ip:";
	cin >> ip;
	for (int i = 0; i < thread_cnt; ++i) {
		test_io.ConnectOneHttp(ip, 5001);
	}

	http_pause();
	test_io.Stop();
	for (std::list<base::thread*>::iterator iter = threads.begin();
		iter != threads.end(); ++iter) {
		(*iter)->join();
		delete (*iter);
	}
	cout << "finish.............." << endl;
}
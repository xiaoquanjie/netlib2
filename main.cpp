#include "netio/netio.hpp"
#include <iostream>
#include "thread.hpp"

using namespace std;

clock_t gbeg_time = 0;
clock_t gend_time = 0;
std::string gstr = "";
int gprint = 0;

int glen = 1024;
void init_gstr() {
	for (int i = 0; i < glen; ++i)
		gstr.append(".");
}
void init_print() {
	cout << "select 1 to print log,or not :" << endl;
	cin >> gprint;
}

#define _print_nothing(info) 0
#define _print_log(info) cout << info
#define print_log(info)\
	(gprint==1 ? (_print_log(info),0) : 0)

void print_clock(bool beg) {
	if (beg)
		gbeg_time = clock();
	else
		gend_time = clock();
	if (beg)
		cout << "begin : " << ((double)gbeg_time / CLOCKS_PER_SEC) << endl;
	else {
		cout << "end : " << ((double)gend_time / CLOCKS_PER_SEC) << endl;
		cout << "elapsed : " << ((double)(gend_time - gbeg_time) / CLOCKS_PER_SEC) << endl;
	}
}

void SendData(netiolib::TcpConnectorPtr clisock) {
	unsigned int data = clisock->GetData();
	if (data > 0) {
		--data;
		clisock->SetData(data);
		clisock->Send(gstr.c_str(), gstr.length());
	}
	else {
		print_clock(false);
		cout << "send over" << endl;
	}
}

void ReplyData(netiolib::TcpSocketPtr clisock, netiolib::BufferPtr buffer) {
	std::string str("svr reply:");
	str.append(buffer->Data(), buffer->Length());
	clisock->Send(str.c_str(), str.length());
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
		print_log("receive data from :" << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port());
		buffer->Write('\0');
		print_log(" : " << buffer->Data() << endl);
		
		// 回复
		ReplyData(clisock, buffer);
	}
	virtual void OnReceiveData(netiolib::TcpConnectorPtr clisock, netiolib::BufferPtr buffer) {
		print_log("receive data from :" << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port());
		buffer->Write('\0');
		print_log(" : " << buffer->Data() << endl);

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
	else
		cout << "listening....." << endl;
	thr.join();
	thr2.join();
}

void client() {
	TestNetIo test_io;
	std::list<thread*> pthreads;
	pthreads.push_back(new thread(&TestNetIo::Start, &test_io, 0));

	thread::sleep(2000);
	netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(test_io, 0));
	connector->AsyncConnect("127.0.0.1",3001);
	
	while (true)
	{
		int i;
		cin >> i;
		print_clock(true);
		connector->SetData(i);
		SendData(connector);
	}
}

void netlib_test() {
	init_gstr();
	init_print();
	cout << "select 1 is server,or client :" << endl;
	int i = 0;
	cin >> i;
	if (i == 1)
		server();
	else
		client();
}

void _other_test(void*p) {

	try {
		char buf[10] = "21e3.";
		SocketLib::TcpConnector<SocketLib::IoService>* pconnector = (SocketLib::TcpConnector<SocketLib::IoService>*)p;
		pconnector->SendSome(buf, 10);
	}
	catch (SocketLib::SocketError& error) {
		cout << thread::ctid() << "  " << error.What() << endl;
	}
}
void other_test() {
	SocketLib::IoService iosverice;
	SocketLib::TcpConnector<SocketLib::IoService> connector(iosverice);
	connector.Connect(SocketLib::Tcp::EndPoint(SocketLib::AddressV4("127.0.0.1"), 3001));
	thread th1(_other_test, &connector);
	thread th2(_other_test, &connector);
	th1.join();
	th2.join();
}

int main() {

	netlib_test();
	//other_test();
	return 0;
}
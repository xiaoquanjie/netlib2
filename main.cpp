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
		cout << clisock.use_count() << endl;
		clisock->Close();
	}
}

void ReplyData(netiolib::TcpSocketPtr clisock, netiolib::Buffer& buffer) {
	std::string str("svr reply:");
	str.append(buffer.Data(), buffer.Length());
	clisock->Send(str.c_str(), str.length());
}

class TestNetIo : public netiolib::NetIo {
public:
	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(const netiolib::TcpSocketPtr& clisock) {
		cout << "OnConnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnConnected(const netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "connect fail :" << error.What() << endl;
		}
		else {
			cout << "connect success : " << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl;
			//SendData(clisock);
		}
	}

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl;
	}

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(const netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
		print_log("receive data from :" << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port()<<endl);
		buffer.Write('\0');
		print_log(" : " << buffer.Data() << endl);
		
		// 回复
		ReplyData(clisock, buffer);
	}
	virtual void OnReceiveData(const netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
		print_log("receive data from :" << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl);
		buffer.Write('\0');
		print_log(" : " << buffer.Data() << endl);
		SendData(clisock);
	}

	void Start(void*) {
		Run();
	}
	
};

void server() {
	TestNetIo test_io;
	for (int i = 0; i < 32; ++i) {
		new thread(&TestNetIo::Start, &test_io, 0);
	}
	
	thread::sleep(200);
	if (!test_io.ListenOne("0.0.0.0", 3001)) {
		cout << test_io.GetLastError().What() << endl;
	}
	else
		cout << "listening....." << endl;

	int i;
	cin >> i;
	test_io.Stop();
	//thr.join();
	//thr2.join();
	//hr3.join();
}

void client() {
	TestNetIo test_io;
	for (int i = 0; i < 32; ++i) {
		new thread(&TestNetIo::Start, &test_io, 0);
	}
	
	std::vector<netiolib::TcpConnectorPtr> ptrlist;
	cout << "select socket count:";
	int i;
	cin >> i;
	cout << "select packet count:";
	int k;
	cin >> k;
	for (int j=0;j<i;j++)
	{
		netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(test_io, 0));
		connector->SetData(k);
		connector->AsyncConnect("127.0.0.1", 3001);
		ptrlist.push_back(connector);
	}

	cout << "enter to begin..." << endl;
	cin >> i;
	print_clock(true);
	for (int j = 0; j < ptrlist.size(); ++j) {
		SendData(ptrlist[j]);
	}
	ptrlist.clear();
	cin >> i;
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
	}
	catch (SocketLib::SocketError& error) {
		cout << thread::ctid() << "  " << error.What() << endl;
	}
}
void other_test() {
	SocketLib::IoService iosverice;
	cout << "select 1 be server or 0 be client :";
	int i;
	cin >> i;

	void* p;
	if (1 == i) {
		SocketLib::TcpAcceptor<SocketLib::IoService> 
			sock(iosverice, SocketLib::Tcp::EndPoint(SocketLib::AddressV4("0.0.0.0"), 3001));
		p = &sock;

		SocketLib::TcpSocket<SocketLib::IoService> clisock(sock.GetIoService());
		sock.Accept(clisock);

		char buf[100] = { 0 };
		int s = clisock.RecvSome(buf, 100);
		cout << buf << endl;
		std::string reply = std::string("svr reply :") + buf;
		clisock.SendSome(reply.c_str(), reply.length());
	}
	else {
		SocketLib::TcpConnector<SocketLib::IoService> sock(iosverice);
		p = &sock;
		sock.Connect(SocketLib::Tcp::EndPoint(SocketLib::AddressV4("127.0.0.1"), 3001));
		sock.SendSome("xiaoquanjie", 11);
		char buf[100] = { 0 };
		int s = sock.RecvSome(buf, 100);
		cout << buf << endl;
	}
	
	thread th1(_other_test, p);
	thread th2(_other_test, p);
	th1.join();
	th2.join();
}

struct TO {
public:
	~TO() {
		cout << this << " ~con" << endl;
	}
};

void slist_test() {
	TO* p = new TO;
	SocketLib::slist<int> sli, sli2;
	sli.push_back(1);
	sli.push_back(2);
	sli2.join(sli);
	cout << sli2.size() << endl;
	sli.push_back(3);
	sli.push_back(4);
	sli2.join(sli);
	while (sli2.size()){
		sli2.pop_front();
	}
}

int main() {

	//slist_test();
	netlib_test();
	//other_test();
	return 0;
}
#include "netio/netio.hpp"
#include <iostream>
#include "base/thread.hpp"
#include "synccall/synccall.hpp"

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
	//std::string str("svr reply:");
	//str.append(buffer.Data(), buffer.Length());
	clisock->Send(buffer.Data(), buffer.Length());
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
			cout << "connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
			//SendData(clisock);
		}
	}
	virtual void OnConnected(netiolib::HttpSocketPtr clisock) {
		cout << "OnConnected one http : " << clisock->RemoteEndpoint().Address()
			<< " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnConnected(netiolib::HttpConnectorPtr clisock, SocketLib::SocketError error) {
		if (error) {
			cout << "http connect fail :" << error.What() << endl;
		}
		else {
			cout << "http connect success : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
		}
	}

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(const netiolib::TcpSocketPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->RemoteEndpoint().Address() << " " << clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(const netiolib::TcpConnectorPtr& clisock) {
		cout << "OnDisconnected one : " << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(netiolib::HttpSocketPtr clisock) {
		cout << "OnDisconnected one http : " << clisock->RemoteEndpoint().Address() << " "
			<< clisock->RemoteEndpoint().Port() << endl;
	}
	virtual void OnDisconnected(netiolib::HttpConnectorPtr clisock) {
		cout << "OnDisconnected one http connector: " << clisock->RemoteEndpoint().Address() << " "
			<< clisock->RemoteEndpoint().Port() << endl;
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
	virtual void OnReceiveData(netiolib::HttpSocketPtr clisock, netiolib::HttpSvrRecvMsg& httpmsg) {
		//cout << httpmsg.GetRequestLine() << endl;
		netiolib::HttpSvrSendMsg& msg = clisock->GetSvrMsg();
		msg.SetBody("newxiaoquanjie", 14);
		clisock->SendHttpMsg();
	}
	virtual void OnReceiveData(netiolib::HttpConnectorPtr, netiolib::HttpCliRecvMsg& httmsg) {
		cout << httmsg.GetRespondLine() << endl;
		cout << httmsg.GetHeader() << endl;
		cout << httmsg.GetBody() << endl;
	}

	void Start(void*) {
		Run();
	}
	
};

void server() {
	TestNetIo test_io;
	for (int i = 0; i < 32; ++i) {
		new base::thread(&TestNetIo::Start, &test_io, 0);
	}
	
	base::thread::sleep(200);
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

void http_server() {
	TestNetIo test_io;
	for (int i = 0; i < 32; ++i) {
		new base::thread(&TestNetIo::Start, &test_io, 0);
	}

	base::thread::sleep(200);
	if (!test_io.ListenOneHttp("0.0.0.0", 3002)) {
		cout << test_io.GetLastError().What() << endl;
	}
	else
		cout << "listening....." << endl;

	int i;
	cin >> i;
	test_io.Stop();
}

void client() {
	TestNetIo test_io;
	for (int i = 0; i < 32; ++i) {
		new base::thread(&TestNetIo::Start, &test_io, 0);
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
		netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(test_io));
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

void http_client() {
	TestNetIo test_io;
	for (int i = 0; i < 1; ++i) {
		new base::thread(&TestNetIo::Start, &test_io, 0);
	}
	base::thread::sleep(200);

	std::string addr = "61.135.169.121";
	unsigned short port = 80;
	/*cout << "please input ip: ";
	cin >> addr;
	cout << "please input port: ";
	cin >> port;*/

	test_io.ConnectOneHttp(addr, port);

	int i;
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

void netlib_http_test() {
	cout << "select 1 is server,or client :" << endl;
	int i = 0;
	cin >> i;
	if (i == 1)
		http_server();
	else
		http_client();
}

void _other_test(void*p) {

	try {
	}
	catch (SocketLib::SocketError& error) {
		cout << base::thread::ctid() << "  " << error.What() << endl;
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
	
	base::thread th1(_other_test, p);
	base::thread th2(_other_test, p);
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
	base::slist<int> sli, sli2;
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

void test1(function_t<void()>& t){
	function_t<void()> f;
	f.swap(t);
}


const char* gHttpReqStr = "GET /?name=xiao&age=18 HTTP/1.1\r\n"
"Host: 192.168.10.128:3002\r\n"
"Connection: keep-alive\r\n"
"Upgrade-Insecure-Requests: 1\r\n"
"Content-Length: 11\r\n"
"User-Agent: ";

const char* gHttpReqStr2 = "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.89 Safari/537.36 OPR/49.0.2725.47\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
"Accept-Encoding: gzip, deflate\r\n"
"Accept-Language: zh-CN,zh;q=0.9\r\n"
"\r\n";

const char* gHttpReqStr3 = "xiaoquan";
const char* gHttpReqStr4 = "jie1";

void httpmsg_test() {
	netiolib::HttpSvrRecvMsg msg;
	time_t beg_t = clock();
	for (int i = 0; i < 10000000; ++i) {
		msg.Parse(gHttpReqStr, strlen(gHttpReqStr));
		msg.Parse(gHttpReqStr2, strlen(gHttpReqStr2));
		msg.Parse(gHttpReqStr3, strlen(gHttpReqStr3));
		msg.Parse(gHttpReqStr4, strlen(gHttpReqStr4));
		if (msg.GetFlag() != netiolib::HttpSvrRecvMsg::E_PARSE_OVER)
			assert(0);
	}
	time_t end_t = clock();
	cout << "elapsed : " << ((double)(end_t-beg_t) / CLOCKS_PER_SEC) << endl;
}

void sync_test() {
	TestNetIo test_io;
	netiolib::TcpConnector connector(test_io);
	SocketLib::Opts::SndTimeOut timeo(5, 0);
	//connector.GetSocket().SetOption(timeo);
	if (connector.Connect("127.0.0.1", 5001)) {
		cout << "connector success" << endl;
	}
	else {
		cout << test_io.GetLastError().What() << endl;
	}
}

int main() {

	//httpmsg_test();
	//test1(TO());
	//slist_test();
	//netlib_test();
	//netlib_http_test();
	//other_test();
	sync_test();

	int pause_i;
	cin >> pause_i;
	return 0;
}
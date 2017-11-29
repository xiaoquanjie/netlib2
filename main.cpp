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
			cout << "connect success : " << clisock->LocalEndpoint().Address() << " " << clisock->LocalEndpoint().Port() << endl;
			//SendData(clisock);
		}
	}
	virtual void OnConnected(netiolib::HttpSocketPtr clisock) {
		cout << "OnConnected one http : " << clisock->RemoteEndpoint().Address()
			<< " " << clisock->RemoteEndpoint().Port() << endl;
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
	virtual void OnReceiveData(netiolib::HttpSocketPtr clisock, SocketLib::Buffer& buffer) {

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

void http_server() {
	TestNetIo test_io;
	for (int i = 0; i < 32; ++i) {
		new thread(&TestNetIo::Start, &test_io, 0);
	}

	thread::sleep(200);
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
		netiolib::TcpConnectorPtr connector(new netiolib::TcpConnector(test_io));
		connector->SetData(k);
		connector->AsyncConnect(/*"192.168.10.128"*/"127.0.0.1", 3001);
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

void test1(function_t<void()>& t){
	function_t<void()> f;
	f.swap(t);
}

struct HttpMsgessage {
private:
	struct strpos {
		int beg;
		int end;
	};

	SocketLib::Buffer _buffer;
	strpos _method;
	strpos _url;
	strpos _ver;
	strpos _header;
	strpos _body;
	int    _flag;
	int	   _idx;
	bool   _assistflag;

	enum {
		E_PARSE_METHOD = 0,
		E_PARSE_URL,
		E_PARSE_VER,
		E_PARSE_HEAD,
		E_PARSE_BODY,
		E_PARSE_OVER,
	};

protected:
	int _Parse1(char* buffer, int len,bool& hit,char chr) {
		hit = false;
		int pos = 0;
		while (pos < len) {
			if (*(buffer + pos) == chr) {
				hit = true;
				break;
			}
			++pos;
		}
		int copy_len = pos != len ? pos + 1 : len;
		return copy_len;
	}
	int _Parse2(char* buffer, int len, bool& hit, char chr1, char chr2) {
		hit = false;
		int pos = 0;
		while (pos + 1 < len) {
			if (*(buffer + pos) == chr1
				&& *(buffer + pos + 1) == chr2) {
				hit = true;
				break;
			}
			++pos;
		}
		int copy_len = (pos + 1 != len) ? pos + 2 : len;
		return copy_len;
	}

	int _ParseMethod(char* buffer, int len) {
		bool hit = false;
		int copy_len = _Parse1(buffer, len, hit, ' ');
		_buffer.Write(buffer, copy_len);
		if (hit) {
			int pos = (int)_buffer.Size();
			_method.end = pos - 1;
			_flag = E_PARSE_URL;
			_url.beg = pos;
			return copy_len + _ParseUrl(buffer + copy_len, len - copy_len);
		}
		return copy_len;
	}
	int _ParseUrl(char* buffer, int len) {
		if (len > 0) {
			bool hit = false;
			int copy_len = _Parse1(buffer, len, hit, ' ');
			_buffer.Write(buffer, copy_len);
			if (hit) {
				int pos = (int)_buffer.Size();
				_url.end = pos - 1;
				_flag = E_PARSE_VER;
				_ver.beg = pos;
				return copy_len + _ParseVer(buffer + copy_len, len - copy_len);
			}
			return copy_len;
		}
		return 0;
	}
	int _ParseVer(char* buffer, int len) {
		if (len > 0) {
			bool hit = false;
			int copy_len = 0;
			if (_assistflag) {
				if (*buffer != '\n') {
					_assistflag = false;
					copy_len = _Parse2(buffer, len, hit, '\r', '\n');
				}
				else {
					hit = true;
					copy_len = 1;
				}
			}
			else {
				copy_len = _Parse2(buffer, len, hit, '\r', '\n');
			}
			_buffer.Write(buffer, copy_len);
			if (hit) {
				int pos = (int)_buffer.Size();
				_ver.end = pos - 2;
				_flag = E_PARSE_HEAD;
				_header.beg = pos;
				_assistflag = false;
				return copy_len + _ParseHead(buffer + copy_len, len - copy_len);
			}
			if (!_assistflag) {
				int pos = (int)_buffer.Size();
				if (*(_buffer.Data() + pos - 1) == '\r')
					_assistflag = true;
			}
			return copy_len;
		}
		return 0;
	}
	int _ParseHead(char* buffer, int len) {
		return 0;
	}
	int _ParseBody(char* buffer, int len) {
		return 0;
	}

public:
	HttpMsgessage() {
		_method.beg = _method.end
			= _url.beg = _url.end
			= _ver.beg = _ver.end
			= _header.beg = _header.end
			= _body.beg = _body.end = 0;
		_flag = E_PARSE_METHOD;
		_idx = 0;
		_assistflag = false;
	}

	// 返回使用的长度值
	int Parse(char* buffer,int len) {
		switch (_flag) {
		case E_PARSE_OVER:
			break;
		case E_PARSE_METHOD:
			return _ParseMethod(buffer, len);
		case E_PARSE_URL:
			return _ParseUrl(buffer, len);
		case E_PARSE_VER:
			return _ParseVer(buffer, len);
		case E_PARSE_HEAD:
			return _ParseHead(buffer, len);
		case E_PARSE_BODY:
			return _ParseBody(buffer, len);
		default:
			break;
		}
		return 0;
	}

	int GetFlag()const {
		return _flag;
	}
	// copy is slow
	std::string GetMethod()const {
		if (_method.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _method.beg, _method.end - _method.beg);
	}
	// copy is slow
	std::string GetUrl()const {
		if (_url.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _url.beg, _url.end - _url.beg);
	}
	// copy is slow
	std::string GetVersion()const {
		if (_ver.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _ver.beg, _ver.end - _ver.beg);
	}
	// copy is slow
	std::string GetHeader()const {
		if (_header.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _header.beg, _header.end - _header.beg);
	}
	// copy is slow
	std::string GetBody()const {
		if (_body.end == 0)
			return std::string("");
		return std::string(_buffer.Data() + _header.beg, _body.end - _body.beg);
	}

};

char* gHttpReqStr = "GET /?name=xiao&age=18 HTTP/1.1\r\n"
"Host: 192.168.10.128:3002\r\n"
"Connection: keep-alive\r\n"
"Upgrade-Insecure-Requests: 1\r\n"
"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.89 Safari/537.36 OPR/49.0.2725.47\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
"Accept-Encoding: gzip, deflate\r\n"
"Accept-Language: zh-CN,zh;q=0.9\r\n"
"\r\n";


int main() {

	gHttpReqStr = "GET /?name=xiao&age=18 HTTP/1.1\r\n";
	//gHttpReqStr = "GET";
	cout << strlen(gHttpReqStr) << endl;
	HttpMsgessage msg;
	cout << msg.Parse(gHttpReqStr, strlen(gHttpReqStr)) << endl;
	//cout << msg.Parse(" a\n\r\nhost", strlen(" a\n\r\nhost")) << endl;
	cout << msg.GetMethod() << endl;
	cout << msg.GetUrl() << endl;
	std::string h = msg.GetVersion();
	cout << h << endl;
	//cout << gHttpReqStr;
	//cout << gHttpReqStr;
	//test1(TO());
	//slist_test();
	//netlib_test();
	//netlib_http_test();
	//other_test();

	int pause_i;
	cin >> pause_i;
	return 0;
}
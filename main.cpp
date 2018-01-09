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


// rpc����
void synccall_server();
void synccall_client();
void synccall_test() {
	int i;
	cout << "select 1 is server or client:";
	cin >> i;
	if (i == 1)
		synccall_server();
	else
		synccall_client();
}


void async_tcp_server();
void async_tcp_client();
void async_tcp_test() {
	int i;
	cout << "select 1 is server or client:";
	cin >> i;
	if (i == 1)
		async_tcp_server();
	else
		async_tcp_client();
}

void http_server();
void http_client();
void http_test() {
	int i;
	cout << "select 1 is server or client:";
	cin >> i;
	if (i == 1)
		http_server();
	else
		http_client();
}

int main() {
	
	synccall_test();
	//async_tcp_test();
	//http_test();

	int pause_i;
	cin >> pause_i;
	return 0;
}
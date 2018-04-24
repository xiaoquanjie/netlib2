#include "netio/netio.hpp"
#include <iostream>
#include "base/thread.hpp"
#include "synccall/synccall.hpp"
#include "base/logger.hpp"
#include "base/condition.hpp"

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


// rpc²âÊÔ
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

void co_synccall_server();
void co_synccall_client();
void co_synccall_test() {
	int i;
	cout << "select 1 is server or client:";
	cin >> i;
	if (i == 1)
		co_synccall_server();
	else
		co_synccall_client();
}

void test_lock() {
	base::MutexLock mutex;
	for (int i = 0; i < 1000000; ++i) {

	}
}

void print_logger(void*) {
	cout << "begin........" << endl;
	for (int i = 0; i < 1000000; ++i) {
		LogDebug("xiaoquanjie " << i << " taoxinzhi.......................\
			......................................");
	}
	cout << "finish........" << endl;
}

void test_logger() {
	SetLogFileName("mylog", false);
	SetLogOutput(0);

	base::thread thr1(print_logger, 0);
	//base::thread thr2(print_logger, 0);
	thr1.join();
	//thr2.join();
}

int main() {
	
	//test_logger();
	//synccall_test();
	async_tcp_test();
	//http_test();
	//co_synccall_test();

	int pause_i;
	cin >> pause_i;
	return 0;
}
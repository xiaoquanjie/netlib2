// socket.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include <iostream>
#include "socket/socket.hpp"
#include "thread.hpp"
#include <map>
#include "demo/async_server_demo.h"
#include "demo/async_client_demo.h"
#include "netio/netio.hpp"

#define _CRT_SECURE_NO_WARNINGS

IoService ioservice;
std::list<std::string> gloglist;
static MutexLock gmutextlock;

void thread_fun(void*)
{
	//while (true)
	{
		try
		{
			ioservice.Run();
			cout << "服务退出:" << endl;
		}
		catch (SocketError error)
		{
			gmutextlock.lock();
			cout << "ioservice happend error,code:" << error.Code() << " what:" << error.What() << endl;
			gmutextlock.unlock();
		}
	}
}

void async_server()
{
	void* p = new AsyncServer(ioservice);
}

void async_client()
{
	new AsyncClient(ioservice);
}


static void sleep_fun(int time)
{
#ifndef M_PLATFORM_WIN
	usleep(time * 1000);
#else
	::Sleep(time);
#endif
}

void thread_log(void*)
{
	static int cnt = 0;
	while (true)
	{
		std::list<std::string> logs;
		gmutextlock.lock();
		logs.swap(gloglist);
		gmutextlock.unlock();
		if (logs.empty())
			sleep_fun(5);

		for (std::list<std::string>::iterator iter=logs.begin();
			iter!=logs.end(); ++iter)
		{
			++cnt;
			cout << cnt << " " << (*iter);
		}
	}
}

void push_log(std::string log)
{
	gmutextlock.lock();
	gloglist.push_back(log);
	gmutextlock.unlock();
}

int main()
{
	thread thr_log(thread_log, 0);
	thread thr1(&thread_fun, 0);
	thread thr2(&thread_fun, 0);
	thread thr3(&thread_fun, 0);
	thread thr4(&thread_fun, 0);
	thread thr5(&thread_fun, 0);
	thread thr6(&thread_fun, 0);

	while (ioservice.ServiceCount()==0)
	{
		sleep_fun(2);
	}
	//async_server();
	async_client();
	int i = 0;
	cin >> i;
	ioservice.Stop();
	thr1.join();
	thr2.join();
	thr3.join();
	thr4.join();
	thr5.join();
	thr6.join();
	//thr_log.join();
	return 0;
}
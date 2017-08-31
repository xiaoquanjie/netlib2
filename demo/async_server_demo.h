#ifndef ASYNC_SERVER_DEMO_H
#define ASYNC_SERVER_DEMO_H

#include <iostream>
#include <list>
#include <string>
#include "../socket/socket.hpp"
using namespace SocketLib;
using namespace std;

typedef shard_ptr_t<TcpSocket<IoService> >   TcpSocketPtr;
typedef shard_ptr_t<TcpAcceptor<IoService> > TcpAcceptorPtr;
typedef shard_ptr_t<TcpConnector<IoService> > TcpConnectorPtr;

void push_log(std::string log);

class AsyncServer
{
public:
	struct init_data
	{
		int read_cnt;
		int write_cnt;
		bool is_writing;
		s_byte_t* read_ptr;
		s_byte_t* write_ptr;
		std::list<std::string> receive;
		std::list<std::string> reply;
		MutexLock lock;
	};
public:
	AsyncServer(IoService& ioservice);

	void Run();

	void WriteHandler(TcpSocketPtr sock, const s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error);

	void ReadHandler(TcpSocketPtr sock, s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error);

	void AcceptHandler(TcpAcceptorPtr acceptor, TcpSocketPtr sock, SocketError error);

	void DestroyHandler(s_byte_t* read_ptr, s_byte_t* write_ptr,init_data*);

	init_data* CreateInitData(s_int32_t s1, s_int32_t s2);

	void FreeInitData(init_data* data);

	void CheckRead();

	void CheckWrite(bool is_send,init_data* data);

private:
	IoService& m_ioservice;

	M_WRITE_HANDLER_TYPE(IoService)		m_write_handler;
	M_READ_HANDLER_TYPE(IoService)		m_read_handler;
	M_ACCEPT_HANDLER_TYPE(IoService)	m_accept_handler;
	function_t<void()> m_destroy_handler;
};


#endif
#ifndef ASYNC_CLIENT_DEMO_H
#define ASYNC_CLIENT_DEMO_H

#include <iostream>
#include "../socket/socket.hpp"
using namespace SocketLib;
using namespace std;

typedef shard_ptr_t<TcpSocket<IoService> >   TcpSocketPtr;
typedef shard_ptr_t<TcpAcceptor<IoService> > TcpAcceptorPtr;
typedef shard_ptr_t<TcpConnector<IoService> > TcpConnectorPtr;

void push_log(std::string log);

class AsyncClient
{
public:
	struct init_data
	{
		int max_cnt;
		int cnt;
		int read_cnt;
		int write_cnt;
		s_byte_t* read_ptr;
		s_byte_t* write_ptr;
	};

	AsyncClient(IoService& ioservice);

	void Init();

	void Init2();

	void Run();

	void ConnectHandler(TcpConnectorPtr connector, SocketError error);

	void ConnectHandler2(SocketError error, TcpConnectorPtr connector);

	void WriteHandler(TcpConnectorPtr connector, const s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error);

	void WriteHandler2(s_uint32_t trans, SocketError error,TcpConnectorPtr connector);

	void ReadHandler(TcpConnectorPtr connector, s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error);

	void ReadHandler2(s_uint32_t trans, SocketError error, TcpConnectorPtr connector);

	void DestroyHandler(s_byte_t* read_ptr, s_byte_t* write_ptr, init_data*);

	init_data* CreateInitData(s_int32_t s1, s_int32_t s2);

	void FreeInitData(init_data* data);

private:
	IoService& m_ioservice;

	M_CONNECT_HANDLER_TYPE(IoService) m_connect_handler;
	M_C_WRITE_HANDLER_TYPE(IoService)   m_write_handler;
	M_C_READ_HANDLER_TYPE(IoService)    m_read_handler;
};

#endif
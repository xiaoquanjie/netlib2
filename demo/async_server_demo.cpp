#include "async_server_demo.h"
#include<sstream>

static const char* gContent = "hi,this is a,!!!!!!!!!!";

static MutexLock glock;
#define M_PRINT_WITH_LOCK(info)\
{\
	std::stringstream ss;\
	ss << info;\
	push_log(ss.str());\
}

static void print_error(const SocketError& error)
{
	M_PRINT_WITH_LOCK("code: " << error.Code() << " what:" << error.What() << endl);
}

static void* m_malloc(size_t size)
{
	void* p = malloc(size);
	return p;
}

static void m_free(void* p)
{
	free(p);
}

static void print_func(const char* ptr)
{
	M_PRINT_WITH_LOCK(ptr << endl);
}

static void print_func(const s_byte_t* ptr, SocketError& error)
{
	M_PRINT_WITH_LOCK(ptr << " code: " << error.Code() << " what:" << error.What() << endl);
}

AsyncServer::AsyncServer(IoService& ioservice):m_ioservice(ioservice)
{
	m_write_handler = bind_t(&AsyncServer::WriteHandler, this, placeholder_1, placeholder_2, placeholder_3, placeholder_4,
		placeholder_5);
	m_read_handler = bind_t(&AsyncServer::ReadHandler, this, placeholder_1, placeholder_2, placeholder_3, placeholder_4,
		placeholder_5);
	m_accept_handler = bind_t(&AsyncServer::AcceptHandler, this, placeholder_1, placeholder_2, placeholder_3);
	m_destroy_handler = bind_t(&AsyncServer::DestroyHandler, this, (s_byte_t*)0, (s_byte_t*)0, (init_data*)0);

	try
	{
		SocketError error;
		Tcp::EndPoint ep(AddressV4(""), 2001);
		TcpAcceptorPtr AcceptorPtr(new TcpAcceptor<IoService>(m_ioservice, ep));

		AcceptorPtr->DestroyHandler(m_destroy_handler);
		AcceptorPtr->AsyncAccept(m_accept_handler);
	}
	catch (SocketError& error)
	{
		print_error(error);
	}
}

void AsyncServer::Run()
{
	m_ioservice.Run();
}

void AsyncServer::WriteHandler(TcpSocketPtr sock, const s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)
{
	init_data* pdata = (init_data*)sock->GetData();
	if (error)
	{
		print_func("write handler error");
		print_error(error);
	}
	else if (trans == 0)
	{
		print_func("write handler remote close connection");
	}
	{
		++pdata->write_cnt;
		string str;
		pdata->lock.lock();
		if (pdata->reply.size())
		{
			pdata->write_ptr[0] = 0;
			str = pdata->reply.front();
			pdata->reply.pop_front();
		}
		else
			pdata->is_writing = false;
		pdata->lock.unlock();
		if (str.size())
		{
			memcpy(pdata->write_ptr, str.c_str(), str.length());
			sock->AsyncSendSome(pdata->write_ptr, (s_uint32_t)str.length(),m_write_handler, error);
			if (error)
				print_error(error);
		}
	}
}

void AsyncServer::ReadHandler(TcpSocketPtr sock, s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)
{
	if (error)
	{
		print_func("read handler error",error);
	}
	else if (trans==0)
	{
		print_func("read handler remote close connection");
	}
	else
	{
		init_data* pdata = (init_data*)sock->GetData();
		data[trans] = '\0';
		std::string reply = std::string("reply:") + std::string(data);

		++pdata->read_cnt;
		M_PRINT_WITH_LOCK("server receive max : " << max << " trans : " << trans << " data : " << data << endl);
		data[0] = 0;
		sock->AsyncRecvSome(data, (s_uint32_t)strlen(gContent), m_read_handler, error);
		if (error)
		{
			print_error(error);
		}

		pdata->lock.lock();
		if (!pdata->is_writing)
		{
			pdata->is_writing = true;
			memcpy(pdata->write_ptr, reply.c_str(), reply.length());
			sock->AsyncSendSome(pdata->write_ptr, (s_uint32_t)reply.length(), m_write_handler, error);
		}
		else			
			pdata->reply.push_back(reply);
		pdata->lock.unlock();
	}
}

void AsyncServer::AcceptHandler(TcpAcceptorPtr acceptor, TcpSocketPtr sock, SocketError error)
{
	if (error)
	{
		print_func("accept handler happen error :", error);
	}
	else
	{
		print_func("accept one....");
		init_data* data = CreateInitData(100, 100);
		sock->SetData(data);
		sock->DestroyHandler(bind_t(&AsyncServer::DestroyHandler, this, (s_byte_t*)data->read_ptr, (s_byte_t*)data->write_ptr, data));

		SocketError error2;
		sock->AsyncRecvSome((s_byte_t*)data->read_ptr, (s_uint32_t)strlen(gContent), m_read_handler, error2);
		if (error2)
		{
			print_func("recv error", error2);
		}
	}
	acceptor->AsyncAccept(m_accept_handler);
}

void AsyncServer::DestroyHandler(s_byte_t* read_ptr, s_byte_t* write_ptr,init_data* pdata)
{
	static int cnt = 0;
	static int read_cnt = 0;
	static int write_cnt = 0;
	static MutexLock lock;
	lock.lock();
	if (pdata)
	{
		read_cnt += pdata->read_cnt;
		write_cnt += pdata->write_cnt;
	}
	++cnt;
	M_PRINT_WITH_LOCK("destroy handler : " << cnt << " read_cnt : " << read_cnt << " write_cnt : " << write_cnt << endl);
	FreeInitData(pdata);
	lock.unlock();
	//_CrtDumpMemoryLeaks();
}

AsyncServer::init_data* AsyncServer::CreateInitData(s_int32_t s1, s_int32_t s2)
{
	init_data* data = new init_data;
	data->read_ptr = (s_byte_t*)m_malloc(s1);
	data->write_ptr = (s_byte_t*)m_malloc(s2);
	data->is_writing = false;
	data->read_cnt = 0;
	data->write_cnt = 0;
	return data;
}

void AsyncServer::FreeInitData(init_data* data)
{
	if (data)
	{
		m_free(data->read_ptr);
		m_free(data->write_ptr);
		delete data;
	}
}

void AsyncServer::CheckRead()
{
}

void AsyncServer::CheckWrite(bool is_send, init_data* data)
{
}
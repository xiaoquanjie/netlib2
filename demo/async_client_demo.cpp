#include "async_client_demo.h"
#include   <time.h>  
#include<sstream>

static MutexLock glock;
#define M_PRINT_WITH_LOCK(info)\
{\
	std::stringstream ss;\
	ss << info;\
	push_log(ss.str());\
}

static const char* gContent = "reply:hi,this is a,!!!!!!!!!!";

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
	M_PRINT_WITH_LOCK(ptr << " code: "<< error.Code() << " what:" << error.What() << endl);
}

void sleep_fun(int time)
{
#ifndef M_PLATFORM_WIN
	usleep(time * 1000);
#else
	::Sleep(time);
#endif
}

const char* gWords[] = {
	"hi,this is a,!!!!!!!!!!",
	"hi,this is b,!!!!!!!!!!",
	"hi,this is c,!!!!!!!!!!",
	"hi,this is d,!!!!!!!!!!",
	"hi,this is e,!!!!!!!!!!",
	"hi,this is f,!!!!!!!!!!",
	"hi,this is g,!!!!!!!!!!"
};

std::pair<const char*, int> GetRandWord()
{
	int idx = sizeof(gWords) / sizeof(char*);
	idx = 0 + rand() % (idx - 1);
	return std::pair<const char*, int>(gWords[idx], (int)strlen(gWords[idx]));
}

AsyncClient::AsyncClient(IoService& ioservice):m_ioservice(ioservice)
{
	srand((unsigned)time(0));
	m_connect_handler = bind_t(&AsyncClient::ConnectHandler, this, placeholder_1, placeholder_2);
	m_write_handler = bind_t(&AsyncClient::WriteHandler, this, placeholder_1, placeholder_2, placeholder_3, placeholder_4, placeholder_5);
	m_read_handler = bind_t(&AsyncClient::ReadHandler, this, placeholder_1, placeholder_2, placeholder_3, placeholder_4, placeholder_5);

	Init2();
}

void AsyncClient::Init()
{
	//for (int i = 0; i < 1000; ++i)
	{
		try
		{
			SocketError error;
			Tcp::EndPoint ep(AddressV4("127.0.0.1"), 2001);
			TcpConnectorPtr ConnectorPtr(new TcpConnector<IoService>(m_ioservice));
			ConnectorPtr->AsyncConnect(ep, m_connect_handler);
		}
		catch (SocketError& error)
		{
			print_error(error);
		}
	}
}

void AsyncClient::Init2()
{
	try
	{
		SocketError error;
		Tcp::EndPoint ep(AddressV4("127.0.0.1"), 2001);
		TcpConnectorPtr ConnectorPtr(new TcpConnector<IoService>(m_ioservice));

		M_COMMON_HANDLER_TYPE(IoService) func = bind_t(&AsyncClient::ConnectHandler2, this, placeholder_1, ConnectorPtr);
		ConnectorPtr->AsyncConnect(func, ep);
	}
	catch (SocketError& error)
	{
		print_error(error);
	}
}

void AsyncClient::Run()
{
	m_ioservice.Run();
}

void AsyncClient::ConnectHandler(TcpConnectorPtr connector, SocketError error)
{
	if (error)
	{
		print_func("connect fail :",error);
		/*Tcp::EndPoint ep(AddressV4("127.0.0.1"), 2001);
		connector.reset(new TcpConnector<IoService>(m_ioservice));
		connector->AsyncConnect(ep, m_connect_handler);*/
	}
	else
	{
		print_func("connect success..............");
		init_data* pdata = CreateInitData(100, 100);
		connector->SetData(pdata);
		connector->DestroyHandler(bind_t(&AsyncClient::DestroyHandler, this, pdata->read_ptr, pdata->write_ptr, pdata));

		SocketError error;
		connector->AsyncRecvSome(pdata->read_ptr, (s_uint32_t)strlen(gContent), m_read_handler, error);
		if (error)
		{
			print_func("async recv some error :", error);
		}

		std::pair<const char*, int> p = GetRandWord();
		memcpy(pdata->write_ptr, p.first, p.second);
		connector->AsyncSendSome(pdata->write_ptr, p.second, m_write_handler,error);
		if (error)
		{
			print_func("async send some error :", error);
		}
	}
}

void AsyncClient::ConnectHandler2(SocketError error, TcpConnectorPtr connector)
{
	if (error)
	{
		print_func("connect fail :", error);
		/*Tcp::EndPoint ep(AddressV4("127.0.0.1"), 2001);
		connector.reset(new TcpConnector<IoService>(m_ioservice));
		connector->AsyncConnect(ep, m_connect_handler);*/
	}
	else
	{
		print_func("connect success..............");
		init_data* pdata = CreateInitData(100, 100);
		connector->SetData(pdata);
		connector->DestroyHandler(bind_t(&AsyncClient::DestroyHandler, this, pdata->read_ptr, pdata->write_ptr, pdata));

		M_RW_HANDLER_TYPE(IoService) read_func = bind_t(&AsyncClient::ReadHandler2, this, placeholder_1, placeholder_2, connector);
		SocketError error;
		connector->AsyncRecvSome(read_func, pdata->read_ptr, (s_uint32_t)strlen(gContent), error);
		if (error)
		{
			print_func("async recv some error :", error);
		}

		M_RW_HANDLER_TYPE(IoService) write_func = bind_t(&AsyncClient::WriteHandler2, this, placeholder_1, placeholder_2, connector);
		std::pair<const char*, int> p = GetRandWord();
		memcpy(pdata->write_ptr, p.first, p.second);
		connector->AsyncSendSome(write_func, pdata->write_ptr, p.second, error);
		if (error)
		{
			print_func("async send some error :", error);
		}
	}
}

void AsyncClient::WriteHandler(TcpConnectorPtr connector, const s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)
{
	if (error)
	{
		print_func("write handler happend error: ", error);
	}
	else if (trans==0)
	{
		print_func("remote peer close the connection");
	}
	else
	{
		init_data* pdata = (init_data*)connector->GetData();
		++pdata->cnt;
		++pdata->write_cnt;
		if (pdata->cnt >= pdata->max_cnt)
		{
			//connector->Close();
			connector->Shutdown(E_Shutdown_WR);
			return;
		}
		else
		{
			int rand_time = 0 + rand() % (30 - 1);
			//sleep_fun(rand_time);
			std::pair<const char*, int> p = GetRandWord();
			memcpy(pdata->write_ptr, p.first, p.second);
			connector->AsyncSendSome(pdata->write_ptr, p.second, m_write_handler);
		}
	}
}

void AsyncClient::WriteHandler2(s_uint32_t trans, SocketError error, TcpConnectorPtr connector)
{
	if (error)
	{
		print_func("write handler happend error: ", error);
	}
	else if (trans == 0)
	{
		print_func("remote peer close the connection");
	}
	else
	{
		init_data* pdata = (init_data*)connector->GetData();
		++pdata->cnt;
		++pdata->write_cnt;
		if (pdata->cnt >= pdata->max_cnt)
		{
			//connector->Close();
			connector->Shutdown(E_Shutdown_WR);
			return;
		}
		else
		{
			int rand_time = 0 + rand() % (30 - 1);
			//sleep_fun(rand_time);
			std::pair<const char*, int> p = GetRandWord();
			memcpy(pdata->write_ptr, p.first, p.second);

			M_RW_HANDLER_TYPE(IoService) write_func = bind_t(&AsyncClient::WriteHandler2, this, placeholder_1, placeholder_2, connector);
			connector->AsyncSendSome(write_func,pdata->write_ptr, p.second);
		}
	}
}

void AsyncClient::ReadHandler(TcpConnectorPtr connector, s_byte_t* data, s_uint32_t max, s_uint32_t trans, SocketError error)
{
	init_data* pdata = (init_data*)connector->GetData();
	if (error)
	{
		print_func("read handler happend error: ", error);
	}
	else if (trans==0)
	{
		print_func("read handler remote peer close the connection");
	}
	else
	{
		++pdata->read_cnt;
		data[trans] = 0;
		M_PRINT_WITH_LOCK("cli ReadHandler recv max :" << max << " trans :" << trans << "  data :" << data << endl);
		data[0] = 0;
		connector->AsyncRecvSome(data, (s_uint32_t)strlen(gContent), m_read_handler,error);
		if (error)
		{
			print_func("async recv some error", error);
		}
	}
}

void AsyncClient::ReadHandler2(s_uint32_t trans, SocketError error, TcpConnectorPtr connector)
{
	init_data* pdata = (init_data*)connector->GetData();
	if (error)
	{
		print_func("read handler happend error: ", error);
	}
	else if (trans == 0)
	{
		print_func("read handler remote peer close the connection");
	}
	else
	{
		++pdata->read_cnt;
		pdata->read_ptr[trans] = 0;
		M_PRINT_WITH_LOCK("cli ReadHandler trans :" << trans << "  data :" << pdata->read_ptr << endl);
		pdata->read_ptr[0] = 0;

		M_RW_HANDLER_TYPE(IoService) read_func = bind_t(&AsyncClient::ReadHandler2, this, placeholder_1, placeholder_2, connector);
		connector->AsyncRecvSome(read_func, pdata->read_ptr, (s_uint32_t)strlen(gContent), error);
		if (error)
		{
			print_func("async recv some error", error);
		}
	}
}

void AsyncClient::DestroyHandler(s_byte_t* read_ptr, s_byte_t* write_ptr, init_data* pdata)
{
	static int cnt = 0;
	static int read_cnt = 0;
	static int write_cnt = 0;
	static MutexLock lock;
	lock.lock();
	read_cnt += pdata->read_cnt;
	write_cnt += pdata->write_cnt;
	++cnt;
	M_PRINT_WITH_LOCK("destroy handler total_cnt : " << cnt << " total_read_cnt: " << read_cnt << " total_write_cnt: " << write_cnt << " max_cnt :" << pdata->max_cnt
		<< " cur_cnt: " << pdata->cnt << endl);
	FreeInitData(pdata);
	lock.unlock();
}

void AsyncClient::FreeInitData(init_data* data)
{
	if (data)
	{
		m_free(data->read_ptr);
		m_free(data->write_ptr);
		delete data;
	}
}

AsyncClient::init_data* AsyncClient::CreateInitData(s_int32_t s1, s_int32_t s2)
{
	init_data* data = new init_data;
	data->read_ptr = (s_byte_t*)m_malloc(s1);
	data->write_ptr = (s_byte_t*)m_malloc(s2);
	data->max_cnt = 2000;// +rand() % (4000 + 20 - 1);
	data->cnt = 0;
	data->read_cnt = 0;
	data->write_cnt = 0;
	return data;
}
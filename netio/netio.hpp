/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_INCLUDE
#define M_NETIO_NETIO_INCLUDE

#include "config.hpp"
M_NETIO_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#pragma pack(1)
struct MessageHeader {
	int			   timestamp;
	unsigned short size;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#pragma pack()
#else
struct __attribute__((__packed__)) MessageHeader {
	int			   timestamp;
	unsigned short size;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#endif

class NetIo;
class TcpSocket;
class TcpConnector;
class HttpSocket;
class HttpConnector;

typedef SocketLib::Buffer Buffer;
typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<TcpConnector>	   TcpConnectorPtr;
typedef shard_ptr_t<HttpSocket>		   HttpSocketPtr;
typedef shard_ptr_t<HttpConnector>	   HttpConnectorPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > NetIoTcpAcceptorPtr;

#define lasterror tlsdata<SocketLib::SocketError,0>::data()

// class netio
class NetIo 
{
public:
	NetIo();
	NetIo(SocketLib::s_uint32_t backlog);

	virtual ~NetIo();

	// 建立一个监听
	bool ListenOne(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOne(const std::string& addr, SocketLib::s_uint16_t port);

	// 建立一个http监听
	bool ListenOneHttp(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port);

	// 异步建接
	void ConnectOne(const SocketLib::Tcp::EndPoint& ep, unsigned int data=0);
	void ConnectOne(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data=0);

	void ConnectOneHttp(const SocketLib::Tcp::EndPoint& ep, unsigned int data = 0);
	void ConnectOneHttp(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data=0);

	virtual void Run();
	virtual void Stop();

	// 获取最后的异常
	inline SocketLib::SocketError GetLastError()const;
	inline SocketLib::IoService& GetIoService();
	inline SocketLib::s_uint32_t LocalEndian()const;

	/*
	 *以下三个函数定义为虚函数，以便根据实际业务的模式来做具体模式的消息包分发处理。
	 *保证同一个socket，以下三个函数的调用遵循OnConnected -> OnReceiveData -> OnDisconnected的顺序。
	 *保证同一个socket，以下后两个函数的调用都在同一个线程中
	 */

	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(const TcpSocketPtr& clisock);
	virtual void OnConnected(const TcpConnectorPtr& clisock,SocketLib::SocketError error);
	virtual void OnConnected(HttpSocketPtr clisock);
	virtual void OnConnected(HttpConnectorPtr clisock, SocketLib::SocketError error);

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(const TcpSocketPtr& clisock);
	virtual void OnDisconnected(const TcpConnectorPtr& clisock);
	virtual void OnDisconnected(HttpSocketPtr clisock);
	virtual void OnDisconnected(HttpConnectorPtr clisock);

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(const TcpSocketPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(const TcpConnectorPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(HttpSocketPtr clisock, HttpSvrRecvMsg& httpmsg);
	virtual void OnReceiveData(HttpConnectorPtr clisock, HttpCliRecvMsg& httpmsg);


protected:
	void _AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor);
	void _AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, NetIoTcpAcceptorPtr& acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

private:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;
	SocketLib::s_uint32_t  _endian;
};

enum {
	E_STATE_STOP =  0,
	E_STATE_START = 1,
	E_STATE_WRITE = 3,
};

template<typename T, typename SocketType>
class TcpBaseSocket : public enable_shared_from_this_t<T>
{
protected:
	struct _writerinfo_ {
		SocketLib::slist<SocketLib::Buffer*> buffer_pool;
		SocketLib::slist<SocketLib::Buffer*> buffer_pool2;
		SocketLib::Buffer*   msgbuffer;
		SocketLib::MutexLock lock;
		bool writing;

		_writerinfo_();
		~_writerinfo_();
	};

public:
	TcpBaseSocket(NetIo& netio);

	virtual ~TcpBaseSocket();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// 调用这个函数不意味着连接立即断开，会等所有的未处理的数据处理完就会断开
	void Close();

	void Send(SocketLib::Buffer*);

	void Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len);

protected:
	void _WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	inline void _CloseHandler();

	void _PostClose(unsigned int state);

	void _Close(unsigned int state);

	bool _TrySendData(bool ignore = false);

protected:
	NetIo&		 _netio;
	SocketType*  _socket;
	_writerinfo_ _writer;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	// 状态标志
	unsigned short _flag;
};

// for stream
template<typename T, typename SocketType>
class TcpStreamSocket : public TcpBaseSocket<T, SocketType>
{
protected:
	struct _readerinfo_ {
		SocketLib::s_byte_t*  readbuf;
		SocketLib::Buffer	  msgbuffer;
		SocketLib::Buffer	  msgbuffer2;
		MessageHeader		  curheader;

		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);
	
	// 裁减出数据包，返回false意味着数据包有错
	bool _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	TcpStreamSocket(NetIo& netio);
};

// class tcpsocket
class TcpSocket : 
	public TcpStreamSocket<TcpSocket, SocketLib::TcpSocket<SocketLib::IoService> >
{
	friend class NetIo;
public:
	TcpSocket(NetIo& netio);

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket();

protected:
	void Init();
};

// class tcpconnector
class TcpConnector : 
	public TcpStreamSocket<TcpConnector, SocketLib::TcpConnector<SocketLib::IoService> >
{
public:
	TcpConnector(NetIo& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port);

	inline void SetData(unsigned int data);

	inline unsigned int GetData()const;

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector);

protected:
	unsigned int _data;
};

// for http
template<typename T, typename SocketType,typename HttpMsgType>
class HttpBaseSocket : 
	public TcpBaseSocket<T, SocketType>
{
protected:
	struct _readerinfo_ {
		SocketLib::s_byte_t*  readbuf;
		HttpMsgType httpmsg;

		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	// 裁减出数据包，返回false意味着数据包有错
	bool _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	HttpBaseSocket(NetIo& netio);
};

// class httpsocket
class HttpSocket : 
	public HttpBaseSocket<HttpSocket, SocketLib::TcpSocket<SocketLib::IoService>, HttpSvrRecvMsg>
{
	friend class NetIo;
public:
	HttpSocket(NetIo& netio);

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket();

	HttpSvrSendMsg& GetSvrMsg();

	void SendHttpMsg();

protected:
	void Init();

	HttpSvrSendMsg _httpmsg;
};

class HttpConnector : 
	public HttpBaseSocket<HttpConnector, SocketLib::TcpConnector<SocketLib::IoService>
	,HttpCliRecvMsg> 
{
public:
	HttpConnector(NetIo& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port);

	inline void SetData(unsigned int data);

	inline unsigned int GetData()const;

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, HttpConnectorPtr conector);

protected:
	unsigned int _data;
};


M_NETIO_NAMESPACE_END
#include "netio_impl.hpp"
#include "tsocket_impl.hpp"
#include "tconnector_impl.hpp"
#include "hsocket_impl.hpp"
#include "hconnector_impl.hpp"
#endif
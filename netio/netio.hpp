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

#include "netio/config.hpp"
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
class SyncConnector;

typedef SocketLib::Buffer Buffer;
typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<TcpConnector>	   TcpConnectorPtr;
typedef shard_ptr_t<HttpSocket>		   HttpSocketPtr;
typedef shard_ptr_t<HttpConnector>	   HttpConnectorPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > TcpAcceptorPtr;
typedef shard_ptr_t<SyncConnector>  SyncConnectorPtr;

#define lasterror base::tlsdata<SocketLib::SocketError,0>::data()

template<typename NetIoType>
class BaseNetIo {
public:
	BaseNetIo();
	BaseNetIo(SocketLib::s_uint32_t backlog);

	virtual ~BaseNetIo();

	// 建立一个监听
	bool ListenOne(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOne(const std::string& addr, SocketLib::s_uint16_t port);

	// 建立一个http监听
	bool ListenOneHttp(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port);

	// 异步建接
	void ConnectOne(const SocketLib::Tcp::EndPoint& ep, unsigned int data = 0);
	void ConnectOne(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data = 0);

	void ConnectOneHttp(const SocketLib::Tcp::EndPoint& ep, unsigned int data = 0);
	void ConnectOneHttp(const std::string& addr, SocketLib::s_uint16_t port, unsigned int data = 0);

	virtual void Run();
	virtual void Stop();
	size_t  ServiceCount();

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
	virtual void OnConnected(TcpSocketPtr& clisock);
	virtual void OnConnected(TcpConnectorPtr& clisock, SocketLib::SocketError error);
	virtual void OnConnected(HttpSocketPtr& clisock);
	virtual void OnConnected(HttpConnectorPtr& clisock, SocketLib::SocketError error);

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(TcpSocketPtr& clisock);
	virtual void OnDisconnected(TcpConnectorPtr& clisock);
	virtual void OnDisconnected(HttpSocketPtr& clisock);
	virtual void OnDisconnected(HttpConnectorPtr& clisock);

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(TcpSocketPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(TcpConnectorPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(HttpSocketPtr& clisock, HttpSvrRecvMsg& httpmsg);
	virtual void OnReceiveData(HttpConnectorPtr& clisock, HttpCliRecvMsg& httpmsg);


protected:
	void _AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, TcpAcceptorPtr& acceptor);
	void _AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, TcpAcceptorPtr& acceptor);

protected:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;
	SocketLib::s_uint32_t  _endian;
};

// class netio
class NetIo : public BaseNetIo<NetIo>
{
public:
	NetIo() :
		BaseNetIo() {
	}
	NetIo(SocketLib::s_uint32_t backlog)
		:BaseNetIo(backlog) {
	}

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);
};

enum {
	E_STATE_STOP = 0,
	E_STATE_START = 1,
	E_STATE_WRITE = 3,
};

template<typename T, typename SocketType>
class TcpBaseSocket : public enable_shared_from_this_t<T>
{
protected:
	struct _writerinfo_ {
		base::slist<SocketLib::Buffer*> buffer_pool;
		base::svector<SocketLib::Buffer*> buffer_pool2;
		SocketLib::Buffer*   msgbuffer;
		SocketLib::MutexLock lock;
		bool writing;

		_writerinfo_();
		~_writerinfo_();
	};

public:
	TcpBaseSocket(BaseNetIo<NetIo>& netio);

	virtual ~TcpBaseSocket();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// 调用这个函数不意味着连接立即断开，会等所有的未处理的数据处理完就会断开
	void Close();

	void Send(SocketLib::Buffer*);

	void Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len);

	bool IsConnected()const;

protected:
	void _WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	inline void _CloseHandler();

	void _PostClose(unsigned int state);

	void _Close(unsigned int state);

	bool _TrySendData(bool ignore = false);

protected:
	BaseNetIo<NetIo>& _netio;
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
	TcpStreamSocket(BaseNetIo<NetIo>& netio);
};

// class tcpsocket
class TcpSocket :
	public TcpStreamSocket<TcpSocket, SocketLib::TcpSocket<SocketLib::IoService> >
{
	friend class BaseNetIo<NetIo>;

public:
	TcpSocket(BaseNetIo<NetIo>& netio)
		:TcpStreamSocket(netio) {
	}

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket() {
		return (*this->_socket);
	}

protected:
	void Init() {
		try {
			_remoteep = _socket->RemoteEndPoint();
			_localep = _socket->LocalEndPoint();
			_flag = E_STATE_START;
			shard_ptr_t<TcpSocket> ref = this->shared_from_this();
			_netio.OnConnected(ref);
			this->_TryRecvData();
		}
		catch (const SocketLib::SocketError& e) {
			lasterror = e;
		}
	}
};

template<typename ConnectorType>
class BaseTcpConnector :
	public TcpStreamSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService> >
{
public:
	BaseTcpConnector(BaseNetIo<NetIo>& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep, SocketLib::s_uint32_t timeo_sec = -1);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::s_uint32_t timeo_sec = -1);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port);

	inline void SetData(unsigned int data);

	inline unsigned int GetData()const;

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector);

protected:
	unsigned int _data;
};

// class tcpconnector
class TcpConnector : public BaseTcpConnector<TcpConnector>
{
public:
	TcpConnector(BaseNetIo<NetIo>& netio)
		:BaseTcpConnector(netio) {
	}
};

// for http
template<typename T, typename SocketType, typename HttpMsgType>
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
	HttpBaseSocket(BaseNetIo<NetIo>& netio);
};

// class httpsocket
class HttpSocket :
	public HttpBaseSocket<HttpSocket, SocketLib::TcpSocket<SocketLib::IoService>, HttpSvrRecvMsg>
{
	friend class BaseNetIo<NetIo>;
public:
	HttpSocket(BaseNetIo<NetIo>& netio)
		:HttpBaseSocket(netio) {
	}

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket() {
		return (*this->_socket);
	}

	HttpSvrSendMsg& GetSvrMsg() {
		return _httpmsg;
	}

	void SendHttpMsg() {
		Send(_httpmsg._pbuffer);
		_httpmsg._flag = 0;
	}

protected:
	void Init() {
		try {
			this->_remoteep = this->_socket->RemoteEndPoint();
			this->_localep = this->_socket->LocalEndPoint();
			this->_flag = E_STATE_START;
			shard_ptr_t<HttpSocket> ref = this->shared_from_this();
			this->_netio.OnConnected(ref);
			this->_TryRecvData();
		}
		catch (const SocketLib::SocketError& e) {
			lasterror = e;
		}
	}

	HttpSvrSendMsg _httpmsg;
};

template<typename ConnectorType>
class BaseHttpConnector :
	public HttpBaseSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService>
	, HttpCliRecvMsg> {
public:
	BaseHttpConnector(BaseNetIo<NetIo>& netio);

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

class HttpConnector : public BaseHttpConnector<HttpConnector>
{
public:
	HttpConnector(BaseNetIo<NetIo>& netio)
		:BaseHttpConnector(netio) {

	}
};

// 同步connector
class SyncConnector {
public:
	SyncConnector();

	~SyncConnector();

	bool Connect(const SocketLib::Tcp::EndPoint& ep, SocketLib::s_uint32_t timeo_sec = -1);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::s_uint32_t timeo_sec = -1);

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// 调用这个函数不意味着连接立即断开，会等所有的未处理的数据处理完就会断开
	void Close();

	bool Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len);

	bool IsConnected()const;

	SocketLib::Buffer* Recv();

protected:
	SocketLib::s_uint32_t _LocalEndian()const;

	SocketLib::Buffer* _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t& tran_byte);

protected:
	SocketLib::IoService _ioservice;
	SocketLib::TcpConnector<SocketLib::IoService>* _socket;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	MessageHeader		  _curheader;
	SocketLib::s_byte_t*  _readbuf;
	SocketLib::s_uint32_t _readsize;

	SocketLib::Buffer _sndbuffer;
	SocketLib::Buffer _rcvbuffer;
	// 状态标志
	unsigned short _flag;
};

M_NETIO_NAMESPACE_END
#include "netio/netio_impl.hpp"
#include "netio/tsocket_impl.hpp"
#include "netio/tconnector_impl.hpp"
#include "netio/hsocket_impl.hpp"
#include "netio/hconnector_impl.hpp"
#endif
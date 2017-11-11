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
	unsigned short checksum;
	unsigned short size;
	int			   timestamp;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#pragma pack()
#else
struct __attribute__((__packed__)) MessageHeader {
	unsigned short checksum;
	unsigned short size;
	int			   timestamp;
	unsigned char  endian; // 0 == little endian, 1 == big endian
};
#endif

class NetIo;
class TcpSocket;

typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > NetIoTcpAcceptorPtr;

typedef function_t<void(TcpSocketPtr, BufferPtr)> MessageReceiver;
typedef function_t<bool(TcpSocketPtr, const MessageHeader&)> MessageHeaderChecker;

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

	void Run();

	void Stop();

	// 获取最后的异常
	inline SocketLib::SocketError GetLastError()const;

	inline SocketLib::IoService& GetIoService();

	/*
	 *以下三个函数定义为虚函数，以便根据实际业务的模式来做具体模式的消息包分发处理。
	 *保证同一个socket，以下三个函数的调用遵循OnConnected -> OnReceiveData -> OnDisconnected的顺序。
	 *保证同一个socket，以下后两个函数的调用都在同一个线程中
	 */

	// 连线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnConnected(TcpSocketPtr clisock);

	// 掉线通知,这个函数里不要处理业务，防止堵塞
	virtual void OnDisconnected(TcpSocketPtr clisock);

	// 数据包通知,这个函数里不要处理业务，防止堵塞
	virtual void OnReceiveData(TcpSocketPtr clisock, BufferPtr buffer);

protected:
	void AcceptHandler(SocketLib::SocketError error, TcpSocketPtr clisock, NetIoTcpAcceptorPtr acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

private:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;

	std::list<TcpSocketPtr> _connected_list;
	std::list<TcpSocketPtr> _disconnected_list; // 断线
};

// class tcpsocket
class TcpSocket : public enable_shared_from_this_t<TcpSocket>
{
public:
	struct _readerinfo_ {
		SocketLib::s_byte_t* readbuf;
		SocketLib::Buffer	 msgbuffer;
		SocketLib::MutexLock lock;

		_readerinfo_();
		~_readerinfo_();
	};
	struct _writerinfo_ {
		std::list<SocketLib::Buffer*> buffer_pool;
		SocketLib::MutexLock lock;
		bool writing;

		_writerinfo_();
	};

public:
	TcpSocket(NetIo& netio, MessageReceiver receiver, MessageHeaderChecker checker);

	~TcpSocket();

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket();

	void Init();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// 调用这个函数不意味着连接立即断开，会等所有的未处理的数据处理完就会断开
	void PostClose();

	void Send(SocketLib::Buffer*);

protected:
	void WriteHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error);

	void ReadHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error);

	void Close();

	// 裁减出数据包，返回false意味着数据包有错
	bool CutMsgPack();

	bool TrySendData();

private:
	NetIo& _netio;
	SocketLib::TcpSocket<SocketLib::IoService>* _socket;

	MessageReceiver		 _message_receiver;
	MessageHeaderChecker _header_checker;

	_readerinfo_ _reader;
	_writerinfo_ _writer;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	bool _stopped; // stop flag
};

M_NETIO_NAMESPACE_END
#include "netio_impl.hpp"
#include "tsocket_impl.hpp"
#endif
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

	inline SocketLib::SocketError GetLastError()const;

	inline SocketLib::IoService& GetIoService();

protected:
	virtual void AcceptHandler(SocketLib::SocketError error, TcpSocketPtr clisock, NetIoTcpAcceptorPtr acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

private:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;
	SocketLib::SocketError _lasterror;
};

// class tcpsocket
class TcpSocket : public enable_shared_from_this_t<TcpSocket>
{
public:
	TcpSocket(NetIo& netio, MessageReceiver receiver, MessageHeaderChecker checker);

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket();

protected:
	void WriteHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error);

	void ReadHandler(SocketLib::s_uint32_t tran_byte, const SocketLib::SocketError& error);

private:
	NetIo& _netio;
	SocketLib::TcpSocket<SocketLib::IoService>* _socket;

	MessageReceiver		 _message_receiver;
	MessageHeaderChecker _header_checker;

	// lock
	SocketLib::MutexLock _write_mutex;
	SocketLib::MutexLock _read_mutex;

	// writer buffer
	std::list<BufferPtr> _wait_write;
	BufferPtr			 _writing_buffer;

	// reader buffer
	BufferPtr			 _reading_buffer;
};

M_NETIO_NAMESPACE_END
#include "netio_impl.hpp"
#include "tsocket_impl.hpp"
#endif
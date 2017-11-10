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

#ifndef M_NETIO_TCP_SOCKET_INCLUDE
#define M_NETIO_TCP_SOCKET_INCLUDE

#include "config.hpp"
M_NETIO_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#pragma pack(1)
struct MessageHeader {
	unsigned short checksum;
	unsigned short size;
	int			   timestamp;
	SocketLib::s_uint8_t endian; // 0 == little endian, 1 == big endian
};
#pragma pack()
#else
struct __attribute__((__packed__)) MessageHeader {
	unsigned short checksum;
	unsigned short size;
	int			   timestamp;  // 0 == little endian, 1 == big endian
};
#endif

class NetIo;
class TcpSocket;

typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;

typedef function_t<void(TcpSocketPtr, BufferPtr)> MessageReceiver;
typedef function_t<bool(TcpSocketPtr, const MessageHeader&)> MessageHeaderChecker;

class TcpSocket : public enable_shared_from_this_t<TcpSocket>
{
public:
	TcpSocket(NetIo& netio,MessageReceiver receiver,MessageHeaderChecker checker);

protected:
	void WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	void ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

private:
	NetIo& _netio;
	SocketLib::TcpSocket<SocketLib::IoService>* _socket;

	MessageReceiver		 _message_receiver;
	MessageHeaderChecker _header_checker;

	// lock
	SocketLib::MutexLock _write_mutex;
	SocketLib::MutexLock _read_mutex;
};

TcpSocket::TcpSocket(NetIo& netio, MessageReceiver receiver, MessageHeaderChecker checker)
	:_netio(netio), _message_receiver(receiver), _header_checker(checker) {
	//_socket = new SocketLib::TcpSocket<SocketLib::IoService>(_netio.GetIoService());
}

void TcpSocket::WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error) {

}

void TcpSocket::ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error) {

}


M_NETIO_NAMESPACE_END
#endif
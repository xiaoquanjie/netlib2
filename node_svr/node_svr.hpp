#ifndef M_TRANSFER_INCLUDE
#define M_TRANSFER_INCLUDE

#include "node_svr/config.hpp"
#include <netio/netio.hpp>
M_NODESVR_NAMESPACE_BEGIN

#define message_id_define(msg,id) \
enum message_id_##id{ \
	msg = id,\
};\

message_id_define(msg_id_keepalive, 1);
message_id_define(msg_id_keepalive_ack, 2);
message_id_define(msg_id_transfer, 3);

struct protocol_base {
	base::s_int32_t msgid;
	protocol_base(base::s_int32_t id)
		:msgid(id) {
	}
	virtual void Read(base::Buffer& buffer) {
		buffer.Read(msgid);
	}
	virtual void Write(base::Buffer& buffer) {
		buffer.Write(msgid);
	}
};

struct protocol_keepalive :
	public protocol_base
{
	protocol_keepalive()
		:protocol_base(msg_id_keepalive) {
	}
	virtual void Read(base::Buffer& buffer) {
	}
	virtual void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
	}
};

struct protocol_keepalive_ack :
	public protocol_base
{
	protocol_keepalive_ack()
		:protocol_base(msg_id_keepalive_ack) {
	}
	virtual void Read(base::Buffer& buffer) {
	}
	virtual void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
	}
};

struct protocol_transfer :
	public protocol_base
{
	enum {
		enum_login_type = 1,
		enum_logout_type = 2,
		enum_signle_point_type = 3,
		enum_type_broadcast_type = 4,
		enum_all_broadcast_type = 5,
	};
	base::s_int32_t msgtype;
	base::s_int32_t from_token_type;
	base::s_int32_t from_token_id;
	base::s_int32_t to_token_type;
	base::s_int32_t to_token_id;
	protocol_transfer()
		:protocol_base(msg_id_transfer) {
		msgtype = 0;
		from_token_type = 0;
		from_token_id = 0;
		to_token_type = 0;
		to_token_id = 0;
	}
	void Read(base::Buffer& buffer) {
		buffer.Read(msgtype);
		buffer.Read(from_token_type);
		buffer.Read(from_token_id);
		buffer.Read(to_token_type);
		buffer.Read(to_token_id);
	}
	void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
		buffer.Write(msgtype);
		buffer.Write(from_token_type);
		buffer.Write(from_token_id);
		buffer.Write(to_token_type);
		buffer.Write(to_token_id);
	}
};

struct protocol_hdr {
	enum {
		enum_login_type = 1,
		enum_logout_type = 2,
		enum_signle_point_type = 3,
		enum_type_broadcast_type = 4,
		enum_all_broadcast_type = 5,
	};
	base::s_int32_t msgtype;
	base::s_int32_t from_token_type;
	base::s_int32_t from_token_id;
	base::s_int32_t to_token_type;
	base::s_int32_t to_token_id;

	protocol_hdr() {
		msgtype = 0;
		from_token_type = 0;
		from_token_id = 0;
		to_token_type = 0;
		to_token_id = 0;
	}

	void Read(base::Buffer& buffer) {
		buffer.Read(msgtype);
		buffer.Read(from_token_type);
		buffer.Read(from_token_id);
		buffer.Read(to_token_type);
		buffer.Read(to_token_id);
	}

	void Write(base::Buffer& buffer) {
		buffer.Write(msgtype);
		buffer.Write(from_token_type);
		buffer.Write(from_token_id);
		buffer.Write(to_token_type);
		buffer.Write(to_token_id);
	}
};

class ClientNodeSvr;

template<typename ClientNodeType>
class NodeSvrNetIo :
	public netiolib::NetIo
{
public:
	NodeSvrNetIo(ClientNodeType* node)
		:_node(node) {
	}

protected:
	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
		if (!error) {
			_beat.OnConnected(clisock, 10);
		}
		_node->OnConnected(clisock, error);
	}

	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
		_node->OnDisconnected(clisock);
	}

	virtual void OnReceiveData(netiolib::TcpConnectorPtr& clisock, SocketLib::Buffer& buffer) {
		_beat.OnReceiveData(clisock);

		protocol_hdr message;
		message.Read(buffer);
		if (message.msgtype
			== protocol_hdr::enum_login_type) {
			_node->OnLoginMsg(buffer, message.to_token_type, message.to_token_id,
				message.from_token_type, message.from_token_id);
		}
		else if (protocol_hdr::enum_logout_type
			== message.msgtype) {
			_node->OnLogoutMsg(message.to_token_type, message.to_token_id,
				message.from_token_type, message.from_token_id);
		}
		else if (protocol_hdr::enum_signle_point_type
			== message.msgtype) {
			_node->OnMessage(buffer, message.to_token_type, message.to_token_id,
				message.from_token_type, message.from_token_id);
		}
		else if (protocol_hdr::enum_type_broadcast_type
			== message.msgtype) {
			_node->OnMessage(buffer, message.to_token_type, message.to_token_id,
				message.from_token_type, message.from_token_id);
		}
		else if (protocol_hdr::enum_all_broadcast_type
			== message.msgtype) {
			_node->OnMessage(buffer, message.to_token_type, message.to_token_id,
				message.from_token_type, message.from_token_id);
		}
	}

	netiolib::HeartBeatMng& GetBeatMng() {
		return _beat;
	}

protected:
	netiolib::HeartBeatMng _beat;
	ClientNodeType* _node;
};

class ClientNodeSvr
{
	friend class NodeSvrNetIo<ClientNodeSvr>;

public:
	ClientNodeSvr()
		:_netio(this) {
	}

	virtual void Start(int thread_cnt) {
		_netio.Start(thread_cnt);
		//_netio.get
	}

	virtual void Stop() {
		_netio.Stop();
	}

	void ConnectOne(const std::string& addr, SocketLib::s_uint16_t port) {
		_netio.ConnectOne(addr, port);
	}

	template<typename MessageType>
	void SendLoginMsg(netiolib::TcpConnectorPtr& clisock, int self_token_type,
		int self_token_id, MessageType& message) {
		base::Buffer* buffer = MakeProtocol(protocol_hdr::enum_login_type,
			self_token_type, self_token_id, 0, 0);
		clisock->Send(buffer);
	}

	template<typename MessageType>
	void SendSglPointMsg(netiolib::TcpConnectorPtr& clisock, int self_token_type,
		int self_token_id, int other_token_type, int other_token_id,
		MessageType& message) {
		base::Buffer* buffer = MakeProtocol(protocol_hdr::enum_signle_point_type,
			self_token_type, self_token_id, other_token_type, other_token_id);
		clisock->Send(buffer);
	}

	template<typename MessageType>
	void SendTypeBroadcastMsg(netiolib::TcpConnectorPtr& clisock, int self_token_type,
		int self_token_id, MessageType& message) {
		base::Buffer* buffer = MakeProtocol(protocol_hdr::enum_type_broadcast_type,
			self_token_type, self_token_id, 0, 0, message);
		clisock->Send(buffer);
	}

	template<typename MessageType>
	void SendAllBroadcastMsg(netiolib::TcpConnectorPtr& clisock, int self_token_type,
		int self_token_id, MessageType& message) {
		base::Buffer* buffer = MakeProtocol(protocol_hdr::enum_all_broadcast_type,
			self_token_type, self_token_id, 0, 0, message);
		clisock->Send(buffer);
	}

protected:

	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) = 0;

	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) = 0;

	virtual void OnLoginMsg(netiolib::Buffer& buffer, int self_token_type,
		int self_token_id, int from_token_type, int from_token_id) = 0;

	virtual void OnLogoutMsg(int self_token_type, int self_token_id, int from_token_type,
		int from_token_id) = 0;

	virtual void OnMessage(netiolib::Buffer& buffer, int self_token_type,
		int self_token_id, int from_token_type, int from_token_id) = 0;

	base::Buffer* MakeProtocol(base::s_int32_t type, int self_token_type,
		int self_token_id, int other_token_type, int other_token_id) {
		protocol_hdr snd_msg;
		snd_msg.msgtype = type;
		snd_msg.from_token_type = self_token_type;
		snd_msg.from_token_id = self_token_id;
		snd_msg.to_token_type = 0;
		snd_msg.to_token_id = 0;
		base::Buffer* buffer = new base::Buffer;
		snd_msg.Write(*buffer);
		return buffer;
	}

	template<typename MessageType>
	base::Buffer* MakeProtocol(base::s_int32_t type, int self_token_type,
		int self_token_id, int other_token_type, int other_token_id,
		MessageType& message) {
		protocol_hdr snd_msg;
		snd_msg.msgtype = type;
		snd_msg.from_token_type = self_token_type;
		snd_msg.from_token_id = self_token_id;
		snd_msg.to_token_type = 0;
		snd_msg.to_token_id = 0;
		base::Buffer* buffer = new base::Buffer;
		snd_msg.Write(*buffer);
		message.Write(*buffer);
		return buffer;
	}

protected:
	NodeSvrNetIo<ClientNodeSvr> _netio;
};

M_NODESVR_NAMESPACE_END
#endif
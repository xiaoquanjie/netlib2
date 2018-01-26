#ifndef M_TRANSFER_INCLUDE
#define M_TRANSFER_INCLUDE

#include "transfer/config.hpp"
#include "base/buffer.hpp"
M_TRANSFER_NAMESPACE_BEGIN

#define M_ALL_TARGET_ID (-1)

#define enum_msgid_define(msg,id) \
enum enum_msgid_##id{\
	msg = id,\
};

// 连接
enum_msgid_define(msgid_login,1)
// 掉线
enum_msgid_define(msgid_logout,2)
// 单点发送
enum_msgid_define(msgid_single_point,3)
// 同类节点广播
enum_msgid_define(msgid_type_broadcast,4)
// 全节点广播
enum_msgid_define(msgid_all_broadcast,5)

struct protocol_base {
	base::s_int32_t msgid;
	protocol_base(base::s_int32_t mid)
		:msgid() {
	}
	virtual void Read(base::Buffer& buffer) {
		buffer.Read(msgid);
	}
	virtual void Write(base::Buffer& buffer) {
		buffer.Write(msgid);
	}
};

struct protocol_login  : 
	public protocol_base
{
	base::s_int32_t type;
	base::s_int32_t id;
	protocol_login() 
		:protocol_base(msgid_login){
		type = 0;
		id = 0;
	}
	virtual void Read(base::Buffer& buffer) {
		buffer.Read(type);
		buffer.Read(id);
	}
	virtual void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
		buffer.Write(type);
		buffer.Write(id);
	}
};

struct protocol_logout :
	public protocol_base
{
	base::s_int32_t type;
	base::s_int32_t id;
	protocol_logout()
		:protocol_base(msgid_logout){
		type = 0;
		id = 0;
	}
	virtual void Read(base::Buffer& buffer) {
		buffer.Read(type);
		buffer.Read(id);
	}
	virtual void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
		buffer.Write(type);
		buffer.Write(id);
	}
};

struct protocol_single_point :
	public protocol_base 
{
	base::s_int32_t type;
	base::s_int32_t id;
	protocol_single_point()
		:protocol_base(msgid_single_point) {
		type = 0;
		id = 0;
	}
	void Read(base::Buffer& buffer) {
		buffer.Read(type);
		buffer.Read(id);
	}
	void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
		buffer.Write(type);
		buffer.Write(id);
	}
};

struct protocol_type_broadcast :
	public protocol_base
{
	base::s_int32_t type;
	protocol_type_broadcast()
		:protocol_base(msgid_type_broadcast) {
		type = 0;
	}
	void Read(base::Buffer& buffer) {
		buffer.Read(type);
	}
	void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
		buffer.Write(type);
	}
};

struct protocol_all_broadcast :
	public protocol_base
{
	protocol_all_broadcast()
		:protocol_base(msgid_all_broadcast) {
	}
	void Read(base::Buffer& buffer) {
	}
	void Write(base::Buffer& buffer) {
		protocol_base::Write(buffer);
	}
};

// 协议构造
struct ProtocolMaker {
	static base::Buffer* MakeLoginMsg(base::s_int32_t type,
		base::s_int32_t id) {
		base::Buffer* buffer = new base::Buffer;
		protocol_login login_msg;
		login_msg.type = type;
		login_msg.id = id;
		login_msg.Write(*buffer);
		return buffer;
	}

	static base::Buffer* MakeLogoutMsg(base::s_int32_t type,
		base::s_int32_t id) {
		base::Buffer* buffer = new base::Buffer;
		protocol_logout logout_msg;
		logout_msg.type = type;
		logout_msg.id = id;
		logout_msg.Write(*buffer);
		return buffer;
	}

	static base::Buffer* MakeSinglePointMsg(base::s_int32_t type,
		base::s_int32_t id, protocol_base*msg) {
		base::Buffer* buffer = new base::Buffer;
		protocol_single_point point_msg;
		point_msg.type = type;
		point_msg.id = id;
		point_msg.Write(*buffer);
		msg->Write(*buffer);
		return buffer;
	}

	static base::Buffer* MakeTypeBroadcastMsg(base::s_int32_t type,
		protocol_base* msg) {
		base::Buffer* buffer = new base::Buffer;
		protocol_type_broadcast bro_msg;
		bro_msg.type = type;
		bro_msg.Write(*buffer);
		msg->Write(*buffer);
		return buffer;
	}

	static base::Buffer* MakeAllBroadcastMsg(protocol_base* msg) {
		base::Buffer* buffer = new base::Buffer;
		protocol_all_broadcast bro_msg;
		bro_msg.Write(*buffer);
		msg->Write(*buffer);
		return buffer;
	}
};


M_TRANSFER_NAMESPACE_END
#endif
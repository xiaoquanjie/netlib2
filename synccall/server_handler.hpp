#ifndef M_SYNCCALL_SERVER_HANDLER_INCLUDE
#define M_SYNCCALL_SERVER_HANDLER_INCLUDE

#include "synccall/config.hpp"
M_SYNCCALL_NAMESPACE_BEGIN

// handler interface
struct IServerHandler {
	virtual void OnOneWayDealer(const int msg_type, netiolib::Buffer& request) = 0;
	virtual void OnTwoWayDealer(const int msg_type, netiolib::Buffer& request, netiolib::Buffer& reply) = 0;
};


M_SYNCCALL_NAMESPACE_END
#endif
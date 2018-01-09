#ifndef M_SYNCCALL_CONFIG_INCLUDE
#define M_SYNCCALL_CONFIG_INCLUDE

#define M_SYNCCALL_NAMESPACE_BEGIN namespace synccall {
#define M_SYNCCALL_NAMESPACE_END }

#include "base/config.hpp"
#include "netio/netio.hpp"
#include <string>

#define M_ONEWAY_TYPE (666)
#define M_TWOWAY_TYPE (999)

// for debug
//#define M_OPEN_DEBUG_LOG 1

M_SYNCCALL_NAMESPACE_BEGIN
struct _ScInfo_ {
	base::s_uint16_t id;
	netiolib::Buffer buffer;
};
struct _CoScInfo_ {
	bool valid;
	base::s_uint32_t thr_id;
	base::s_uint32_t co_id;
	netiolib::Buffer buffer;
	SocketLib::MutexLock mutex;
};

inline void free_coscinfo(void* data) {
	delete ((_CoScInfo_*)data);
}

M_SYNCCALL_NAMESPACE_END

#endif
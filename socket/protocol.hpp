/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/8/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_PROTOCOL_INCLUDE
#define M_PROTOCOL_INCLUDE

#include "socket/endpoint.hpp"
M_SOCKET_NAMESPACE_BEGIN

class Tcp
{
public:
	typedef BasicEndPoint<Tcp> EndPoint;

	static Tcp V4()
	{
		return Tcp(M_AF_INET);
	}

	static Tcp V6()
	{
		return Tcp(M_AF_INET6);
	}

	s_int32_t Type()const
	{
		return M_SOCK_STREAM;
	}

	s_int32_t Protocol()const
	{
		return M_IPPROTO_TCP;
	}

	s_int32_t Family()const
	{
		return _family;
	}

	friend bool operator==(const Tcp& t1, const Tcp& t2)
	{
		return (t1._family == t2._family);
	}

	friend bool operator!=(const Tcp& t1, const Tcp& t2)
	{
		return (t1._family != t2._family);
	}

protected:
	Tcp(s_int32_t family) :_family(family) {}

	s_int32_t _family;
};

class Udp
{
public:
	typedef BasicEndPoint<Udp> EndPoint;

	static Udp V4()
	{
		return Udp(M_AF_INET);
	}

	static Udp V6()
	{
		return Udp(M_AF_INET6);
	}

	s_int32_t Type()const
	{
		return M_SOCK_DGRAM;
	}

	s_int32_t Protocol()const
	{
		return M_IPPROTO_UDP;
	}

	s_int32_t Family()const
	{
		return _family;
	}

	friend bool operator==(const Udp& t1, const Udp& t2)
	{
		return (t1._family == t2._family);
	}

	friend bool operator!=(const Udp& t1, const Udp& t2)
	{
		return (t1._family != t2._family);
	}

protected:
	Udp(s_int32_t family) :_family(family) {}

	s_int32_t _family;
};

M_SOCKET_NAMESPACE_END
#endif
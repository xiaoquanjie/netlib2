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

#ifndef M_ADDRESS_INCLUDE
#define M_ADDRESS_INCLUDE

#include "socket/config.hpp"
M_SOCKET_NAMESPACE_BEGIN

struct in4_addr_type
{
	s_uint32_t _addr;
};

struct in6_addr_type 
{ 
	in6_addr _addr;
};

struct AddressV4
{
	static bool IsV4()
	{
		return true;
	}

	static bool IsV6()
	{
		return false;
	}

	std::string Address()const
	{
		if (_addr._addr != 0)
		{
			char addr_ary[128] = { 0 };
			g_inet_ntop(M_AF_INET, (void*)&_addr, addr_ary, sizeof(addr_ary));
			return std::string(addr_ary);
		}
		return std::string("");
	}

	/// Get the address in bytes, in network byte order
	in4_addr_type ToBytes()const
	{
		return _addr;
	}

	/// Get the address as an unsigned long in host byte order
	s_uint32_t ToULong()const
	{
		return g_ntohl(_addr._addr);
	}

	/// Determine whether the address is a loopback address.
	bool IsLoopback() const
	{
		return (ToULong() & 0xFF000000) == 0x7F000000;
	}

	/// Determine whether the address is unspecified.
	bool IsUnspecified() const
	{
		return (ToULong() == 0);
	}

	/// Determine whether the address is a class A address.
	bool IsClassA() const
	{
		return (ToULong() & 0x80000000) == 0;
	}

	/// Determine whether the address is a class B address.
	bool IsClassB() const
	{
		return (ToULong() & 0xC0000000) == 0x80000000;
	}

	/// Determine whether the address is a class C address.
	bool IsClassC() const
	{
		return (ToULong() & 0xE0000000) == 0xC0000000;
	}

	/// Determine whether the address is a multicast address.
	bool IsMulticast() const
	{
		return (ToULong() & 0xF0000000) == 0xE0000000;
	}

	AddressV4(in4_addr_type addr):_addr(addr)
	{}

	AddressV4(s_uint32_t addr)
	{
		_addr._addr = g_htonl(addr);
	}

	explicit AddressV4(const char* addr){
		g_inet_pton(M_AF_INET, addr, (void*)&_addr);
	}

	AddressV4(const std::string& addr){
		g_inet_pton(M_AF_INET, addr.c_str(), (void*)&_addr);
	}

	AddressV4(const AddressV4& other):_addr(other._addr)
	{}

	AddressV4& operator=(const AddressV4& other){
		_addr = other._addr;
		return *this;
	}

	friend bool operator==(const AddressV4& i1, const AddressV4& i2)
	{
		return (i1._addr._addr == i2._addr._addr);
	}

	friend bool operator!=(const AddressV4& i1, const AddressV4& i2)
	{
		return (i1._addr._addr != i2._addr._addr);
	}

protected:
	in4_addr_type _addr;
};

struct AddressV6
{
	static bool IsV4()
	{
		return false;
	}

	static bool IsV6()
	{
		return true;
	}

	std::string Address()const
	{
		char addr_ary[128] = { 0 };
		g_inet_ntop(M_AF_INET6, (void*)&_addr, addr_ary, sizeof(addr_ary));
		return std::string(addr_ary);
	}

	/// Get the address in bytes, in network byte order
	in6_addr_type ToBytes()const
	{
		return _addr;
	}

	/// Determine whether the address is a loopback address.
	bool IsLoopback() const
	{
		return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
			&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
			&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
			&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
			&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
			&& (_addr._addr.s6_addr[10] == 0) && (_addr._addr.s6_addr[11] == 0)
			&& (_addr._addr.s6_addr[12] == 0) && (_addr._addr.s6_addr[13] == 0)
			&& (_addr._addr.s6_addr[14] == 0) && (_addr._addr.s6_addr[15] == 1));
	}

	/// Determine whether the address is unspecified.
	bool IsUnspecified() const
	{
		return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
			&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
			&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
			&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
			&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
			&& (_addr._addr.s6_addr[10] == 0) && (_addr._addr.s6_addr[11] == 0)
			&& (_addr._addr.s6_addr[12] == 0) && (_addr._addr.s6_addr[13] == 0)
			&& (_addr._addr.s6_addr[14] == 0) && (_addr._addr.s6_addr[15] == 0));
	}

	/// Determine whether the address is a mapped IPv4 address.
	bool IsV4Mapped() const
	{
		return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
			&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
			&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
			&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
			&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
			&& (_addr._addr.s6_addr[10] == 0xff) && (_addr._addr.s6_addr[11] == 0xff));
	}

	/// Determine whether the address is a multicast address.
	bool IsMulticast() const
	{
		return (_addr._addr.s6_addr[0] == 0xff);
	}

	/// Determine whether the address is an IPv4-compatible address.
	bool IsV4Compatible() const
	{
		return ((_addr._addr.s6_addr[0] == 0) && (_addr._addr.s6_addr[1] == 0)
			&& (_addr._addr.s6_addr[2] == 0) && (_addr._addr.s6_addr[3] == 0)
			&& (_addr._addr.s6_addr[4] == 0) && (_addr._addr.s6_addr[5] == 0)
			&& (_addr._addr.s6_addr[6] == 0) && (_addr._addr.s6_addr[7] == 0)
			&& (_addr._addr.s6_addr[8] == 0) && (_addr._addr.s6_addr[9] == 0)
			&& (_addr._addr.s6_addr[10] == 0) && (_addr._addr.s6_addr[11] == 0)
			&& !((_addr._addr.s6_addr[12] == 0)
				&& (_addr._addr.s6_addr[13] == 0)
				&& (_addr._addr.s6_addr[14] == 0)
				&& ((_addr._addr.s6_addr[15] == 0) || (_addr._addr.s6_addr[15] == 1))));
	}

	/// Determine whether the address is a global multicast address.
	bool IsMulticastGlobal() const
	{
		return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x0e));
	}

	/// Determine whether the address is a link-local multicast address.
	bool IsMulticastLinkLocal() const
	{
		return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x02));
	}

	/// Determine whether the address is a node-local multicast address.
	bool IsMulticastNodeLocal() const
	{
		return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x01));
	}

	/// Determine whether the address is a org-local multicast address.
	bool IsMulticastOrgLocal() const
	{
		return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x08));
	}

	/// Determine whether the address is a site-local multicast address.
	bool IsMulticastSiteLocal() const
	{
		return ((_addr._addr.s6_addr[0] == 0xff) && ((_addr._addr.s6_addr[1] & 0x0f) == 0x05));
	}

	AddressV6(in6_addr_type addr):_addr(addr)
	{}

	explicit AddressV6(const char* addr){
		g_inet_pton(M_AF_INET6, addr, (void*)&_addr);
	}

	AddressV6(const std::string& addr){
		g_inet_pton(M_AF_INET6, addr.c_str(), (void*)&_addr);
	}

	AddressV6(const AddressV6& other) :_addr(other._addr)
	{}

	AddressV6& operator=(const AddressV6& other)
	{
		_addr = other._addr;
		return *this;
	}

	friend bool operator==(const AddressV6& i1, const AddressV6& i2)
	{
		return g_memcpy((void*)&i1, (void*)&i2, sizeof(in6_addr_type)) == 0;
	}

	friend bool operator!=(const AddressV6& i1, const AddressV6& i2)
	{
		return !(i1 == i2);
	}
protected:
	in6_addr_type _addr;
};


M_SOCKET_NAMESPACE_END
#endif
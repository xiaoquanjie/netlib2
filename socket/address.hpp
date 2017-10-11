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

#include "config.hpp"
M_SOCKET_NAMESPACE_BEGIN

struct in4_addr_type
{
	s_uint32_t _addr;
};

struct in6_addr_type 
{ 
	s_uint8_t _addr[16];
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

	explicit AddressV4(const char* addr)
	{
		if (g_inet_pton(M_AF_INET, addr, (void*)&_addr) == NULL)
			_addr._addr = 0;
	}

	AddressV4(const std::string& addr)
	{
		if (g_inet_pton(M_AF_INET, addr.c_str(), (void*)&_addr) == NULL)
			_addr._addr = 0;
	}

	AddressV4(const AddressV4& other):_addr(other._addr)
	{}

	AddressV4& operator=(const AddressV4& other)
	{
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
	//std::string _addr;
	in4_addr_type _addr;
};

struct AddressV6
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
		return _addr;
	}

	explicit AddressV6(const char* addr) :_addr(addr)
	{}

	AddressV6(const std::string& addr) :_addr(addr)
	{}

	AddressV6(const AddressV6& other) :_addr(other._addr)
	{}

	AddressV6& operator=(const AddressV6& other)
	{
		_addr = other._addr;
		return *this;
	}

	friend bool operator==(const AddressV6& i1, const AddressV6& i2)
	{
		return (i1._addr == i2._addr);
	}

	friend bool operator!=(const AddressV6& i1, const AddressV6& i2)
	{
		return (i1._addr != i2._addr);
	}
protected:
	std::string _addr;
};


M_SOCKET_NAMESPACE_END
#endif
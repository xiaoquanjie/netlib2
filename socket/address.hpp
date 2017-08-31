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
		return _addr;
	}

	explicit AddressV4(const char* addr):_addr(addr)
	{}

	AddressV4(const std::string& addr):_addr(addr)
	{}

	AddressV4(const AddressV4& other):_addr(other._addr)
	{}

	AddressV4& operator=(const AddressV4& other)
	{
		_addr = other._addr;
		return *this;
	}

	friend bool operator==(const AddressV4& i1, const AddressV4& i2)
	{
		return (i1._addr == i2._addr);
	}

	friend bool operator!=(const AddressV4& i1, const AddressV4& i2)
	{
		return (i1._addr != i2._addr);
	}

protected:
	std::string _addr;
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
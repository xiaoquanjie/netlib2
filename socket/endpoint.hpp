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

#ifndef M_ENDPOINT_INCLUDE
#define M_ENDPOINT_INCLUDE

#include "config.hpp"
#include "util.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename ProtocolType>
class BasicEndPoint
{
public:
	struct Impl;

	template<typename AddressType>
	M_SOCKET_DECL BasicEndPoint(const AddressType& addr, s_uint16_t port);

	M_SOCKET_DECL BasicEndPoint(const BasicEndPoint& ep);

	M_SOCKET_DECL BasicEndPoint();

	M_SOCKET_DECL BasicEndPoint(const Impl& impl);

	M_SOCKET_DECL BasicEndPoint& operator=(const BasicEndPoint& ep);

	M_SOCKET_DECL BasicEndPoint& operator=(const Impl& impl);

	M_SOCKET_DECL const Impl& getImpl()const;

	M_SOCKET_DECL Impl& getImpl();

	M_SOCKET_DECL bool IsV4()const;

	M_SOCKET_DECL bool IsV6()const;

	M_SOCKET_DECL ProtocolType Protocol()const;

	M_SOCKET_DECL s_uint16_t Port()const;

	M_SOCKET_DECL std::string Address()const;

	M_SOCKET_DECL bool Valid()const;

	friend bool operator==(const BasicEndPoint& ep1, const BasicEndPoint& ep2)
	{
		return (ep1._impl == ep2._impl);
	}

	friend bool operator!=(const BasicEndPoint& ep1, const BasicEndPoint& ep2)
	{
		return (ep1._impl != ep2._impl);
	}

protected:
	Impl _impl;
};

template<typename ProtocolType>
struct BasicEndPoint<ProtocolType>::Impl
{
	class endpoint_impl_access;

	template<typename AddressType>
	Impl(const AddressType& addr, s_uint16_t port):_pSockAddr(0)
	{
		endpoint_impl_access::Construct(*this, addr, port);
	}

	Impl(const Impl& other) :_pSockAddr(0)
	{
		endpoint_impl_access::Construct(*this, other);
	}

	Impl():_pSockAddr(0){}

	~Impl()
	{
		endpoint_impl_access::Destroy(*this);
	}

	Impl& operator=(const Impl& other)
	{
		if (_pSockAddr)
			endpoint_impl_access::Copy(*this, other);
		else
			endpoint_impl_access::Construct(*this, other);
		return *this;
	}

	friend bool operator==(const Impl& impl1, const Impl& impl2)
	{
		if (!impl1._pSockAddr || !impl2._pSockAddr)
			return false;

		if (impl1._pSockAddr == impl2._pSockAddr)
			return true;

		return (g_memcmp(impl1._pSockAddr, impl2._pSockAddr, detail::Util::SockAddrLen()) == 0);
	}

	friend bool operator!=(const Impl& imp1, const Impl& impl2)
	{
		return !(imp1 == impl2);
	}

protected:
	sockaddr_t* _pSockAddr;
};

template<typename ProtocolType>
template<typename AddressType>
M_SOCKET_DECL BasicEndPoint<ProtocolType>::BasicEndPoint(const AddressType& addr, s_uint16_t port):_impl(addr,port)
{}

template<typename ProtocolType>
M_SOCKET_DECL BasicEndPoint<ProtocolType>::BasicEndPoint(const BasicEndPoint& ep):_impl(ep._impl)
{}

template<typename ProtocolType>
M_SOCKET_DECL BasicEndPoint<ProtocolType>::BasicEndPoint()
{}

template<typename ProtocolType>
M_SOCKET_DECL BasicEndPoint<ProtocolType>::BasicEndPoint(const Impl& impl):_impl(impl)
{}

template<typename ProtocolType>
M_SOCKET_DECL BasicEndPoint<ProtocolType>& BasicEndPoint<ProtocolType>::operator=(const BasicEndPoint& ep)
{
	_impl = ep._impl;
	return *this;
}

template<typename ProtocolType>
M_SOCKET_DECL BasicEndPoint<ProtocolType>& BasicEndPoint<ProtocolType>::operator=(const Impl& impl)
{
	_impl = impl;
	return *this;
}

template<typename ProtocolType>
M_SOCKET_DECL const typename BasicEndPoint<ProtocolType>::Impl& BasicEndPoint<ProtocolType>::getImpl()const
{
	return _impl;
}

template<typename ProtocolType>
M_SOCKET_DECL typename BasicEndPoint<ProtocolType>::Impl& BasicEndPoint<ProtocolType>::getImpl()
{
	return _impl;
}

template<typename ProtocolType>
M_SOCKET_DECL bool BasicEndPoint<ProtocolType>::IsV4()const
{
	return (Impl::endpoint_impl_access::Family(_impl) == M_AF_INET);
}

template<typename ProtocolType>
M_SOCKET_DECL bool BasicEndPoint<ProtocolType>::IsV6()const
{
	return (Impl::endpoint_impl_access::Family(_impl) == M_AF_INET6);
}

template<typename ProtocolType>
M_SOCKET_DECL ProtocolType BasicEndPoint<ProtocolType>::Protocol()const
{
	if (IsV4())
		return ProtocolType::V4();

	return ProtocolType::V6();
}

template<typename ProtocolType>
M_SOCKET_DECL s_uint16_t BasicEndPoint<ProtocolType>::Port()const
{
	return Impl::endpoint_impl_access::Port(_impl);
}

template<typename ProtocolType>
M_SOCKET_DECL std::string BasicEndPoint<ProtocolType>::Address()const
{
	return Impl::endpoint_impl_access::Address(_impl);
}

template<typename ProtocolType>
M_SOCKET_DECL bool BasicEndPoint<ProtocolType>::Valid()const
{
	return Impl::endpoint_impl_access::Valid(_impl);
}

template<typename ProtocolType>
class BasicEndPoint<ProtocolType>::Impl::endpoint_impl_access
{
public:
	template<typename AddressType>
	static void Construct(Impl& impl, const AddressType& addr, s_uint16_t port)
	{
		s_int32_t family = addr.IsV4() ? M_AF_INET : M_AF_INET6;
		impl._pSockAddr = detail::Util::AllocSockAddr();
		g_bzero(impl._pSockAddr, detail::Util::SockAddrLen());
		detail::Util::InitSockAddr(family, impl._pSockAddr, (addr.Address().empty() ? NULL : addr.Address().c_str()), port);
	}

	static void Construct(Impl& impl,const Impl& other)
	{
		if (!other._pSockAddr)
			return;
		impl._pSockAddr = detail::Util::AllocSockAddr();
		g_bzero(impl._pSockAddr, detail::Util::SockAddrLen());
		g_memcpy(impl._pSockAddr, other._pSockAddr, detail::Util::SockAddrLen());
	}

	static void Copy(Impl& impl, const Impl& other)
	{
		if (!other._pSockAddr)
			return;
		g_bzero(impl._pSockAddr, detail::Util::SockAddrLen());
		g_memcpy(impl._pSockAddr, other._pSockAddr, detail::Util::SockAddrLen());
	}

	static void Destroy(Impl& impl)
	{
		detail::Util::FreeSockAddr(impl._pSockAddr);
		impl._pSockAddr = 0;
	}

	static s_int32_t Family(const Impl& impl)
	{
		if (impl._pSockAddr)
			return detail::Util::SockAddrToFamily(impl._pSockAddr);
		return 0;
	}

	static s_uint16_t Port(const Impl& impl)
	{
		if (impl._pSockAddr)
			return detail::Util::SockAddrToPort(impl._pSockAddr);
		return 0;
	}

	static std::string Address(const Impl& impl)
	{
		if (impl._pSockAddr)
			return detail::Util::SockAddrToAddress(impl._pSockAddr);
		return "";
	}

	static bool Valid(const Impl& impl)
	{
		return (impl._pSockAddr!=0);
	}

	static sockaddr_t* SockAddr(const BasicEndPoint& ep)
	{
		return ep._impl._pSockAddr;
	}

	static s_uint32_t SockAddrLen(const BasicEndPoint&)
	{
		return detail::Util::SockAddrLen();
	}

	static bool GetLocalEndPoint(socket_t fd, BasicEndPoint& ep)
	{
		if (!ep._impl._pSockAddr)
			ep._impl._pSockAddr = detail::Util::AllocSockAddr();
	
		g_bzero(ep._impl._pSockAddr, detail::Util::SockAddrLen());
		if (detail::Util::SockeToLocAddrStorge(fd, ep._impl._pSockAddr))
			return true;
		else
		{
			detail::Util::FreeSockAddr(ep._impl._pSockAddr);
			ep._impl._pSockAddr = 0;
			return false;
		}
	}

	static bool GetRemoteEndPoint(socket_t fd, BasicEndPoint& ep)
	{
		if (!ep._impl._pSockAddr)
			ep._impl._pSockAddr = detail::Util::AllocSockAddr();

		g_bzero(ep._impl._pSockAddr, detail::Util::SockAddrLen());
		if (detail::Util::SockeToRmtAddrStorge(fd, ep._impl._pSockAddr))
			return true;
		else
		{
			detail::Util::FreeSockAddr(ep._impl._pSockAddr);
			ep._impl._pSockAddr = 0;
			return false;
		}
	}
};



M_SOCKET_NAMESPACE_END
#endif
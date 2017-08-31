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

#ifndef M_UTIL_INCLUDE
#define M_UTIL_INCLUDE

#include "config.hpp"
M_SOCKET_NAMESPACE_BEGIN

namespace detail
{
	class Util
	{
	public:
		M_SOCKET_DECL static s_uint32_t LocalEndian();

		M_SOCKET_DECL static sockaddr_t* AllocSockAddr();
		
		M_SOCKET_DECL static s_uint32_t SockAddrLen();
		
		M_SOCKET_DECL static void FreeSockAddr(sockaddr_t* pSockAddr);
		
		M_SOCKET_DECL static bool InitSockAddr(s_int32_t family, sockaddr_t* pSockAddr, const char* pStrAddr, s_uint16_t port);
		
		M_SOCKET_DECL static void ConvertToSockAddr(const char* pStrAddr, sockaddr_t* pSockAddr);
		
		M_SOCKET_DECL static s_int32_t SockAddrToFamily(sockaddr_t* pSockAddr);
		
		M_SOCKET_DECL static s_uint16_t SockAddrToPort(sockaddr_t* pSockAddr);
		
		M_SOCKET_DECL static std::string SockAddrToAddress(const sockaddr_t* pSockAddr);

		M_SOCKET_DECL static bool SockeToRmtAddrStorge(socket_t fd, void* pAddr);

		M_SOCKET_DECL static bool SockeToLocAddrStorge(socket_t fd, void* pAddr);

		M_SOCKET_DECL static bool SetBlock(socket_t fd);

		M_SOCKET_DECL static bool SetNonBlock(socket_t fd);
	};

	M_SOCKET_DECL s_uint32_t Util::LocalEndian()
	{
		union
		{
			s_uint16_t s;
			char c[sizeof(s_uint16_t)];
		}un;
		un.s = 0x0102;
		if (un.c[0] == 0x2 && un.c[1] == 0x01)
			return M_LITTLE_ENDIAN;
		else
			return M_BIG_ENDIAN;
	}

	M_SOCKET_DECL sockaddr_t* Util::AllocSockAddr()
	{
		s_int32_t len = sizeof(sockaddr_storage_t);
		sockaddr_t* pAddr = (sockaddr_t*)g_malloc(len);
		return pAddr;
	}

	M_SOCKET_DECL s_uint32_t Util::SockAddrLen()
	{
		return sizeof(sockaddr_storage_t);;
	}

	M_SOCKET_DECL void Util::FreeSockAddr(sockaddr_t* pSockAddr)
	{
		g_free(pSockAddr);
	}

	M_SOCKET_DECL bool Util::InitSockAddr(s_int32_t family, sockaddr_t* pSockAddr, const char* pStrAddr, s_uint16_t port)
	{
		if (M_AF_INET == family)
		{
			sockaddr_in_t* p = (sockaddr_in_t*)pSockAddr;
			p->sin_family = family;
			p->sin_port = g_htons(port);
			if (pStrAddr) ConvertToSockAddr(pStrAddr, pSockAddr); else p->sin_addr.s_addr = htonl(M_INADDR_ANY);
			return true;
		}
		if (M_AF_INET6 == family)
		{
			sockaddr_in6_t* p = (sockaddr_in6_t*)pSockAddr;
			p->sin6_family = family;
			p->sin6_port = g_htons(port);
			if (pStrAddr) ConvertToSockAddr(pStrAddr, pSockAddr); else p->sin6_addr = in6addr_any;
			return true;
		}
		assert(0);
		return false;
	}

	M_SOCKET_DECL void Util::ConvertToSockAddr(const char* pStrAddr, sockaddr_t* pSockAddr)
	{
		if (M_AF_INET == pSockAddr->sa_family)
		{
			sockaddr_in_t* p = (sockaddr_in_t*)pSockAddr;
			g_inet_pton(pSockAddr->sa_family, pStrAddr, &p->sin_addr);
		}
		else if (M_AF_INET6 == pSockAddr->sa_family)
		{
			sockaddr_in6_t* p = (sockaddr_in6_t*)pSockAddr;
			g_inet_pton(pSockAddr->sa_family, pStrAddr, &p->sin6_addr);
		}
	}

	M_SOCKET_DECL s_int32_t Util::SockAddrToFamily(sockaddr_t* pSockAddr)
	{
		sockaddr_storage_t* p = (sockaddr_storage_t*)pSockAddr;
		return (p->ss_family);
	}

	M_SOCKET_DECL s_uint16_t Util::SockAddrToPort(sockaddr_t* pSockAddr)
	{
		if (M_AF_INET == pSockAddr->sa_family)
		{
			sockaddr_in_t* p = (sockaddr_in_t*)pSockAddr;
			return g_ntohs(p->sin_port);
		}
		else if (M_AF_INET6 == pSockAddr->sa_family)
		{
			sockaddr_in6_t* p = (sockaddr_in6_t*)pSockAddr;
			return g_ntohs(p->sin6_port);
		}
		return (0);
	}

	M_SOCKET_DECL std::string Util::SockAddrToAddress(const sockaddr_t* pSockAddr)
	{
		char addr_ary[128] = { 0 };
		if (M_AF_INET == pSockAddr->sa_family)
		{
			sockaddr_in_t* p = (sockaddr_in_t*)pSockAddr;
			if (g_inet_ntop(pSockAddr->sa_family, &p->sin_addr, addr_ary, sizeof(addr_ary)) != NULL)
				return std::string(addr_ary);
		}
		else if (M_AF_INET6 == pSockAddr->sa_family)
		{
			sockaddr_in6_t* p = (sockaddr_in6_t*)pSockAddr;
			if (g_inet_ntop(pSockAddr->sa_family, &p->sin6_addr, addr_ary, sizeof(addr_ary)) != NULL)
				return std::string(addr_ary);
		}
		return std::string("");
	}

	M_SOCKET_DECL bool Util::SockeToRmtAddrStorge(socket_t fd, void* pAddr)
	{
		sockaddr_storage_t& addr = *(sockaddr_storage_t*)(pAddr);
		socklen_t len = sizeof(addr);
		if (g_getpeername(fd, (sockaddr_t*)&addr, &len) < 0)
			return false;
		return true;
	}

	M_SOCKET_DECL bool Util::SockeToLocAddrStorge(socket_t fd, void* pAddr)
	{
		sockaddr_storage_t& addr = *(sockaddr_storage_t*)(pAddr);
		socklen_t len = sizeof(addr);
		if (g_getsockname(fd, (sockaddr_t*)&addr, &len) < 0)
			return false;
		return true;
	}

	M_SOCKET_DECL bool Util::SetBlock(socket_t fd)
	{
#ifdef M_PLATFORM_WIN
		unsigned long on = 0;
		return (g_ioctlsocket(fd, FIONBIO, &on) == 0);
#else
		s_int32_t flags = fcntl(fd, F_GETFL, 0);
		return (g_fcntl(fd, F_SETFL, flags  & ~O_NONBLOCK) != -1);
#endif
	}

	M_SOCKET_DECL bool Util::SetNonBlock(socket_t fd)
	{
#ifdef M_PLATFORM_WIN
		unsigned long on = 1;
		return (g_ioctlsocket(fd, FIONBIO, &on) == 0);
#else
		s_int32_t flags = fcntl(fd, F_GETFL, 0);
		return (g_fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
#endif
	}
}

M_SOCKET_NAMESPACE_END
#endif
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

#ifndef M_GFUNCTION_INCLUDE
#define M_GFUNCTION_INCLUDE
M_SOCKET_NAMESPACE_BEGIN

// global function
#define g_bzero(dst,s)	 memset(dst,0,s)
#define g_malloc		 malloc
#define g_free			 free
#define g_memcpy		 memcpy
#define g_memcmp		 memcmp
#define g_memset		 memset

// global function about socket 
#define g_socket		::socket
#define g_bind			::bind
#define g_listen		::listen
#define g_accept		::accept
#define g_connect		::connect
#define g_recv			::recv
#define g_send			::send
#define g_recvfrom		::recvfrom
#define g_sendto		::sendto
#define g_shutdown		::shutdown
#define g_getsockopt	::getsockopt
#define g_setsockopt	::setsockopt
#define g_htons			::htons
#define g_ntohs			::ntohs
#define g_htonl			::htonl
#define g_ntohl			::ntohl
#define g_inet_addr		::inet_addr	// 错误时将返回1，意味着(255.255.255.255)不能被用来转换
#define g_inet_ntoa		::inet_ntoa	// 非线程安全
#define g_inet_pton		::inet_pton
#define g_inet_ntop		::inet_ntop
#define g_getsockname	::getsockname	// 获取本地socket地址信息，未调用bind情况下调用connect
#define g_getpeername	::getpeername
#define g_gethostbyname	::gethostbyname	// 仅支持ipv4
#define g_gethostbyaddr	::gethostbyaddr	// 仅支持ipv4
#define g_getservbyname	::getservbyname
#define g_getservbyport	::getservbyport
#define g_getaddrinfo	::getaddrinfo
#define g_freeaddrinfo	::freeaddrinfo


// IPV6地址测试函数
#define g_in6_is_addr_unspecified	IN6_IS_ADDR_UNSPECIFIED
#define g_in6_is_addr_loopback		IN6_IS_ADDR_LOOPBACK
#define g_in6_is_addr_multicast		IN6_IS_ADDR_MULTICAST
#define g_in6_is_addr_linklocal		IN6_IS_ADDR_LINKLOCAL
#define g_in6_is_addr_sitelocal		IN6_IS_ADDR_SITELOCAL
#define g_in6_is_addr_v4mapped		IN6_IS_ADDR_V4MAPPED
#define g_in6_is_addr_v4compat		IN6_IS_ADDR_V4COMPAT
#define g_in6_is_addr_mc_nodelocal	IN6_IS_ADDR_MC_NODELOCAL
#define g_in6_is_addr_mc_linklocal	IN6_IS_ADDR_MC_LINKLOCAL
#define g_in6_is_addr_mc_sitelocal	IN6_IS_ADDR_MC_SITELOCAL
#define g_in6_is_addr_mc_orglocal	IN6_IS_ADDR_MC_ORGLOCAL
#define g_in6_is_addr_mc_global		IN6_IS_ADDR_MC_GLOBAL

#ifdef M_PLATFORM_WIN
#define g_strncpy(p1,p2,p3)	strncpy_s(p1,p3,p2,p3)
#define g_getlasterr		WSAGetLastError
#define g_setlasterr		WSASetLastError
#define g_closesocket		closesocket
#define g_wsastartup		::WSAStartup
#define g_wsacleanup		::WSACleanup
#define g_ioctlsocket		::ioctlsocket
#define g_wsarecv			::WSARecv
#define g_wsasend			::WSASend
#define g_closehandle		::CloseHandle
#define g_createiocompletionport		::CreateIoCompletionPort
#define g_cancelio			::CancelIo
#define	g_postqueuedcompletionstatus	::PostQueuedCompletionStatus
#define g_getqueuedcompletionstatus		::GetQueuedCompletionStatus
#endif

#ifndef M_PLATFORM_WIN
#define g_strncpy		    strncpy
#define g_epoll_create		::epoll_create
#define g_epoll_ctl			::epoll_ctl
#define g_epoll_wait		::epoll_wait
#define g_closesocket		close
#define g_closeudperr(fd)
#define g_openudperr(fd)
#define g_fcntl				fcntl
#endif

#ifndef M_PLATFORM_WIN
M_SOCKET_DECL s_int32_t g_getlasterr() { return errno; }
M_SOCKET_DECL void g_setlasterr(s_int32_t error) { errno = error; }
M_SOCKET_DECL std::string g_getlastmsg(s_int32_t error)
{
	return strerror(error);
	char buff[1024] = { 0 };
	strerror_r(error, buff, sizeof(buff));
	return std::string(buff);
}
#endif

#ifdef M_PLATFORM_WIN
M_SOCKET_DECL std::string g_getlastmsg(s_int32_t error)
{
	char buff[1024] = { 0 };
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buff, sizeof(buff), NULL);
	return std::string(buff);
}
M_SOCKET_DECL void g_closeudperr(int fd)
{
	BOOL bNewBehavior = false;
	DWORD dwBytesReturned = 0;
	WSAIoctl(fd, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
}
M_SOCKET_DECL void g_openudperr(int fd)
{
	BOOL bNewBehavior = true;
	DWORD dwBytesReturned = 0;
	WSAIoctl(fd, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
}
#endif


M_SOCKET_NAMESPACE_END
#endif // M_GFUNCTION_INCLUDE
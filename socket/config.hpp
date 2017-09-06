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

#ifndef M_CONFIG_INCLUDE
#define M_CONFIG_INCLUDE

#define M_SOCKET_NAMESPACE_BEGIN namespace SocketLib{
#define M_SOCKET_NAMESPACE_END   }

#ifndef M_PLATFORM_WIN32
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define M_PLATFORM_WIN32 1
#endif
#endif

#ifndef M_PLATFORM_WIN
#if defined(M_PLATFORM_WIN32) || defined(WIN64)
#define M_PLATFORM_WIN 1
#endif
#endif

#include <assert.h>
#include <string>
#include <stdlib.h>

#ifdef M_PLATFORM_WIN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <exception>
#include <string>
#include <memory>
#include <functional>
#include <MSWSock.h>
#include <Windows.h>					// 要在socket头文件后面
#pragma comment(lib,"WS2_32.lib")
#else
#include <arpa/inet.h>
#include <unistd.h>					// close函数在这儿
#include <sys/socket.h>				// 在in.h里已经包含了，可以省了
#include <netinet/in.h>				//大部分都在这儿
#include <errno.h>					// 错误处理
#include <sys/types.h>				// 系统类型定义
#include <fcntl.h>
#include <tr1/memory>
#include <tr1/functional>
#include <sys/epoll.h>
#include <string.h>
#include <signal.h>
#endif

// print debug info
#define M_DEBUG_PRINT_INFO
#ifdef M_DEBUG_PRINT_INFO
#include <iostream>
#define M_DEBUG_PRINT(info) std::cout << "debug info : " << info << std::endl;
#else
#define M_DEBUG_PRINT(info)
#endif


#ifndef M_SOCKET_DECL
#define M_SOCKET_DECL inline
#endif

// socket option define
#define M_SOL_SOCKET SOL_SOCKET
#define M_IPPROTO_IPV6 IPPROTO_IPV6
#ifdef  SO_BROADCAST
#define M_SO_BROADCAST SO_BROADCAST
#endif
#ifdef  SO_DEBUG
#define M_SO_DEBUG SO_DEBUG
#endif
#ifdef  SO_DONTROUTE
#define M_SO_DONTROUTE SO_DONTROUTE
#endif
#ifdef  SO_ERROR
#define M_SO_ERROR SO_ERROR
#endif
#ifdef  SO_KEEPALIVE
#define M_SO_KEEPALIVE SO_KEEPALIVE
#endif
#ifdef  SO_LINGER
#define M_SO_LINGER SO_LINGER
#endif
#ifdef  SO_OOBINLINE
#define M_SO_OOBINLINE SO_OOBINLINE
#endif
#ifdef  SO_RCVBUF
#define M_SO_RCVBUF SO_RCVBUF
#endif
#ifdef  SO_SNDBUF
#define M_SO_SNDBUF SO_SNDBUF
#endif
#ifdef  SO_RCVLOWAT
#define M_SO_RCVLOWAT SO_RCVLOWAT
#endif
#ifdef  SO_SNDLOWAT
#define M_SO_SNDLOWAT SO_SNDLOWAT
#endif
#ifdef  SO_RCVTIMEO
#define M_SO_RCVTIMEO SO_RCVTIMEO
#endif
#ifdef  SO_SNDTIMEO
#define M_SO_SNDTIMEO SO_SNDTIMEO
#endif
#ifdef  SO_REUSEADDR
#define M_SO_REUSEADDR SO_REUSEADDR
#endif
#ifdef	SO_REUSEPORT
#define M_SO_REUSEPORT SO_REUSEPORT
#endif
#ifdef  SO_TYPE
#define M_SO_TYPE SO_TYPE
#endif
#ifdef  SO_USELOOPBACK
#define M_SO_USELOOPBACK SO_USELOOPBACK
#endif
#ifdef  IP_TOS
#define M_IP_TOS IP_TOS
#endif
#ifdef  IP_TTL
#define M_IP_TTL IP_TTL
#endif
#ifdef  IPV6_DONTFRAG
#define M_IPV6_DONTFRAG IPV6_DONTFRAG
#endif
#ifdef  IPV6_UNICAST_HOPS
#define M_IPV6_UNICAST_HOPS IPV6_UNICAST_HOPS
#endif
#ifdef  IPV6_V6ONLY
#define M_IPV6_V6ONLY IPV6_V6ONLY
#endif
#ifdef  TCP_MAXSEG
#define M_TCP_MAXSEG TCP_MAXSEG
#endif
#ifdef  TCP_NODELAY
#define M_TCP_NODELAY TCP_NODELAY
#endif

// 协议族
#define M_AF_UNSPEC AF_UNSPEC // 无意义
#define M_AF_INET AF_INET
#define M_AF_INET6 AF_INET6
#define M_AF_BTH AF_BTH

// socket类型
#define M_SOCK_UNSPEC SOCK_UNSPEC // 无意义
#define M_SOCK_STREAM SOCK_STREAM
#define M_SOCK_DGRAM SOCK_DGRAM
#define M_SOCK_RAW SOCK_RAW
#define M_SOCK_RDM SOCK_RDM
#define M_SOCK_SEQPACKET SOCK_SEQPACKET

// 选项类型
#define M_SOL_SOCKET  SOL_SOCKET		// socket选项
#define M_IPPROTO_IP  IPPROTO_IP		// ip选项
#define M_IPPROTO_IPV6 IPPROTO_IPV6		// ipv6选项
//#define M_IPPROTO_TCP  IPPROTO_TCP		// tcp选项
//#define M_IPPROTO_ICMPV6 IPPROTO_ICMPV6

// protocol类型
#define M_IPPROTO_UNSPEC IPPROTO_UNSPEC // 无意义
#define M_IPPROTO_ICMP IPPROTO_ICMP
#define M_IPPROTO_IGMP IPPROTO_IGMP
//#define M_BTHPROTO_RFCOMM BTHPROTO_RFCOMM
#define M_IPPROTO_TCP IPPROTO_TCP
#define M_IPPROTO_UDP IPPROTO_UDP
#define M_IPPROTO_ICMPV6 IPPROTO_ICMPV6
//#define M_IPPROTO_RM IPPROTO_RM

// ICMP报文宏
#define M_ICMP_ECHOREPLY 0			// 回显应答
#define M_ICMP_UNREACH_NET 1			// 网络不可达
#define M_ICMP_UNREACH_HOST 2		// 主机不可达
#define M_ICMP_UNREACH_PROTOCOL 3	// 协议不可达
#define M_ICMP_UNREACH_PORT	4		// 端口不可达
#define M_ICMP_UNREACH_NEEDFRAG 5	// 需要进行分片但设置了不分片比特
#define M_ICMP_UNREACH_SRCFAIL  6	// 源站选路失败
#define M_ICMP_UNREACH_NET_UNKNOWN 7 // 目的网络不认识
#define M_ICMP_UNREACH_HOST_UNKNOWN 8// 目的主机不认识
#define M_ICMP_UNREACH_ISOLATED	9	// 源主机被隔离
#define M_ICMP_UNREACH_NET_PROHIB 10 // 目的网络被强制禁止
#define M_ICMP_UNREACH_HOST_PROHIB 11// 目的主机被强制禁止
#define M_ICMP_UNREACH_TOSNET 12		// 由于服务类型TOS，网络不可达
#define M_ICMP_UNREACH_TOSHOST 13	// 由于服务类型TOS，主机不可达
#define M_ICMP_UNREACH_FILTHIB 14	// 由于过滤，通信被强制禁止
#define M_ICMP_UNREACH_HOST_HIB 15	// 主机越权
#define M_ICMP_ECHO	21				// 请求回显
#define M_ICMP_ROUTERADVERT 22       // 路由通告
#define M_ICMP_ROUTERSOLICIT 23		// 路由请求
#define M_ICMP_TIMXCEED_INTRANS 24   // 传输期间生存时间为0
#define M_ICMP_TIMXCEED_REASS 25		// 在数据报组装期间生存时间为0
#define M_ICMP_PRRAMPROB	26			// 坏的IP首部
#define M_ICMP_PRRAMPROB_OPTABSENT 27// 缺少必需的选项
#define M_ICMP_TSTAMP	28			// 时间戳请求
#define M_ICMP_TSTAMPREPLY 29		// 时间戳应答
#define M_ICMP_MASKREQ 32			// 地址掩码请求
#define M_ICMP_MASKREPLY 33			// 地址掩码应答

#define M_SOCKADDR_PTR	(struct sockaddr_t*)
#define M_LITTLE_ENDIAN (0)	// 小端
#define M_BIG_ENDIAN	(1) // 大端
#define M_INADDR_NONE INADDR_NONE
#define M_INADDR_ANY  INADDR_ANY	

#define M_SET_BIT(x,y) x |= (1<<y);
#define M_CLR_BIT(x,y) x &= ~(1<<y);
#define M_GET_BIT(x,y) (x & (1<<y))
#define M_CLR_MBIT(x,s,e) x &= ~(((1<<(e-s+1))-1)<<s); 
#define M_SET_MBIT(x,y,s,e) {M_CLR_MBIT(x,s,e); x |= (y<<s);}
#define M_GET_MBIT(x,s,e) ((x>>s) & ((1<<(e-s+1))-1))


#endif // M_CONFIG_INCLUDE

#include "win_config.hpp"
#include "linux_config.hpp"
#include "type.hpp"
#include "gfunction.hpp"
#include "error.hpp"
#include "mutexlock.hpp"
#include "lock_dispatcher.hpp"
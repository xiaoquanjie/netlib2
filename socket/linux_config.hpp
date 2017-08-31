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

#ifndef M_LINUX_CONFIG_INCLUDE
#define M_LINUX_CONFIG_INCLUDE

#ifndef M_PLATFORM_WIN
#define M_INVALID_SOCKET	(-1)
#define M_SOCKET_ERROR		(-1)
#define M_EALREADY			EALREADY			// 已调用过connect
#define M_EISCONN			EISCONN				// 已连接
#define M_EWOULDBLOCK		EWOULDBLOCK			// 本该阻塞
#define M_ECONNRESET		ECONNRESET			// 返回前连接中止
#define M_EINPROGRESS		EINPROGRESS
#define M_ETIMEDOUT			ETIMEDOUT			// 超时
#define M_SHUT_RD			SHUT_RD				// 链接关闭方式
#define M_SHUT_WR			SHUT_WR
#define M_SHUT_RDWR			SHUT_RDWR
#define M_ECONNABORTED		ECONNABORTED
#define M_SOCKET_T          s_int32_t
#define M_ASYNC_CON_ERR		M_EINPROGRESS
#define M_EPOLL_CTL_ADD		EPOLL_CTL_ADD		// 添加监听
#define M_EPOLL_CTL_MOD		EPOLL_CTL_MOD		// 修改监听
#define M_EPOLL_CTL_DEL		EPOLL_CTL_DEL		// 删除监听
#define M_EPOLLIN			EPOLLIN				// 描述符可以读
#define M_EPOLLOUT			EPOLLOUT			// 描述符可以写
#define M_EPOLLPRI			EPOLLPRI			// 带外数据到来
#define M_EPOLLERR			EPOLLERR			// 描述符发生错误
#define M_EPOLLHUP			EPOLLHUP			// 描述符被挂断
#define M_EPOLLET			EPOLLET				// 边缘触发
#define M_EPOLLLT			EPOLLLT				// 水平触发(default)
#define M_EPOLLONESHOT		EPOLLONESHOT		// 只监听一次
#define M_EINTR				EINTR				// interrupted system call
#endif // M_PLATFORM_WIN

#endif // LINUX_CONFIG_INCLUDE
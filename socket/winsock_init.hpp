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

#ifndef M_WINSOCK_INIT_INCLUDE
#define M_WINSOCK_INIT_INCLUDE

class WinSockBase
{
protected:
	struct data
	{
		s_uint32_t _init_cnt;
		s_uint32_t _result;
	};
};

template<s_int32_t Major=2,s_int32_t Minor=2>
class WinSockInit : public WinSockBase
{
public:
	M_SOCKET_DECL WinSockInit();

	M_SOCKET_DECL ~WinSockInit();

	M_SOCKET_DECL LPFN_ACCEPTEX GetAcceptEx()const;

	M_SOCKET_DECL LPFN_CONNECTEX GetConnectEx()const;

protected:
	static data _data;
	static LPFN_ACCEPTEX _acceptex;
	static LPFN_CONNECTEX _connectex;
};

template<s_int32_t Major, s_int32_t Minor>
WinSockBase::data WinSockInit<Major, Minor>::_data;

template<s_int32_t Major, s_int32_t Minor>
LPFN_ACCEPTEX WinSockInit<Major, Minor>::_acceptex;

template<s_int32_t Major, s_int32_t Minor>
LPFN_CONNECTEX WinSockInit<Major, Minor>::_connectex;

template<s_int32_t Major, s_int32_t Minor>
M_SOCKET_DECL WinSockInit<Major, Minor>::WinSockInit()
{
	if (::InterlockedIncrement(&_data._init_cnt) == 1)
	{
		WSADATA wsa_data;
		long result = g_wsastartup(MAKEWORD(Major, Minor), &wsa_data);
		::InterlockedExchange(&_data._result, result);
		M_THROW_DEFAULT_SOCKET_ERROR(result != 0);
	
		socket_t fd = g_socket(M_AF_INET, M_SOCK_STREAM, M_IPPROTO_IP);

		static LPFN_ACCEPTEX lpfnAcceptEx = NULL;
		GUID guidAcceptEx = WSAID_ACCEPTEX;
		DWORD dwBytes = 0;
		if (WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx),
			&dwBytes, NULL, NULL) != 0)
		{
			g_closesocket(fd);
			M_THROW_DEFAULT_SOCKET_ERROR(true);
		}
		else
			_acceptex = lpfnAcceptEx;

		static LPFN_CONNECTEX lpfnConnectEx = NULL;
		GUID guidConnectEx = WSAID_CONNECTEX;
		if (WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidConnectEx, sizeof(guidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx),
			&dwBytes, NULL, NULL) != 0)
		{
			g_closesocket(fd);
			M_THROW_DEFAULT_SOCKET_ERROR(true);
		}
		else
			_connectex = lpfnConnectEx;

		g_closesocket(fd);
	}
}

template<s_int32_t Major, s_int32_t Minor>
M_SOCKET_DECL WinSockInit<Major, Minor>::~WinSockInit()
{
	if (::InterlockedDecrement(&_data._init_cnt) == 0){
		g_wsacleanup();
	}
}

template<s_int32_t Major, s_int32_t Minor>
M_SOCKET_DECL LPFN_ACCEPTEX WinSockInit<Major, Minor>::GetAcceptEx()const
{
	return _acceptex;
}

template<s_int32_t Major, s_int32_t Minor>
M_SOCKET_DECL LPFN_CONNECTEX WinSockInit<Major, Minor>::GetConnectEx()const
{
	return _connectex;
}


static const WinSockInit<>& gWinSockInstance = WinSockInit<>();


static LPFN_ACCEPTEX  gAcceptEx = gWinSockInstance.GetAcceptEx();
static LPFN_CONNECTEX gConnectEx = gWinSockInstance.GetConnectEx();

#endif
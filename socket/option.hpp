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

#ifndef M_SOCKET_OPT_INCLUDE
#define M_SOCKET_OPT_INCLUDE

#include "config.hpp"
M_SOCKET_NAMESPACE_BEGIN

namespace detail
{
	template <int LevelType, int NameType>
	class Boolean
	{
	public:
		Boolean() : _Value(0)
		{}

		explicit Boolean(bool v) : _Value(v ? 1 : 0)
		{}

		Boolean& operator=(bool v)
		{
			_Value = v ? 1 : 0;
			return *this;
		}

		bool Value() const
		{
			return !!_Value;
		}

		operator bool() const
		{
			return !!_Value;
		}

		bool operator!() const
		{
			return !_Value;
		}

		int Level() const
		{
			return LevelType;
		}

		int Name() const
		{
			return NameType;
		}

		int* Data()
		{
			return &_Value;
		}

		const int* Data() const
		{
			return &_Value;
		}

		socklen_t Size() const
		{
			return (socklen_t)sizeof(_Value);
		}

	private:
		int _Value;
	};

	template <int LevelType, int NameType>
	class Integer
	{
	public:
		Integer(): _Value(0)
		{}

		explicit Integer(int v): _Value(v)
		{}

		Integer& operator=(int v)
		{
			_Value = v;
			return *this;
		}

		int Value() const
		{
			return _Value;
		}

		int Level() const
		{
			return LevelType;
		}

		int Name() const
		{
			return NameType;
		}

		int* Data()
		{
			return &_Value;
		}

		const int* Data() const
		{
			return &_Value;
		}

		socklen_t Size() const
		{
			return (socklen_t)sizeof(_Value);
		}

	private:
		int _Value;
	};

	template <int LevelType, int NameType>
	class Linger
	{
	public:
		Linger()
		{
			_Value.l_onoff = 0;
			_Value.l_linger = 0;
		}

		Linger(bool e, int t)
		{
			Enabled(e);
			TimeOut(t);
		}

		void Enabled(bool value)
		{
			_Value.l_onoff = value ? 1 : 0;
		}

		bool Enabled() const
		{
			return _Value.l_onoff != 0;
		}

		void TimeOut(s_int32_t value)
		{
#if defined(M_PLATFORM_WIN)
			_Value.l_linger = static_cast<s_uint16_t>(value);
#else
			_Value.l_linger = value;
#endif
		}

		s_int32_t TimeOut()const
		{
			return static_cast<s_int32_t>(_Value.l_linger);
		}

		int Level() const
		{
			return LevelType;
		}

		int Name() const
		{
			return NameType;
		}

		linger_t* Data()
		{
			return &_Value;
		}

		const linger_t* Data() const
		{
			return &_Value;
		}

		socklen_t Size() const
		{
			return (socklen_t)sizeof(_Value);
		}

	private:
		linger_t _Value;
	};
} 

struct Opts
{
#ifdef M_SO_BROADCAST
	typedef detail::Boolean<M_SOL_SOCKET, M_SO_BROADCAST> Broadcast;
#endif

#ifdef M_SO_DEBUG
	typedef detail::Boolean<M_SOL_SOCKET, SO_DEBUG> Debug;
#endif

#ifdef M_SO_DONTROUTE
	typedef detail::Boolean<M_SOL_SOCKET, M_SO_DONTROUTE> Dontroute;
#endif

#ifdef M_SO_KEEPALIVE
	typedef detail::Boolean<M_SOL_SOCKET, M_SO_KEEPALIVE> Keepalive;
#endif

#ifdef M_SO_REUSEADDR
	typedef detail::Boolean<M_SOL_SOCKET, M_SO_REUSEADDR> ReuseAddress;
#endif

#ifdef M_TCP_NODELAY 
	typedef detail::Boolean<IPPROTO_TCP, M_TCP_NODELAY> Nodelay;
#endif

#ifdef M_SO_SNDBUF
	typedef detail::Integer<M_SOL_SOCKET, M_SO_SNDBUF> SendSize;
#endif

#ifdef M_IPV6_V6ONLY
	typedef detail::Boolean<M_IPPROTO_IPV6, M_IPV6_V6ONLY> V6only;
#endif

#ifdef M_SO_SNDLOWAT
	typedef detail::Integer<M_SOL_SOCKET, M_SO_SNDLOWAT> SendWatermark;
#endif

#ifdef M_SO_RCVBUF
	/*对于客户端，必须在connect函数调用前，对于服务器端，必须在listen之前调用*/
	typedef detail::Integer<M_SOL_SOCKET, M_SO_RCVBUF> RecvSize;
#endif

#ifdef M_SO_RCVLOWAT
	typedef detail::Integer<M_SOL_SOCKET, M_SO_RCVLOWAT> RecvWatermark;
#endif

#ifdef M_SO_LINGER
	typedef detail::Linger<M_SOL_SOCKET, M_SO_LINGER> LingerT;
#endif

#ifdef M_SO_ERROR
	typedef detail::Integer<M_SOL_SOCKET, M_SO_ERROR> SoError;
#endif
};


M_SOCKET_NAMESPACE_END
#endif
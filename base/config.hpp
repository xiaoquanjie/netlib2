#ifndef M_BASE_CONFIG_INCLUDE
#define M_BASE_CONFIG_INCLUDE

#define M_BASE_NAMESPACE_BEGIN namespace base{
#define M_BASE_NAMESPACE_END }

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

M_BASE_NAMESPACE_BEGIN
/*
基本数据类型
windows: 使用ILP32数据模型 及 LLP64数据模型
linux:	 平台使用 ILP32 及 LP64 数据模型
*/
#ifndef M_PLATFORM_DATA_MODE 
#define M_PLATFORM_DATA_MODE
typedef signed char				s_int8_t;
typedef signed short			s_int16_t;
typedef signed int				s_int32_t;
typedef signed long long		s_int64_t;
typedef unsigned char			s_uint8_t;
typedef unsigned short			s_uint16_t;
typedef unsigned int			s_uint32_t;
typedef unsigned long long		s_uint64_t;
typedef char					s_byte_t;
#endif  // M_PLATFORM_DATA_MODE
M_BASE_NAMESPACE_END

#endif
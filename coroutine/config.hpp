#ifndef M_COROUTINE_CONFIG_INCLUDE
#define M_COROUTINE_CONFIG_INCLUDE

#define M_COROUTINE_NAMESPACE_BEGIN namespace coroutine{
#define M_COROUTINE_NAMESPACE_END }

#include "base/config.hpp"

#ifdef M_PLATFORM_WIN
#ifndef M_WIN32_LEAN_AND_MEAN  
#define WIN32_LEAN_AND_MEAN // 在所有Windows.h包含的地方加上它
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <ucontext.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#endif
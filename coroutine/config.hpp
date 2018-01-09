#ifndef M_COROUTINE_CONFIG_INCLUDE
#define M_COROUTINE_CONFIG_INCLUDE

#define M_COROUTINE_NAMESPACE_BEGIN namespace coroutine{
#define M_COROUTINE_NAMESPACE_END }

#include "base/config.hpp"

#ifdef M_PLATFORM_WIN
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
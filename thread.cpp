#include "thread.hpp"
#ifndef M_PLATFORM_WIN

namespace detail
{
	__thread int t_cachedTid;
	__thread char t_tidString[32];

	void cacheTid()
	{
		if (t_cachedTid == 0)
		{
			t_cachedTid = gettid();
			int n = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
			assert(n == 6); (void) n;
		}
	}

	int tid()
	{
		if (__builtin_expect(t_cachedTid == 0, 0))
		{
			cacheTid();
		}
		return t_cachedTid;
	}

	const char* tidString() // for logging
	{
		return t_tidString;
	}
}
#endif

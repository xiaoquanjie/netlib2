#ifndef M_BASE_TIMER_INCLUDE
#define M_BASE_TIMER_INCLUDE

#include "base/config.hpp"
#include <time.h>
#include <limits>
#include <string>

#ifndef M_PLATFORM_WIN
#include <sys/time.h>
#else
#ifndef M_WIN32_LEAN_AND_MEAN  
#define WIN32_LEAN_AND_MEAN // 在所有Windows.h包含的地方加上它
#endif
#include <windows.h>
#include <WinSock2.h>	// // for timeval 
#endif

M_BASE_NAMESPACE_BEGIN

class timer
{
public:
	timer() {
		_starttime = clock();
	}

	void restart() {
		_starttime = clock();
	}

	double elapsed() const {
		return  double(clock() - _starttime) / CLOCKS_PER_SEC;
	}

	double elapsed_max() const {
		return (double((std::numeric_limits<clock_t>::max)())
			- double(_starttime)) / double(CLOCKS_PER_SEC);
	}

	double elapsed_min() const {
		return double(1) / double(CLOCKS_PER_SEC);
	}

private:
	clock_t _starttime;
};

class second_timer
{
public:
	second_timer() {
		_starttime = time(0);
	}

	void restart()
	{
		_starttime = time(0);
	}

	time_t elapsed() const
	{
		return (time(0) - _starttime);
	}

private:
	time_t _starttime;
};

struct c_time
{
	static tm* localtime(const time_t* t, tm* result)
	{
#ifndef M_PLATFORM_WIN
		result = ::localtime_r(t, result);
#else
		::localtime_s(result, t);
#endif
		return result;
	}

	static tm* localtime(tm* result)
	{
		time_t t = c_time::time(0);
		return c_time::localtime(&t, result);
	}

	static tm* gmtime(const time_t* t, tm* result)
	{
#ifndef M_PLATFORM_WIN
		result = ::gmtime_r(t, result);
#else
		::gmtime_s(result, t);
#endif
		return result;
	}

	static tm* gmtime(tm* result)
	{
		time_t t = c_time::time(0);
		return c_time::gmtime(&t, result);
	}

	static time_t time(time_t* t)
	{
		return ::time(t);
	}
};

class timestamp
{
public:
	timestamp()
	{
#ifndef M_PLATFORM_WIN
		gettimeofday(&tv_, NULL);
#else
		SYSTEMTIME st;
		struct tm  tm;
		time_t     clock;

		GetLocalTime(&st);
		tm.tm_year = st.wYear - 1900;
		tm.tm_mon = st.wMonth - 1;
		tm.tm_mday = st.wDay;
		tm.tm_hour = st.wHour;
		tm.tm_min = st.wMinute;
		tm.tm_sec = st.wSecond;
		tm.tm_isdst = -1;
		clock = mktime(&tm);
		tv_.tv_sec = (long)clock;
		tv_.tv_usec = st.wMilliseconds * 1000;
#endif
	}

	bool operator==(const timestamp& rhs)const {
		return (rhs.tv_.tv_sec == this->tv_.tv_sec && rhs.tv_.tv_usec == this->tv_.tv_usec);
	}
	bool operator!=(const timestamp& rhs)const {
		return !(rhs == *this);
	}
	bool operator<(const timestamp& rhs)const {
		return (this->tv_.tv_sec * 1000000 + this->tv_.tv_usec)<(rhs.tv_.tv_sec * 1000000 + rhs.tv_.tv_usec);
	}
	bool operator<=(const timestamp& rhs)const {
		return !(*this>rhs);
	}
	bool operator>(const timestamp& rhs)const {
		return (this->tv_.tv_sec * 1000000 + this->tv_.tv_usec)>(rhs.tv_.tv_sec * 1000000 + rhs.tv_.tv_usec);
	}
	bool operator>=(const timestamp& rhs)const {
		return !(*this<rhs);
	}
	long millisecond()const {
		return (this->tv_.tv_sec * 1000 + this->tv_.tv_usec / 1000);
	}
	long microsecond()const {
		return (this->tv_.tv_sec * 1000000 + this->tv_.tv_usec);
	}
	time_t second()const {
		return (tv_.tv_sec + tv_.tv_usec / (1000 * 1000));
	}
	std::string format_ymd()const
	{
		char carray[20] = { 0 };
		format_ymd(carray, 20);
		return std::string(carray);
	}
	void format_ymd(char* buf, size_t len)const {
		struct tm tm;
		time_t t = second();
		c_time::localtime(&t, &tm);
		snprintf(buf, len, "%4d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	}
	std::string format_ymd_hms()const {
		char carray[30] = { 0 };
		format_ymd_hms(carray, 30);
		return std::string(carray);
	}
	void format_ymd_hms(char* buf, size_t len)const {
		struct tm tm;
		time_t t = second();
		c_time::localtime(&t, &tm);
		snprintf(buf, len, "%4d%02d%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	std::string format_ymd_hms_mil()const {
		char carray[40] = { 0 };
		format_ymd_hms_mil(carray, 40);
		return std::string(carray);
	}
	void format_ymd_hms_mil(char* buf, size_t len)const {
		struct tm tm;
		time_t t = second();
		c_time::localtime(&t, &tm);
		snprintf(buf, len, "%4d%02d%02d %02d:%02d:%02d.%03d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, millisecond());
	}
	std::string format_mil()const {
		char carray[10] = { 0 };
		format_mil(carray, 10);
		return std::string(carray);
	}
	void format_mil(char* buf, size_t len)const {
		snprintf(buf, len, "%03d", millisecond());
	}

private:
	timeval tv_;
};

M_BASE_NAMESPACE_END
#endif
#ifndef M_BASE_LOGGER_INCLUDE
#define M_BASE_LOGGER_INCLUDE

#include "base/config.hpp"
#include "base/thread.hpp"
#include "base/tls.hpp"
#include <string>
#include <time.h>
#include <stdio.h>
#ifdef M_PLATFORM_WIN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

M_BASE_NAMESPACE_BEGIN

namespace logger {

	static std::string _getpid_() {
		char tmp_buf[20] = { 0 };
#ifdef M_PLATFORM_WIN
		snprintf(tmp_buf, sizeof(tmp_buf), "%d", GetCurrentProcessId());
#else
		snprintf(tmp_buf, sizeof(tmp_buf), "%d", getpid());
#endif
		return std::string(tmp_buf);
	}

	static std::string _getcurtime_(const time_t& now) {
		struct tm ltm;
#ifdef M_PLATFORM_WIN
		localtime_s(&ltm, &now);
#else
		localtime_r(&now, &ltm);
#endif
		char tmp_buf[20] = { 0 };
		strftime(tmp_buf, sizeof(tmp_buf), "%Y%m%d", &ltm);
		return std::string(tmp_buf);
	}

	static std::string _logname_(const time_t& now, const std::string& pid, 
		const std::string& filename, size_t logidx) {
		char tmp_buf[100] = { 0 };
		snprintf(tmp_buf, sizeof(tmp_buf), "%s_%s_%s_%d.log", filename.c_str(),
			_getcurtime_(now).c_str(), pid.c_str(), logidx);
		return std::string(tmp_buf);
	}

	static bool _issameday_(time_t& t1, time_t& t2) {
		struct tm ltm1, ltm2;
#ifdef M_PLATFORM_WIN
		localtime_s(&ltm1, &t1);
		localtime_s(&ltm2, &t2);
#else
		localtime_r(&t1, &ltm1);
		localtime_r(&t2, &ltm2);
#endif
		return (ltm1.tm_year == ltm2.tm_year
			&& ltm1.tm_mon == ltm2.tm_mon
			&& ltm1.tm_mday == ltm2.tm_mday);
	}

	// 毫秒级
	static long long _gettime_() {
		long long t = 0;
#ifdef M_PLATFORM_WIN
		SYSTEMTIME st;
		struct tm  tm;
		time_t clock;
		GetLocalTime(&st);
		tm.tm_year = st.wYear - 1900;
		tm.tm_mon = st.wMonth - 1;
		tm.tm_mday = st.wDay;
		tm.tm_hour = st.wHour;
		tm.tm_min = st.wMinute;
		tm.tm_sec = st.wSecond;
		tm.tm_isdst = -1;
		clock = mktime(&tm);
		t = clock * 1000 + st.wMilliseconds;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		t = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
		return t;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	// 日志文件类
	class logfile {
	protected:
		logfile(const logfile&);
		logfile& operator=(const logfile&);

	private:
		std::string _filename;
		FILE* _file;
		size_t _writesize;
		size_t _rollsize;
		size_t _logidx;
		time_t _rolltime;
		time_t _lastctime;
		std::string _pid;

	public:
		logfile(const std::string filename, size_t rollsize);

		~logfile();

		bool write(const char* data, size_t len);

		void flush();

	protected:
		void _roll_file(bool newday, const time_t& now);

		void _close();
	};

	inline logfile::logfile(const std::string filename, size_t rollsize)
		:_filename(filename) {
		_rollsize = rollsize;
		_file = 0;
		_writesize = 0;
		_logidx = 0;
		_rolltime = 0;
		_lastctime = 0;
		_pid = _getpid_();
		_roll_file(true, time(0));
	}

	inline logfile::~logfile() {
		_close();
	}

	inline bool logfile::write(const char* data, size_t len) {
		if (_file) {
			time_t now = time(0);
			if (now != _lastctime) {
				_lastctime = now;
				if (!_issameday_(_rolltime, now))
					_roll_file(false, now);
			}
			if (_writesize >= _rollsize) {
				_roll_file(false, now);
			}
			size_t wsize = 0;
			while (len) {
				wsize = fwrite(data, 1, len, _file);
				if (wsize == 0)
					break;
				len -= wsize;
				_writesize += wsize;
			}
		}
		return (len == 0);
	}

	inline void logfile::flush() {
		if (_file) {
			fflush(_file);
		}
	}

	inline void logfile::_close() {
		if (_file) {
			fflush(_file);
			fclose(_file);
			_file = 0;
		}
	}

	inline void logfile::_roll_file(bool newday, const time_t& now) {
		if (_file) {
			_close();
		}
		_writesize = 0;
		_rolltime = now;
		_logidx = newday ? 1 : _logidx + 1;
		std::string name = _logname_(now, _pid, _filename, _logidx);
#ifdef M_PLATFORM_WIN
		fopen_s(&_file, name.c_str(), "w");
#else
		_file = fopen(name.c_str(), "w");
#endif
	}

///////////////////////////////////////////////////////////////////////////////////////////////

	// 固定缓存
	template<size_t SIZE>
	struct fixedbuffer {
	public:
		fixedbuffer();

		void append(const char* data, size_t len);

		const char* data()const;

		size_t length()const;

		const char* current()const;

		char* current();

		size_t avail()const;

		void seek(int incr);

		void clear();

		const char* getstring();

	protected:
		fixedbuffer(const fixedbuffer&);
		fixedbuffer& operator=(const fixedbuffer&);

	private:
		char _data[SIZE+1];
		size_t _pos;
	};

	template<size_t SIZE>
	fixedbuffer<SIZE>::fixedbuffer() {
		clear();
	}

	template<size_t SIZE>
	void fixedbuffer<SIZE>::append(const char* data, size_t len) {
		if (len > (SIZE - _pos))
			len = SIZE - _pos;
		memcpy(_data + _pos, data, len);
		_pos += len;
	}

	template<size_t SIZE>
	const char* fixedbuffer<SIZE>::data()const {
		return _data;
	}

	template<size_t SIZE>
	size_t fixedbuffer<SIZE>::length()const {
		return _pos;
	}
	
	template<size_t SIZE>
	const char* fixedbuffer<SIZE>::current()const {
		return &_data[_pos];
	}

	template<size_t SIZE>
	char* fixedbuffer<SIZE>::current() {
		return &_data[_pos];
	}

	template<size_t SIZE>
	size_t fixedbuffer<SIZE>::avail()const {
		return (SIZE - _pos);
	}

	template<size_t SIZE>
	void fixedbuffer<SIZE>::seek(int incr) {
		if (incr >= 0) {
			_pos += incr;
			if (_pos > SIZE)
				_pos = SIZE;
		}
		else if ((size_t)(incr*-1) > _pos) {
			_pos = 0;
		}
		else {
			_pos += incr;
		}
	}

	template<size_t SIZE>
	void fixedbuffer<SIZE>::clear() {
		_pos = 0;
		memset(_data, 0, SIZE + 1);
	}

	template<size_t SIZE>
	const char* fixedbuffer<SIZE>::getstring() {
		_data[_pos] = '\0';
		return _data;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	// 日志流,一条日志最大4k
	class logstream {
	public:
		typedef fixedbuffer<4 * 1024> buffer_type;

		logstream& operator<<(const bool& value);

		logstream& operator<<(const short& value);

		logstream& operator<<(const unsigned short& value);

		logstream& operator<<(const int& value);

		logstream& operator<<(const unsigned int& value);

		logstream& operator<<(const long long& value);

		logstream& operator<<(const unsigned long long& value);

		logstream& operator<<(const float& value);

		logstream& operator<<(const double& value);

		logstream& operator<<(const char& value);

		logstream& operator<<(const std::string& value);

		template<int len>
		logstream& operator<<(const char(&value)[len]);

		logstream& operator<<(const char* value);

		// support for address
		logstream& operator<<(const void* value);

		buffer_type& buffer();

	protected:
		template<typename T>
		void _convert(const T&value, size_t type);

	protected:
		buffer_type _buffer;
	};

	inline logstream& logstream::operator<<(const bool& value) {
		_buffer.append(value ? "1" : "0", 1);
		return *this;
	}

	inline logstream& logstream::operator<<(const short& value) {
		*this << static_cast<int>(value);
		return *this;
	}

	inline logstream& logstream::operator<<(const unsigned short& value) {
		*this << static_cast<unsigned int>(value);
		return *this;
	}

	inline logstream& logstream::operator<<(const int& value) {
		_convert(value, 0);
		return *this;
	}

	inline logstream& logstream::operator<<(const unsigned int& value) {
		_convert(value, 0);
		return *this;
	}

	inline logstream& logstream::operator<<(const long long& value) {
		_convert(value, 0);
		return *this;
	}

	inline logstream& logstream::operator<<(const unsigned long long& value) {
		_convert(value, 0);
		return *this;
	}

	inline logstream& logstream::operator<<(const float& value) {
		*this << static_cast<double>(value);
		return *this;
	}

	inline logstream& logstream::operator<<(const double& value) {
		_convert(value, 1);
		return *this;
	}

	inline logstream& logstream::operator<<(const char& value) {
		_buffer.append(&value, 1);
		return *this;
	}

	inline logstream& logstream::operator<<(const std::string& value) {
		_buffer.append(value.c_str(), value.size());
		return *this;
	}

	template<int len>
	inline logstream& logstream::operator<<(const char(&value)[len]) {
		_buffer.append(value, len - 1);
		return *this;
	}

	inline logstream& logstream::operator<<(const char* value) {
		_buffer.append(value, strlen(value));
		return *this;
	}

	inline logstream& logstream::operator<<(const void* value) {
		_convert(value, 2);
		return *this;
	}

	template<typename T>
	void logstream::_convert(const T&value, size_t type) {
		// 整数最长接受32个字符
		if (_buffer.avail() >= 32) {
			const char* ptype = 0;
			switch (type) {
			case 0:
				ptype = "%d";
				break;
			case 1:
				ptype = "%.12g";
				break;
			default:
				ptype = "0x%0X";
				break;
			}
			int len = snprintf(_buffer.current(), _buffer.avail(), ptype, value);
			_buffer.seek(len);
		}
	}

	logstream::buffer_type& logstream::buffer() {
		return _buffer;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	struct logtime {
		char _time_str[32];
		char _mil_str[10];
		time_t _last;
		logtime() {
			_last = 0;
		}
		void to_format(logstream& ls) {
			long long now_in_mil = _gettime_();
			time_t now = now_in_mil / 1000;
			int mil = now_in_mil % 1000;
			if (now != _last) {
				_last = now;
				struct tm ltm;
#ifdef M_PLATFORM_WIN
				localtime_s(&ltm, &now);
#else
				localtime_r(&now, &ltm);
#endif
				strftime(_time_str, sizeof(_time_str), "%Y%m%d %H:%M:%S", 
					&ltm);
			}
			snprintf(_mil_str, sizeof(_mil_str), ".%03d ", mil);
			ls.buffer().append(_time_str, 17);
			ls.buffer().append(_mil_str, 5);
		}
	};

///////////////////////////////////////////////////////////////////////////////////////////////////

	struct logthread{
		std::string _thrid;
		logthread() {
			_thrid = thread::ctid_str();
			_thrid.append(" ");
		}
		void to_format(logstream& ls) {
			ls << _thrid;
		}
	};

	enum loglevel{
		LOG_LEVEL_TRACE = 0,
		LOG_LEVEL_DEBUG = 1,
		LOG_LEVEL_INFO = 2,
		LOG_LEVEL_WARN = 3,
		LOG_LEVEL_ERROR = 4,
		LOG_LEVEL_FATAL = 5,
		LOG_LEVEL_MAX = 6,
	};

///////////////////////////////////////////////////////////////////////////////////////////////////

	inline void console_output(const char* data, size_t len) {
		fwrite(data, 1, len, stdout);
	}

	class logger {
	public:
		const char* _level_desc[LOG_LEVEL_MAX];
	public:
		void setlevel(loglevel level);

		loglevel getlevel()const;

		void setfilename(const std::string& filename);

		void setrollsize(const size_t rollsize);

		static logger& instance();

		void log(const char* data, size_t len);

		void setoutput(void(*output)(const char*, size_t)) {

		}

	protected:
		logger();

		~logger();

		void dump(void*);

	private:
		std::string _filename;
		loglevel _level;
		logfile* _file;
		thread* _thread;
		size_t _rollsize;
		void(*_output)(const char*, size_t);
	};

	inline logger::logger() {
		_level = LOG_LEVEL_TRACE;
		_level_desc[LOG_LEVEL_TRACE] = "TRACE ";
		_level_desc[LOG_LEVEL_DEBUG] = "DEBUG ";
		_level_desc[LOG_LEVEL_INFO] = "INFO  ";
		_level_desc[LOG_LEVEL_WARN] = "WARN  ";
		_level_desc[LOG_LEVEL_ERROR] = "ERROR ";
		_level_desc[LOG_LEVEL_FATAL] = "FATAL ";
		_file = NULL;
		_thread = NULL;
		_rollsize = 1024 * 1024 * 300;
		_output = console_output;
	}

	inline logger::~logger() {
		if (_thread) {
			_thread->join();
			delete _thread;
		}
		if (_file) {
			delete _file;
		}
	}

	inline void logger::setlevel(loglevel level) {
		_level = level;
	}

	inline loglevel logger::getlevel()const {
		return _level;
	}

	inline void logger::setfilename(const std::string& filename) {
		if (_filename.empty()) {
			_filename = filename;
			_file = new logfile(_filename, _rollsize);
			_thread = new thread(&logger::dump, this, 0);
		}
	}

	inline void logger::setrollsize(const size_t rollsize) {
		_rollsize = rollsize;
	}

	inline logger& logger::instance() {
		static logger static_logger;
		return static_logger;
	}

	inline void logger::log(const char* data, size_t len) {
		if (_output) {
			_output(data, len);
		}
	}

	inline void logger::dump(void*) {

	}

	struct logimpl {
		logstream _stream;
		logimpl(loglevel level) {
			logtime& lg = tlsdata<logtime, 0>::data();
			lg.to_format(_stream);
			_stream << logger::instance()._level_desc[level];
			logthread& lt = tlsdata<logthread, 0>::data();
			lt.to_format(_stream);
		}
		~logimpl() {
			_stream << "\n\0";
			logger::instance().log(_stream.buffer().data(), _stream.buffer().length());
		}
		logstream& stream() {
			return _stream;
		}
	};

}

M_BASE_NAMESPACE_END

#define SetLogLevel(level)\
	base::logger::logger::instance().setlevel((base::logger::loglevel)level)
#define GetLogLevel()\
	base::logger::logger::instance().getlevel()
#define SetLogFileName(name)\
	base::logger::logger::instance().setfilename(name)

#define LogTrace(content)\
{\
	if (base::logger::LOG_LEVEL_TRACE>=GetLogLevel()){\
		base::logger::logimpl(base::logger::LOG_LEVEL_TRACE).stream()<<content;\
	}\
}

#define LogDebug(content)\
{\
	if (base::logger::LOG_LEVEL_DEBUG>=GetLogLevel()){\
		base::logger::logimpl(base::logger::LOG_LEVEL_DEBUG).stream()<<content;\
	}\
}

#define LogInfo(content)\
{\
	if (base::logger::LOG_LEVEL_INFO>=GetLogLevel()){\
		base::logger::logimpl(base::logger::LOG_LEVEL_INFO).stream()<<content;\
	}\
}

#define LogWarn(content)\
{\
	if (base::logger::LOG_LEVEL_WARN>=GetLogLevel()){\
		base::logger::logimpl(base::logger::LOG_LEVEL_WARN).stream()<<content;\
	}\
}

#define LogError(content)\
{\
	if (base::logger::LOG_LEVEL_ERROR>=GetLogLevel()){\
		base::logger::logimpl(base::logger::LOG_LEVEL_ERROR).stream()<<content;\
	}\
}

#define LogFatal(content)\
{\
	if (base::logger::LOG_LEVEL_FATAL>=GetLogLevel()){\
		base::logger::logimpl(base::logger::LOG_LEVEL_FATAL).stream()<<content;\
	}\
}

#endif

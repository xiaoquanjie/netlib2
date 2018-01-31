#ifndef M_BASE_LOGGER_INCLUDE
#define M_BASE_LOGGER_INCLUDE

#include "base/config.hpp"
#include <string>
#include <time.h>
#include <stdio.h>
#ifdef M_PLATFORM_WIN
#include <windows.h>
#else
#include <unistd.h>
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
		_file = fopen_s(name.c_str(), "w");
#endif
	}

///////////////////////////////////////////////////////////////////////////////////////////////

	// 固定缓存
	template<int SIZE>
	struct fixedbuffer {
	public:
		fixedbuffer();

		void append(const char* data, size_t len);

		const char* data()const;

		size_t length()const;

		void clear();

		const char* getstring();

	private:
		char _data[SIZE+1];
		size_t _pos;
	};

	template<int SIZE>
	fixedbuffer<SIZE>::fixedbuffer() {
		clear();
	}

	template<int SIZE>
	void fixedbuffer<SIZE>::append(const char* data, size_t len) {
		if (len > (SIZE - _pos))
			len = SIZE - _pos;
		memcpy(_data + _pos, data, len);
		_pos += len;
	}

	template<int SIZE>
	const char* fixedbuffer<SIZE>::data()const {
		return _data;
	}

	template<int SIZE>
	size_t fixedbuffer<SIZE>::length()const {
		return _pos;
	}
	
	template<int SIZE>
	void fixedbuffer<SIZE>::clear() {
		_pos = 0;
		memset(_data, 0, SIZE + 1);
	}

	template<int SIZE>
	const char* fixedbuffer<SIZE>::getstring() {
		_data[_pos] = '\0';
		return _data;
	}
}


M_BASE_NAMESPACE_END
#endif

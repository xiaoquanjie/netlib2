#ifndef M_NETIO_HEART_BEAT_INCLUDE
#define M_NETIO_HEART_BEAT_INCLUDE

#include "netio/config.hpp"
#include "base/atomic.hpp"
#include "base/mutexlock.hpp"
#include "base/thread.hpp"
#include "base/slist.hpp"
M_NETIO_NAMESPACE_BEGIN

class TcpSocket;
class HttpSocket;
typedef shard_ptr_t<TcpSocket> TcpSocketPtr;
typedef shard_ptr_t<HttpSocket>	HttpSocketPtr;

class HeartBeatMng {
	struct HeartInfo {
		struct SocketInfo {
			time_t updtime;
			base::s_uint32_t count;
		};
		base::MutexLock mutex;
		typedef std::map<TcpSocketPtr, SocketInfo*> InfoType;
		typedef std::map<HttpSocketPtr, SocketInfo*> InfoType2;
		InfoType info;
		InfoType2 info2;
		~HeartInfo() {
			for (InfoType::iterator iter = info.begin(); iter != info.end();
				++iter)
				delete iter->second;
			for (InfoType2::iterator iter = info2.begin(); iter != info2.end();
				++iter)
				delete iter->second;
		}
		base::slist<TcpSocketPtr> co_tcplist;
		base::slist<HttpSocketPtr> co_httplist;
		base::slist<TcpSocketPtr> data_tcplist;
		base::slist<HttpSocketPtr> data_httplist;
	};

public:
	HeartBeatMng() {
		_heartinfo = 0;
		_thread = 0;
		_svr_cnt = 0;
		_startflag = false;
		_beat = 30;
	}

	~HeartBeatMng() {
		Stop();
	}

	void Start(int svr_cnt) {
		if (!_heartinfo) {
			_svr_cnt = svr_cnt;
			_heartinfo = new HeartInfo[svr_cnt];
			_startflag = true;
			_thread = new base::thread(&HeartBeatMng::_Run, this, 0);
		}
	}

	void Stop() {
		_startflag = false;
		_thread->join();
		delete _heartinfo;
		_heartinfo = 0;
		delete _thread;
		_thread = 0;
	}

	void SetBeat(unsigned int beat) {
		_beat = beat;
	}

	void OnConnected(TcpSocketPtr& clisock) {
		unsigned int idx = clisock->GetSocket().GetFd();
		idx = idx % _svr_cnt;
		_heartinfo[idx].mutex.lock();
		_heartinfo[idx].co_tcplist.push_back(clisock);
		_heartinfo[idx].mutex.unlock();
	}

	void OnConnected(HttpSocketPtr& clisock) {
		unsigned int idx = clisock->GetSocket().GetFd();
		idx = idx % _svr_cnt;
		_heartinfo[idx].mutex.lock();
		_heartinfo[idx].co_httplist.push_back(clisock);
		_heartinfo[idx].mutex.unlock();
	}

	void OnReceiveData(TcpSocketPtr& clisock) {
		unsigned int idx = clisock->GetSocket().GetFd();
		idx = idx % _svr_cnt;
		_heartinfo[idx].mutex.lock();
		_heartinfo[idx].data_tcplist.push_back(clisock);
		_heartinfo[idx].mutex.unlock();
	}

	void OnReceiveData(HttpSocketPtr& clisock) {
		unsigned int idx = clisock->GetSocket().GetFd();
		idx = idx % _svr_cnt;
		_heartinfo[idx].mutex.lock();
		_heartinfo[idx].data_httplist.push_back(clisock);
		_heartinfo[idx].mutex.unlock();
	}

protected:
	void _Run(void*p) {
		base::slist<TcpSocketPtr> tmp_co_tcplist;
		base::slist<HttpSocketPtr> tmp_co_httplist;
		base::slist<TcpSocketPtr> tmp_data_tcplist;
		base::slist<HttpSocketPtr> tmp_data_httplist;
		while (_startflag) {
			time_t now = time(0);
			for (unsigned int idx = 0; idx < _svr_cnt; ++idx) {
				_heartinfo[idx].mutex.lock();
				tmp_co_tcplist.swap(_heartinfo[idx].co_tcplist);
				tmp_co_httplist.swap(_heartinfo[idx].co_httplist);
				tmp_data_tcplist.swap(_heartinfo[idx].data_tcplist);
				tmp_data_httplist.swap(_heartinfo[idx].data_httplist);
				_heartinfo[idx].mutex.unlock();
				
				for (HeartInfo::InfoType::iterator iter = _heartinfo[idx].info.begin();
					iter != _heartinfo[idx].info.end();) {
					if (iter->second->updtime + _beat < now) {
						if (iter->first->IsConnected()) {
							iter->first->Close();
						}
						_heartinfo[idx].info.erase(iter++);
					}
					else
						++iter;
				}

				for (HeartInfo::InfoType2::iterator iter = _heartinfo[idx].info2.begin();
					iter != _heartinfo[idx].info2.end();) {
					if (iter->second->updtime + _beat < now) {
						if (iter->first->IsConnected()) {
							iter->first->Close();
						}
						_heartinfo[idx].info2.erase(iter++);
					}
					else
						++iter;
				}

				while (tmp_co_tcplist.size()) {
					TcpSocketPtr ptr = tmp_co_tcplist.front();
					HeartInfo::SocketInfo* pinfo = new HeartInfo::SocketInfo;
					pinfo->count = 0;
					pinfo->updtime = time(0);
					_heartinfo[idx].info[ptr] = pinfo;
					tmp_co_tcplist.pop_front();
				}
				while (tmp_co_httplist.size()) {
					HttpSocketPtr ptr = tmp_co_httplist.front();
					HeartInfo::SocketInfo* pinfo = new HeartInfo::SocketInfo;
					pinfo->count = 0;
					pinfo->updtime = time(0);
					_heartinfo[idx].info2[ptr] = pinfo;
					tmp_co_httplist.pop_front();
				}
				while (tmp_data_tcplist.size()) {
					TcpSocketPtr ptr = tmp_data_tcplist.front();
					HeartInfo::InfoType::iterator iter = _heartinfo[idx].info.find(ptr);
					if (iter != _heartinfo[idx].info.end()) {
						iter->second->count += 1;
						iter->second->updtime = time(0);
					}
					tmp_data_tcplist.pop_front();
				}
				while (tmp_data_httplist.size()) {
					HttpSocketPtr ptr = tmp_data_httplist.front();
					HeartInfo::InfoType2::iterator iter = _heartinfo[idx].info2.find(ptr);
					if (iter != _heartinfo[idx].info2.end()) {
						iter->second->count += 1;
						iter->second->updtime = time(0);
					}
					tmp_data_httplist.pop_front();
				}
			}
			base::thread::sleep(10);
		}
	}

protected:
	HeartInfo* _heartinfo;
	base::thread* _thread;
	bool _startflag;
	unsigned int _svr_cnt;
	unsigned int _beat;
	base::atomicint32 _socketid;
};

M_NETIO_NAMESPACE_END
#endif
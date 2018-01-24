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
class TcpConnector;
typedef shard_ptr_t<TcpSocket> TcpSocketPtr;
typedef shard_ptr_t<HttpSocket>	HttpSocketPtr;
typedef shard_ptr_t<TcpConnector> TcpConnectorPtr;

class HeartBeatMng {
	struct HeartInfo {
		struct SocketInfo {
			time_t updtime;
			base::s_uint32_t count;
		};
		base::MutexLock mutex;
		typedef std::map<TcpSocketPtr, SocketInfo*> InfoType;
		typedef std::map<HttpSocketPtr, SocketInfo*> InfoType2;
		InfoType tcp_info;
		InfoType2 http_info;
		~HeartInfo() {
			for (InfoType::iterator iter = tcp_info.begin(); iter != tcp_info.end();
				++iter)
				delete iter->second;
			for (InfoType2::iterator iter = http_info.begin(); iter != http_info.end();
				++iter)
				delete iter->second;
		}
		base::slist<TcpSocketPtr> co_tcplist;
		base::slist<HttpSocketPtr> co_httplist;
		base::slist<TcpSocketPtr> data_tcplist;
		base::slist<HttpSocketPtr> data_httplist;

		struct ConnSocketInfo {
			unsigned int idle;
			time_t snd_time;
			time_t rcv_time;
			unsigned int rcv_cnt;
		};
		typedef std::map<TcpConnectorPtr, ConnSocketInfo*> ConnInfoType;
		ConnInfoType conn_tcp_info;

		struct ConnSetInfo {
			TcpConnectorPtr ptr;
			unsigned int idle;
		};

		base::slist<ConnSetInfo> co_conn_tcplist;
		base::slist<TcpConnectorPtr> data_conn_tcplist;
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

	// 心跳的数据格式
	void SetConnSndBuffer(netiolib::Buffer& buffer) {
		_conn_snd_buffer.Write(buffer.Data(), buffer.Length());
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

	void OnConnected(TcpConnectorPtr& clisock,unsigned int idle) {
		unsigned int idx = clisock->GetSocket().GetFd();
		idx = idx % _svr_cnt;
		HeartInfo::ConnSetInfo info;
		info.ptr = clisock;
		info.idle = idle;
		_heartinfo[idx].mutex.lock();
		_heartinfo[idx].co_conn_tcplist.push_back(info);
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

	void OnReceiveData(TcpConnectorPtr& clisock) {
		unsigned int idx = clisock->GetSocket().GetFd();
		idx = idx % _svr_cnt;
		_heartinfo[idx].mutex.lock();
		_heartinfo[idx].data_conn_tcplist.push_back(clisock);
		_heartinfo[idx].mutex.unlock();
	}

protected:
	void _Run(void*p) {
		base::slist<TcpSocketPtr> tmp_co_tcplist;
		base::slist<HttpSocketPtr> tmp_co_httplist;
		base::slist<TcpSocketPtr> tmp_data_tcplist;
		base::slist<HttpSocketPtr> tmp_data_httplist;
		base::slist<HeartInfo::ConnSetInfo> tmp_co_conn_tcplist;
		base::slist<TcpConnectorPtr> tmp_data_conn_tcplist;
		while (_startflag) {
			time_t now = time(0);
			for (unsigned int idx = 0; idx < _svr_cnt; ++idx) {
				_heartinfo[idx].mutex.lock();
				tmp_co_tcplist.swap(_heartinfo[idx].co_tcplist);
				tmp_co_httplist.swap(_heartinfo[idx].co_httplist);
				tmp_data_tcplist.swap(_heartinfo[idx].data_tcplist);
				tmp_data_httplist.swap(_heartinfo[idx].data_httplist);
				tmp_co_conn_tcplist.swap(_heartinfo[idx].co_conn_tcplist);
				tmp_data_conn_tcplist.swap(_heartinfo[idx].data_conn_tcplist);
				_heartinfo[idx].mutex.unlock();
				
				for (HeartInfo::InfoType::iterator iter = _heartinfo[idx].tcp_info.begin();
					iter != _heartinfo[idx].tcp_info.end();) {
					if (iter->second->updtime + _beat < now) {
						if (iter->first->IsConnected()) {
							iter->first->Close();
						}
						delete iter->second;
						_heartinfo[idx].tcp_info.erase(iter++);
					}
					else
						++iter;
				}

				for (HeartInfo::InfoType2::iterator iter = _heartinfo[idx].http_info.begin();
					iter != _heartinfo[idx].http_info.end();) {
					if (iter->second->updtime + _beat < now) {
						if (iter->first->IsConnected()) {
							iter->first->Close();
						}
						delete iter->second;
						_heartinfo[idx].http_info.erase(iter++);
					}
					else
						++iter;
				}

				while (tmp_co_tcplist.size()) {
					TcpSocketPtr ptr = tmp_co_tcplist.front();
					HeartInfo::SocketInfo* pinfo = new HeartInfo::SocketInfo;
					pinfo->count = 0;
					pinfo->updtime = now;
					_heartinfo[idx].tcp_info[ptr] = pinfo;
					tmp_co_tcplist.pop_front();
				}
				while (tmp_co_httplist.size()) {
					HttpSocketPtr ptr = tmp_co_httplist.front();
					HeartInfo::SocketInfo* pinfo = new HeartInfo::SocketInfo;
					pinfo->count = 0;
					pinfo->updtime = now;
					_heartinfo[idx].http_info[ptr] = pinfo;
					tmp_co_httplist.pop_front();
				}
				while (tmp_data_tcplist.size()) {
					TcpSocketPtr ptr = tmp_data_tcplist.front();
					HeartInfo::InfoType::iterator iter = _heartinfo[idx].tcp_info.find(ptr);
					if (iter != _heartinfo[idx].tcp_info.end()) {
						iter->second->count += 1;
						iter->second->updtime = now;
					}
					tmp_data_tcplist.pop_front();
				}
				while (tmp_data_httplist.size()) {
					HttpSocketPtr ptr = tmp_data_httplist.front();
					HeartInfo::InfoType2::iterator iter = _heartinfo[idx].http_info.find(ptr);
					if (iter != _heartinfo[idx].http_info.end()) {
						iter->second->count += 1;
						iter->second->updtime = now;
					}
					tmp_data_httplist.pop_front();
				}

				for (HeartInfo::ConnInfoType::iterator iter = _heartinfo[idx].conn_tcp_info.begin();
					iter != _heartinfo[idx].conn_tcp_info.end();) {
					if (iter->second->rcv_time + _beat < now) {
						if (iter->first->IsConnected()) {
							iter->first->Close();
						}
						delete iter->second;
						_heartinfo[idx].conn_tcp_info.erase(iter++);
						continue;
					}
					if (iter->second->snd_time + iter->second->idle < now) {
						iter->first->Send(_conn_snd_buffer.Data(), _conn_snd_buffer.Length());
						iter->second->snd_time = now;
					}
					++iter;
				}

				while (tmp_co_conn_tcplist.size()) {
					HeartInfo::ConnSetInfo info = tmp_co_conn_tcplist.front();
					HeartInfo::ConnSocketInfo* pinfo = new HeartInfo::ConnSocketInfo;
					pinfo->rcv_cnt = 0;
					pinfo->rcv_time = pinfo->snd_time = now;
					pinfo->idle = info.idle;
					_heartinfo[idx].conn_tcp_info[info.ptr] = pinfo;
					tmp_co_conn_tcplist.pop_front();
				}
				while (tmp_data_conn_tcplist.size()) {
					TcpConnectorPtr ptr = tmp_data_conn_tcplist.front();
					HeartInfo::ConnInfoType::iterator iter = _heartinfo[idx].conn_tcp_info.find(ptr);
					if (iter != _heartinfo[idx].conn_tcp_info.end()) {
						iter->second->rcv_cnt += 1;
						iter->second->rcv_time = now;
					}
					tmp_data_conn_tcplist.pop_front();
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
	netiolib::Buffer _conn_snd_buffer;
};

M_NETIO_NAMESPACE_END
#endif
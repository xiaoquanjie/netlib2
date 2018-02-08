#ifndef M_NETIO_MAIL_INCLUDE
#define M_NETIO_MAIL_INCLUDE

#include "base/tool.hpp"
M_NETIO_NAMESPACE_BEGIN

class Mail {
public:
	enum {
		enum_state_null,
		enum_state_connect,
		enum_state_ehlo,
		enum_state_auth,
		enum_state_name,
		enum_state_pwd,
		enum_state_from,
		enum_state_to,
		enum_state_data,
		enum_state_send,
		enum_state_quit,
	};

	Mail();

	~Mail();

	void SetMailSvrIp(const char* ip);

	void SetMailSvrPort(unsigned short port);

	void SetUserName(const char* name);

	void SetPassWord(const char* pwd);

	void SetMailFrom(const char* from);

	void SetMailTo(const std::vector<std::string>& tos);

	void AddMailTo(const char* to);

	void SetData(const char* data, size_t len);

	bool Send();

	const std::string& Error()const;

	void Reset();

protected:
	std::vector<std::pair<int,std::string> > _getip(const char* hostname);

	SocketLib::TcpConnector<SocketLib::IoService>* _connect();

	bool _recv();

protected:
	SocketLib::IoService _ioservice;
	SocketLib::TcpConnector<SocketLib::IoService>* _socket;
	
	SocketLib::Buffer _rcvbuffer;
	SocketLib::s_byte_t* _readbuf;
	SocketLib::s_uint32_t _readsize;

	unsigned int _flag;
	std::string _error;
	std::string _ip;
	unsigned short _port;
	std::string _name;
	std::string _pwd;
	std::string _from;
	std::vector<std::string> _tos;
	SocketLib::Buffer _data;
};

inline Mail::Mail() {
	_readsize = 1024;
	_readbuf = (SocketLib::s_byte_t*)malloc(_readsize);
	_socket = 0;
	_flag = enum_state_null;
	_port = 0;
}

inline Mail::~Mail() {
	delete _socket;
	delete _readbuf;
}

inline void Mail::SetMailSvrIp(const char* ip) {
	_ip = ip;
}

inline void Mail::SetMailSvrPort(unsigned short port) {
	_port = port;
}

inline void Mail::SetUserName(const char* name) {
	_name = name;
}

inline void Mail::SetPassWord(const char* pwd) {
	_pwd = pwd;
}

inline void Mail::SetMailFrom(const char* from) {
	_from = from;
}

inline void Mail::SetMailTo(const std::vector<std::string>& tos) {
	_tos = tos;
}

inline void Mail::AddMailTo(const char* to) {
	_tos.push_back(to);
}

inline void Mail::SetData(const char* data, size_t len) {
	_data.Write(data, len);
}

inline bool Mail::Send() {
	if (_socket)
		return false;
	try {
		do {
			_flag = enum_state_connect;
			_socket = _connect();
			if (!_socket) {
				_error = "can't connect mail server";
				break;
			}
			if (!_recv())
				break;

			_flag = enum_state_ehlo;
			const std::string ehlo = "EHLO " + _ip + std::string("\r\n");
			_socket->SendSome(ehlo.c_str(), ehlo.length());
			if (!_recv())
				break;
			
			_flag = enum_state_auth;
			const std::string auth = "AUTH LOGIN\r\n";
			_socket->SendSome(auth.c_str(), auth.length());
			if (!_recv())
				break;

			_flag = enum_state_name;
			std::string codename;
			if (!base::Base64Encode(_name, codename)) {
				_error = "user name base64 encode error";
				break;
			}
			codename += "\r\n";
			_socket->SendSome(codename.c_str(), codename.length());
			if (!_recv())
				break;

			_flag = enum_state_pwd;
			std::string codepwd;
			if (!base::Base64Encode(_pwd, codepwd)) {
				_error = "user pwd base64 encode error";
				break;
			}
			codepwd += "\r\n";
			_socket->SendSome(codepwd.c_str(), codepwd.length());
			if (!_recv())
				break;

			_flag = enum_state_from;
			const std::string from = "mail from:" + _from + std::string("\r\n");
			_socket->SendSome(from.c_str(), from.length());
			if (!_recv())
				break;

			for (std::vector<std::string>::iterator iter = _tos.begin();
				iter != _tos.end(); ++iter) {
				_flag = enum_state_to;
				const std::string to = "RCPT TO:" + *iter + std::string("\r\n");
				_socket->SendSome(to.c_str(), to.length());
				if (!_recv())
					break;
			}

			_flag = enum_state_data;
			_socket->SendSome("DATA\r\n", 6);
			if (!_recv())
				break;

			_flag = enum_state_send;
			_socket->SendSome(_data.Data(), _data.Length());
			_socket->SendSome("\r\n.\r\n", 5);
			if (!_recv())
				break;

			_flag = enum_state_quit;
			_socket->SendSome("QUIT\r\n", 6);
			delete _socket;
			_socket = 0;
			return true;

		} while (false);
		delete _socket;
		_socket = 0;
	}
	catch (SocketLib::SocketError& error) {
		_error = error.What();
	}
	return false;
}

inline const std::string& Mail::Error()const {
	return _error;
}

inline void Mail::Reset() {
	delete _socket;
	_socket = 0;
	_flag = enum_state_null;
	_error.clear();
	_rcvbuffer.Clear();
	_ip.clear();
	_port = 0;
	_name.clear();
	_pwd.clear();
	_from.clear();
	_tos.clear();
	_data.Clear();
}

inline std::vector<std::pair<int, std::string> > Mail::_getip(const char* hostname) {
	struct addrinfo* result = 0;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	std::vector<std::pair<int, std::string> > pair_result;
	int ret = getaddrinfo(hostname, 0, &hints, &result);
	if (ret == 0) {
		std::pair<int, std::string> tmp_pair;
		for (struct addrinfo* ptr = result; ptr != 0; ptr = ptr->ai_next) {
			switch (ptr->ai_family) {
			case AF_INET:
				tmp_pair.first = 0;
				tmp_pair.second = SocketLib::detail::Util::SockAddrToAddress(ptr->ai_addr);
				pair_result.push_back(tmp_pair);
				break;
			case AF_INET6:
				tmp_pair.first = 1;
				tmp_pair.second = SocketLib::detail::Util::SockAddrToAddress(ptr->ai_addr);
				pair_result.push_back(tmp_pair);
				break;
			}
		}
	}
	return pair_result;
}

inline SocketLib::TcpConnector<SocketLib::IoService>* Mail::_connect() {
	std::vector<std::pair<int, std::string> > ip_pair = _getip(_ip.c_str());
	for (std::vector<std::pair<int, std::string> >::iterator iter = ip_pair.begin();
		iter != ip_pair.end(); ++iter) {
		_socket = new SocketLib::TcpConnector<SocketLib::IoService>(_ioservice);
		SocketLib::SocketError error;
		if (iter->first == 0) {
			SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(iter->second), _port);
			_socket->Connect(ep, error, 5);
		}
		else {
			SocketLib::Tcp::EndPoint ep(SocketLib::AddressV6(iter->second), _port);
			_socket->Connect(ep, error, 5);
		}
		if (!error)
			break;
		else {
			delete _socket;
			_socket = 0;
		}
	}
	return _socket;
}

inline bool Mail::_recv() {
	_rcvbuffer.Clear();
	while (true) {
		SocketLib::s_uint32_t recv_cnt = _socket->RecvSome(_readbuf, _readsize);
		_rcvbuffer.Write(_readbuf, recv_cnt);
		if (recv_cnt<_readsize)
			break;
	}
	// ½á¹û¼ì²é
	if (_flag == enum_state_connect) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("220"))
				return true;
		}
	}
	else if (_flag == enum_state_ehlo) {
		return true;
	}
	else if (_flag == enum_state_auth) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("334"))
				return true;
		}
	}
	else if (_flag == enum_state_name) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("334"))
				return true;
		}
	}
	else if (_flag == enum_state_pwd) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("235"))
				return true;
		}
	}
	else if (_flag == enum_state_from) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("250"))
				return true;
		}
	}
	else if (_flag == enum_state_to) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("250"))
				return true;
		}
	}
	else if (_flag == enum_state_data) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("354"))
				return true;
		}
	}
	else if (_flag == enum_state_send) {
		if (_rcvbuffer.Length() >= 3) {
			std::string code;
			code.append(_rcvbuffer.Data(), 3);
			if (code == std::string("250"))
				return true;
		}
	}
	_error.append(_rcvbuffer.Data(), _rcvbuffer.Length());
	return false;
}

M_NETIO_NAMESPACE_END
#endif
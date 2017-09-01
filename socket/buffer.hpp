#ifndef M_BUFFER_INCLUDE
#define M_BUFFER_INCLUDE

#include "config.hpp"
M_SOCKET_NAMESPACE_BEGIN
#define M_BUFFER_DEFAILT_SIZE 1024

class Buffer
{
public:
	Buffer();

	~Buffer();

	s_byte_t* Data();

	const s_byte_t* Data()const;

	s_uint32_t Capacity()const;

	s_uint32_t Size()const;

	void Write(void* data, s_uint32_t len);

	template<typename T>
	void Write(T const& value);

	template<typename T>
	Buffer& operator<<(T const& value);

	void Read(void* data, s_uint32_t len);

	template<typename T>
	void Read(T& value);

	template<typename T>
	Buffer& operator>>(T& value);

private:
	s_byte_t*	_data;
	s_uint32_t	_size;
	s_uint32_t	_pos;
	s_uint32_t	_offset;
};

Buffer::Buffer()
	:_data(0),_size(0),_pos(0),_offset(0)
{
	_size = M_BUFFER_DEFAILT_SIZE;
	_data = (s_byte_t*)g_malloc(_size);
}

Buffer::~Buffer()
{
	g_free(_data);
}


M_SOCKET_NAMESPACE_END
#endif
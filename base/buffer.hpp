#ifndef M_BUFFER_INCLUDE
#define M_BUFFER_INCLUDE

#include "base/config.hpp"
#include <string>
#include <stdlib.h>
M_BASE_NAMESPACE_BEGIN
#define M_BUFFER_DEFAILT_SIZE 1024

class Buffer
{
public:
	struct _data_ {
		s_uint32_t	_size;
		s_uint32_t	_pos;
		s_uint32_t	_offset;
		s_byte_t*	_data;
	};

	Buffer();

	Buffer(s_uint32_t hdrlen);

	~Buffer();

	void Clear();

	void RemoveData(s_int32_t len);

	s_byte_t* Data();

	const s_byte_t* Data()const;

	s_byte_t* Raw();

	const s_byte_t* Raw()const;

	s_uint32_t Capacity()const;

	s_uint32_t Size()const;

	s_uint32_t Length()const;

	void Write(const void* data, s_uint32_t len);

	template<typename T>
	void Write(T const& value);

	void Write(std::string const& value);

	template<typename T>
	Buffer& operator<<(T const& value){
		Write(value);
		return *this;
	}

	void Read(void* data, s_uint32_t len);

	template<typename T>
	void Read(T& value);

	void Read(std::string& value);

	template<typename T>
	Buffer& operator >> (T& value){
		Read(value);
		return *this;
	}

	void Swap(Buffer& buffer);

protected:
	Buffer(const Buffer&);
	Buffer& operator=(const Buffer&);

private:
	_data_ _data;
};

inline Buffer::Buffer(){
	_data._pos = _data._offset = (0);
	_data._size = M_BUFFER_DEFAILT_SIZE;
	_data._data = (s_byte_t*)malloc(_data._size);
}

inline Buffer::Buffer(s_uint32_t hdrlen) {
	// hdrlen 不能大于M_BUFFER_DEFAILT_SIZE
	_data._pos = _data._offset = (0);
	_data._size = M_BUFFER_DEFAILT_SIZE;
	_data._data = (s_byte_t*)malloc(_data._size);
	if (hdrlen <= _data._size)
		_data._pos += hdrlen;
}

inline Buffer::~Buffer(){
	free(_data._data);
}

inline void Buffer::Clear() {
	_data._pos = _data._offset = (0);
}

inline void Buffer::RemoveData(s_int32_t len) {
	if (_data._offset + len > _data._pos)
		_data._offset = _data._pos;
	_data._offset += len;
}

inline s_byte_t* Buffer::Data(){
	return _data._data + _data._offset;
}

inline const s_byte_t* Buffer::Data()const{
	return _data._data + +_data._offset;
}

inline s_byte_t* Buffer::Raw() {
	return _data._data;
}

inline const s_byte_t* Buffer::Raw()const {
	return _data._data;
}

inline s_uint32_t Buffer::Capacity()const{
	return _data._size;
}

inline s_uint32_t Buffer::Size()const{
	return _data._pos;
}

inline s_uint32_t Buffer::Length()const{
	return (_data._pos - _data._offset);
}

inline void Buffer::Write(const void* data, s_uint32_t len){
	if (_data._pos + len > _data._size){
		_data._size = M_BUFFER_DEFAILT_SIZE*((_data._pos + len) / M_BUFFER_DEFAILT_SIZE + 1);
		s_byte_t* ptmp = (s_byte_t*)malloc(_data._size);
		memcpy(ptmp, _data._data, _data._pos);
		free(_data._data);
		_data._data = ptmp;
	}
	memcpy(_data._data + _data._pos, data, len);
	_data._pos += len;
}

template<typename T>
inline void Buffer::Write(T const& value){
	Write((void*)&value, sizeof(T));
}

inline void Buffer::Write(std::string const& value) {
	s_uint32_t len = value.length();
	Write((void*)&len, sizeof(s_uint32_t));
	Write((void*)value.c_str(), len);
}

inline void Buffer::Read(void* data, s_uint32_t len){
	if (_data._offset + len > _data._pos)
		return;

	memcpy(data, _data._data + _data._offset, len);
	_data._offset += len;
}

template<typename T>
inline void Buffer::Read(T& value){
	Read((void*)&value, sizeof(T));
}

inline void Buffer::Read(std::string& value) {
	s_uint32_t len = 0;
	Read(len);
	if (len > 0) {
		char* tmp = (char*)malloc(len + 1);
		Read(tmp, len);
		tmp[len] = 0;
		value = tmp;
		free(tmp);
	}
}

inline void Buffer::Swap(Buffer& buffer) {
	_data_ d = this->_data;
	this->_data = buffer._data;
	buffer._data = d;
}

M_BASE_NAMESPACE_END
#endif
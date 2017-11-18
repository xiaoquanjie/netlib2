/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/8/10
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_IO_OBJECT_INCLUDE
#define M_IO_OBJECT_INCLUDE

#include "config.hpp"
M_SOCKET_NAMESPACE_BEGIN

template<typename IoObjectService>
class SocketIoObject
{
public:
	typedef typename IoObjectService::IoServiceType IoServiceType;
	typedef typename IoServiceType::Impl ImplType;

	M_SOCKET_DECL IoObjectService& GetObjectService();

	M_SOCKET_DECL const IoObjectService& GetObjectService()const;

	M_SOCKET_DECL IoServiceType& GetIoService() {
		return _ioobjectservice.GetIoService();
	}

	M_SOCKET_DECL IoServiceType& GetIoService()const {
		return _ioobjectservice.GetIoService();
	}

	M_SOCKET_DECL ImplType& GetImpl();

	M_SOCKET_DECL const ImplType& GetImpl()const;

	M_SOCKET_DECL void DestroyHandler(function_t<void()> func);

	M_SOCKET_DECL void SetData(void* data);

	M_SOCKET_DECL void* GetData();

protected:
	M_SOCKET_DECL ~SocketIoObject();

	M_SOCKET_DECL SocketIoObject(IoServiceType& ioservice);

protected:
	SocketIoObject(const SocketIoObject&);
	SocketIoObject& operator=(const SocketIoObject&);

protected:
	IoObjectService	   _ioobjectservice;
	ImplType		   _impl;
	function_t<void()> _func;
	void*			   _data;
};

template<typename IoObjectService>
M_SOCKET_DECL SocketIoObject<IoObjectService>::~SocketIoObject()
{
	_ioobjectservice.Destroy(_impl);
	if (_func)
		_func();
}

template<typename IoObjectService>
M_SOCKET_DECL SocketIoObject<IoObjectService>::SocketIoObject(IoServiceType& ioservice)
	:_ioobjectservice(ioservice), _data(0)
{
}

template<typename IoObjectService>
M_SOCKET_DECL IoObjectService& SocketIoObject<IoObjectService>::GetObjectService()
{
	return _ioobjectservice;
}

template<typename IoObjectService>
M_SOCKET_DECL const IoObjectService& SocketIoObject<IoObjectService>::GetObjectService()const
{
	return _ioobjectservice;
}

template<typename IoObjectService>
M_SOCKET_DECL typename SocketIoObject<IoObjectService>::ImplType& SocketIoObject<IoObjectService>::GetImpl()
{
	return _impl;
}

template<typename IoObjectService>
M_SOCKET_DECL const typename SocketIoObject<IoObjectService>::ImplType& SocketIoObject<IoObjectService>::GetImpl()const
{
	return _impl;
}

template<typename IoObjectService>
M_SOCKET_DECL void SocketIoObject<IoObjectService>::DestroyHandler(function_t<void()> func)
{
	_func = func;
}

template<typename IoObjectService>
M_SOCKET_DECL void SocketIoObject<IoObjectService>::SetData(void* data)
{
	_data = data;
}

template<typename IoObjectService>
M_SOCKET_DECL void* SocketIoObject<IoObjectService>::GetData()
{
	return _data;
}


M_SOCKET_NAMESPACE_END
#endif
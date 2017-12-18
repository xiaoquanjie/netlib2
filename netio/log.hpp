/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// 修改人：xiaoquanjie
// 时间：2017/11/11
//
// 修改人：xiaoquanjie
// 时间：
// 修改说明：
//
// 版本：V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_LOG_INCLUDE
#define M_NETIO_LOG_INCLUDE

#include "netio/config.hpp"
M_NETIO_NAMESPACE_BEGIN

static void set_netio_loginterface();

#define M_NETIO_LOGGER(info) 
#define M_ERROR_DESC_STR(error) " code("<<error.Code()<<"),what("<<error.What()<<") "

M_NETIO_NAMESPACE_END
#endif
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

#ifndef M_PROTOCOL_HDR_INCLIDE
#define M_PROTOCOL_HDR_INCLIDE

#include "socket/config.hpp"
M_SOCKET_NAMESPACE_BEGIN


// 20 bytes，由固定头部加选项(最多40字节)组成
typedef struct _ip_hdr_t
{
	s_uint8_t  HlenVer;		// 4位头部长部 + 4位版本号,头部以4个字节为单位
	s_uint8_t  Tos;			// 8位服务类型TOS
	s_uint16_t TotalLen;		// 16位总长度,包括头部
	s_uint16_t Ident;			// 16位标识
	s_uint16_t FragFlags;// 16位标志位
	s_uint8_t  Ttl;			// 生存时间
	s_uint8_t  Protocol;			// 协议
	s_uint16_t CheckSum;		// 校验和
	s_uint32_t SrcIp;
	s_uint32_t DstIp;
}ip_hdr_t;

// icmp协议的公共报头
typedef struct _icmp_hdr_t
{
	s_uint8_t Type;		// 类型码
	s_uint8_t Code;		// 代码
	s_uint16_t CheckSum;		// 校验和
}icmp_hdr_t;

// icmp报文件类型,不是报头
typedef struct _icmp_type_t
{
	s_uint32_t  Type;		// 类型
	s_uint32_t  Code;		// 代码
	const char* Desc;		// 描述
}icmp_type_t;

static const icmp_type_t gIcmpTypeTab[]=
{
	{0,0,"回显应答"},		// 0
	{3,0,"网络不可达"},
	{3,1,"主机不可达"},
	{3,2,"协议不可达"},
	{3,3,"端口不可达"},
	{3,4,"需要进行分片但设置了不分片比特"}, // 5
	{3,5,"源站选路失败"},
	{3,6,"目的网络不认识"},
	{3,7,"目的主机不认识"},
	{3,8,"源主机被隔离"},
	{3,9,"目的网络被强制禁止"},	// 10
	{3,10,"目的主机被强制禁止"},
	{3,11,"由于服务类型TOS，网络不可达"},
	{3,12,"由于服务类型TOS，主机不可达"},
	{3,13,"由于过滤，通信被强制禁止"},
	{3,14,"主机越权"}, // 15
	{4,0,"源端被关闭"},
	{5,0,"对网络重定向"},
	{5,1,"对主机重定向"},
	{5,2,"对服务类型和网络重定向"},
	{5,3,"对服务类型和主机重定向"}, // 20
	{8,0,"请求回显"},
	{9,0,"路由通告"},
	{10,0,"路由请求"},
	{11,0,"传输期间生存时间为0"},
	{11,1,"在数据报组装期间生存时间为0"}, // 25
	{12,0,"坏的IP首部"},
	{12,1,"缺少必需的选项"},
	{13,0,"时间戳请求"},
	{14,0,"时间戳应答"},
	{15,0,"信息请求(作废不用)"}, // 30
	{16,0,"信息应答(作废不用)"},
	{17,0,"地址掩码请求"},
	{18,0,"地址掩码应答"}, // 33
};

// echo and echo_reply header
typedef struct _icmp_echo_hdr_t : public icmp_hdr_t
{
	s_uint16_t Ident;		// 标识符
	s_uint16_t Seq;			// 序列号
}icmp_echo_hdr_t;

// 时间戳请求及应答
typedef struct _icmp_tstamp_hdr_t : public icmp_hdr_t
{
	s_uint16_t Ident;		// 标识符
	s_uint16_t Seq;			// 序列号
	s_uint32_t ReqTime;	// 发起时间
	s_uint32_t recv_time;	// 收到时间
	s_uint32_t TransTime;	// 传送时间
}icmp_tstamp_hdr_t;

// 地址掩码请求及应答
typedef struct _icmp_mask_hdr_t : public icmp_hdr_t
{
	s_uint16_t Ident;		// 标识符
	s_uint16_t Seq;			// 序列号
	s_uint32_t Mask;		// 掩码

}icmp_mask_hdr_t;

M_SOCKET_NAMESPACE_END
#endif
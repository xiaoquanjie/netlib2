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

#ifndef M_HDR_HELPER_INCLUDE
#define M_HDR_HELPER_INCLUDE

#include "socket/pro_hdr.hpp"
M_SOCKET_NAMESPACE_BEGIN

class HdrHelper
{
public:

	virtual ~HdrHelper() {}

	// 计算校验码，算法适用于IPV4，ICMPV4，ICMPV6，IGMPV4，UDP，TCP
	static s_uint16_t CheckSum(const s_uint16_t* pAddr, const s_int32_t aiLen)
	{
		s_int32_t   liLeft = aiLen;
		s_uint32_t  liSum = 0;
		const s_uint16_t* pW = pAddr;
		s_uint16_t  liAnswer = 0;

		while (liLeft > 1)
		{
			liSum += *pW++;
			liLeft -= 2;
		}

		if (liLeft == 1)
		{
			*(s_uint8_t*)(&liAnswer) = *(s_uint8_t*)pW;
			liSum += liAnswer;
		}

		liSum = (liSum >> 16) + (liSum & 0xffff);
		liSum += liSum >> 16;
		liAnswer = (s_uint16_t)~liSum;
		return (liAnswer);
	}

	inline static s_int32_t GetIcmpTypeCnt()
	{
		return (sizeof(gIcmpTypeTab) / sizeof(icmp_type_t));
	}

	static const icmp_type_t* GetIcmpType(s_int32_t aiIdx)
	{
		if (aiIdx >= 0 && aiIdx < GetIcmpTypeCnt())
			return &gIcmpTypeTab[aiIdx];

		return 0;
	}
};

class IpHdrHelper : public HdrHelper
{
public:

	IpHdrHelper(void* pBuf) :_pBuf(pBuf)
	{
		assert(_pBuf);
	}

	// 获取版本号
	s_uint32_t GetVersion()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		s_uint8_t liVer = pHdr->HlenVer >> 4; // 取高4位
		return (liVer);
	}

	void SetVersion(const s_uint32_t aiVer)
	{
		// 存到高4位
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		s_uint8_t liVer = aiVer & 0xf;				// 截断
		liVer <<= 4;
		pHdr->HlenVer &= 0xf;
		pHdr->HlenVer |= liVer;
	}

	// 获取头部长度
	s_uint32_t GetHdrLen()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		s_uint8_t liLen = pHdr->HlenVer & 0xf; // 取低4位
		return (liLen << 2);
	}

	void SetHdrLen(const s_uint32_t aiLen)
	{
		// 存到低4位
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		s_uint32_t liLen = aiLen >> 2;
		s_uint8_t liLen2 = liLen & 0xf;	// 截断
		pHdr->HlenVer &= 0xf0;
		pHdr->HlenVer |= liLen2;
	}

	s_uint8_t GetTos()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (pHdr->Tos);
	}

	void SetTos(const s_uint16_t aiTos)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->Tos = (s_uint8_t)(aiTos);
	}

	s_uint16_t GetTotalLen()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->TotalLen));
	}

	void SetTotalLen(const s_uint16_t aiLen)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->TotalLen = g_htons(aiLen);
	}

	s_uint16_t GetIdent()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->Ident));
	}

	void SetIdent(const s_uint16_t aiId)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->Ident = g_htons(aiId);
	}

	s_uint16_t GetFragFlags()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->FragFlags));
	}

	void SetFragFlags(const s_uint16_t aiFF)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->FragFlags = g_htons(aiFF);
	}

	s_uint8_t GetTtl()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (pHdr->Ttl);
	}

	void SetTtl(const s_uint16_t aiTtl)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->Ttl = (s_uint8_t)(aiTtl);
	}

	s_uint8_t GetProtocol()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (pHdr->Protocol);
	}

	void SetProtocol(const s_uint16_t aiPro)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->Protocol = (s_uint8_t)(aiPro);
	}

	s_uint16_t GetCheckSum()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->CheckSum));
	}

	void SetCheckSum(const s_uint16_t aiSum)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->CheckSum = g_htons(aiSum);
	}

	s_uint32_t TetSrcIp()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (g_ntohl(pHdr->SrcIp));
	}

	void SetSrcIp(const s_uint32_t aiIp)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->SrcIp = g_htonl(aiIp);
	}

	s_uint32_t GetDstIp()const
	{
		const ip_hdr_t* pHdr = (const ip_hdr_t*)_pBuf;
		return (g_ntohl(pHdr->DstIp));
	}

	void SetDstIp(const s_uint32_t aiIp)
	{
		ip_hdr_t* pHdr = (ip_hdr_t*)_pBuf;
		pHdr->DstIp = g_htonl(aiIp);
	}

protected:
	void* _pBuf; // 内容指向
};

class IcmpHdrHelper : public HdrHelper
{
public:
	IcmpHdrHelper(void* pBuf) :_pBuf(pBuf)
	{
		assert(_pBuf);
	}

	s_uint8_t GetType()const
	{
		const icmp_hdr_t* pHdr = (const icmp_hdr_t*)_pBuf;
		return (pHdr->Type);
	}

	void SetType(const s_uint32_t aiType)
	{
		icmp_hdr_t* pHdr = (icmp_hdr_t*)_pBuf;
		pHdr->Type = (s_uint8_t)(aiType);
	}

	s_uint8_t GetCode()const
	{
		const icmp_hdr_t* pHdr = (const icmp_hdr_t*)_pBuf;
		return (pHdr->Code);
	}

	void SetCode(const s_uint32_t aiCode)
	{
		icmp_hdr_t* pHdr = (icmp_hdr_t*)_pBuf;
		pHdr->Code = (s_uint8_t)(aiCode);
	}

	s_uint16_t GetCheckSum()const
	{
		const icmp_hdr_t* pHdr = (const icmp_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->CheckSum));
	}

	void SetCheckSum(const s_uint16_t aiSum)
	{
		icmp_hdr_t* pHdr = (icmp_hdr_t*)_pBuf;
		pHdr->CheckSum = g_htons(aiSum);
	}

protected:
	void* _pBuf;
};

class IcmpEchoHelper : public IcmpHdrHelper
{
public:
	IcmpEchoHelper(void* buf) :IcmpHdrHelper(buf) {}

	s_uint16_t GetIdent()const
	{
		const icmp_echo_hdr_t* pHdr = (const icmp_echo_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->Ident));
	}

	void SetIdent(const s_uint16_t aiId)
	{
		icmp_echo_hdr_t* pHdr = (icmp_echo_hdr_t*)_pBuf;
		pHdr->Ident = g_htons(aiId);
	}

	s_uint16_t GetSeq()const
	{
		const icmp_echo_hdr_t* pHdr = (const icmp_echo_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->Seq));
	}

	void SetSeq(const s_uint16_t aiSeq)
	{
		icmp_echo_hdr_t* pHdr = (icmp_echo_hdr_t*)_pBuf;
		pHdr->Seq = g_htons(aiSeq);
	}

	const char* GetOpt()const
	{
		const icmp_echo_hdr_t* pHdr = (const icmp_echo_hdr_t*)_pBuf;
		return (const char*)(pHdr + 1);
	}

	void SetOpt(const char* pOpt, const s_uint32_t aiLen)
	{
		icmp_echo_hdr_t* pHdr = (icmp_echo_hdr_t*)_pBuf;
		char* p = (char*)(pHdr + 1);
		g_strncpy(p, pOpt, aiLen);
	}
};

class IcmpTStampHelper : public IcmpHdrHelper
{
public:
	IcmpTStampHelper(void* pBuf) :IcmpHdrHelper(pBuf) {}

	s_uint16_t GetIdent()const
	{
		const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->Ident));
	}

	void SetIdent(const s_uint16_t aiId)
	{
		icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
		pHdr->Ident = g_htons(aiId);
	}

	s_uint16_t GetSeq()const
	{
		const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->Seq));
	}

	void SetSeq(const s_uint16_t aiSeq)
	{
		icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
		pHdr->Seq = g_htons(aiSeq);
	}

	s_uint32_t GetReqTime()const
	{
		const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
		return (g_ntohl(pHdr->ReqTime));
	}

	void SetReqTime(const s_uint32_t aiTime)
	{
		icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
		pHdr->ReqTime = g_htonl(aiTime);
	}

	s_uint32_t GetRecvTime()const
	{
		const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
		return (g_ntohl(pHdr->recv_time));
	}

	void SetRecvTime(const s_uint32_t aiTime)
	{
		icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
		pHdr->recv_time = g_htonl(aiTime);
	}

	s_uint32_t GetTransTime()const
	{
		const icmp_tstamp_hdr_t* pHdr = (const icmp_tstamp_hdr_t*)_pBuf;
		return (g_ntohl(pHdr->TransTime));
	}

	void SetTransTime(const s_uint32_t aiTime)
	{
		icmp_tstamp_hdr_t* pHdr = (icmp_tstamp_hdr_t*)_pBuf;
		pHdr->TransTime = g_htonl(aiTime);
	}
};

class IcmpMaskHelper : public IcmpHdrHelper
{
public:
	IcmpMaskHelper(void* pBuf) :IcmpHdrHelper(pBuf) {}

	s_uint16_t GetIdent()const
	{
		const icmp_mask_hdr_t* pHdr = (const icmp_mask_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->Ident));
	}

	void SetIdent(const s_uint16_t aiId)
	{
		icmp_mask_hdr_t* pHdr = (icmp_mask_hdr_t*)_pBuf;
		pHdr->Ident = g_htons(aiId);
	}

	s_uint16_t GetSeq()const
	{
		const icmp_mask_hdr_t* pHdr = (const icmp_mask_hdr_t*)_pBuf;
		return (g_ntohs(pHdr->Seq));
	}

	void SetSeq(const s_uint16_t aiSeq)
	{
		icmp_mask_hdr_t* pHdr = (icmp_mask_hdr_t*)_pBuf;
		pHdr->Seq = g_htons(aiSeq);
	}

	s_uint32_t GetMask()const
	{
		const icmp_mask_hdr_t* pHdr = (const icmp_mask_hdr_t*)_pBuf;
		return (g_ntohl(pHdr->Mask));
	}

	void SetMask(const s_uint32_t aiMask)
	{
		icmp_mask_hdr_t* pHdr = (icmp_mask_hdr_t*)_pBuf;
		pHdr->Mask = g_htonl(aiMask);
	}
};

M_SOCKET_NAMESPACE_END
#endif
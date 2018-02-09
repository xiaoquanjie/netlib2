#ifndef M_BASE_TOOL_INCLUDE
#define M_BASE_TOOL_INCLUDE

#include "base/config.hpp"
#include <string>
M_BASE_NAMESPACE_BEGIN

#define M_ATL_BASE64_FLAG_NONE	0
#define M_ATL_BASE64_FLAG_NOPAD	1
#define M_ATL_BASE64_FLAG_NOCRLF  2

inline bool Base64Encode(const std::string& input, std::string& output) {
	if (input.empty())
		return false;

	static const char s_chBase64EncodingTable[64] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
		'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',	'h',
		'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

	const char* pbSrcData = input.c_str();
	int nSrcLen = input.length();
	unsigned int dwFlags = M_ATL_BASE64_FLAG_NONE;
	int nWritten(0);
	int nLen1((nSrcLen / 3) * 4);
	int nLen2(nLen1 / 76);
	int nLen3(19);

	for (int i = 0; i <= nLen2; i++){
		if (i == nLen2)
			nLen3 = (nLen1 % 76) / 4;

		for (int j = 0; j < nLen3; j++){
			DWORD dwCurr(0);
			for (int n = 0; n < 3; n++){
				dwCurr |= *pbSrcData++;
				dwCurr <<= 8;
			}
			for (int k = 0; k < 4; k++){
				BYTE b = (BYTE)(dwCurr >> 26);
				output.push_back(s_chBase64EncodingTable[b]);
				dwCurr <<= 6;
			}
		}
		nWritten += nLen3 * 4;

		if ((dwFlags & M_ATL_BASE64_FLAG_NOCRLF) == 0){
			output.push_back('\r');
			output.push_back('\n');
			nWritten += 2;
		}
	}

	if (nWritten && (dwFlags & M_ATL_BASE64_FLAG_NOCRLF) == 0){
		output.pop_back();
		output.pop_back();
		nWritten -= 2;
	}

	nLen2 = (nSrcLen % 3) ? (nSrcLen % 3 + 1) : 0;
	if (nLen2){
		DWORD dwCurr(0);
		for (int n = 0; n < 3; n++){
			if (n < (nSrcLen % 3))
				dwCurr |= *pbSrcData++;
			dwCurr <<= 8;
		}
		for (int k = 0; k < nLen2; k++){
			BYTE b = (BYTE)(dwCurr >> 26);
			output.push_back(s_chBase64EncodingTable[b]);
			dwCurr <<= 6;
		}
		nWritten += nLen2;
		if ((dwFlags & M_ATL_BASE64_FLAG_NOPAD) == 0){
			nLen3 = nLen2 ? 4 - nLen2 : 0;
			for (int j = 0; j < nLen3; j++){
				output.push_back('=');
			}
			nWritten += nLen3;
		}
	}
	return true;
}

inline int DecodeBase64Char(unsigned int ch) {
	if (ch >= 'A' && ch <= 'Z')
		return ch - 'A' + 0;	// 0 range starts at 'A'
	if (ch >= 'a' && ch <= 'z')
		return ch - 'a' + 26;	// 26 range starts at 'a'
	if (ch >= '0' && ch <= '9')
		return ch - '0' + 52;	// 52 range starts at '0'
	if (ch == '+')
		return 62;
	if (ch == '/')
		return 63;
	return -1;
}

inline bool Base64Decode(const std::string& input, std::string& output) {
	if (input.empty())
		return false;

	const char* szSrc = input.c_str();
	const char* szSrcEnd = szSrc + input.length();

	while (szSrc < szSrcEnd && (*szSrc) != 0){
		DWORD dwCurr = 0;
		int i;
		int nBits = 0;
		for (i = 0; i < 4; i++){
			if (szSrc >= szSrcEnd)
				break;
			int nCh = DecodeBase64Char(*szSrc);
			szSrc++;
			if (nCh == -1){
				// skip this char
				i--;
				continue;
			}
			dwCurr <<= 6;
			dwCurr |= nCh;
			nBits += 6;
		}

		// dwCurr has the 3 bytes to write to the output buffer
		// left to right
		dwCurr <<= 24 - nBits;
		for (i = 0; i < nBits / 8; i++){
			output.push_back((BYTE)((dwCurr & 0x00ff0000) >> 16));
			dwCurr <<= 8;
		}
	}
	return true;
}

M_BASE_NAMESPACE_END
#endif
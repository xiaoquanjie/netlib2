#ifndef M_BASE_TOOL_INCLUDE
#define M_BASE_TOOL_INCLUDE

#include "base/config.hpp"
#include <string>
M_BASE_NAMESPACE_BEGIN

inline bool Base64Encode(const std::string& input, std::string& output) {
	static const char* encodemap = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	size_t in_len = input.length();
	size_t loop_count = in_len / 3;
	size_t left_count = in_len % 3;
	size_t out_count = left_count == 0 ? loop_count * 4 : (loop_count + 1) * 4;

	output.clear();
	output.reserve(out_count);

	// 3个字节换4个字符
	for (size_t i = 0; i < loop_count; ++i){
		char c1 = input[3 * i];
		char c2 = input[3 * i + 1];
		char c3 = input[3 * i + 2];

		output.push_back(encodemap[c1 >> 2 & 0x3F]);
		output.push_back(encodemap[(c1 << 4 & 0x30) | (c2 >> 4 & 0xF)]);
		output.push_back(encodemap[(c2 << 2 & 0x3C) | (c3 >> 6 & 0x3)]);
		output.push_back(encodemap[c3 & 0x3F]);
	}

	// 最后剩一个或两个字符处理
	if (left_count == 1){
		char c1 = input.back();
		output.push_back(encodemap[c1 >> 2 & 0x3F]);
		output.push_back(encodemap[c1 << 4 & 0x30]);
		output.push_back('=');
		output.push_back('=');
	}
	else if (left_count == 2){
		char c1 = *(++input.rbegin());
		char c2 = input.back();
		output.push_back(encodemap[c1 >> 2 & 0x3F]);
		output.push_back(encodemap[(c1 << 4 & 0x30) | (c2 >> 4 & 0xF)]);
		output.push_back(encodemap[(c2 << 2 & 0x3C)]);
		output.push_back('=');
	}
	return true;
}

M_BASE_NAMESPACE_END
#endif
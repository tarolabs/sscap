#ifndef _BASE64_HEADER_FILE_
#define _BASE64_HEADER_FILE_
#pragma once

/** @brief base64的编解类工具.
* 以前使用的编解码函数,如果是其它程序进行编码后,并且将结尾的==号去掉后, 我们再解码时, 会丢失结尾的一些字符.这个函数处理了这个问题.
* 比如 ss://YWVzLTI1Ni1jZmI6dGVzdHRAMS4yLjQuNDoxMjM0NQ== ( 对应 aes-256-cfb:testt@1.2.4.4:12345 ) 
* 是其它程序编码的,然后把后边的== 去掉变成: ss://YWVzLTI1Ni1jZmI6dGVzdHRAMS4yLjQuNDoxMjM0NQ, 我们解出来是: aes-256-cfb:testt@1.2.4.4:1234 
* 少了5.
* 
* 这个函数不会出现这个情况
*/

#include <string>

namespace base64{
	std::string encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	std::string decode(std::string const& encoded_string);
}

#endif
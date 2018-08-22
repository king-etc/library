#pragma once

#include <vector>

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);
std::string EncodeBase58(const std::vector<unsigned char>& vch);
bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet);
bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet);
std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn);
bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet);
bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet);

#ifndef AUTO_OPERATOR_COMMON_H
# define AUTO_OPERATOR_COMMON_H
#include <string>
#include <vector>

void readLines(std::vector<std::string>& commandLines, const std::string& filePath, const int startLine, const int endLine);
std::vector<std::string> split(const std::string& str, const char delimiter = ' ');
std::string strip(const std::string& str);
std::vector<std::string> slice(const std::vector<std::string>& vec, const int start, const int end = 0x7fffffff);
#endif
#pragma once
#include <string>

/**
 * @brief 字符串/通用工具
 */
namespace Utils {

/* 转小写：用于搜索、大小写无关比较 */
std::string ToLower(const std::string& s);

/* 判断字符串是否是合法数字（整数或小数）*/
bool IsNumber(const std::string& s);

} // namespace Utils

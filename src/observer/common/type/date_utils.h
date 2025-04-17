#pragma once

#include <cstdint>
#include <string>
#include "common/rc.h" // 假设 RC 枚举在这里定义

namespace common {
namespace type {

// 检查是否为闰年
bool is_leap_year(int year);

// 将 "YYYY-MM-DD" 格式的字符串转换为自公元 0000-01-01 以来的天数 (int64_t)
// 返回 RC::SUCCESS 表示成功，RC::INVALID_ARGUMENT 或 RC::FAILURE 表示失败
RC string_to_days(const char *str, int64_t &days);

// 将自公元 0000-01-01 以来的天数 (int64_t) 转换为 "YYYY-MM-DD" 格式的字符串
std::string days_to_string(int64_t days);

// 获取某年某月的天数
int days_in_month(int year, int month);

} // namespace type
} // namespace common 
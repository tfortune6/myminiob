#include "date_utils.h"
#include <cstdio>
#include <stdexcept>
#include <limits>
#include <cstring> // For strlen, strncpy
#include <cstdlib> // For atoi
#include <cctype>  // For isdigit
#include "common/log/log.h" // 假设日志头文件

// 如果没有定义 RC::FAILURE，我们在这里简单定义一个
#ifndef RC_FAILURE_DEFINED
#define RC_FAILURE_DEFINED
namespace common {
// 假设 RC 定义类似这样
// enum class RC { SUCCESS = 0, FAILURE = -1, INVALID_ARGUMENT = -2, ... };
// 为了编译通过，如果 rc.h 没有 FAILURE，临时添加
// 请根据实际情况调整
// if (!defined(FAILURE)) {
//  constexpr int FAILURE = -1; // 或者使用 RC::FAILURE
// }
// 更好的方式是检查 RC 定义
// #include "common/rc.h" // 确保包含了 RC 定义
// static_assert(std::is_enum_v<common::RC>, "RC should be an enum");
// 这里我们假设 RC::FAILURE 存在
}
#endif


namespace common {
namespace type {

// 累积每月天数（非闰年）
const int CUMULATIVE_DAYS[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
// 累积每月天数（闰年）
const int CUMULATIVE_DAYS_LEAP[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

bool is_leap_year(int year) {
    // 注意：公元0年不存在，但为了计算方便，可以假设存在并为闰年，或从公元1年开始。
    // 格里高利历规则：能被4整除但不能被100整除，或者能被400整除。
    // 这里我们处理常见的正数年份。对于负数年份或非常大的年份，规则可能需要调整。
    // 考虑到题目要求可能小于1970，我们至少要支持到公元1年。
    if (year <= 0) return false; // 或者根据具体需求处理公元前的闰年规则
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month(int year, int month) {
    if (month < 1 || month > 12) {
        return 0; // 无效月份
    }
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}


RC string_to_days(const char *str, int64_t &days) {
    int year = 0, month = 0, day = 0;
    // 尝试解析 YYYY-MM-DD 格式
    int ret = std::sscanf(str, "%d-%d-%d", &year, &month, &day);

    if (ret != 3) {
        // 可以增加对其他格式的支持，例如 YYYYMMDD
        if (std::strlen(str) == 8) {
             bool all_digits = true;
             for(int i=0; i<8; ++i) {
                 if (!std::isdigit(str[i])) {
                     all_digits = false;
                     break;
                 }
             }
             if (all_digits) {
                 char year_str[5], month_str[3], day_str[3];
                 std::strncpy(year_str, str, 4); year_str[4] = '\0';
                 std::strncpy(month_str, str + 4, 2); month_str[2] = '\0';
                 std::strncpy(day_str, str + 6, 2); day_str[2] = '\0';
                 year = std::atoi(year_str);
                 month = std::atoi(month_str);
                 day = std::atoi(day_str);
                 ret = 3; // 认为解析成功
             }
        }
        if (ret != 3) {
            // 检查是否为NULL字面量
            if (common::string_case_cmp(str, "NULL") == 0) {
                // 如何处理 NULL？通常由上层处理，这里返回错误或特定值
                // LOG_DEBUG("Input string is NULL literal");
                // days = 0; // 或者其他表示 NULL 的哨兵值？
                // return RC::SUCCESS; // ? 这取决于如何定义 NULL
                 return RC::INVALID_ARGUMENT; // 认为这不是一个有效的日期
            }
            LOG_WARN("Invalid date format: %s. Expected YYYY-MM-DD or YYYYMMDD.", str);
            return RC::INVALID_ARGUMENT; // 使用 INVALID_ARGUMENT 可能更合适
            // return RC::FAILURE; // 或者直接用 FAILURE
        }
    }


    // 基本范围检查
    // 题目没有限制范围，但 int 一般有限制，我们假设年份在合理范围内，例如 1 到 9999
    // 如果需要支持公元前或更大的年份，需要调整
    if (year < 1 || year > 9999) { // 调整此范围以匹配需求，int64_t 足够大
        LOG_WARN("Year out of range [1, 9999]: %d", year);
        return RC::INVALID_ARGUMENT;
    }
    if (month < 1 || month > 12) {
        LOG_WARN("Invalid month: %d", month);
        return RC::INVALID_ARGUMENT;
    }
    int max_day = days_in_month(year, month);
    if (day < 1 || day > max_day) {
        LOG_WARN("Invalid day %d for month %d in year %d (max: %d)", day, month, year, max_day);
        return RC::INVALID_ARGUMENT;
    }

    // 计算自公元 0000-01-01 以来的天数 (假设公元0年存在且为闰年，或从公元1年开始计算)
    // 选择从公元1年1月1日开始计算更符合常规历法，记作第1天
    // year 0 不存在，所以 year - 1
    int y = year - 1; // 计算之前有多少个完整的年份

    // 计算之前所有年份的总天数
    // 每400年一个周期：(97个闰年 * 366天) + (303个平年 * 365天)
    // 或者简单计算：y个365天 + 期间的闰年数
    // 闰年数 = y/4 - y/100 + y/400
    int64_t total_days = static_cast<int64_t>(y) * 365LL + (y / 4) - (y / 100) + (y / 400);

    // 加上当年已经过去的天数
    total_days += (is_leap_year(year) ? CUMULATIVE_DAYS_LEAP[month - 1] : CUMULATIVE_DAYS[month - 1]);
    total_days += day;

    // 检查溢出 (对于 int64_t，基本不会发生在此日期计算中)
    // if (total_days < 0) { // 理论上的检查
    //    LOG_ERROR("Date calculation resulted in overflow for %s", str);
    //    return RC::FAILURE;
    // }

    days = total_days;
    return RC::SUCCESS;
}


// 将天数转换回 YYYY-MM-DD (这是一个简化的实现，可能需要更鲁棒的库)
// 参考: https://howardhinnant.github.io/date_algorithms.html
std::string days_to_string(int64_t days) {
    if (days <= 0) {
         // 处理公元前的日期或无效输入
         // 为了简化，我们可能只支持公元后的日期
         LOG_WARN("Cannot convert non-positive days (%lld) to YYYY-MM-DD string. Returning default.", days);
         // 根据 SQL 标准或数据库行为，无效日期可能返回 NULL 或特定字符串
         return "0000-00-00"; // 或者返回 "NULL"? 需要确认行为
    }

    // 使用 Howard Hinnant 的算法 (公共领域)
    days -= 719528; // 将纪元从 0000-03-01 调整到 1970-01-01 (Unix epoch), 再调整到 March 1, 0000 (方便计算)
                    // 719528 = days from 0000-03-01 to 1970-01-01
                    // 如果我们的基准是 0001-01-01，需要调整这个偏移量
                    // days_from_0001_01_01 = days - 1 (因为我们从第1天开始计数)
    int64_t era = (days >= 0 ? days : days - 146096) / 146097; // 每个 era 是 400 年
    int64_t doe = days - era * 146097;                          // day of era [0, 146096]
    int64_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365; // year of era [0, 399]
    int year = static_cast<int>(yoe + era * 400);
    int64_t doy = doe - (365*yoe + yoe/4 - yoe/100);             // day of year [0, 365]
    int64_t mp = (5*doy + 2)/153;                                // month_prime [0, 11] (March = 0)
    int day = static_cast<int>(doy - (153*mp + 2)/5 + 1);        // day [1, 31]
    int month = static_cast<int>(mp + (mp < 10 ? 3 : -9));        // month [1, 12]

    year += (month <= 2); // 如果月份是1或2，年份加1 (因为计算从3月开始)

    char buf[11]; // YYYY-MM-DD\0
    // 对于小于1000年的情况，确保补零
    if (year < 0 || year > 9999) {
        LOG_WARN("Year %d out of standard range [0001, 9999] after conversion from days %lld", year, days + 719528);
        // 可能需要返回错误或特定格式
        // 临时处理：如果需要支持更大范围，snprintf可能需要调整格式或使用其他库
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", std::max(0, std::min(9999, year)), month, day); // 限制在 0000-9999
    } else {
       std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    }
    return std::string(buf);
}


} // namespace type
} // namespace common


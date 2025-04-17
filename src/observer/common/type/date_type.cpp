#include "common/type/date_type.h"
#include "common/value.h"
#include "common/lang/comparator.h" // 包含 compare_date
#include "common/log/log.h"

namespace common {
namespace type {

int DateType::compare(const Value &left, const Value &right) const {
    if (left.attr_type() != AttrType::DATES || right.attr_type() != AttrType::DATES) {
        // 尝试进行类型转换，如果右侧是字符串
        if (right.attr_type() == AttrType::CHARS) {
            Value right_casted;
            int64_t days;
            RC rc = string_to_days(right.get_string().c_str(), days);
            if (OB_SUCCESS(rc)) {
                right_casted.set_date(days);
                int64_t left_days = left.get_date();
                return common::compare_date(&left_days, &days);
            } else {
                 LOG_WARN("Failed to cast CHARS to DATE for comparison. Right: %s", right.to_string().c_str());
                 // 无法比较，抛出异常或返回错误
                 throw common::Exception(OB_INVALID_ARGUMENT, "Cannot compare DATE with uncastable CHARS: %s", right.to_string().c_str());
            }
        } else if (left.attr_type() == AttrType::CHARS && right.attr_type() == AttrType::DATES) {
             // 如果左侧是字符串，尝试转换左侧
            Value left_casted;
            int64_t days;
            RC rc = string_to_days(left.get_string().c_str(), days);
             if (OB_SUCCESS(rc)) {
                left_casted.set_date(days);
                int64_t right_days = right.get_date();
                return common::compare_date(&days, &right_days);
            } else {
                 LOG_WARN("Failed to cast CHARS to DATE for comparison. Left: %s", left.to_string().c_str());
                 throw common::Exception(OB_INVALID_ARGUMENT, "Cannot compare uncastable CHARS with DATE: %s", left.to_string().c_str());
            }
        }

        LOG_WARN("Comparing incompatible types with DATE. Left: %s, Right: %s",
                 attr_type_to_string(left.attr_type()), attr_type_to_string(right.attr_type()));
         throw common::Exception(OB_INVALID_ARGUMENT, "Cannot compare DATE with %s", attr_type_to_string(right.attr_type()));
        //return 0; // 或者根据基类约定返回
    }
    int64_t left_days = left.get_date(); // 使用 get_date() 获取天数
    int64_t right_days = right.get_date();
    // 调用全局的 compare_date 函数
    return common::compare_date(&left_days, &right_days);
}

RC DateType::to_string(const Value &value, string &out) const {
    if (value.attr_type() != AttrType::DATES) {
         LOG_WARN("Trying to convert non-DATE type (%s) to string using DateType.", attr_type_to_string(value.attr_type()));
         out = ""; // 或者返回错误
         return RC::INVALID_ARGUMENT;
    }
    int64_t days = value.get_date();
    out = common::type::days_to_string(days);
    return RC::SUCCESS;
}

// 实现其他需要的方法...
// RC DateType::cast_to(...) const { ... }
// RC DateType::add(...) const { ... }
// RC DateType::subtract(...) const { ... }
// RC DateType::serialize(...) const { ... }
// RC DateType::deserialize(...) const { ... }


} // namespace type
} // namespace common 
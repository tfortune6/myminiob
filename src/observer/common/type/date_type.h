#pragma once

#include "common/type/data_type.h"
#include "common/type/date_utils.h" // 需要日期工具函数

namespace common {
namespace type {

class DateType : public DataType {
public:
    DateType() : DataType(AttrType::DATES, sizeof(int64_t), "DATE") {}
    ~DateType() override = default;

    // 覆盖比较方法
    int compare(const Value &left, const Value &right) const override;

    // 覆盖转换为字符串方法
    RC to_string(const Value &value, string &out) const override;

    // 覆盖类型转换方法
    RC cast_to(const Value &value, AttrType to_type, Value &result) const override;

    // 可能需要覆盖的其他方法 (根据 DataType 基类接口决定)
    // 例如: cast_to, add, subtract 等 (日期加减可能需要实现)
    // RC cast_to(const Value &value, AttrType to_type, Value &result) const override;
    // RC add(const Value &left, const Value &right, Value &result) const override;
    // RC subtract(const Value &left, const Value &right, Value &result) const override;

    // 序列化/反序列化（如果 DataType 有定义）
    // RC serialize(const Value &value, char *buf, int64_t len, int64_t &pos) const override;
    // RC deserialize(Value &value, const char *buf, int64_t len, int64_t &pos) const override;
};

} // namespace type
} // namespace common 
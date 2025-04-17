/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/type/char_type.h"
#include "common/value.h"
#include "common/type/date_utils.h"

int CharType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::CHARS && right.attr_type() == AttrType::CHARS, "invalid type");
  return common::compare_string(
      (void *)left.value_.pointer_value_, left.length_, (void *)right.value_.pointer_value_, right.length_);
}

RC CharType::set_value_from_str(Value &val, const string &data) const
{
  val.set_string(data.c_str());
  return RC::SUCCESS;
}

RC CharType::cast_to(const Value &val, AttrType to_type, Value &result) const
{
  ASSERT(val.attr_type() == AttrType::CHARS, "Invalid source type for CharType::cast_to");
  const char* str_data = val.data();
  if (str_data == nullptr) {
      LOG_WARN("Cannot cast NULL CHARS to other types.");
      return RC::INVALID_ARGUMENT;
  }

  switch (to_type) {
    case AttrType::CHARS:
        result.set_string(str_data, val.length());
        return RC::SUCCESS;
    case AttrType::DATES: {
        int64_t days;
        RC rc = common::type::string_to_days(str_data, days);
        if (OB_SUCCESS(rc)) {
            result.set_date(days);
            return RC::SUCCESS;
        } else {
            LOG_WARN("Failed to cast string '%s' to DATE.", str_data);
            return rc == RC::INVALID_ARGUMENT ? RC::INVALID_ARGUMENT : RC::FAILURE;
        }
    }
    case AttrType::INTS: {
        try {
            result.set_int(std::stoi(str_data));
            return RC::SUCCESS;
        } catch (const std::exception& e) {
            LOG_WARN("Failed to cast string '%s' to INT: %s", str_data, e.what());
            return RC::FAILURE;
        }
    }
    case AttrType::FLOATS: {
        try {
            result.set_float(std::stof(str_data));
            return RC::SUCCESS;
        } catch (const std::exception& e) {
            LOG_WARN("Failed to cast string '%s' to FLOAT: %s", str_data, e.what());
            return RC::FAILURE;
        }
    }
    default:
     LOG_WARN("Unsupported cast from CHARS to %s", attr_type_to_string(to_type));
     return RC::TYPE_MISMATCH;
  }
}

int CharType::cast_cost(AttrType type)
{
  if (type == AttrType::CHARS) {
    return 0;
  }
  return INT32_MAX;
}

RC CharType::to_string(const Value &val, string &result) const
{
  stringstream ss;
  ss << val.value_.pointer_value_;
  result = ss.str();
  return RC::SUCCESS;
}
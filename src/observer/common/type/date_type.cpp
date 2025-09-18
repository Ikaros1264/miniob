/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Ruiyang Xu on 2025/9/17.
//

#include "common/lang/comparator.h"
#include "common/lang/sstream.h"
#include "common/log/log.h"
#include "common/type/date_type.h"
#include "common/type/attr_type.h"
#include "common/value.h"
#include "storage/common/column.h"

#include <regex>
#include <ctime>

// 日期格式验证和转换辅助函数
bool is_leap_year(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool is_valid_date(int year, int month, int day) {
  if (year < 1900 || year > 2100) return false;
  if (month < 1 || month > 12) return false;
  if (day < 1) return false;

  int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (is_leap_year(year)) {
    days_in_month[1] = 29;
  }

  return day <= days_in_month[month - 1];
}

int date_to_int(int year, int month, int day) {
  return year * 10000 + month * 100 + day;
}

void int_to_date(int date_int, int &year, int &month, int &day) {
  year = date_int / 10000;
  month = (date_int % 10000) / 100;
  day = date_int % 100;
}

bool parse_date_string(const string &date_str, int &year, int &month, int &day) {
  // 只支持格式: YYYY-MM-DD
  std::regex date_regex(R"(^(\d{4})-(\d{1,2})-(\d{1,2})$)");
  std::smatch matches;
  
  if (!std::regex_match(date_str, matches, date_regex)) {
    return false;
  }
  
  try {
    year = std::stoi(matches[1].str());
    month = std::stoi(matches[2].str());
    day = std::stoi(matches[3].str());
    
    return is_valid_date(year, month, day);
  } catch (...) {
    return false;
  }
}

int DateType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::DATES, "left type is not date");
  ASSERT(right.attr_type() == AttrType::DATES || right.attr_type() == AttrType::CHARS, "right type is not date or string");

  if (right.attr_type() == AttrType::DATES) {
    return common::compare_int((void *)&left.value_.int_value_, (void *)&right.value_.int_value_);
  } else if (right.attr_type() == AttrType::CHARS) {
    int year, month, day;
    if (!parse_date_string(right.get_string(), year, month, day)) {
      return INT32_MAX; // 无效日期
    }
    
    int right_date = date_to_int(year, month, day);
    return common::compare_int((void *)&left.value_.int_value_, (void *)&right_date);
  }
  
  return INT32_MAX;
}

int DateType::compare(const Column &left, const Column &right, int left_idx, int right_idx) const
{
  ASSERT(left.attr_type() == AttrType::DATES, "left type is not date");
  ASSERT(right.attr_type() == AttrType::DATES, "right type is not date");
  
  return common::compare_int((void *)&((int*)left.data())[left_idx],
      (void *)&((int*)right.data())[right_idx]);
}

RC DateType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
  case AttrType::CHARS: {
    string date_str;
    RC rc = to_string(val, date_str);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    result.set_string(date_str.c_str());
    return RC::SUCCESS;
  }
  default:
    LOG_WARN("trying to cast date to unsupported type: %s", attr_type_to_string(type));
    return RC::UNIMPLEMENTED;
  }
}

RC DateType::add(const Value &left, const Value &right, Value &result) const
{
  // 日期加法暂不支持
  LOG_WARN("date addition is not supported");
  return RC::UNSUPPORTED;
}

RC DateType::subtract(const Value &left, const Value &right, Value &result) const
{
  // 日期减法暂不支持
  LOG_WARN("date subtraction is not supported");
  return RC::UNSUPPORTED;
}

RC DateType::multiply(const Value &left, const Value &right, Value &result) const
{
  // 日期乘法不支持
  LOG_WARN("date multiplication is not supported");
  return RC::UNSUPPORTED;
}

RC DateType::divide(const Value &left, const Value &right, Value &result) const
{
  // 日期除法不支持
  LOG_WARN("date division is not supported");
  return RC::UNSUPPORTED;
}

RC DateType::negative(const Value &val, Value &result) const
{
  // 日期取负不支持
  LOG_WARN("date negation is not supported");
  return RC::UNSUPPORTED;
}

RC DateType::set_value_from_str(Value &val, const string &data) const
{
  RC                rc = RC::SUCCESS;
  stringstream deserialize_stream;
  deserialize_stream.clear();  // 清理stream的状态，防止多次解析出现异常
  deserialize_stream.str(data);
  int int_value;
  deserialize_stream >> int_value;
  if (!deserialize_stream || !deserialize_stream.eof()) {
    rc = RC::SCHEMA_FIELD_TYPE_MISMATCH;
  } else {
    val.set_int(int_value);
  }
  return rc;
}

RC DateType::to_string(const Value &val, string &result) const
{
  int date_int = val.value_.int_value_;
  int year, month, day;
  int_to_date(date_int, year, month, day);
  
  stringstream ss;
  ss << year << "-" << (month < 10 ? "0" : "") << month << "-" << (day < 10 ? "0" : "") << day;
  result = ss.str();
  return RC::SUCCESS;
}
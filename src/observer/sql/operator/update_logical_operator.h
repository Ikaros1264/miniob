#pragma once

#include "update_logical_operator.h"
#include "sql/operator/logical_operator.h"

class UpdateLogicalOperator : public LogicalOperator
{
public:
  UpdateLogicalOperator(
      Table *table, FieldMeta meta, Value value)
      : table_(table), field_meta_(meta), value_(value)
  {}
  ~UpdateLogicalOperator() override = default;

  LogicalOperatorType                       type() const override { return LogicalOperatorType::UPDATE; }
  Table                                    *table() const { return table_; }
  FieldMeta                                 field_meta() const { return field_meta_; }
  Value                                     value() const { return value_; }

private:
  Table                                    *table_ = nullptr;
  FieldMeta                                 field_meta_;
  Value                                     value_;
};

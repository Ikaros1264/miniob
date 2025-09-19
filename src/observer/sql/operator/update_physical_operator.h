#pragma once

#include "sql/operator/physical_operator.h"

class Trx;
class DeleteStmt;

class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(
      Table *table, FieldMeta field_meta, Value value)
      : table_(table), field_meta_(field_meta), value_(value)
  {}

  ~UpdatePhysicalOperator() override = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override { return nullptr; }

private:
  Table                                   *table_ = nullptr;
  FieldMeta                                field_meta_;
  Value                                    value_;
};

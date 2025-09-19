#include "update_physical_operator.h"
#include "common/log/log.h"
#include "sql/expr/tuple.h"

RC UpdatePhysicalOperator::open(Trx *trx)
{
  auto &child = children_[0];
  RC    rc    = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("child operator open failed: %s", strrc(rc));
    return rc;
  }

  if (value_.attr_type() != field_meta_.type()) {
    Value to_value;
    rc = Value::cast_to(value_, field_meta_.type(), to_value);
    if (OB_FAIL(rc)) {
      LOG_WARN("cannot cast from %s to %s", attr_type_to_string(value_.attr_type()), attr_type_to_string(field_meta_.type()));
      return RC::INVALID_ARGUMENT;
    }
    value_ = to_value;
  }

  while ((rc = child->next()) == RC::SUCCESS) {
    Tuple *tuple_ = child->current_tuple();
    RowTuple *tuple = dynamic_cast<RowTuple *>(tuple_);
    ASSERT(tuple != nullptr, "tuple cannot cast to RowTuple here!");

    rc = table_->visit_record(tuple->record().rid(), [this, tuple](Record &record) {
      Record             old_record(record);
      tuple->set_cell_at(field_meta_.field_id(), value_, record.data());

      RC rc = table_->update_index(old_record, record);
      if (rc != RC::SUCCESS) {
        LOG_WARN("update index failed: %s", strrc(rc));
      }
      return true;
    });
  }

  if (rc == RC::RECORD_EOF) {
    rc = RC::SUCCESS;
  }

  child->close();
  return rc;
}

RC UpdatePhysicalOperator::close() { return RC::SUCCESS; }

RC UpdatePhysicalOperator::next() { return RC::RECORD_EOF; }

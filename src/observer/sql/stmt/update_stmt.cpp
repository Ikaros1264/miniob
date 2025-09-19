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
// Created by Ruiyang Xu on 2025/9/19.
//

#include "sql/stmt/update_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

UpdateStmt::UpdateStmt(
    Table *table, FieldMeta meta, Value value, FilterStmt *stmt)
    : table_(table), field_meta_(meta), value_(value), filter_stmt_(stmt)
{}

UpdateStmt::~UpdateStmt()
{
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC UpdateStmt::create(Db *db, UpdateSqlNode &update_sql, Stmt *&stmt)
{
  const char *table_name = update_sql.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  TableMeta meta = table->table_meta();
  const FieldMeta* field_meta = meta.field(update_sql.attribute_name.c_str());
  if (field_meta == nullptr) {
    LOG_WARN("field %s not found in table %s", update_sql.attribute_name.c_str(), table->name());
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  std::unordered_map<std::string, Table *> table_map   = {{update_sql.relation_name, table}};
  FilterStmt                              *filter_stmt = nullptr;
  RC rc = FilterStmt::create(db, table, &table_map, update_sql.conditions.data(), static_cast<int>(update_sql.conditions.size()), filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("cannot construct filter stmt");
    return rc;
  }
  stmt = new UpdateStmt(table, *field_meta, update_sql.value, filter_stmt);
  return RC::SUCCESS;
}
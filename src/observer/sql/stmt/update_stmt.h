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

#pragma once

#include "sql/stmt/filter_stmt.h"
#include "common/sys/rc.h"
#include "sql/stmt/stmt.h"

class Table;

/**
 * @brief 更新语句
 * @ingroup Statement
 */
class UpdateStmt : public Stmt
{
public:
  UpdateStmt() = default;
  UpdateStmt(Table *table, FieldMeta meta, Value value, FilterStmt *stmt);
  virtual ~UpdateStmt() override;

public:
  static RC create(Db *db, UpdateSqlNode &update_sql, Stmt *&stmt);

public:
  StmtType                                  type() const override { return StmtType::UPDATE; }
  Table                                    *table() { return table_; }
  FieldMeta                                 field_meta() { return field_meta_; }
  Value                                    &value() { return value_; }
  FilterStmt                               *filter_stmt() { return filter_stmt_; }

private:
  Table *table_ = nullptr;
  FieldMeta  field_meta_;
  Value value_;
  FilterStmt *filter_stmt_;
};
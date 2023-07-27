#ifndef _DB01_EXECUTE_H_
#define _DB01_EXECUTE_H_
#include "row.h"
#include "table.h"

typedef enum {
  EXECUTE_SUCCESS,
  EXECUTE_DUPLICATE_KEY,
  EXECUTE_TABLE_FULL
} ExecuteResult;

typedef enum { 
    STATEMENT_INSERT, 
    STATEMENT_SELECT 
} StatementType;

typedef struct {
  StatementType type;
  Row row_to_insert;  // only used by insert statement
} Statement;

ExecuteResult execute_statement(Statement* statement, Table* table);

#endif
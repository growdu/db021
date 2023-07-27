
#ifndef _DB01_TABLE_H_
#define _DB01_TABLE_H_
#include <stdint.h>
#include "pager.h"
#include "row.h"

#define TABLE_MAX_PAGES 100
#define ROWS_PER_PAGE  (PAGE_SIZE / ROW_SIZE)
#define TABLE_MAX_ROWS  (ROWS_PER_PAGE * TABLE_MAX_PAGES)

typedef struct {
  uint32_t root_page_num;
  Pager* pager;
} Table;

Table* db_open(const char* filename);
void db_close(Table* table);

#endif
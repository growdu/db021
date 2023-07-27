#ifndef _DB01_CURSOR_H_
#define _DB01_CURSOR_H_
#include <stdint.h>
#include <stdbool.h>
#include "table.h"

typedef struct {
  Table* table;
  uint32_t page_num;
  uint32_t cell_num;
  bool end_of_table;  // Indicates a position one past the last element
} Cursor;

void* cursor_value(Cursor* cursor);
void cursor_advance(Cursor* cursor);
Cursor* table_end(Table* table);
Cursor* table_start(Table* table);

#endif

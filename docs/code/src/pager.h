
#ifndef _DB01_PAGER_H_
#define _DB01_PAGER_H_
#include <stdint.h>
#include <stdio.h>

#define PAGER_MAX_PAGES 100
#define PAGE_SIZE  4096

typedef struct {
  FILE *file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void* pages[PAGER_MAX_PAGES];
} Pager;

void* get_page(Pager* pager, uint32_t page_num);
Pager* pager_open(const char* filename);
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);

#endif
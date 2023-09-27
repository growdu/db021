#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pager.h"
#include "row.h"
#include "table.h"

void* get_page(Pager* pager, uint32_t page_num) {
  Row *row = NULL;
  if (page_num > TABLE_MAX_PAGES) {
    printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
           TABLE_MAX_PAGES);
    exit(EXIT_FAILURE);
  }

  if (pager->pages[page_num] == NULL) {
    // Cache miss. Allocate memory and load from file.
    void* page = malloc(PAGE_SIZE);
    memset(page, 0, PAGE_SIZE);
    uint32_t num_pages = pager->file_length / PAGE_SIZE;

    // We might save a partial page at the end of the file
    if (pager->file_length % PAGE_SIZE) {
      num_pages = 1;
    }

    if (page_num <= num_pages) {
      fseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = fread(page, PAGE_SIZE, 1, pager->file_descriptor);
      row = (Row*)page;
      if (bytes_read == -1) {
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
      }
    }

    pager->pages[page_num] = page;

    if (page_num >= pager->num_pages) {
        pager->num_pages = page_num + 1;
    }
  }

  return pager->pages[page_num];
}

Pager* pager_open(const char* filename) {
  FILE* fd = fopen(filename, "rw+");
  if (fd == NULL) {
    fd = fopen(filename, "w+");
    if (fd == NULL) {
      printf("Unable to open file\n");
      exit(EXIT_FAILURE);
    }
  }
  fseek(fd, 0, SEEK_END);
  off_t file_length = ftell(fd);
  Pager* pager = malloc(sizeof(Pager));
  pager->file_descriptor = fd;
  pager->file_length = file_length;
  pager->num_pages = (file_length / PAGE_SIZE);

  if (file_length % PAGE_SIZE != 0) {
    printf("Db file is not a whole number of pages. Corrupt file.\n");
    exit(EXIT_FAILURE);
  }
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    pager->pages[i] = NULL;
  }

  return pager;
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size) {
  if (pager->pages[page_num] == NULL) {
    printf("Tried to flush null page\n");
    exit(EXIT_FAILURE);
  }

  fseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
  off_t offset = ftell(pager->file_descriptor); 
  if (offset == -1) {
    printf("Error seeking: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_written = fwrite(pager->pages[page_num], size, 1, pager->file_descriptor);
      //write(pager->file_descriptor, pager->pages[page_num], size);

  if (bytes_written == -1) {
    printf("Error writing: %d\n", errno);
    exit(EXIT_FAILURE);
  }
}

#ifndef _DB01_BTREE_H_
#define _DB01_BTREE_H_
#include <stdint.h>
#include "pager.h"
#include "row.h"
#include "cursor.h"

typedef enum { 
    NODE_INTERNAL, 
    NODE_LEAF 
} NodeType;

/*
 * Common Node Header Layout
 */
#define NODE_TYPE_SIZE  sizeof(uint8_t)
#define NODE_TYPE_OFFSET  0
#define IS_ROOT_SIZE  sizeof(uint8_t)
#define IS_ROOT_OFFSET  NODE_TYPE_SIZE
#define PARENT_POINTER_SIZE  sizeof(uint32_t)
#define PARENT_POINTER_OFFSET  ((IS_ROOT_OFFSET)  + (IS_ROOT_SIZE))
#define COMMON_NODE_HEADER_SIZE  ((NODE_TYPE_SIZE) + (IS_ROOT_SIZE) +  (PARENT_POINTER_SIZE))

/*
 * Leaf Node Header Layout
 */
#define LEAF_NODE_NUM_CELLS_SIZE  sizeof(uint32_t)
#define LEAF_NODE_NUM_CELLS_OFFSET  COMMON_NODE_HEADER_SIZE
#define LEAF_NODE_HEADER_SIZE  ((COMMON_NODE_HEADER_SIZE) + (LEAF_NODE_NUM_CELLS_SIZE))

/*
 * Leaf Node Body Layout
 */
#define LEAF_NODE_KEY_SIZE  sizeof(uint32_t)
#define LEAF_NODE_KEY_OFFSET  0;
#define LEAF_NODE_VALUE_SIZE  ROW_SIZE
#define LEAF_NODE_VALUE_OFFSET  ((LEAF_NODE_KEY_OFFSET) + (LEAF_NODE_KEY_SIZE))
#define LEAF_NODE_CELL_SIZE  ((LEAF_NODE_KEY_SIZE) + (LEAF_NODE_VALUE_SIZE))
#define LEAF_NODE_SPACE_FOR_CELLS  ((PAGE_SIZE) - (LEAF_NODE_HEADER_SIZE))
#define LEAF_NODE_MAX_CELLS  (LEAF_NODE_SPACE_FOR_CELLS) / (LEAF_NODE_CELL_SIZE)

static inline uint32_t* leaf_node_num_cells(void* node) {
  return node  + LEAF_NODE_NUM_CELLS_OFFSET;
}

static inline void* leaf_node_cell(void* node, uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE +  cell_num * LEAF_NODE_CELL_SIZE;
}

static inline uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num);
}

static inline void* leaf_node_value(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) +  LEAF_NODE_KEY_SIZE;
}

static inline void initialize_leaf_node(void* node) { 
    *leaf_node_num_cells(node) = 0; 
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);

void print_leaf_node(void* node);

#endif
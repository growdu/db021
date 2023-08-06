#ifndef _DB01_COMMAND_H_
#define _DB01_COMMAND_H_
#include <stdint.h>

#ifdef _WIN32
#define MAX_INPUT_BUFFER 4096
#endif

 typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum { 
    PREPARE_SUCCESS, 
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT 
} PrepareResult;


typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

int command_loop(char *filename);

#endif
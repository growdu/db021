#include <stdio.h>
#include "command.h"

int main(int argc, char* argv[]) {
  char* filename = NULL;
  if (argc < 2) {
    printf("[warnning] maybe you need supply a database filename,we would use \"mydb.db\".\n");
    filename = "mydb.db";
  } else {
    filename = argv[1];
  }
  command_loop(filename);
}
//  Copyright 2016 Ryo Shinoki. Released under the MIT license.

#include "calculator.h"

int write_text(const char *formula, const char *name) {
  FILE *file;
  const char *cPtr = formula;
  if ((file = fopen(name, "w")) == NULL) {
    fprintf(stderr, "Can not open file %s to write. \n", name);
    return -1;
  } else {
    while (*cPtr != '\0') {
      putc(*cPtr, file);
      ++cPtr;
    }
    fclose(file);
  }
  return 0;
}

FILE* get_file_ptr(const char *fileName) {
  FILE *file;
  if ((file = fopen(fileName, "r")) == NULL) {
    fprintf(stderr, "Can not open file %s to read. \n", fileName);
    return NULL;
  }
  return file;
}
#include "parser.h"

int main(int argc, char *argv[]) {
  int is_invalid = 1;
  if (argc != 2) {
    printf("Usage: json_parse file_name\n");
  } else {
    is_invalid = parse(argv[1]);
  }
  return is_invalid;
}

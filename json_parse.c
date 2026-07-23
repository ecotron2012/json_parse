#include "parser.h"

int main(int argc, char *argv[]) {
  int is_invalid = 0;
  if (argc <= 1) {
    printf("Usage: json_parse file1 file2 ...\n");
  } else {
    while (--argc > 0) {
      is_invalid = parse(*++argv);
      if (is_invalid == 1) {
        printf("Invalid JSON structure at position: \n");
      } else {
        printf("JSON is valid\n");
      }
    }
  }
  return 0;
}

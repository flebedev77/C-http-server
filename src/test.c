#include <stdio.h>
#include "util.h"

int tests_failed, tests_passed;

void ASSERT(bool expr, const char* msg) {
  if (expr) {
    tests_passed++;
    printf("PASSED %s\n", msg);
  } else {
    tests_failed++;
    fprintf(stderr, "FAILED %s\n", msg);
  }
}

int main(void) {
  filedata_t read = read_file("test", 100);
  ASSERT(read.len != 0, "read_file find index.html read"); 
  ASSERT((read.mime != 0 && read.mime != NULL), "read_file mime");
  ASSERT(strcmp(read.mime, "text/html") == 0, "read_file mime type html");
  free(read.data);
  free(read.mime);
  read = read_file("test", 20);
  ASSERT(read.len == 0, "read_file mime min length");
  free(read.data);
  free(read.mime);
  read = read_file("./", 100);
  ASSERT(read.len != 0, "read_file directory without index.html");
  free(read.data);
  free(read.mime);
  read = read_file("./test/subtest", 100);
  ASSERT(read.len != 0, "read_file directory without index.html with a deeper path");
  free(read.data);
  free(read.mime);

  random_init();
  unsigned int prev_seed = random_seed;
  ASSERT(random_seed != 0, "random_init seed");
  random_int(0, 10);
  ASSERT((random_seed != 0 && random_seed != prev_seed), "random_int seed");

  ASSERT((generate_http_header(100, 100, "text/plain") != NULL), "generate_http_header");

  ASSERT(strcmp((const char*)get_file_extension("index.html"), "html") == 0, "get_file_extension");
  ASSERT(get_file_extension("index") == NULL, "get_file_extension no dot");


  printf("TESTS FAILED %d  TESTS PASSED %d  TOTAL TESTS %d\n", tests_failed, tests_passed, tests_failed + tests_passed);

  return 0;
}

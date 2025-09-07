#include "util.h"

unsigned int random_seed;

void random_init() {
  srand((unsigned int)time(0));
}

int random_int(int min, int max) {
  return (rand() % (max - min)) + min;
}

void simulate_latency(int min, int max) {
  sleep(random_int(min, max));
}

char* generate_http_header(size_t data_len, size_t header_len) {
  char* header = (char*)malloc(header_len);
  char* headerfmt = "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %zd\r\n"
    "Connection: keep-alive\r\n\r\n";
  int result = snprintf(header, header_len, headerfmt, data_len);

  if (result < 0) {
    return nullptr;
  }
  return header;
}

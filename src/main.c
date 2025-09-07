#include "server.h"

int main() {
  // This is what we need to receive
  // GET / HTTP/1.1
  //   Host: 127.0.0.1:3000
  //   User-Agent: curl/8.14.1
  //   Accept: */*

  server_t server;
  server_init(&server);
  server_run(&server);
  server_free(&server);
  return 0;
}

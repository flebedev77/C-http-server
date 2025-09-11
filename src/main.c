#include "server.h"

void display_help() {
  printf("usage: server [port]\n");
}

int main(int argc, char** argv) {
  if (argc > 1 && argv[1][0] == '-') {
    display_help();
    return 0;
  }

  uint16_t port = 8080;
  if (argc == 2) {
    port = (uint16_t)atoi(argv[1]);

    if (port == 0) {
      display_help();
      return 0;
    }
  }

  server_t server;
  server_init(&server, port);
  server_run(&server);
  server_free(&server);
  return 0;
}

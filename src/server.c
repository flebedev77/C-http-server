#include "server.h"

void server_init(server_t* server) {
  memset(server, 0, sizeof(server_t));

  // This option will allow multiple servers running on the same host
  // If set to zero, will fail to bind server if a server is already being run
  // Even if on a different port
  server->reuseaddr_opt = 1;

  server->buffer = (uint8_t*)malloc(BUFFER_SIZE);
  server->address_length = sizeof(struct sockaddr_in);

  server->address.sin_family = AF_INET;
  server->address.sin_port = htons(PORT);
  server->address.sin_addr.s_addr = INADDR_ANY;

}

int server_run(server_t* server) {

  server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket_fd == 0) {
    perror("socket");
    return 0;
  }

  if (setsockopt(server->socket_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &server->reuseaddr_opt,
        sizeof(server->reuseaddr_opt))
     ) {
    perror("setsockopt");
    return 0;
  }

  if (bind(server->socket_fd, (struct sockaddr*)&server->address, sizeof(server->address)) != 0) {
    perror("bind");
    return 0;
  }

  if (listen(server->socket_fd, 3) != 0) {
    perror("listen");
    return 0;
  }
  
  printf("Server is listening on port %d\n", PORT);

  while (true) {
    // TODO: fix this incorrect ai slop accept call
    int incoming_socket = accept(server->socket_fd,
        (struct sockaddr*)&server->address,
        (socklen_t*)&server->address_length);

    if (incoming_socket < 0) {
      perror("accept");
      return 0;
    }

    server_handle_socket(server, incoming_socket);

    close(incoming_socket);
  }

  return 1;
}

void server_handle_socket(server_t* server, int socket_fd) {
  ssize_t bytes_read = read(socket_fd, server->buffer, BUFFER_SIZE);  
  if (bytes_read == -1) {
    perror("read");
    return;
  }

  // Last byte is usually a new line, replace that with null terminator
  server->buffer[bytes_read-1] = 0;
  printf("Received %zd bytes: %s  strlen: %zd\n", bytes_read, server->buffer, strlen((char*)server->buffer));

  // Use send() maybe?
  ssize_t bytes_sent = write(socket_fd, server->buffer, bytes_read);
  if (bytes_sent == -1) {
    perror("write");
    return;
  }

  printf("Echoed %zd bytes\n", bytes_sent);
}

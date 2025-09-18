#include "server.h"

void server_init(server_t* server, uint16_t port) {
  memset(server, 0, sizeof(server_t));

  // This option will allow multiple servers running on the same host
  // If set to zero, will fail to bind server if a server is already being run
  // Even if on a different port
  server->reuseaddr_opt = 1;

  server->address_length = sizeof(struct sockaddr_in);

  server->address.sin_family = AF_INET;
  server->address.sin_port = htons(port);
  server->address.sin_addr.s_addr = INADDR_ANY;

  server->threads_amount = 6;

  server->threads = (pthread_t*)malloc(sizeof(pthread_t) * server->threads_amount);
  
  memset(server->threads, 0, sizeof(pthread_t) * server->threads_amount);

  random_init();
}

int server_run(server_t* server) {
  server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket_fd == 0) {
    perror("socket");
    return -1;
  }

  if (setsockopt(server->socket_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &server->reuseaddr_opt,
        sizeof(server->reuseaddr_opt))
     ) {
    perror("setsockopt");
    return -1;
  }

  if (bind(server->socket_fd, (struct sockaddr*)&server->address, sizeof(server->address)) != 0) {
    perror("bind");
    return -1;
  }

  if (listen(server->socket_fd, server->threads_amount) != 0) {
    perror("listen");
    return -1;
  }
  
  printf("Server is listening on port %d\n", ntohs(server->address.sin_port));

  while (true) {
    connection_t* connection_data = (connection_t*)calloc(1, sizeof(connection_t));
    connection_data->socket_fd = accept(server->socket_fd,
        &connection_data->address,
        (socklen_t*)&server->address_length);

    if (connection_data->socket_fd < 0) {
      perror("accept");
      free(connection_data);
      continue;
    }

    uint8_t thread_data[sizeof(server_t*) + sizeof(connection_t*)];
    *(server_t**)thread_data = server;
    *(connection_t**)(thread_data + sizeof(server_t*)) = connection_data;
#ifndef DEBUG
    int tid = pthread_create(&server->threads[server->connections_amount], 0, server_handle_socket, thread_data);

    if (tid != 0) {
      server->threads[server->connections_amount] = (pthread_t){0};
      perror("pthread_create");
      free(connection_data);
    }
#endif
#ifdef DEBUG
  server_handle_socket(thread_data);
#endif
  }

  return 0;
}

void* server_handle_socket(void* args) {
  server_t* server = *(server_t**)args;
  connection_t* connection = *(connection_t**)(args + sizeof(server_t*));

  atomic_fetch_add(&server->connections_amount, 1);

  size_t port_offset = sizeof(uint16_t);
  uint8_t* ip = (uint8_t*)(connection->address.sa_data + port_offset);
  printf("Connection #%d received connection from: %d.%d.%d.%d:%d ", server->connections_amount,
      ip[0], ip[1], ip[2], ip[3], ntohs(((uint16_t*)connection->address.sa_data)[0]));

  uint8_t incoming_data[BUFFER_SIZE] = {0};
  ssize_t bytes_read = read(connection->socket_fd, incoming_data, BUFFER_SIZE);  
  if (bytes_read == -1) {
    perror("read");
    goto socket_cleanup;
  }

  // Last byte is usually a new line, replace that with null terminator
  incoming_data[bytes_read-1] = 0;

  char route[ROUTE_MAX_LENGTH] = {0};
  get_route((char*)incoming_data, route, ROUTE_MAX_LENGTH, bytes_read);
  printf("%s\n", route);

#ifdef DEBUG
  printf("Received %zd bytes: %s  strlen: %zd\n", bytes_read, incoming_data, strlen((char*)incoming_data));
  simulate_latency(1, 10);
#endif
 
  filedata_t file = read_file(route, MIME_MAX_LENGTH);
  if (file.len == 0) {
    perror("read_file");
    goto socket_cleanup;
  }

  char* http_header = generate_http_header(file.len, BUFFER_SIZE-1, file.mime);

  // Use send() maybe?
  size_t total_bytes_sent = 0;
  ssize_t bytes_sent = write(connection->socket_fd, http_header, strlen((char*)http_header));
  free(http_header);
  if (bytes_sent == -1) {
    perror("write");
    goto socket_cleanup;
  }
  total_bytes_sent += bytes_sent;

  bytes_sent = write(connection->socket_fd, file.data, file.len);
  if (bytes_sent == -1) {
    perror("write");
    goto socket_cleanup;
  }
  total_bytes_sent += bytes_sent;
  printf("Sent %zd bytes\n", total_bytes_sent);


socket_cleanup:
  atomic_fetch_sub(&server->connections_amount, 1);

  if (close(connection->socket_fd) == -1) {
    perror("close");
  }
  if (file.len != 0 && file.data != NULL && file.mime != NULL) {
    free(file.data);
    free(file.mime);
  }
  free(connection);
  return 0;
}

void server_free(server_t* server) {
  for (size_t i = 0; i < server->threads_amount; i++) {
    if (server->threads[i] != 0)
      pthread_join(server->threads[i], NULL);
  }
  free(server->threads);

  close(server->socket_fd);
}

#include "server.h"

void server_init(server_t* server) {
  memset(server, 0, sizeof(server_t));

  // This option will allow multiple servers running on the same host
  // If set to zero, will fail to bind server if a server is already being run
  // Even if on a different port
  server->reuseaddr_opt = 1;

  server->address_length = sizeof(struct sockaddr_in);

  server->address.sin_family = AF_INET;
  server->address.sin_port = htons(PORT);
  server->address.sin_addr.s_addr = INADDR_ANY;

  server->threads_amount = 6;

  server->thread_ids = (int*)malloc(sizeof(int) * server->threads_amount);
  server->threads = (pthread_t*)malloc(sizeof(pthread_t) * server->threads_amount);

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
  
  printf("Server is listening on port %d\n", PORT);

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
    int tid = pthread_create(&server->threads[server->connections_amount], 0, server_handle_socket, thread_data);

    if (tid != 0) {
      perror("pthread_create");
      free(connection_data);
    }

    server->thread_ids[server->connections_amount] = tid;
  }

  return 0;
}

void* server_handle_socket(void* args) {
  server_t* server = *(server_t**)args;
  connection_t* connection = *(connection_t**)(args + sizeof(server_t*));

  atomic_fetch_add(&server->connections_amount, 1);

  size_t port_offset = sizeof(uint16_t);
  uint8_t* ip = (uint8_t*)(connection->address.sa_data + port_offset);
  printf("Connection #%d received connection from: %d.%d.%d.%d:%d \n", server->connections_amount,
      ip[0], ip[1], ip[2], ip[3], ntohs(((uint16_t*)connection->address.sa_data)[0]));

  uint8_t incoming_data[BUFFER_SIZE] = {0};
  ssize_t bytes_read = read(connection->socket_fd, incoming_data, BUFFER_SIZE);  
  if (bytes_read == -1) {
    perror("read");
    pthread_exit(0);
  }


  // Last byte is usually a new line, replace that with null terminator
  incoming_data[bytes_read-1] = 0;
  printf("Received %zd bytes: %s  strlen: %zd\n", bytes_read, incoming_data, strlen((char*)incoming_data));

#ifdef DEBUG
  simulate_latency(1, 10);
#endif

  uint8_t outgoing_data[BUFFER_SIZE] = {0};

  char* http_header = generate_http_header(3, BUFFER_SIZE-1);
  strcpy((char*)outgoing_data, http_header);
  free(http_header);

  // Use send() maybe?
  ssize_t bytes_sent = write(connection->socket_fd, outgoing_data, strlen((char*)outgoing_data)+1);
  if (bytes_sent == -1) {
    perror("write");
    pthread_exit(0);
  }

  bytes_sent = write(connection->socket_fd, "hi\0", 3);
  if (bytes_sent == -1) {
    perror("write");
    pthread_exit(0);
  }


  printf("Echoed %zd bytes\n", bytes_sent);

  atomic_fetch_sub(&server->connections_amount, 1);

  if (close(connection->socket_fd) == -1) {
    perror("close");
  }
  free(connection);
  pthread_exit(0);
}

void server_free(server_t* server) {
  close(server->socket_fd);

  for (size_t i = 0; i < server->threads_amount; i++) {
    pthread_join(server->threads[i], NULL);
  }
  free(server->thread_ids);
  free(server->threads);
}

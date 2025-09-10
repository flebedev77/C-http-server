#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdatomic.h>

#include "util.h"

#ifndef PORT
  #define PORT 8080
#endif
#ifndef BUFFER_SIZE
  #define BUFFER_SIZE 1024
#endif

#define ROUTE_MAX_LENGTH 100

// #define DEBUG true

typedef struct {
  int socket_fd; 
  struct sockaddr address;
  uint8_t* buffer;
} connection_t;

typedef struct {
  int socket_fd,
      new_socket;
  atomic_int connections_amount;

  size_t threads_amount;
  pthread_t* threads;

  // OPTIONS
  int reuseaddr_opt;

  struct sockaddr_in address;
  size_t address_length;
} server_t;

// init only initialises the server_t struct
void server_init(server_t* server);
void server_free(server_t* server);

// run executes an infinite application loop
// returns -1 on failure
int server_run(server_t* server);

void* server_handle_socket(void* args);

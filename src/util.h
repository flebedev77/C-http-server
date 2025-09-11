#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

extern unsigned int random_seed;

void random_init();
int random_int(int min, int max);

void simulate_latency(int min, int max);

char* generate_http_header(
    size_t data_len,
    size_t header_len,
    const char* mime_type
    );
void get_route(char* req, char* out, size_t out_len, size_t req_len);

const char *get_file_extension(const char *s);

typedef struct {
  size_t len;
  char* mime;
  uint8_t* data;
} filedata_t;

filedata_t read_file(const char* filename, size_t mime_len);

#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

extern unsigned int random_seed;

void random_init();
int random_int(int min, int max);

void simulate_latency(int min, int max);

char* generate_http_header(size_t data_len, size_t header_len);

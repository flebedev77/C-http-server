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

void get_route(char *req, char *out, size_t out_len, size_t req_len) {
    bool is_route = false;
    size_t out_index = 0;
    out[out_index++] = '.';
    out[out_index++] = '/';
    for (size_t i = 0; i < req_len; i++) {
      if ((req[i] == ' ' || req[i] == 0) && is_route) {
        break;
      }
      if (is_route) {
        if (out_len-1 == out_index) {
          break;
        }
        out[out_index++] = req[i];
      }
      if (req[i] == '/' && is_route == false) {
        is_route = true;
      }
    }

    out[out_index++] = '\0';
}


// C stdlib
#ifndef POSIX
filedata_t read_file(const char* filename) {
  printf("Reading %s\n\n", filename);
  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    printf("%s ", filename);
    perror("fopen");
    return (filedata_t){ 0 };
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    perror("fseek");
    goto cleanup;
  }
  long file_size = ftell(file);
  if (fseek(file, 0, SEEK_SET) != 0) {
    perror("fseek");
    goto cleanup;
  }

  uint8_t* buffer = (uint8_t*)malloc(file_size+1);
  buffer[file_size-1] = 0;

  size_t bytes_read = fread(buffer, 1, file_size, file);

  if (bytes_read != (size_t)file_size) {
    perror("fread");
    goto cleanup;
  }

  if (fclose(file) != 0) {
    perror("fclose");
  }
  return (filedata_t){ bytes_read, buffer };

cleanup:
  if (fclose(file) != 0) {
    perror("fclose");
  }
  return (filedata_t){ 0 };
}
#endif

#ifdef POSIX
filedata_t read_file(const char* filename) {
  int file_fd = open(filename, O_RDONLY);  
  if (file_fd == -1) {
    perror("open");
    goto fail;
  }

  struct stat file_stat = {0};
  if (fstat(file_fd, &file_stat) != 0) {
    perror("fstat");
    goto fail;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(file_stat.st_size);
  ssize_t bytes_read = read(file_fd, buffer, file_stat.st_size);
  if (bytes_read == -1) {
    perror("read");
    goto fail;
  }

  return (filedata_t) { (size_t)file_stat.st_size, buffer };

fail:
  close(file_fd);
  return (filedata_t) {0};
}
#endif

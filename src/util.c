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



char* generate_http_header(size_t data_len, size_t header_len, const char* mime_type) {
  char* header = (char*)malloc(header_len);
  char* headerfmt = "HTTP/1.1 200 OK\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %zd\r\n"
    "Connection: keep-alive\r\n\r\n";
  int result = snprintf(header, header_len, headerfmt, mime_type, data_len);

  if (result < 0) {
    return 0;
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

const char *get_file_extension(const char *s) {
    if (!s) return NULL;
    const char *dot = strrchr(s, '.');
    if (!dot || dot[1] == '\0') return NULL;
    return dot + 1;
}

filedata_t read_file(const char* filename, size_t mime_len) {
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
  char* mime_type = (char*)malloc(mime_len);
  if (buffer == NULL || mime_type == NULL) {
    perror("malloc");
    goto fail;
  }

  memset(buffer, 0, file_stat.st_size);
  memset(mime_type, 0, mime_len);

  ssize_t bytes_read = read(file_fd, buffer, file_stat.st_size);
  if (bytes_read == -1) {
    perror("read");
    goto fail;
  }
  
  const char* ext = get_file_extension(filename);

  if (strcmp(ext, "html") == 0)
    strcpy(mime_type, "text/html");
  else if (strcmp(ext, "js") == 0)
    strcpy(mime_type, "application/javascript");

  printf("%s", mime_type);


  return (filedata_t) { (size_t)file_stat.st_size, mime_type, buffer };

fail:
  close(file_fd);
  return (filedata_t) {0};
}

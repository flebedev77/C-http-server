#include "util.h"

unsigned int random_seed = 0;

void random_init() {
  random_seed = (unsigned int)time(0);
  srand(random_seed);
}

int random_int(int min, int max) {
  random_seed = rand();
  srand(random_seed);
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
    // out[out_index++] = '.';
    // out[out_index++] = '/';
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

    out[out_index] = '\0';
}

const char *get_file_extension(const char *s) {
    if (!s) return NULL;
    const char *dot = strrchr(s, '.');
    if (!dot || dot[1] == '\0') return NULL;
    return dot + 1;
}

filedata_t read_file(const char* filename, size_t mime_len) {
  if (mime_len < strlen("application/vnd.openxmlformats-officedocument.wordprocessingml.document")) {
#ifdef DEBUG
    fprintf(stderr, "mime_len is too short\n");
#endif
    return (filedata_t) {0};
  }

  size_t filename_len = strlen(filename);
  char* index_html = (filename[filename_len-1] == '/') ? "index.html" : "/index.html";
  char filename_fmt[PATH_MAX];//filename_len + strlen(index_html) + 1];
  strcpy(filename_fmt, filename);

  bool is_current_dir = false;

  int file_fd = open(filename_fmt, O_RDONLY);  
  if (file_fd == -1) {
    file_fd = open(".", O_RDONLY);
    if (file_fd != -1) {
      is_current_dir = true;
    } else {
      perror("open");
      goto fail;
    }
  }

  struct stat file_stat = {0};
  if (fstat(file_fd, &file_stat) != 0) {
    perror("fstat");
    goto fail;
  }

  if (S_ISDIR(file_stat.st_mode)) {
    if (close(file_fd) != 0) {
      perror("close");
      goto fail;
    }

    strcat(filename_fmt, index_html);

    file_fd = open(filename_fmt, O_RDONLY);
    if (file_fd == -1) {
      size_t list_len = 100;
      size_t list_size = (list_len * PATH_MAX) + 1 + 100;
      char* list_str = (char*)malloc(list_size);
      char* list_str_temp = (char*)malloc(list_size);
      char* mime = (char*)malloc(strlen("text/html") + 1);
      strcpy(mime, "text/html");
      
      DIR* dir = opendir((is_current_dir) ? "." : filename);
      if (dir == NULL) {
        perror("opendir");
        goto fail;
      }

      struct dirent* entry;
      size_t entry_index = 0;
      while ((entry = readdir(dir)) != NULL) {
        if (entry_index >= list_len-1) break;

        char* entry_filename = entry->d_name;

        if (*entry_filename == '.' || *entry_filename == ' ') continue;

        char full_path[PATH_MAX] = {0};
        int fmt_result = snprintf(full_path, PATH_MAX, (filename[strlen(filename)-1] == '/') ? "%s%s" : "%s/%s", filename, entry_filename);
        if (fmt_result < 0) continue;

        char li_el[PATH_MAX];
        fmt_result = snprintf(li_el, PATH_MAX + 500, "<li><a href=\"%s\">%s</a></li>\n", full_path, entry_filename);
        if (fmt_result < 0) continue;
        strcpy(list_str + strlen(list_str), li_el);

        entry_index++;
      }

      if (closedir(dir) != 0) {
        perror("closedir");
      }

      strcpy(list_str_temp, list_str);
      int list_fmt_result = snprintf(list_str, list_size, "<h1>%s</h1></br><ul>%s</ul>", filename, list_str_temp);
      free(list_str_temp);
      // printf("Completed list_str %s\n", list_str);

      if (list_fmt_result < 0) {
        perror("snprintf");
        return (filedata_t) {0};
      }

      return (filedata_t){ strlen(list_str), mime, (uint8_t*)list_str };
    }


    if (fstat(file_fd, &file_stat) != 0) {
      perror("fstat");
      goto fail;
    }
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
  
  const char* ext = get_file_extension(filename_fmt);

  if (ext == NULL) strcpy(mime_type, "application/octet-stream");
  else if (strcmp(ext, "html") == 0) strcpy(mime_type, "text/html");
  else if (strcmp(ext, "css") == 0) strcpy(mime_type, "text/css");
  else if (strcmp(ext, "txt") == 0) strcpy(mime_type, "text/plain");
  else if (strcmp(ext, "js") == 0) strcpy(mime_type, "application/javascript");
  else if (strcmp(ext, "wasm") == 0) strcpy(mime_type, "application/wasm");
  else if (strcmp(ext, "png") == 0) strcpy(mime_type, "image/png");
  else if (strcmp(ext, "gif") == 0) strcpy(mime_type, "image/gif");
  else if (strcmp(ext, "jpeg") == 0) strcpy(mime_type, "image/jpeg");
  else if (strcmp(ext, "webp") == 0) strcpy(mime_type, "image/webp");
  else if (strcmp(ext, "svg") == 0) strcpy(mime_type, "image/svg+xml");
  else if (strcmp(ext, "bmp") == 0) strcpy(mime_type, "image/bmp");
  else if (strcmp(ext, "ico") == 0) strcpy(mime_type, "image/vnd.microsoft.icon");
  else if (strcmp(ext, "mp3") == 0) strcpy(mime_type, "audio/mpeg");
  else if (strcmp(ext, "wav") == 0) strcpy(mime_type, "audio/wav");
  else if (strcmp(ext, "ogg") == 0) strcpy(mime_type, "audio/ogg");
  else if (strcmp(ext, "opus") == 0) strcpy(mime_type, "audio/opus");
  else if (strcmp(ext, "aac") == 0) strcpy(mime_type, "audio/aac");
  else if (strcmp(ext, "m4a") == 0) strcpy(mime_type, "audio/mp4");

  else if (strcmp(ext, "mp4") == 0) strcpy(mime_type, "video/mp4");
  else if (strcmp(ext, "webm") == 0) strcpy(mime_type, "video/webm");
  else if (strcmp(ext, "ogv") == 0) strcpy(mime_type, "video/ogg");
  else if (strcmp(ext, "avi") == 0) strcpy(mime_type, "video/x-msvideo");
  else if (strcmp(ext, "mov") == 0) strcpy(mime_type, "video/quicktime");
  else if (strcmp(ext, "mkv") == 0) strcpy(mime_type, "video/x-matroska");
  else if (strcmp(ext, "mpeg") == 0 || strcmp(ext, "mpg") == 0) strcpy(mime_type, "video/mpeg");
  else strcpy(mime_type, "application/octet-stream");

  return (filedata_t) { (size_t)file_stat.st_size, mime_type, buffer };

fail:
  close(file_fd);
  return (filedata_t) {0};
}

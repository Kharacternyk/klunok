#include "deref.h"
#include "set.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

int main(int argc, const char **argv) {
  struct set *editors = create_set();
  for (int i = 1; i < argc; ++i) {
    add_to_set(argv[i], editors);
  }

  int status = 0;
  /* TODO Why does it behave strange with O_RDWR? */
  int fanotify_fd = fanotify_init(FAN_CLASS_CONTENT, O_RDONLY);

  if (fanotify_fd < 0) {
    fprintf(stderr, "Cannot init fanotify: %s\n", strerror(errno));
    return 1;
  }

  status = fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                         FAN_OPEN_PERM, 0, "/home");

  if (status < 0) {
    fprintf(stderr, "Cannot mark /home: %s\n", strerror(errno));
    return 1;
  }

  for (;;) {
    struct fanotify_event_metadata event;
    status = read(fanotify_fd, &event, sizeof event);

    if (status < sizeof event) {
      fprintf(stderr, "Cannot read an event: %s\n", strerror(errno));
      return 1;
    }

    char *exe_path = deref_pid(event.pid);

    if (!exe_path) {
      fprintf(stderr, "Cannot resolve executable path: %s\n", strerror(errno));
      return 1;
    }

    char *exe_filename = strrchr(exe_path, '/');

    if (!exe_filename) {
      fprintf(stderr, "Cannot resolve executable path: %s\n", strerror(errno));
      return 1;
    }

    ++exe_filename;

    if (is_in_set(exe_filename, editors)) {
      char *file_path = deref_fd(event.fd);

      if (status < 0) {
        fprintf(stderr, "Cannot resolve file path: %s\n", strerror(errno));
        return 1;
      }

      printf("%s\n", file_path);
      free(file_path);
    }

    struct fanotify_response response = {
        .fd = event.fd,
        .response = FAN_ALLOW,
    };
    write(fanotify_fd, &response, sizeof response);
    close(event.fd);
    free(exe_path);
  }

  return fanotify_fd;
}

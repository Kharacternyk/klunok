#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

int main() {
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

    char path[PATH_MAX];

    snprintf(path, PATH_MAX, "/proc/%d/exe", event.pid);
    status = readlink(path, path, sizeof path - 1);

    if (status < 0) {
      fprintf(stderr, "Cannot process executable path: %s\n", strerror(errno));
      return 1;
    }

    path[status] = 0;
    printf("%s\n", path);

    snprintf(path, PATH_MAX, "/proc/self/fd/%d", event.fd);
    status = readlink(path, path, sizeof path - 1);

    if (status < 0) {
      fprintf(stderr, "Cannot resolve file path: %s\n", strerror(errno));
      return 1;
    }

    path[status] = 0;
    printf("\t%s\n", path);

    struct fanotify_response response = {
        .fd = event.fd,
        .response = FAN_ALLOW,
    };
    write(fanotify_fd, &response, sizeof response);
    close(event.fd);
  }

  return fanotify_fd;
}

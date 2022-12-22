#include "config.h"
#include "deref.h"
#include "store.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

static void log_errno(const char *prefix) {
  fprintf(stderr, "%s: %s\n", prefix, strerror(errno));
}

static void panic(const char *message) {
  log_errno(message);
  exit(1);
}

static void typed_bearable_error_callback_function(const char **message) {
  log_errno(*message);
}

static void typed_fatal_error_callback_function(const char **message) {
  panic(*message);
}

static void bearable_error_callback_function(void *message) {
  typed_bearable_error_callback_function(message);
}

static void fatal_error_callback_function(void *message) {
  typed_fatal_error_callback_function(message);
}

int main(int argc, const char **argv) {
  const char *error_message = NULL;
  struct callback *bearable_error_callback =
      create_callback(bearable_error_callback_function, &error_message);
  struct callback *fatal_error_callback =
      create_callback(fatal_error_callback_function, &error_message);

  struct set *editors = get_editors();
  const char *store_root = argc > 1 ? argv[1] : "./klunok-store";
  error_message = "Cannot create store";
  struct store *store = create_store(store_root, fatal_error_callback);

  int status = 0;
  /* TODO Why does it behave strange with O_RDWR? */
  int fanotify_fd = fanotify_init(FAN_CLASS_CONTENT, O_RDONLY);

  if (fanotify_fd < 0) {
    panic("Cannot init fanotirfy");
  }

  status = fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                         FAN_OPEN_PERM, 0, "/home");

  if (status < 0) {
    panic("Cannot mark /home");
  }

  for (;;) {
    struct fanotify_event_metadata event;
    status = read(fanotify_fd, &event, sizeof event);

    if (status < sizeof event) {
      panic("Cannot read an event");
    }

    error_message = "Cannot resolve executable path";
    char *exe_path = deref_pid(event.pid, fatal_error_callback);
    char *exe_filename = strrchr(exe_path, '/');

    if (!exe_filename) {
      panic("Cannot resolve executable path");
    }

    ++exe_filename;

    if (is_in_set(exe_filename, editors)) {
      error_message = "Cannot resolve file path";
      char *file_path = deref_fd(event.fd, fatal_error_callback);
      error_message = "Cannot link file to store";
      link_to_store(file_path, store, bearable_error_callback);
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

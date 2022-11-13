#include <fcntl.h>
#include <sys/fanotify.h>

int main() {
    int fanotify_fd = fanotify_init(FAN_CLASS_CONTENT, O_RDWR);

    return fanotify_fd;
}

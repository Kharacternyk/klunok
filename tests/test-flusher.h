#include <stdint.h>
#include <sys/types.h>

#define BOOT_ID_SIZE 36

struct buffer;
struct trace;

int set_flush_xattr(const char *path, uint8_t version, const char *boot_id,
                    uint64_t timestamp, uint64_t id, uint64_t time);
void read_boot_id(char *boot_id);
void assert_acknowledgement(int fd, uint64_t id);
struct buffer *create_acknowledgement_path(struct trace *trace);

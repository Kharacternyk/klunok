struct translation {
  struct {
    const char *overflow;
  } timestamp;
  struct {
    struct {
      const char *file_does_not_exist;
      const char *permission_denied;
      const char *version_already_exists;
    } copy;
  } store;
  struct {
    struct {
      const char *has_slashes;
    } version;
    struct {
      const char *cannot_load;
      const char *cannot_reload;
    } config;
    struct {
      const char *cannot_load;
      const char *cannot_reload;
      const char *cannot_get_head;
      const char *cannot_push;
    } linq;
    struct {
      const char *cannot_copy;
    } store;
  } handler;
};

extern const struct translation messages;

struct translation {
  struct {
    const char *overflow;
  } timestamp;
  struct {
    const char *cannot_create_ancestor;
    const char *cannot_remove_ancestor;
  } parents;
  struct {
    struct {
      const char *file_does_not_exist;
      const char *permission_denied;
      const char *version_already_exists;
    } copy;
  } store;
  struct {
    const char *is_static;
  } config;
  struct {
    const char *invalid_entry;
  } linq;
  struct {
    const char *unknown_option;
    const char *stray_option;
  } params;
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
    struct {
      const char *cannot_open;
      const char *cannot_write_to;
    } journal;
  } handler;
};

extern const struct translation messages;

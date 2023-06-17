struct translation {
  struct {
    const char *because_of;
    const char *which_is;
    const char *message_dropped;
  } trace;
  struct {
    const char *overflow;
  } timestamp;
  struct {
    const char *cannot_create_ancestor;
    const char *cannot_remove_ancestor;
  } parents;
  struct {
    const char *source_is_not_regular_file;
    const char *source_does_not_exist;
    const char *source_permission_denied;
    const char *destination_already_exists;
  } copy;
  struct {
    const char *is_static;
  } config;
  struct {
    const char *invalid_entry;
  } linq;
  struct {
    const char *unknown_option;
    const char *stray_option;
    const char *redefined_option;
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
  struct {
    const char *cannot_bootstrap;
    const char *out_of_memory;
    const char *cli_usage_violated;
    const char *cannot_parse_cli;
    const char *cannot_drop_privileges;
    const char *cannot_load_handler;
    const char *cannot_handle_exec;
    const char *cannot_handle_write;
    const char *cannot_handle_timeout;
    struct {
      const char *cannot_init;
      const char *cannot_poll;
      const char *cannot_read_event;
      const char *version_mismatch;
      const char *queue_overflow;
    } fanotify;
    struct {
      const char *cannot_list;
      const char *cannot_watch;
    } mount;
    struct {
      const char *version;
      const char *usage;
      const char *help;
    } info;
  } main;
};

extern const struct translation messages;

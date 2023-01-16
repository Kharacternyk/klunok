struct translation {
  struct {
    const char *overflow;
  } timestamp;
  struct {
    struct {
      const char *file_does_not_exist;
    } copy;
  } store;
};

extern const struct translation messages;

# Migration

## v2.0.0 {/* #v2.0.0 */}

- Queues of older Klunok versions may not be compatible with v2 if
  [`history_paths`](./configuration.md#history_paths)
  or [Projects](./projects.md) have been used.
  Klunok v2 will exit with an error telling so if this is the case.
  Let the queue be drained empty by the previous version to proceed.

- In configuration sets, `false` is now equivalent to `nil`: both exclude the key.
  Previously, keys with `false` values were included;
  use `true` or another truthy value to keep them included.

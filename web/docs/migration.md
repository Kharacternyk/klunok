# Migration

## 2.0.0

- In configuration sets, `false` is now equivalent to `nil`: both exclude the key.
  Previously, keys with `false` values were included;
  use `true` or another truthy value to keep them included.

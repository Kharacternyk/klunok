---
sidebar_position: 6
---

# Projects

Projects are an experimental feature that lets you track the history
of directories as a whole in addition to the history of individual files.
Add a directory to [the `project_roots` setting](./configuration.md#project_roots)
to track it.
The backed up versions of the directory will then appear at `klunok/projects` by default.
Files within the versions are hard links to files in the ordinary store
(`klunok/store` by default).

[The `debounce_seconds` setting](./configuration.md#debounce_seconds) applies to
the projects as well.
A new version of a project is stored if no files within the project are edited
for this amount of seconds.

# Mounts

Klunok uses the Linux fanotify API to monitor entire subtrees of the file system.
This feature works only when the subtree is a mount point.
`/` is always a mount point.

If Klunok is requested to monitor a directory that is not a mount point,
it mounts the directory to itself via a bind mount.
Such a mount is completely transparent for most use cases.
If you would like to avoid an automatic bind mount for some reason,
mount the directory yourself before launching Klunok.

Klunok does not monitor nested mount points by default.
For example, if `/home/nazar/mnt` is a mount point,
`klunok -w /home/nazar` won't monitor files within `/home/nazar/mnt`.
If this is not the desired behavior, list nested mount points explicitly:
`klunok -w /home/nazar/mnt -w /home/nazar`.

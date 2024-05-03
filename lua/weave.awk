/^```$/ {
    is_pre = 0
    is_post = 0
}
{
    if (is_pre) {
        print > pre
    } else if (is_post) {
        print > post
    }
}
/^```lua title="pre-config"/ {
    is_pre = 1
}
/^```lua title="post-config"/ {
    is_post = 1
}

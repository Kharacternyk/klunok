project = 'klunok'
title = 'klunok configuration'
format = 'markdown'
no_lua_ref = true
not_luadoc = true
style = '!pale'

local man_pattern = 'https://man.archlinux.org/man/core/man-pages/%s.%s.en'
custom_see_handler('^([%w_]+)%((%d)%)$', function(name, section)
    local url = man_pattern:format(name, section)
    local name = name .. '(' .. section .. ')'
    return name, url
end)

config = assert(io.open(arg[1], 'r'))
pre_config = assert(io.open(arg[2], 'w'))
post_config = assert(io.open(arg[3], 'w'))

is_post_config = false

for line in config:lines() do
  if string.sub(line, 1, 7) == '--_' then
    is_post_config = not is_post_config
  elseif string.sub(line, 1, 1) == '_' then
    post_config:write(line, '\n')
  elseif string.sub(line, 1, 2) ~= '--' and string.len(line) > 0 then
    if is_post_config then
      post_config:write(line, '\n')
    else
      pre_config:write(line, '\n')
    end
  end
end

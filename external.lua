
local s = ''

function engine_loop(dt, close)
	if close then
		print "A"
	end

	local c = raylib.GetCharPressed()
	local changed = c and true or false

	if raylib.IsKeyPressed(259) then
		s = string.sub(s, 1, -2)
		changed = true
		raylib.PlaySound("assets/boom.ogg")
	end

	while c do
		c = utf8.char(c)
		s = s .. c
		c = raylib.GetCharPressed()
	end

	if changed then print(s) end

	return close
end

raylib.SetWindowTitle("among");

print "loaded!"

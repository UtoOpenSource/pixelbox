-- luajit source
-- compiles lua files and archives them into one c file in stdout! yay!

--[[
-- This file is a part of PixelBox - infinite sandbox game
-- Compile & Archive lua files to C code.
-- Never ask me why, please, never ask me...
-- Copyright (C) 2023 UtoECat
--
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
--
-- This file needs custom compress() function. It's implemented as
-- raylib's CompressData() wrapper in archiver.c!
--
--]]

local err = io.stderr
local out = io.stdout

local RELEASE = false

function print(...) 
	for i = 1, select('#', ...) do
		local s = select(i, ...)
		io.stderr:write(tostring(s))
		io.stderr:write('\t')
	end
	io.stderr:write('\n')
end

local buff = 
[[/* THIS FILE IS AUTOGENERATED! 
* SEE ./tools/ar.lua and ./tools/archiver.c FOR DETAILS!
* DO NOT EDIT! */
// Compressed using DEFLATE algo!

]]

local function readfile(name)
	local f = io.open(name)
	if not f then
		print("Error : can't open file!")
		os.exit(5)
	end
	local s = f:read("a")
	f:close()
	return s
end

local function compile(code)
	if not RELEASE then return code end -- for debug
	local f, err = load(code)
	if not f then
		print(err)
		os.exit(6)
	end
	local s = string.dump(f)
	assert(load(s))
	return s
end

local function pass(code)
	return code
end

local extensions = {
	['.lua'] = compile,
	['.lcc'] = pass
}

local function better_name(name)
	return name:gsub('[^%w_]', '_')
end

local function toccode(name, data)
	name = better_name(name)
	buff = buff .. string.format("static unsigned char %s[] = {\n\t", name)
	for i = 1, #data do
		buff = buff .. string.byte(data, i, i) .. (i ~= #data and ', ' or '')
		if i % 8 == 7 then
			buff = buff .. '\n\t'
		end
	end
	buff = buff .. "};\n\n"
end	

local function readdata(name)
	local ext = name:sub(-3, -1)
	local fun = extensions[ext] and extensions[ext] or pass
	local data = compress(fun(readfile(name)))
	return data
end

for i = 1, #arg do
	toccode(arg[i], readdata(arg[i]))
end

buff = buff ..
[[const struct archive_node {
	const char* name;
	const long unsigned int length;
	const unsigned char* value;
} __main_arcive[] = {]] .. "\n\t"

for i = 1, #arg do
	buff = buff .. string.format("{%q, sizeof(%s), %s},\n\t", 
		arg[i], better_name(arg[i]), better_name(arg[i]))
end

buff = buff.."{(const char*)0, 0, (const unsigned char*)0}\n};"

out:write(buff)
out:flush()
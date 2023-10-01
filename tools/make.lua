--[[
-- Copyright (C) 2023 UtoECat <utopia.egor.cat.allandall@gmail.com>
-- 
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
--
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
-- IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
-- CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
-- TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
-- SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--]]

-- some declarations first

local COPYRIGHT = [[
 Copyright (C) 1994-2020 Lua.org, PUC-Rio.
 Copyright (C) 2023 UtoECat <utopia.egor.cat.allandall@gmail.com>

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
]]

local function unpacks(files)
	local t = {}
	for s in string.gmatch(files, '%S*') do
		t[#t+1] = s
	end
	return t
end

-- core
local sources = unpacks("lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltests.c ltm.c lundump.c lvm.c lzio.c")

-- libraries
local libsources = unpacks("lauxlib.c lbaselib.c lcorolib.c ldblib.c liolib.c lmathlib.c loadlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c")

-- some useful functions

local function readfile(n)
	local f, err = io.open(n, 'r')
	if err then
		print("Can't open file " ..n.. " : " .. err)
		return "#include <"..n..">\n", false
	end
	local str = f:read('a')
	f:close()
	return str, true
end

local included = {}

-- recursive :D
local function incfile(oname)
	local unused1, unused2, name = oname:find("/?([%w_.,]*)$")
	if not included[name] then
		print("Including", name, "!")
		local txt, stat = readfile(oname)
		included[name] = 0
		if stat then
			return ("\n"..txt):gsub('\n?%s*#%s*include%s+["<](.-)[">]', incfile)
		else
			included[name] = -1;
			return "\n"..txt -- include error :(
		end	
	elseif included[name] > 0 then
		return '\n'
	end
	if included[name] >= 0 then included[name] = included[name] + 1 end
	return '//included "'..name..'" \n' -- no double inclusion
end

-- unused macros to remove
local unused_macros_orig = {
	"LUA_CORE", "LUA_LIB"
}
local unused_macros = {}
for _,v in pairs(unused_macros_orig) do
	unused_macros[v] = true
end

local function unused_check(m, v)
	if unused_macros[m] or included[m:gsub("_", ".")] then
		--print("[MACRO] : unused macros '"..m.."' removed!")
		return "\n\n"
	end
	--print("[MACRO] : '"..m.."' = "..v)
end

local buf = {}
local header = "\x1bLua"

local MAJOR = 5
local MINOR = 5
local LUAC_VERSION = (MAJOR)*16+(MINOR)

local hH = "#define COMPILED_BYTECODE\n"

local function parseLua(str)
	local name, code = str:match('%s*([_%w]-)%s*,%s*"([^"]*)"')
	if not name then return end
	code = code
	if _VERSION == "Lua 5.4" then 
		local f, err = load(code)
		if not f then
			error('bad function '..name..'\n reason : '..err)
		end
		code = string.dump(f, true)
		-- change bytecode version
		code = header .. string.char(LUAC_VERSION) .. code:sub(#header + 2, -1)
		print("compiled!")
	else
		code = code:gsub('[\n\t ]+', ' ')
		code = code:gsub('"', '\'')
	end
 	-- to C code
		local t = {}
		t[#t+1] = ("static const char BC_%s_DATA[] = {\n"):format(name)
		for i = 1, #code do
			local c = string.byte(code, i, i)
			t[#t+1] = tostring(c)..(i==#code and '' or ', ')
			if i % 7 == 0 then
				t[#t+1] = '\n '
			end
		end
		t[#t+1] = '};\n'
		t[#t+1] = ("static const size_t BC_%s_SIZE = %i;\n"):format(name, #code)
	local v = table.concat(t)
	print(("Parsed %q"):format(code))
	return v
end

local function donefile(file, append, prepend)
	local str = table.concat(buf, '\n')
	buf = {}
	collectgarbage()
	
	str = str:gsub("/%*.-%*/", "") -- remove /* */ commentaries
	str = str:gsub("\\%s-\n", "") -- remove continue line
	str = str:gsub("\n?%s*#%s*define[ \t]+([_%w]*)[ \t]?(.-)\n", unused_check)
	str = str:gsub("\t", " ") -- remove tabs
	str = str:gsub("\n\n+", "\n") -- remove extra newlines...

	-- bytecode definition
	str = str:gsub(
	"LUA_BCDEF%s*%b()", parseLua)
	str = (append or "") .. str .. (prepend or "")

	for k, v in pairs(included) do
		if v > 0 then
			included[k] = 1
		elseif v == -1 then
			included[k] = nil
		end
	end

	local f = io.open(file, "w")
	f:write(str)
	f:flush()
	f:close()
end

-- do lua.h
buf[1] = incfile("lua.h")
buf[#buf + 1] = incfile("lualib.h")
buf[#buf + 1] = incfile("lauxlib.h")

-- pack and write :D
--donefile('../lua.h', "/*\n".. COPYRIGHT .."\n*/")
donefile('./lua.h', 
"/*\n".. COPYRIGHT .. [[
*/
#ifndef lua_inc
#define lua_inc

#ifdef __cplusplus
extern C {
#endif
]],
[[
#ifdef __cplusplus
};
#endif
#endif /* lua_inc */
]])

-- do lua.c

for _, v in pairs(sources) do
	buf[#buf + 1] = "// root include "..v.."\n"
	buf[#buf + 1] = incfile(v)
end


-- pack and write lua.c!
donefile('./lua.c', [[
#define LUA_CORE
#include "lua.h"
/*
]]
.. COPYRIGHT ..
[[
*/
]])

-- lualib.c
for _, v in pairs(libsources) do
	v = v
	buf[#buf + 1] = "// root include "..v.."\n"
	buf[#buf + 1] = incfile(v)
end

-- pack and write lualib.c
donefile('./lualib.c', [[
#define LUA_LIB
#include "lua.h"
/*
]]
.. COPYRIGHT ..
[[
*/
]])

print("Done! :) See files lua.c, lialib.c and lua.h in THE ROOT OF THIS REPOSITORY!")

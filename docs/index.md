# Pixelbox

Pixelbox is an unlimited falling-sand sandbox game.
Still in alpha development.

# Controls 
- Left Mouse Button - Hold and move to move your camera.
- Right Mouse button - Hold and move to draw pixels
- WASD for precise camera movement

# Special world names
- `:null:` - no database is created, all your changsw will lost when chunks will be unloaded (*used in development, for testing*)
- `:memory:` - In memory database is created, all your changes will lost when you exit the world.
- `any other name` - database file is created, all changes are saved on the disk.

**WARNING:** database is working with `pragma journal_mode=MEMORY`, so it's unsafe to teminate program using task manager or due to system shutdown. Power loss is in effect too. Make sure to properly exit your worlds to minimize risk of the database corruption.
In beta versions it's planned to add automatic world backups, now you may want to do it manually.

Also, since pixelbox is still in alpha, and internal structure may change significantly, **any sort of backwards compatability is not guaranteed!**

# Pages
- See list of [Licenses](LICENSES.md) for code and resources.
- See screenshots (TODO)
- See todo list in Projects tab. (TODO)
- telegram group with news about development [in Russian](https://t.me/pixebox_dev)

# raylib makefile flags for TARGET\_OS=windows, HOST\_OS=linux
```
make RAYLIB_LIBTYPE=STATIC USE_EXTERNAL_GLFW=FALSE PLATFORM_OS=Windows CC=x86_64-w64-mingw32-gcc
```

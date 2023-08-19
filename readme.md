# Pixelbox
my last attempt to create pixelbox.
This time it MUST be simple and pretty fast.

*Luajit FFI turns into mess when you trying to use small integers.
Now we are in PURE C, no dynamic languages.*
~~Do you want modification/plugin? Edit source and compile it! Easy!~~

# Pages
- See list of [Licenses](LICENSES.md) for code and resources.
- See screenshots (TODO)
- See todo list in Projects tab. (TODO)

# Web build
- Install emscripten
- Download raylib sources, add `-s USE_PTHREADS=1` to PLATFORM\_WEB CFLAGS
- Make raylib with emscripten using
```
export PATH="$PATH:/lib/emscripten"
make -e PLATFORM=PLATFORM_WEB
```
- Move output binary to pixelbox repo root and run 
```
make -F makeweb
```

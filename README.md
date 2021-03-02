# libpcem-sdl2
Very simple [SDL2](https://www.libsdl.org/) implementation with [libpcem](https://github.com/mborjesson/libpcem).
Uses SDL2 for both rendering and audio. Click window to capture the mouse and CTRL-END to release it.

Tested with SDL2 2.0.10.

How to compile:
```
g++ pcem-sdl2.cc -L/path/to/libpcem/.libs -Wl,-Bstatic -lpcem -Wl,-Bdynamic $(pkg-config --libs --cflags sdl2) -pthread -o pcem-sdl2
```

How to run:
```
./pcem-sdl2 ~/.pcem/pcem.cfg ~/.pcem/configs/mymachine.cfg
```

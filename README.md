Waymod
======

A suckless inspired modular window manager.

Goals:
------

- At least dwl feature parity
- Kernel like modularity

Planned Core Features:
----------------------

- Dynamic loading of modules at runtime to modify behaviour
- Tagging
- Tiling, Pseudotiling and free floating mode
- Animations (?)
- Master - Stack layout
- Grid - Layout
- Tiled -  Layout
- Monocle (Fullscreen)

Roadmap:
--------

- Provide Core functionality
- Provide Core - API
- Enable module loading
- Provide Documentation and Wiki

Dependencies:
-------------

- wlroots v0.16 / the included submodule
- wayland
- wayland-protocol
- pkg-config


Building:
---------

To build first wlroots
```
$ cd wlroots
$ meson build/
$ ninja -C build
$ sudo ninja -C build install
$ cd ..
```

After that configure waymod:
```
$ make menuconfig
```
Or:

```
$ make allyesconfig
```

Then build:

```
$ make
```

To run:

```
$ ./waymod
```

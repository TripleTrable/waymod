Waymod
======

A suckless inspired modular window manager.

Goals:
------

- At least dwl feature parity
- Kernel like modularity

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

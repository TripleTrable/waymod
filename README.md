Waymod
======

A suckless inspired modular window manager.

Table of contents:
-----------------

- [Waymod](#waymod)
  - [Goals:](#goals)
  - [Planned Core Features:](#planned-core-features)
  - [Roadmap:](#roadmap)
  - [Dependencies:](#dependencies)
  - [Building:](#building)
    - [From Source:](#from-source)
      - [1. Make sure you have the needed dependencies installed:](#1-make-sure-you-have-the-needed-dependencies-installed)
      - [2. Clone this repository:](#2-clone-this-repository)
      - [3. Build wlroots first](#3-build-wlroots-first)
      - [4. Configure waymod:](#4-configure-waymod)
      - [5. Build Waymod:](#5-build-waymod)
      - [6. Running:](#6-running)

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

### From Source:

#### 1. Make sure you have the needed dependencies installed:

 **Fedora 37:**
 ```
 sudo dnf install meson ninja make
 ```
(wayland and wayland-protocols should be provided by default on most fedora systems)

**Arch:**
```
sudo pacman -S meson ninja make wayland wayland-protocols
```



#### 2. Clone this repository:
```
git clone --recursive URL git://github.com/TripleTrable/waymod.git
cd waymod
```

#### 3. Build wlroots first
```
$ cd wlroots
$ meson build/
$ ninja -C build
$ sudo ninja -C build install
$ cd ..
```

#### 4. Configure waymod:
```
$ make menuconfig
```
**Or:**

```
$ make allyesconfig
```

#### 5. Build Waymod:

```
$ make
```

#### 6. Running:

```
$ ./waymod
```

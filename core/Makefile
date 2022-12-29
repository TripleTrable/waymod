obj-y := core.o

ifeq ($(CONFIG_WLR_USE_UNSTABLE),y)
KBUILD_CFLAGS   += -DWLR_USE_UNSTABLE
endif

WAYLAND_SCANER = `pkg-config --variable=wayland_scanner wayland-scanner`
WAYLAND_PROTOCOLS = `pkg-config --variable=pkgdatadir wayland-protocols`

xdg-shell-protocol.h:
	$(WAYLAND_SCANER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

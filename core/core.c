#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_single_pixel_buffer_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <generated/autoconf.h>
#ifdef CONFIG_XDG_SHELL
#include <wlr/types/wlr_xdg_shell.h>
#endif /* CONFIG_XDG_SHELL */

struct wmod_server {
    struct wl_display *wl_display;
    struct wl_event_loop *wl_event_loop;

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_cursor *cursor;

    struct wlr_compositor *compositor;
    struct wlr_output_layout *output_layout;

    struct wl_list outputs;
    struct wl_listener new_output;

#ifdef CONFIG_XDG_SHELL
    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_surface;
    struct wl_list views;
#endif /* CONFIG_XDG_SHELL */
};

struct wmod_output {
    struct wl_list link;
    struct wmod_server *server;
    struct wlr_output *wlr_output;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

struct wmod_view {
    struct wl_list link;
    struct wmod_server *server;
    struct wlr_xdg_toplevel *toplevel;
    struct wlr_scene_tree *scene_tree;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
    int x,y;
};

#ifdef CONFIG_XDG_SHELL
//TODO: moudlarize
static void new_xdg_surface_cb(struct wl_listener *listener, void* data)
{
    struct wmod_server *server = wl_container_of(listener, server, new_xdg_surface);
    struct wlr_xdg_surface *xdg_surface = data;
    struct wmod_view *view;
    if(xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP)
    {
        struct wlr_xdg_surface *parent = wlr_xdg_surface_from_wlr_surface(xdg_surface->popup->parent);
        struct wlr_scene_tree *parent_tree = parent->data;
        xdg_surface->data = wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
        return;
    }
    assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
    view = calloc(1,sizeof(struct wmod_view));
    view->server = server;
    view->toplevel = xdg_surface->toplevel;
    view->scene_tree = wlr_scene_xdg_surface_create(&view->server->scene->tree, view->toplevel->base);
    xdg_surface->data = view->scene_tree;
    //TODO: implement rest of listeners like in tinywl


}
#endif /* CONFIG_XDG_SHELL */

//TODO: moudlarize
static void output_destroy_cb(struct wl_listener *listener, void* data)
{
    struct wmod_output *output = wl_container_of(listener,output,destroy);

    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);
}

//TODO: moudlarize
static void frame_ready_cb(struct wl_listener *listener, void* data)
{
    struct timespec now;
    struct wmod_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->server->scene;
    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);
    wlr_scene_output_commit(scene_output);
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

//TODO: moudlarize
static void new_output_cb(struct wl_listener *listener, void *data)
{
    struct wmod_output *output;
    struct wmod_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;
    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    if(!wl_list_empty(&wlr_output->modes))
    {
        struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
        wlr_output_set_mode(wlr_output, mode);
        wlr_output_enable(wlr_output, true);
        if(!wlr_output_commit(wlr_output))
            return;
    }

    output = calloc(1,sizeof(struct wmod_output));
    output->wlr_output = wlr_output;
    output->server = server;

    // frame ready notification
    output->frame.notify = frame_ready_cb;
    wl_signal_add(&wlr_output->events.frame,&output->frame);

    // destroy notification
    output->destroy.notify = output_destroy_cb;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wlr_output_layout_add_auto(server->output_layout, wlr_output);

}

int main(int argc, char** argv)
{
    wlr_log_init(WLR_INFO, NULL);
        wlr_log(WLR_ERROR, "start");
#if defined(CONFIG_HELLO_WORLD)
        wlr_log(WLR_ERROR, "CONFIG_HELLO_WORLD");
#endif
#if defined(HELLO_WORLD)
        wlr_log(WLR_ERROR, "HELLO_WORLD");
#endif

    struct wmod_server server;
    const char *socket = NULL;


    server.wl_display = wl_display_create();
    assert(server.wl_display);

    server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
    assert(server.wl_event_loop);

    server.backend = wlr_backend_autocreate(server.wl_display);
    assert(server.backend);


    server.renderer = wlr_renderer_autocreate(server.backend);
    if (server.renderer == NULL) {
        wlr_log(WLR_ERROR, "failed to create wlr_renderer");
        return 1;
    }

    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.allocator = wlr_allocator_autocreate(server.backend,server.renderer);
    if (server.allocator == NULL) {
        wlr_log(WLR_ERROR, "failed to create wlr_allocator");
        return 1;
    }

    server.compositor = wlr_compositor_create(server.wl_display, server.renderer);

#ifdef EXPORT_DMABUF
	wlr_log(WLR_INFO, "EXPORT_DMABUF");
#endif /* CONFIG_EXPORT_DMABUF */
#ifdef CONFIG_EXPORT_DMABUF
	wlr_log(WLR_INFO, "enableing export_dmabuf");
    wlr_export_dmabuf_manager_v1_create(server.wl_display);
#else
	wlr_log(WLR_INFO, "export_dmabuf");
#endif /* CONFIG_EXPORT_DMABUF */
#ifdef CONFIG_SCREENCOPY
	wlr_log(WLR_INFO, "enableing screencopy");
    wlr_screencopy_manager_v1_create(server.wl_display);
#endif /* CONFIG_SCREENCOPY */
#ifdef CONFIG_DATA_CONTROL
	wlr_log(WLR_INFO, "enableing data_control");
    wlr_data_control_manager_v1_create(server.wl_display);
#endif /* CONFIG_DATA_CONTROL */
#ifdef CONFIG_DATA_DEVICE
	wlr_log(WLR_INFO, "enableing data_device_manager");
    wlr_data_device_manager_create(server.wl_display);
#endif /* CONFIG_DATA_DEVICE */
#ifdef CONFIG_GAMMA_CONTROLL
	wlr_log(WLR_INFO, "enableing gamma_control");
    wlr_gamma_control_manager_v1_create(server.wl_display);
#endif /* CONFIG_GAMMAC_CONTROLL */
#ifdef CONFIG_PRIMARY_SELECTION
	wlr_log(WLR_INFO, "enableing primary_selection");
    wlr_primary_selection_v1_device_manager_create(server.wl_display);
#endif /* CONFIG_PRIMARY_SELECTION */
#ifdef CONFIG_VIEWPORTER
	wlr_log(WLR_INFO, "enableing viewport");
    wlr_viewporter_create(server.wl_display);
#endif /* CONFIG_VIEWPORTER */
#ifdef CONFIG_SINGLE_PIXEL_BUFFER
	wlr_log(WLR_INFO, "enableing single_pixel_buffer");
    wlr_single_pixel_buffer_manager_v1_create(server.wl_display);
#endif /* CONFIG_SINGLE_PIXEL_BUFFER */
#ifdef CONFIG_SUBCOMPOSITOR
	wlr_log(WLR_INFO, "enableing subcompositor");
	wlr_subcompositor_create(server.wl_display);
#endif /* CONFIG_SUBCOMPOSITOR */

    server.output_layout = wlr_output_layout_create();

    wl_list_init(&server.outputs);
    server.new_output.notify = new_output_cb;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    server.scene = wlr_scene_create();
    wlr_scene_attach_output_layout(server.scene, server.output_layout);

    //TODO: finish xdg shell
#ifdef CONFIG_XDG_SHELL
    wl_list_init(&server.views);
    server.xdg_shell = wlr_xdg_shell_create(server.wl_display, 3);
    server.new_xdg_surface.notify = new_xdg_surface_cb;
    wl_signal_add(&server.xdg_shell->events.new_surface,&server.new_xdg_surface);
#endif /* CONFIG_XDG_SHELL */

#ifdef CONFIG_SHOW_CURSOR
    server.cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server.cursor,server.output_layout);
#endif /* CONFIG_SHOW_CURSOR */

    //TODO: add cursor manager

    //TODO: add cuosr motion

    //TODO: add seat configuration



    //Add unix socket
    socket = wl_display_add_socket_auto(server.wl_display);
    if(!socket)
    {
        wlr_backend_destroy(server.backend);
        return 1;
    }


    if(!wlr_backend_start(server.backend))
    {
        fprintf(stderr,"Failed to start backend\n");
        wlr_backend_destroy(server.backend);
        wl_display_destroy(server.wl_display);
        return 1;
    }

    //Set WAYLAND_DISPLAY
    setenv("WAYLAND_DISPLAY", socket, true);
	wlr_log(WLR_ERROR, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);





    wl_display_run(server.wl_display);
    wl_display_destroy_clients(server.wl_display);
    wl_display_destroy(server.wl_display);
    return 0;
}

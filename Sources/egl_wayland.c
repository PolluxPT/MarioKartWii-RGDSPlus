/* Experimental RGDS+ adapter for the supplied H700 MKW-3DS executable.
 * MIT license. Does not contain or extract Nintendo game data.
 * The executable explicitly dlopens this library through MKW_EGL_LIBRARY.
 * Forward EGL to the system driver, replacing the fbdev native window with
 * an xdg-shell Wayland window. All Wayland event handling stays on the caller
 * of eglSwapBuffers after the initial setup.
 */
#define _GNU_SOURCE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "xdg-shell-client-protocol.h"
#include <dlfcn.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rgds_runtime.h"

static void *driver;
static struct wl_display *display;
static struct wl_registry *registry;
static struct wl_compositor *compositor;
static struct xdg_wm_base *wm;
static struct wl_output *outputs[16];
static unsigned output_count;
static struct wl_surface *surface;
static struct xdg_surface *xdg;
static struct xdg_toplevel *toplevel;
static struct wl_egl_window *window;
static int configured, closed, width = 640, height = 480;
static EGLint local_error = EGL_SUCCESS;

static void *entry(const char *name) {
    if (!driver) {
        const char *override = getenv("MKW_REAL_EGL");
        if (override && *override) driver = dlopen(override, RTLD_NOW | RTLD_LOCAL);
        if (!driver) driver = dlopen("libEGL.so.1", RTLD_NOW | RTLD_LOCAL);
        if (!driver) driver = dlopen("libEGL.so", RTLD_NOW | RTLD_LOCAL);
        if (!driver) { fprintf(stderr, "[rgds] no system EGL driver: %s\n", dlerror()); exit(71); }
    }
    void *p = dlsym(driver, name);
    if (!p) { fprintf(stderr, "[rgds] EGL symbol missing: %s\n", name); exit(72); }
    return p;
}

static void ping(void *data, struct xdg_wm_base *base, uint32_t serial) {
    (void)data; xdg_wm_base_pong(base, serial);
}
static const struct xdg_wm_base_listener wm_listener = { .ping = ping };
static void global(void *data, struct wl_registry *reg, uint32_t id, const char *iface, uint32_t version) {
    (void)data;
    if (!strcmp(iface, "wl_compositor"))
        compositor = wl_registry_bind(reg, id, &wl_compositor_interface, version < 3 ? version : 3);
    else if (!strcmp(iface, "xdg_wm_base")) {
        wm = wl_registry_bind(reg, id, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(wm, &wm_listener, NULL);
    } else if (!strcmp(iface, "wl_output") && output_count < 16) {
        outputs[output_count++] = wl_registry_bind(reg, id, &wl_output_interface, 1);
        fprintf(stderr, "[rgds] Wayland output %u registry=%u\n", output_count - 1, id);
    }
}
static void global_remove(void *data, struct wl_registry *reg, uint32_t id) {
    (void)data; (void)reg; (void)id;
}
static const struct wl_registry_listener registry_listener = { .global = global, .global_remove = global_remove };
static void xdg_configure(void *data, struct xdg_surface *s, uint32_t serial) {
    (void)data; xdg_surface_ack_configure(s, serial); configured = 1;
}
static const struct xdg_surface_listener surface_listener = { .configure = xdg_configure };
static void top_configure(void *data, struct xdg_toplevel *t, int32_t w, int32_t h, struct wl_array *states) {
    (void)data; (void)t; (void)states;
    if (w > 0 && h > 0) { width = w; height = h; }
    if (window) wl_egl_window_resize(window, width, height, 0, 0);
    fprintf(stderr, "[rgds] configure %dx%d\n", width, height);
}
static void top_close(void *data, struct xdg_toplevel *t) { (void)data; (void)t; closed = 1; }
static const struct xdg_toplevel_listener top_listener = { .configure = top_configure, .close = top_close };

static void teardown(void) {
    if (window) wl_egl_window_destroy(window);
    if (toplevel) xdg_toplevel_destroy(toplevel);
    if (xdg) xdg_surface_destroy(xdg);
    if (surface) wl_surface_destroy(surface);
    for (unsigned i = 0; i < output_count; ++i) wl_output_destroy(outputs[i]);
    if (wm) xdg_wm_base_destroy(wm);
    if (compositor) wl_compositor_destroy(compositor);
    if (registry) wl_registry_destroy(registry);
    if (display) wl_display_disconnect(display);
    window = NULL; toplevel = NULL; xdg = NULL; surface = NULL;
    wm = NULL; compositor = NULL; registry = NULL; display = NULL;
    output_count = 0; configured = 0;
}

EGLDisplay eglGetDisplay(EGLNativeDisplayType ignored) {
    (void)ignored;
    rgds_runtime_init();
    if (!display) {
        display = wl_display_connect(NULL);
        if (!display) { fprintf(stderr, "[rgds] cannot connect to Wayland\n"); local_error = EGL_BAD_DISPLAY; return EGL_NO_DISPLAY; }
        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registry_listener, NULL);
        if (wl_display_roundtrip(display) < 0 || !compositor || !wm) {
            fprintf(stderr, "[rgds] compositor or xdg-shell unavailable\n"); local_error = EGL_BAD_DISPLAY; teardown(); return EGL_NO_DISPLAY;
        }
    }
    PFNEGLGETDISPLAYPROC real = entry("eglGetDisplay");
    PFNEGLGETPLATFORMDISPLAYEXTPROC platform = dlsym(driver, "eglGetPlatformDisplayEXT");
    if (!platform) {
        PFNEGLGETPROCADDRESSPROC proc = entry("eglGetProcAddress");
        platform = (PFNEGLGETPLATFORMDISPLAYEXTPROC)proc("eglGetPlatformDisplayEXT");
    }
    if (platform) return platform(0x31D8 /* EGL_PLATFORM_WAYLAND_EXT */, display, NULL);
    return real((EGLNativeDisplayType)display);
}
EGLDisplay eglGetPlatformDisplay(EGLenum platform, void *native, const EGLAttrib *attrs) {
    (void)platform; (void)native; (void)attrs; return eglGetDisplay(EGL_DEFAULT_DISPLAY);
}

EGLSurface eglCreateWindowSurface(EGLDisplay d, EGLConfig config, EGLNativeWindowType native, const EGLint *attrs) {
    /* The supplied executable passes {u16 width,height}. It is no longer
     * the native window; fullscreen is placed by Wayland on MKW_OUTPUT. */
    if (!display || window) return EGL_NO_SURFACE;
    const uint16_t *size = (const uint16_t *)native;
    if (size && size[0] && size[1]) { width = size[0]; height = size[1]; }
    surface = wl_compositor_create_surface(compositor);
    xdg = xdg_wm_base_get_xdg_surface(wm, surface);
    xdg_surface_add_listener(xdg, &surface_listener, NULL);
    toplevel = xdg_surface_get_toplevel(xdg);
    xdg_toplevel_add_listener(toplevel, &top_listener, NULL);
    xdg_toplevel_set_title(toplevel, "Mario Kart Wii - RGDS+ experimental");
    xdg_toplevel_set_app_id(toplevel, "mkw-rgds");
    int which = getenv("MKW_OUTPUT") ? atoi(getenv("MKW_OUTPUT")) : 0;
    struct wl_output *out = which >= 0 && (unsigned)which < output_count ? outputs[which] : NULL;
    fprintf(stderr, "[rgds] fullscreen output=%d available=%u\n", which, output_count);
    xdg_toplevel_set_fullscreen(toplevel, out);
    wl_surface_commit(surface);
    /* Bound the initial handshake so a compositor issue cannot hang APPS. */
    int elapsed = 0;
    while (!configured && !closed && elapsed < 5000) {
        while (wl_display_prepare_read(display) != 0) {
            if (wl_display_dispatch_pending(display) < 0) return EGL_NO_SURFACE;
        }
        wl_display_flush(display);
        struct pollfd p = { wl_display_get_fd(display), POLLIN, 0 };
        int n = poll(&p, 1, 100); elapsed += 100;
        if (n > 0 && (p.revents & POLLIN)) {
            if (wl_display_read_events(display) < 0) return EGL_NO_SURFACE;
            if (wl_display_dispatch_pending(display) < 0) return EGL_NO_SURFACE;
        } else wl_display_cancel_read(display);
    }
    if (!configured || closed) { fprintf(stderr, "[rgds] fullscreen configure timed out\n"); return EGL_NO_SURFACE; }
    /* Return the configured size to the executable's fbdev-size variables
     * is impossible here; the launcher passes the known panel dimensions. */
    window = wl_egl_window_create(surface, width, height);
    if (!window) return EGL_NO_SURFACE;
    PFNEGLCREATEWINDOWSURFACEPROC real = entry("eglCreateWindowSurface");
    return real(d, config, (EGLNativeWindowType)window, attrs);
}

EGLBoolean eglSwapBuffers(EGLDisplay d, EGLSurface s) {
    rgds_runtime_tick();
    if (closed) { fprintf(stderr, "[rgds] compositor closed window\n"); exit(0); }
    if (display) {
        while (wl_display_prepare_read(display) != 0)
            if (wl_display_dispatch_pending(display) < 0) return EGL_FALSE;
        wl_display_flush(display);
        struct pollfd p = { wl_display_get_fd(display), POLLIN, 0 };
        if (poll(&p, 1, 0) > 0 && (p.revents & POLLIN)) {
            if (wl_display_read_events(display) < 0) return EGL_FALSE;
            if (wl_display_dispatch_pending(display) < 0) return EGL_FALSE;
        } else wl_display_cancel_read(display);
    }
    PFNEGLSWAPBUFFERSPROC real = entry("eglSwapBuffers"); return real(d, s);
}
EGLBoolean eglTerminate(EGLDisplay d) {
    PFNEGLTERMINATEPROC real = entry("eglTerminate"); EGLBoolean result = real(d); teardown(); return result;
}

#define FORWARD(ret, name, params, args) ret name params { __typeof__(&name) real = entry(#name); return real args; }
FORWARD(EGLBoolean, eglInitialize, (EGLDisplay d, EGLint *a, EGLint *b), (d,a,b))
FORWARD(EGLBoolean, eglChooseConfig, (EGLDisplay d, const EGLint *a, EGLConfig *c, EGLint n, EGLint *count), (d,a,c,n,count))
FORWARD(EGLBoolean, eglGetConfigAttrib, (EGLDisplay d, EGLConfig c, EGLint a, EGLint *v), (d,c,a,v))
FORWARD(EGLContext, eglCreateContext, (EGLDisplay d, EGLConfig c, EGLContext share, const EGLint *a), (d,c,share,a))
FORWARD(EGLBoolean, eglDestroyContext, (EGLDisplay d, EGLContext c), (d,c))
FORWARD(EGLSurface, eglCreatePbufferSurface, (EGLDisplay d, EGLConfig c, const EGLint *a), (d,c,a))
FORWARD(EGLBoolean, eglDestroySurface, (EGLDisplay d, EGLSurface s), (d,s))
FORWARD(EGLBoolean, eglMakeCurrent, (EGLDisplay d, EGLSurface draw, EGLSurface read, EGLContext c), (d,draw,read,c))
FORWARD(EGLBoolean, eglSwapInterval, (EGLDisplay d, EGLint n), (d,n))
EGLint eglGetError(void) {
    if (local_error != EGL_SUCCESS) { EGLint error = local_error; local_error = EGL_SUCCESS; return error; }
    if (!driver) return EGL_SUCCESS;
    PFNEGLGETERRORPROC real = entry("eglGetError"); return real();
}
FORWARD(EGLBoolean, eglBindAPI, (EGLenum api), (api))
FORWARD(const char *, eglQueryString, (EGLDisplay d, EGLint n), (d,n))
FORWARD(__eglMustCastToProperFunctionPointerType, eglGetProcAddress, (const char *n), (n))

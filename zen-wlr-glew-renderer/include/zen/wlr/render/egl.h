#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <assert.h>
#include <drm_fourcc.h>
#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/render/dmabuf.h>
#include <wlr/render/drm_format_set.h>
#include <wlr/util/log.h>

struct zn_wlr_egl {
  EGLDisplay display;
  EGLContext context;
  EGLDeviceEXT device;  // may be EGL_NO_DEVICE_EXT
  struct gbm_device *gbm_device;

  struct {
    // Display extensions
    bool KHR_image_base;
    bool EXT_image_dma_buf_import;
    bool EXT_image_dma_buf_import_modifiers;
    bool IMG_context_priority;

    // Device extensions
    bool EXT_device_drm;
    bool EXT_device_drm_render_node;

    // Client extensions
    bool EXT_device_query;
    bool KHR_platform_gbm;
    bool EXT_platform_device;
  } exts;

  struct {
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
    PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
    PFNEGLQUERYDMABUFFORMATSEXTPROC eglQueryDmaBufFormatsEXT;
    PFNEGLQUERYDMABUFMODIFIERSEXTPROC eglQueryDmaBufModifiersEXT;
    PFNEGLDEBUGMESSAGECONTROLKHRPROC eglDebugMessageControlKHR;
    PFNEGLQUERYDISPLAYATTRIBEXTPROC eglQueryDisplayAttribEXT;
    PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT;
    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT;
  } procs;

  bool has_modifiers;
  struct wlr_drm_format_set dmabuf_texture_formats;
  struct wlr_drm_format_set dmabuf_render_formats;
};

struct zn_wlr_egl *zn_wlr_egl_create_with_context(
    EGLDisplay display, EGLContext context);

/**
 * Make the EGL context current.
 *
 * Callers are expected to clear the current context when they are done by
 * calling zn_wlr_egl_unset_current.
 */
bool zn_wlr_egl_make_current(struct zn_wlr_egl *egl);

bool zn_wlr_egl_unset_current(struct zn_wlr_egl *egl);

bool zn_wlr_egl_is_current(struct zn_wlr_egl *egl);

struct zn_wlr_egl_context {
  EGLDisplay display;
  EGLContext context;
  EGLSurface draw_surface;
  EGLSurface read_surface;
};

/**
 * Initializes an EGL context for the given DRM FD.
 *
 * Will attempt to load all possibly required API functions.
 */
struct zn_wlr_egl *zn_wlr_egl_create_with_drm_fd(int drm_fd);

/**
 * Frees all related EGL resources, makes the context not-current and
 * unbinds a bound wayland display.
 */
void zn_wlr_egl_destroy(struct zn_wlr_egl *egl);

/**
 * Creates an EGL image from the given dmabuf attributes. Check usability
 * of the dmabuf with zn_wlr_egl_check_import_dmabuf once first.
 */
EGLImageKHR zn_wlr_egl_create_image_from_dmabuf(struct zn_wlr_egl *egl,
    struct wlr_dmabuf_attributes *attributes, bool *external_only);

/**
 * Get DMA-BUF formats suitable for sampling usage.
 */
const struct wlr_drm_format_set *zn_wlr_egl_get_dmabuf_texture_formats(
    struct zn_wlr_egl *egl);

/**
 * Get DMA-BUF formats suitable for rendering usage.
 */
const struct wlr_drm_format_set *zn_wlr_egl_get_dmabuf_render_formats(
    struct zn_wlr_egl *egl);

/**
 * Destroys an EGL image created with the given zn_wlr_egl.
 */
bool zn_wlr_egl_destroy_image(struct zn_wlr_egl *egl, EGLImageKHR image);

int zn_wlr_egl_dup_drm_fd(struct zn_wlr_egl *egl);

/**
 * Save the current EGL context to the structure provided in the argument.
 *
 * This includes display, context, draw surface and read surface.
 */
void zn_wlr_egl_save_context(struct zn_wlr_egl_context *context);

/**
 * Restore EGL context that was previously saved using
 * zn_wlr_egl_save_current().
 */
bool zn_wlr_egl_restore_context(struct zn_wlr_egl_context *context);

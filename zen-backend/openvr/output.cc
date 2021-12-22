#include "output.h"

#include <libzen-compositor/libzen-compositor.h>
#include <sys/time.h>
#include <wayland-server.h>
#include <zen-renderer/opengl-renderer.h>

#include "gl-window.h"
#include "hmd.h"

static void
OpenvrOutputRepaint(struct zen_output* zen_output)
{
  struct timespec next_repaint;
  struct openvr_output* output = (struct openvr_output*)zen_output;
  struct zen_opengl_renderer_camera cameras[2];

  output->hmd->GetCameras(cameras);
  zen_opengl_renderer_set_cameras(
      output->renderer, cameras, ARRAY_LENGTH(cameras));
  zen_opengl_renderer_render(output->renderer);

  output->hmd->Submit();

  output->hmd->DrawPreview(output->window->framebuffer(),
      output->window->width(), output->window->height());
  output->window->Swap();

  output->hmd->WaitUpdatePoses();
  timespec_get(&output->base.frame_time, TIME_UTC);

  if (!output->window->Poll()) {
    wl_display_terminate(output->compositor->display);
    return;
  }

  timespec_get(&next_repaint, TIME_UTC);

  timespec_add_msec(&next_repaint, &next_repaint, 2);

  zen_compositor_finish_frame(output->compositor, next_repaint);
}

WL_EXPORT struct zen_output*
zen_output_create(struct zen_compositor* compositor)
{
  struct openvr_output* output;
  struct zen_opengl_renderer* renderer;
  GlWindow* window;
  Hmd* hmd;

  output = (struct openvr_output*)zalloc(sizeof *output);
  if (output == NULL) {
    zen_log("openvr output: failed to allocate memory\n");
    goto err;
  }

  window = new GlWindow();
  if (!window->Init()) {
    zen_log("openvr output: failed to initialize GlWindow\n");
    goto err_window;
  }

  hmd = new Hmd();
  if (!hmd->Init()) {
    zen_log("openvr output: failed to initialize HMD\n");
    goto err_hmd;
  }

  renderer = zen_opengl_renderer_get(compositor->renderer);
  if (renderer == NULL) {
    zen_log("openvr output: renderer type not supported: %s\n",
        compositor->renderer->type);
    goto err_renderer;
  }

  timespec_get(&output->base.frame_time, TIME_UTC);
  output->base.repaint = OpenvrOutputRepaint;
  output->compositor = compositor;
  output->renderer = renderer;
  output->window = window;
  output->hmd = hmd;

  compositor->repaint_window_msec = 1;
  OpenvrOutputRepaint(&output->base);

  return &output->base;

err_renderer:
  delete hmd;

err_hmd:
  delete window;

err_window:
  free(output);

err:
  return NULL;
}

WL_EXPORT void
zen_output_destroy(struct zen_output* zen_output)
{
  struct openvr_output* output = (struct openvr_output*)zen_output;

  delete output->hmd;
  delete output->window;
  free(output);
}

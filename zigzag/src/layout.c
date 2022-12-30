#include <cairo-ft.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <zen-common.h>
#include <zigzag.h>

struct zigzag_layout *
zigzag_layout_create(const struct zigzag_layout_impl *implementation,
    double screen_width, double screen_height, const char *font_file_path,
    void *user_data)
{
  struct zigzag_layout *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->screen_width = screen_width;
  self->screen_height = screen_height;

  self->user_data = user_data;

  self->implementation = implementation;

  FT_Library library;
  FT_Init_FreeType(&library);
  FT_Face ft_face;

  if (FT_New_Face(library, font_file_path, 0, &ft_face) == 0) {
    self->system_font = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
    cairo_status_t status = cairo_font_face_set_user_data(self->system_font,
        &self->ft_font_face_key, ft_face, (cairo_destroy_func_t)FT_Done_Face);
    if (status) {
      zn_error("Failed to bind the FT_Face to cairo_font_face_t");
      goto err_font;
    }
    status = cairo_font_face_set_user_data(self->system_font,
        &self->ft_library_key, library, (cairo_destroy_func_t)FT_Done_FreeType);
    if (status) {
      zn_error("Failed to bind the FT_Library to cairo_font_face_t");
      goto err_font;
    }
  } else {
    self->system_font = cairo_toy_font_face_create(
        "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  }

  wl_list_init(&self->node_list);

  return self;
err_font:
  cairo_font_face_destroy(self->system_font);
  FT_Done_Face(ft_face);
  FT_Done_FreeType(library);
err:
  return NULL;
}

void
zigzag_layout_destroy(struct zigzag_layout *self)
{
  wl_list_remove(&self->node_list);
  cairo_font_face_destroy(self->system_font);
  free(self);
}

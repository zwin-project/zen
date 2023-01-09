#include "zen-common/font.h"

#include <cairo-ft.h>
#include <fontconfig/fontconfig.h>
#include <stddef.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

static struct zn_font *font_singleton = NULL;

static const char *font_list[ZN_FONT_TYPE_COUNT] = {
    [ZN_FONT_REGULAR] = "Ubuntu:style=Regular",
    [ZN_FONT_BOLD] = "Ubuntu:bold",
};

struct zn_font_face {
  FT_Face ft_face;
  cairo_font_face_t *cairo_font_face;
};

struct zn_font {
  FcConfig *config;
  FT_Library library;

  struct zn_font_face font_faces[ZN_FONT_TYPE_COUNT];
};

cairo_font_face_t *
zn_font_face_get_cairo_font_face(enum zn_font_type type)
{
  struct zn_font *self = font_singleton;
  zn_assert(self, "Call zn_font_init before other zn_font_* functions");

  return self->font_faces[type].cairo_font_face;
}

static bool
zn_font_init_font_face(
    struct zn_font *self, enum zn_font_type type, const char *font_name)
{
  FcPattern *search_pattern, *match_pattern;
  FcResult result;
  FcChar8 *fc_file_path;
  FT_Error ft_error;
  struct zn_font_face *font_face = &self->font_faces[type];

  search_pattern = FcNameParse((const FcChar8 *)font_name);
  if (!search_pattern) {
    zn_error("Failed to create FcPattern from '%s'", font_name);
    goto err;
  }

  if (!FcConfigSubstitute(self->config, search_pattern, FcMatchPattern)) {
    zn_error("Failed to configure substitute");
    goto err_search_pattern;
  }

  FcDefaultSubstitute(search_pattern);

  match_pattern = FcFontMatch(self->config, search_pattern, &result);
  if (!match_pattern) {
    zn_error("Failed to search font");
    goto err_search_pattern;
  }

  result = FcPatternGetString(match_pattern, FC_FILE, 0, &fc_file_path);
  if (result != FcResultMatch) {
    zn_error("Failed to get font file path");
    goto err_match_pattern;
  }

  ft_error =
      FT_New_Face(self->library, (char *)fc_file_path, 0, &font_face->ft_face);
  if (ft_error != FT_Err_Ok) {
    zn_error("Failed to initialize ft_face for '%s'", font_name);
    goto err_match_pattern;
  }

  font_face->cairo_font_face =
      cairo_ft_font_face_create_for_ft_face(font_face->ft_face, 0);
  if (font_face->cairo_font_face == NULL) {
    zn_error("Failed to create cairo_ft_font_face for '%s'", font_name);
    goto err_ft_face;
  }

  {
    FcChar8 *format =
        (FcChar8 *)"%{family}%{:style=}%{:weight=}%{:slant=}%{:file=}";

    FcChar8 *name = FcPatternFormat(match_pattern, format);
    zn_debug("Using '%s' for font type: %d", (char *)name, type);
    free(name);
  }

  FcPatternDestroy(match_pattern);
  FcPatternDestroy(search_pattern);

  return true;

err_ft_face:
  FT_Done_Face(font_face->ft_face);

err_match_pattern:
  FcPatternDestroy(match_pattern);

err_search_pattern:
  FcPatternDestroy(search_pattern);

err:
  return false;
}

static void
zn_font_fini_font_face(struct zn_font *self, enum zn_font_type type)
{
  cairo_font_face_destroy(self->font_faces[type].cairo_font_face);
  FT_Done_Face(self->font_faces[type].ft_face);
}

bool
zn_font_init(void)
{
  if (font_singleton) {
    return true;
  }

  struct zn_font *self;

  if (!FcInit()) {
    zn_error("Failed to initialize fontconfig");
    goto err;
  }

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err_fc;
  }

  self->config = FcInitLoadConfigAndFonts();
  if (!self->config) {
    zn_error("Failed to load config and fonts");
    goto err_free;
  }

  FT_Error err = FT_Init_FreeType(&self->library);
  if (err != FT_Err_Ok) {
    zn_error("Failed to initialize FreeType library object");
    goto err_config;
  }

  for (enum zn_font_type font_type = 0; font_type < ZN_FONT_TYPE_COUNT;
       font_type++) {
    if (!zn_font_init_font_face(self, font_type, font_list[font_type])) {
      zn_error("Failed to init font face");
      goto err_library;
    }
  }

  font_singleton = self;

  return true;

err_library:
  for (enum zn_font_type font_type = 0; font_type < ZN_FONT_TYPE_COUNT;
       font_type++) {
    if (self->font_faces[font_type].cairo_font_face != NULL) {
      zn_font_fini_font_face(self, font_type);
    }
  }

  FT_Done_FreeType(self->library);

err_config:
  FcConfigDestroy(self->config);

err_free:
  free(self);

err_fc:
  FcFini();

err:
  return false;
}

void
zn_font_fini(void)
{
  if (!font_singleton) return;
  struct zn_font *self = font_singleton;

  enum zn_font_type font_type;
  for (font_type = 0; font_type < ZN_FONT_TYPE_COUNT; font_type++) {
    zn_font_fini_font_face(self, font_type);
  }

  FT_Done_FreeType(self->library);
  FcConfigDestroy(self->config);
  free(self);
  FcFini();

  font_singleton = NULL;
}

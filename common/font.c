#include <fontconfig/fontconfig.h>
#include <string.h>

#include "zen-common.h"

char *
zn_fontconfig_get_font_path(FcConfig **config, char *font_name)
{
  if (!FcInit()) {
    zn_error("Failed to initialize fontconfig");
    return NULL;
  }
  *config = FcInitLoadConfigAndFonts();
  if (!config) {
    zn_error("Failed to load config and fonts");
    return NULL;
  }

  FcPattern *substitute_pattern = FcNameParse((const FcChar8 *)font_name);
  if (!substitute_pattern) {
    zn_error("Failed to create pattern");
    return NULL;
  }
  if (!FcConfigSubstitute(*config, substitute_pattern, FcMatchPattern)) {
    zn_error("Failed to configure substitute");
    FcPatternDestroy(substitute_pattern);
    return NULL;
  }
  FcDefaultSubstitute(substitute_pattern);

  FcPattern *match_pattern;
  {
    FcResult result;
    match_pattern = FcFontMatch(*config, substitute_pattern, &result);
  }
  FcPatternDestroy(substitute_pattern);
  if (!match_pattern) {
    zn_error("Failed to search font");
    return NULL;
  }

  FcChar8 *fc_file_path;
  FcResult result =
      FcPatternGetString(match_pattern, FC_FILE, 0, &fc_file_path);

  char *file_path = NULL;
  if (result == FcResultMatch) {
    file_path = strdup((char *)fc_file_path);
  }

  FcPatternDestroy(match_pattern);
  return file_path;
}

void
zn_fontconfig_fini(FcConfig *config)
{
  FcConfigDestroy(config);
  FcFini();
}

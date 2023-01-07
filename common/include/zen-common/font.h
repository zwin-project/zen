#pragma once

#include <fontconfig/fontconfig.h>

char *zn_fontconfig_get_font_path(FcConfig **config, char *font_name);

void zn_fontconfig_fini(FcConfig *config);

#include "png-loader.h"

#include <png.h>
#include <stdio.h>
#include <string.h>

#define SIGNATURE_NUM 8

PngLoader::PngLoader(const char *filename) { this->filename_ = filename; }

bool
PngLoader::Load()
{
  FILE *fp;
  uint32_t read_size;

  unsigned char **rows = NULL;
  unsigned char *data = NULL;
  png_struct *png;
  png_info *info;
  png_byte type, depth, compression, interlace, filter;
  png_byte signature[8];

  fp = fopen(filename_, "rb");
  if (!fp) {
    fprintf(stderr, "Fail to open file: %s\n", filename_);
    goto error_open_file;
  }

  read_size = fread(signature, 1, SIGNATURE_NUM, fp);

  if (png_sig_cmp(signature, 0, SIGNATURE_NUM)) {
    fprintf(stderr, "File is not png format.\n");
    goto error_not_png;
  }

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == NULL) {
    fprintf(stderr, "Fail to crate png struct.\n");
    goto error_create_png_struct;
  }

  info = png_create_info_struct(png);
  if (info == NULL) {
    fprintf(stderr, "Fail to crate png image info.\n");
    goto error_create_info;
  }

  png_init_io(png, fp);
  png_set_sig_bytes(png, read_size);
  png_read_png(png, info, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

  width_ = png_get_image_width(png, info);
  height_ = png_get_image_height(png, info);
  depth = png_get_bit_depth(png, info);
  type = png_get_color_type(png, info);
  compression = png_get_compression_type(png, info);
  interlace = png_get_interlace_type(png, info);
  filter = png_get_filter_type(png, info);

  // only support 8bit color depth image
  if (depth != 8) {
    fprintf(
        stderr, "Unsupported PNG format. We support only 8 bit color depth.\n");
    goto error_invalid_png_format;
  }

  if (compression != PNG_COMPRESSION_TYPE_BASE) {
    fprintf(stderr, "Unsupported PNG format.\n");
    goto error_invalid_png_format;
  }
  if (interlace != PNG_INTERLACE_NONE) {
    fprintf(stderr, "Unsupported PNG format.\n");
    goto error_invalid_png_format;
  }
  if (filter != PNG_FILTER_TYPE_BASE) {
    fprintf(stderr, "Unsupported PNG format.\n");
    goto error_invalid_png_format;
  }

  // only RGB and RGBA support for now
  if (type == PNG_COLOR_TYPE_RGB)
    channel_ = 3;
  else if (type == PNG_COLOR_TYPE_RGB_ALPHA)
    channel_ = 4;
  else {
    fprintf(stderr,
        "Unsupported PNG format. We support only RGB and RGBA format. (%u)\n",
        type);
    goto error_invalid_png_format;
  }

  rows = png_get_rows(png, info);
  data_ = (uint8_t *)malloc(sizeof(*data) * width_ * height_ * channel_);
  if (data_ == NULL) goto err_malloc_return_val;

  for (uint32_t i = 0; i < height_; i++) {
    memcpy(data_ + i * width_ * channel_, rows[i], width_ * channel_);
  }

  return true;

err_malloc_return_val:
error_invalid_png_format:
error_create_info:
  png_destroy_read_struct(&png, &info, NULL);

error_create_png_struct:
error_not_png:
  fclose(fp);

error_open_file:
  return false;
}

#pragma once

#include "zen/backend.h"

struct zn_mock_backend {
  struct zn_backend base;
};

struct zn_mock_backend *zn_mock_backend_create(void);

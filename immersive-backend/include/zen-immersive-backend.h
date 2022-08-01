#ifndef ZEN_IMMERSIVE_BACKEND_H
#define ZEN_IMMERSIVE_BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zn_immersive_backend {};
#pragma GCC diagnostic pop

bool zn_immersive_backend_connect(struct zn_immersive_backend* self);

void zn_immersive_backend_disconnect(struct zn_immersive_backend* self);

struct zn_immersive_backend* zn_immersive_backend_create(void);

void zn_immersive_backend_destroy(struct zn_immersive_backend* self);

#ifdef __cplusplus
}
#endif

#endif  //  ZEN_IMMERSIVE_BACKEND_H

#ifndef ZEN_SCENE_H
#define ZEN_SCENE_H

struct zn_scene;

struct zn_scene* zn_scene_create();

void zn_scene_destroy(struct zn_scene* self);

#endif  //  ZEN_SCENE_H

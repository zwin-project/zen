#ifndef ZEN_SCENE_H
#define ZEN_SCENE_H

struct zn_scene;
struct zn_scene_output;

/* zn_scene */

struct zn_scene* zn_scene_create();

void zn_scene_destroy(struct zn_scene* self);

/* zn_scene_output */

struct zn_scene_output* zn_scene_output_create(struct zn_scene* scene);

void zn_scene_output_destroy(struct zn_scene_output* self);

#endif  //  ZEN_SCENE_H

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*zn_terminate_func_t)(int exit_code, void *data);

void zn_terminate(int exit_code);

void zn_set_terminate_func(zn_terminate_func_t terminate, void *data);

#ifdef __cplusplus
}
#endif

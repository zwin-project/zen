#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void zn_terminate(int exit_code);

typedef void (*terminate_func_t)(int exit_code, void *data);

void zn_set_terminate_func(terminate_func_t terminate, void *data);

#ifdef __cplusplus
}
#endif

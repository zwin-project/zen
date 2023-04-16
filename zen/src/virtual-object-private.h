#include "zen/virtual-object.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_virtual_object *zn_virtual_object_create(void *impl_data);

/// Called by impl object
void zn_virtual_object_destroy(struct zn_virtual_object *self);

#ifdef __cplusplus
}
#endif

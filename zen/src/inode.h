#pragma once

#include "zen/inode.h"

/// @param self must be root
/// @param xr_system is nullable
/// If this is called with nonnull xr_system, the caller must call this with
/// NULL when the xr_system is disconnected or destroyed
void zn_inode_set_xr_system(
    struct zn_inode *self, struct zn_xr_system *xr_system);

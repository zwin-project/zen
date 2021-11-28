#include <libzen-compositor/libzen-compositor.h>

static void
zen_weak_link_destroy_listener(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_weak_link *link;

  link = wl_container_of(listener, link, listener);

  link->resource = NULL;
  wl_list_init(&link->listener.link);
}

WL_EXPORT void
zen_weak_link_init(struct zen_weak_link *link)
{
  link->resource = NULL;
  wl_list_init(&link->listener.link);
}

WL_EXPORT void
zen_weak_link_set(struct zen_weak_link *link, struct wl_resource *resource)
{
  wl_list_remove(&link->listener.link);
  link->resource = resource;
  link->listener.notify = zen_weak_link_destroy_listener;
  wl_resource_add_destroy_listener(resource, &link->listener);
}

WL_EXPORT void
zen_weak_link_unset(struct zen_weak_link *link)
{
  link->resource = NULL;
  wl_list_remove(&link->listener.link);
  wl_list_init(&link->listener.link);
}

WL_EXPORT void *
zen_weak_link_get_user_data(struct zen_weak_link *link)
{
  if (link->resource == NULL) return NULL;
  return wl_resource_get_user_data(link->resource);
}
